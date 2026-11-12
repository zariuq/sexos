#!/usr/bin/env python3
"""Epoch journal sector, replay and recovery controls with fault injection."""
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
    work = args.work.resolve() if args.work else Path(tempfile.mkdtemp(prefix='epoch-journal-', dir=HERE/'build'))
    if args.work:
        work.mkdir(parents=True, exist_ok=False)
    run([args.cesta, 'export-c', 'src/epoch_types_interface.cesta', '-o', work / 'epoch_types.h', '--'])
    run([args.cesta, 'export-c', 'src/epoch_journal_interface.cesta', '-o', work / 'epoch_journal.h', '--'])
    source = work / 'epoch_journal.c'
    run([args.cesta, 'export-c', 'src/epoch_journal.cesta', '--std=c11', '-o', source,
         '--', '-I' + str(work), '-Igenerated/include', '-Iruntime/kernel'])
    flags = ['-std=c11', '-g0', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
             '-ffreestanding', '-fno-builtin', '-ffunction-sections', '-fdata-sections',
             '-DSEXOS_KERNEL_CURRENT_PDX=1', '-I' + str(work), '-Igenerated/include',
             '-Itests/kernel_memory', '-Iruntime/kernel', '-Wl,--gc-sections']

    def build(cc, extra, name):
        output = work / name
        run([cc, *flags, *extra, 'tests/epoch_journal/host.c', '-o', output])
        return output

    builds = []
    for cc in ('gcc', 'clang'):
        for mode in ('O0', 'O2', 'sanitize'):
            extra = ['-O1', '-fsanitize=address,undefined', '-fno-sanitize-recover=all'] if mode == 'sanitize' else ['-' + mode]
            output = run([build(cc, extra, cc + '-' + mode)])
            (work / (cc + '-' + mode + '.log')).write_bytes(output)
            builds.append({'compiler': cc, 'mode': mode})
            print(cc, mode, 'epoch-journal controls passed', flush=True)

    mutations = [
        ('metadata_retry_lost', 'int64_t status = journal->read(journal->base + journal->sectors + slot, metadata[slot]);\n        if (status != EPOCH_JOURNAL_OK) return status;', 'int64_t status = journal->read(journal->base + journal->sectors + slot, metadata[slot]);\n        if (status != EPOCH_JOURNAL_OK) return EPOCH_JOURNAL_IO;'),
        ('sync_retry_lost', 'int64_t status = journal->sync();\n    if (status != EPOCH_JOURNAL_OK) {\n        journal->uncertain = true;\n        return status;', 'int64_t status = journal->sync();\n    if (status != EPOCH_JOURNAL_OK) {\n        journal->uncertain = true;\n        return EPOCH_JOURNAL_IO;'),
        ('write_retry_lost', 'if (write_data) status = journal->write(lba, expected);\n    if (status != EPOCH_JOURNAL_OK) {\n        journal->uncertain = true;\n        return status;', 'if (write_data) status = journal->write(lba, expected);\n    if (status != EPOCH_JOURNAL_OK) {\n        journal->uncertain = true;\n        return EPOCH_JOURNAL_IO;'),
        ('uncertain_reconciliation_skipped', 'if (journal->uncertain) {\n        uint8_t current[EPOCH_JOURNAL_SECTOR_SIZE];', 'if (false) {\n        uint8_t current[EPOCH_JOURNAL_SECTOR_SIZE];'),
        ('checksum_skipped', 'epoch_journal_get64(sector + 16) != expected_sequence ||\n        epoch_journal_get64(sector + EPOCH_JOURNAL_CHECKSUM_OFFSET) !=\n            epoch_journal_checksum(sector)', 'false'),
        ('append_not_advanced', '++journal->next_sequence;\n    journal->uncertain = false;', 'journal->next_sequence += 0;\n    journal->uncertain = false;'),
        ('recovery_gap', 'if (blank_seen || !epoch_journal_sector_valid(sector, index + 1))', 'if ((blank_seen && false) || !epoch_journal_sector_valid(sector, index + 1))'),
        ('readback_skipped', 'if (!epoch_journal_sector_valid(expected, journal->next_sequence))', 'if (false)'),
        ('span_readback_skipped', 'if (!epoch_journal_sector_valid(encoded + index * EPOCH_JOURNAL_SECTOR_SIZE,\n                                        journal->next_sequence + index))', 'if (false)'),
        ('missing_allowed', 'sequence >= journal->next_sequence', 'false'),
        ('capacity_overwrite', 'if (journal->next_sequence > journal->sectors) return EPOCH_JOURNAL_FULL;', 'if (false) return EPOCH_JOURNAL_FULL;'),
        ('replay_order', 'uint64_t sequence = 1;\n    while (sequence < journal->next_sequence) {', 'uint64_t sequence = 2;\n    while (sequence < journal->next_sequence) {'),
        ('replay_semantics_skipped', 'status = epoch_journal_accept(model, event);', 'status = EPOCH_JOURNAL_OK;\n        (void)model;'),
        ('replay_boundary_not_checked', 'if (model.sessions != journal->sessions || model.next_event != journal->next_event)\n        return EPOCH_JOURNAL_CORRUPT;', 'if (false)\n        return EPOCH_JOURNAL_CORRUPT;'),
        ('open_needs_blank', 'journal->next_sequence = first_blank ? first_blank : journal->sectors + 1;', 'journal->next_sequence = journal->sectors + 1;'),
        ('raw_epoch_allowed', 'event->kind == EPOCH_JOURNAL_SESSION_KIND || !journal->sessions ||', 'event->kind == EPOCH_JOURNAL_SESSION_KIND || false ||'),
        ('commit_generation_not_advanced', 'journal->commit_generation = generation;', 'journal->commit_generation += 0;'),
        ('pending_recovery_skipped', 'while (journal->next_sequence <= journal->sectors)', 'while (false)'),
        ('commit_generation_wrapped', 'journal->commit_generation == UINT64_MAX', 'false'),
    ]
    caught = []
    if args.mutations:
        original = LayoutText(source.read_text())
        for name, before, after in mutations:
            assert original.count(before) == 1, (name, 'anchor')
            try:
                source.write_text(original.replace(before, after, 1))
                binary = build('gcc', ['-O2'], name)
                result = subprocess.run([binary], cwd=HERE, capture_output=True)
                assert result.returncode != 0, ('mutation survived', name)
                caught.append(name)
            finally:
                source.write_text(original)
    names = ['src/epoch_journal.cesta', 'src/epoch_journal_interface.cesta',
             'tests/epoch_journal.py', 'tests/epoch_journal/host.c']
    summary = {
        'builds': builds,
        'mutations_caught': caught,
        'source_sha256': {name: hashlib.sha256((HERE / name).read_bytes()).hexdigest() for name in names},
        'scope': ('actual epoch event encoding, synchronous sector append/readback, contiguous recovery, '
                  'replay callbacks, corruption and capacity refusal on RAM; no SexFiles integration, '
                  'production persistence or whole-OS rollback claim')
    }
    (work / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n')
    print('epoch-journal qualification passed:', len(caught), 'mutations caught', flush=True)


if __name__ == '__main__':
    main()
