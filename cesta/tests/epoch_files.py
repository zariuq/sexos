#!/usr/bin/env python3
"""SexFiles epoch batch adapter and real block-path controls."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile
from layout_text import LayoutText

HERE = Path(__file__).resolve().parent.parent


def run(argv):
    result = subprocess.run(list(map(str, argv)), cwd=HERE, capture_output=True)
    if result.returncode:
        raise RuntimeError((argv, result.returncode, result.stdout[-2000:], result.stderr[-6000:]))
    return result.stdout


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cesta', type=Path, required=True)
    parser.add_argument('--work', type=Path)
    parser.add_argument('--mutations', action='store_true')
    args = parser.parse_args()
    work = args.work.resolve() if args.work else Path(tempfile.mkdtemp(prefix='epoch-files-', dir=HERE/'build'))
    if args.work:
        work.mkdir(parents=True, exist_ok=False)
    run([args.cesta, 'export-c', 'src/epoch_types_interface.cesta', '-o', work / 'epoch_types.h', '--'])
    for unit in ('epoch_journal', 'epoch_files'):
        if unit == 'epoch_journal':
            run([args.cesta, 'export-c', 'src/epoch_journal_interface.cesta', '-o', work / 'epoch_journal.h', '--'])
        else:
            run([args.cesta, 'export-c', 'src/epoch_files_interface.cesta', '-o', work / 'epoch_files.h', '--'])
        run([args.cesta, 'export-c', 'src/' + unit + '.cesta', '--std=c11', '-o', work / (unit + '.c'),
             '--', '-I' + str(work), '-Igenerated/include', '-Iruntime/kernel'])
    flags = ['-std=c11', '-g0', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
             '-ffreestanding', '-fno-builtin', '-ffunction-sections', '-fdata-sections',
             '-DSEXOS_KERNEL_CURRENT_PDX=1', '-DEPOCH_FILES_SECTORS=6u',
             '-I' + str(work), '-Igenerated/include', '-Itests/kernel_memory', '-Iruntime/kernel',
             '-Wl,--gc-sections']

    def build(cc, extra, name):
        output = work / name
        run([cc, *flags, *extra, 'tests/epoch_files/host.c', '-o', output])
        return output

    builds = []
    for cc in ('gcc', 'clang'):
        for mode in ('O0', 'O2', 'sanitize'):
            extra = ['-O1', '-fsanitize=address,undefined', '-fno-sanitize-recover=all'] if mode == 'sanitize' else ['-' + mode]
            output = run([build(cc, extra, cc + '-' + mode)])
            (work / (cc + '-' + mode + '.log')).write_bytes(output)
            builds.append({'compiler': cc, 'mode': mode})
            print(cc, mode, 'epoch-files controls passed', flush=True)

    chunk = work / 'chunk-gcc-O2'
    run(['gcc', *flags, '-O2', '-DEPOCH_FILES_FLUSH_LIMIT=2u', 'tests/epoch_files/chunk_host.c', '-o', chunk])
    run([chunk])
    print('bounded flush chunk controls passed', flush=True)

    journal_mutations = [
        ('checksum_skipped', 'epoch_journal_get64(sector + 16) != expected_sequence ||\n        epoch_journal_get64(sector + EPOCH_JOURNAL_CHECKSUM_OFFSET) !=\n            epoch_journal_checksum(sector)', 'false'),
        ('append_not_advanced', '++journal->next_sequence;\n    journal->uncertain = false;', 'journal->next_sequence += 0;\n    journal->uncertain = false;'),
        ('readback_skipped', 'if (!epoch_journal_sector_valid(encoded + index * EPOCH_JOURNAL_SECTOR_SIZE,\n                                        journal->next_sequence + index))', 'if (false)'),
        ('capacity_overwrite', 'journal->next_sequence > journal->sectors ||\n        count > journal->sectors - journal->next_sequence + 1', 'false ||\n        false'),
    ]
    files_mutations = [
        ('prefix_published_before_commit', 'int64_t status = epoch_journal_commit(&epoch_files_journal);',
         '__atomic_store_n(&batch->flushed_through, next - 1, __ATOMIC_RELEASE);\n    int64_t status = epoch_journal_commit(&epoch_files_journal);'),
        ('uncertain_prefix_reappended', 'if (epoch_files_journal.next_event > next)', 'if (false)'),
        ('commit_retry_poisoned', 'int64_t status = epoch_journal_commit(&epoch_files_journal);\n    if (status == EPOCH_JOURNAL_RETRY) return status;',
         'int64_t status = epoch_journal_commit(&epoch_files_journal);'),
        ('quiet_commit_forgotten', 'producer == flushed + 1 && !epoch_files_journal.uncertain',
         'producer == flushed + 1'),
        ('timeout_not_retry', 'status == BLOCK_ERR_TIMEOUT ? EPOCH_JOURNAL_RETRY : EPOCH_JOURNAL_IO', 'status == BLOCK_ERR_TIMEOUT ? EPOCH_JOURNAL_IO : EPOCH_JOURNAL_IO'),
        ('scalar_read_timeout_lost', 'int64_t status = epoch_files_block_status(\n        diskfs_block_read(lba * 512, EPOCH_JOURNAL_SECTOR_SIZE, SEXOS_SLOT_buf_lend));', 'int64_t status = diskfs_block_read(lba * 512, EPOCH_JOURNAL_SECTOR_SIZE, SEXOS_SLOT_buf_lend) ? EPOCH_JOURNAL_IO : EPOCH_JOURNAL_OK;'),
        ('scalar_write_timeout_lost', 'return epoch_files_block_status(\n        diskfs_block_write(lba * 512, EPOCH_JOURNAL_SECTOR_SIZE, SEXOS_SLOT_buf_lend));', 'return diskfs_block_write(lba * 512, EPOCH_JOURNAL_SECTOR_SIZE, SEXOS_SLOT_buf_lend);'),
        ('sync_timeout_lost', 'return epoch_files_block_status(diskfs_block_sync());', 'return diskfs_block_sync();'),
        ('deferred_commit_omitted', 'int64_t status = epoch_journal_commit(&epoch_files_journal);', 'int64_t status = EPOCH_JOURNAL_OK;'),
        ('session_not_started', 'status = epoch_journal_begin_session(&epoch_files_journal);', 'status = EPOCH_JOURNAL_OK;'),
        ('batch_validation_skipped', 'if (!epoch_journal_event_sector_valid(sector, next))', 'if (false)'),
        ('gap_not_failed', 'if (events[count].epoch != next)', 'if (false)'),
        ('flush_wrong_epoch', '__atomic_store_n(&batch->flushed_through, next - 1, __ATOMIC_RELEASE);', '__atomic_store_n(&batch->flushed_through, next - 2, __ATOMIC_RELEASE);'),
        ('failed_ignored', 'int64_t epoch_files_flush(EpochLogBatch *batch) {\n    if (!batch) return EPOCH_JOURNAL_INVALID;\n    uint64_t failed = __atomic_load_n(&batch->failed, __ATOMIC_ACQUIRE);\n    if (failed) return (int64_t)failed;', 'int64_t epoch_files_flush(EpochLogBatch *batch) {\n    if (!batch) return EPOCH_JOURNAL_INVALID;\n    uint64_t failed = __atomic_load_n(&batch->failed, __ATOMIC_ACQUIRE);\n    if (failed && false) return (int64_t)failed;'),
        ('open_failure_not_sticky', 'int64_t status = epoch_files_open();\n        if (status == EPOCH_JOURNAL_RETRY) return status;\n        if (status != EPOCH_JOURNAL_OK) {\n            __atomic_store_n(&batch->failed, (uint64_t)status, __ATOMIC_RELEASE);', 'int64_t status = epoch_files_open();\n        if (status == EPOCH_JOURNAL_RETRY) return status;\n        if (status != EPOCH_JOURNAL_OK) {\n            __atomic_store_n(&batch->failed, EPOCH_JOURNAL_OK, __ATOMIC_RELEASE);'),
        ('open_retry_latched', 'int64_t status = epoch_files_open();\n        if (status == EPOCH_JOURNAL_RETRY) return status;', 'int64_t status = epoch_files_open();\n        if (false) return status;'),
        ('bad_read_buffer_allowed', 'static int64_t epoch_files_read_block(uint64_t lba, uint8_t *out) {\n    uint64_t va = diskfs_bridge_get_buf_va();\n    if (!va || va == UINT64_MAX || lba > (UINT64_MAX - 511) / 512)', 'static int64_t epoch_files_read_block(uint64_t lba, uint8_t *out) {\n    uint64_t va = diskfs_bridge_get_buf_va();\n    if (false)'),
        ('bad_write_buffer_allowed', 'static int64_t epoch_files_write_blocks(uint64_t lba, uint64_t sectors, const uint8_t *data) {\n    if (!sectors || sectors > EPOCH_JOURNAL_SPAN_SECTORS) return EPOCH_JOURNAL_INVALID;\n    uint64_t va = diskfs_bridge_get_buf_va();\n    if (!va || va == UINT64_MAX || lba > (UINT64_MAX - 511) / 512)', 'static int64_t epoch_files_write_blocks(uint64_t lba, uint64_t sectors, const uint8_t *data) {\n    if (!sectors || sectors > EPOCH_JOURNAL_SPAN_SECTORS) return EPOCH_JOURNAL_INVALID;\n    uint64_t va = diskfs_bridge_get_buf_va();\n    if (false)'),
        ('retry_limit_ignored', 'if (retries > 4) {', 'if (false) {'),
        ('retry_progress_limited', 'if (status == EPOCH_JOURNAL_RETRY &&\n        __atomic_load_n(&batch->flushed_through, __ATOMIC_ACQUIRE) == flushed_before) {', 'if ((void)flushed_before, status == EPOCH_JOURNAL_RETRY) {'),
    ]
    caught = []
    if args.mutations:
        for unit, name, before, after in ([('epoch_journal', *m) for m in journal_mutations] +
                                          [('epoch_files', *m) for m in files_mutations]):
            path = work / (unit + '.c')
            original = LayoutText(path.read_text())
            assert original.count(before) == 1, (name, 'anchor')
            try:
                path.write_text(original.replace(before, after, 1))
                if name == 'retry_progress_limited':
                    binary = work / name
                    run(['gcc', *flags, '-O2', '-DEPOCH_FILES_FLUSH_LIMIT=2u',
                         'tests/epoch_files/chunk_host.c', '-o', binary])
                else:
                    binary = build('gcc', ['-O2'], name)
                result = subprocess.run([binary], cwd=HERE, capture_output=True)
                assert result.returncode != 0, ('mutation survived', name)
                caught.append(name)
            finally:
                path.write_text(original)
    names = ['src/epoch_journal.cesta', 'src/epoch_journal_interface.cesta',
             'src/epoch_files.cesta', 'src/epoch_files_interface.cesta',
             'tests/epoch_files.py', 'tests/epoch_files/host.c', 'tests/epoch_files/chunk_host.c']
    summary = {
        'builds': builds,
        'chunked_flush_limit': 2,
        'mutations_caught': caught,
        'source_sha256': {name: hashlib.sha256((HERE / name).read_bytes()).hexdigest() for name in names},
        'scope': ('actual SexFiles block-buffer path, bounded batch flush and commit recovery, corruption and '
                  'capacity refusal on RAM; no production persistence or whole-OS rollback claim')
    }
    (work / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n')
    print('epoch-files qualification passed:', len(caught), 'mutations caught', flush=True)


if __name__ == '__main__':
    main()
