#!/usr/bin/env python3
"""Actual RAM/page-table controls for tracked program-frame leases."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile
from layout_text import LayoutText

HERE = Path(__file__).resolve().parent.parent


def run(argv, data=None):
    result = subprocess.run(list(map(str, argv)), cwd=HERE, input=data, capture_output=True)
    if result.returncode:
        raise RuntimeError((argv, result.returncode, result.stdout[-2000:], result.stderr[-6000:]))
    return result.stdout


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cesta', type=Path, required=True)
    parser.add_argument('--work', type=Path)
    parser.add_argument('--mutations', action='store_true')
    args = parser.parse_args()
    work = args.work or Path(tempfile.mkdtemp(prefix='program-frames-', dir=HERE / 'build'))
    work.mkdir(parents=True, exist_ok=True)
    for unit in ('frame_lease', 'kernel_memory'):
        (work / (unit + '.c')).write_bytes(run([
            args.cesta, 'export-c', 'src/' + unit + '.cesta', '--std=c11', '--',
            '-Igenerated/include', '-Iruntime/kernel']))
    flags = ['-std=c11', '-g0', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
             '-ffreestanding', '-fno-builtin', '-ffunction-sections', '-fdata-sections',
             '-DSEXOS_KERNEL_CURRENT_PDX=1', '-I' + str(work), '-Itests/kernel_memory',
             '-Igenerated/include', '-Iruntime/kernel', '-Wl,--gc-sections']

    def build(cc, extra, name):
        output = work / name
        run([cc, *flags, *extra, 'tests/program_frames/host.c', work / 'frame_lease.c', '-o', output])
        return output

    builds = []
    for cc in ('gcc', 'clang'):
        for mode in ('O0', 'O2', 'sanitize'):
            extra = (['-O1', '-fsanitize=address,undefined', '-fno-sanitize-recover=all']
                     if mode == 'sanitize' else ['-' + mode])
            output = run([build(cc, extra, cc + '-' + mode)])
            (work / (cc + '-' + mode + '.log')).write_bytes(output)
            builds.append({'compiler': cc, 'mode': mode})
            print(cc, mode, 'native program-frame controls passed', flush=True)

    mutations = [
        ('bypass_mapping', 'if (program_frame_blocked(phys) && !program_frames.map_active) return false;',
         'if (program_frame_blocked(phys) && !program_frames.map_active && phys == 0) return false;'),
        ('mapped_return', 'if (binding->mapped) return FRAME_LEASE_RETRY;',
         'if (false) return FRAME_LEASE_RETRY;'),
        ('detach_flush', 'cpu_invlpg(binding->va);', '(void)binding->va;'),
        ('reuse_zero', 'memset(phys_to_virt(result.value.base), 0, KERNEL_PAGE_SIZE);',
         '(void)result.value.base;'),
        ('quarantine', 'if (binding->mapped || binding->returning) return false;',
         'if (false) return false;'),
        ('pte_match', 'uint64_t actual = *leaf;\n        if ((actual & ~UINT64_C(0x60)) != (binding->expected & ~UINT64_C(0x60)))',
         'uint64_t actual = *leaf;\n        if ((actual & ~UINT64_C(0x60)) == (binding->expected & ~UINT64_C(0x60)))'),
        ('stale_binding', 'a.incarnation == b.incarnation', 'true'),
        ('commit_binding', 'frames.leases.entries[cores[i]].holder = 0;\n            *binding = (ProgramFrameBinding){0};',
         'frames.leases.entries[cores[i]].holder = 0;\n            (void)binding;'),
        ('managed_commit_kind', 'binding->lease.kind = FRAME_LEASE_PROGRAM;',
         'binding->lease.kind = FRAME_LEASE_TRANSIENT;'),
        ('managed_commit_lent', 'frames.leases.lent += frames.leases.entries[cores[i]].pages;',
         'frames.leases.lent += 0;'),
        ('handover_owner', 'binding->lease.holder = holder;', 'binding->lease.holder = target;'),
        ('handover_old_run', 'binding->run = 0;', 'binding->run = run;'),
        ('release_zero_run', 'if (!target || !run) return false;', 'if (!target) return false;'),
    ]
    caught = []
    if args.mutations:
        path = work / 'kernel_memory.c'
        original = LayoutText(path.read_text())
        for name, before, after in mutations:
            assert original.count(before) == 1, (name, 'anchor')
            try:
                path.write_text(original.replace(before, after, 1))
                binary = build('gcc', ['-O2'], name)
                result = subprocess.run([binary], cwd=HERE, capture_output=True)
                assert result.returncode != 0, ('mutation survived', name)
                caught.append(name)
            finally:
                path.write_text(original)
    names = ['src/frame_lease.cesta', 'src/frame_lease_interface.cesta',
             'src/kernel_memory.cesta', 'src/kernel_interface.cesta',
             'tests/program_frames.py', 'tests/program_frames/host.c']
    summary = {
        'builds': builds,
        'mutations_caught': caught,
        'source_sha256': {name: hashlib.sha256((HERE / name).read_bytes()).hexdigest()
                          for name in names},
        'scope': ('production allocator, tracked mappings and physical RAM on a host provider; '
                  'exact PTE clear/invalidation and zero-on-reuse; no scheduler, worker/restart '
                  'lifecycle, hardware execution or boot-reuse claim')
    }
    (work / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n')
    print('program-frame native qualification passed:', len(caught), 'mutations caught', flush=True)


if __name__ == '__main__':
    main()
