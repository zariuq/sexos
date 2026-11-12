#!/usr/bin/env python3
"""Lifecycle, rollback and mutation controls for program memory."""
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
    parser.add_argument('--cesta-source', type=Path)
    parser.add_argument('--lean-root', type=Path)
    args = parser.parse_args()
    work = args.work or Path(tempfile.mkdtemp(prefix='program-memory-', dir=HERE / 'build'))
    work.mkdir(parents=True, exist_ok=True)
    for unit in ('frame_lease', 'kernel_memory', 'program_memory'):
        (work / (unit + '.c')).write_bytes(run([
            args.cesta, 'export-c', 'src/' + unit + '.cesta', '--std=c11', '--',
            '-Igenerated/include', '-Iruntime/kernel']))
    plan = work / 'program_memory.plan'
    run([args.cesta, 'export-c', 'src/program_memory.cesta', '--std=c11', '--plan', plan,
         '-o', work / 'program_memory.c', '--', '-Igenerated/include', '-Iruntime/kernel'])
    flags = ['-std=c11', '-g0', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
             '-ffreestanding', '-fno-builtin', '-ffunction-sections', '-fdata-sections',
             '-DSEXOS_KERNEL_CURRENT_PDX=1', '-I' + str(work), '-Itests/kernel_memory',
             '-Igenerated/include', '-Iruntime/kernel', '-Wl,--gc-sections']

    def build(cc, extra, name):
        output = work / name
        run([cc, *flags, *extra, 'tests/program_memory/host.c', work / 'frame_lease.c', '-o', output])
        return output

    builds = []
    for cc in ('gcc', 'clang'):
        for mode in ('O0', 'O2', 'sanitize'):
            extra = (['-O1', '-fsanitize=address,undefined', '-fno-sanitize-recover=all']
                     if mode == 'sanitize' else ['-' + mode])
            output = run([build(cc, extra, cc + '-' + mode)])
            (work / (cc + '-' + mode + '.log')).write_bytes(output)
            builds.append({'compiler': cc, 'mode': mode})
            print(cc, mode, 'program-memory lifecycle controls passed', flush=True)

    mutations = [
        ('grow_commit', 'state->grown += want;', 'state->grown += 0;'),
        ('heap_base', 'if (!want) {\n        reply.status = PROGRAM_MEMORY_OK;\n        reply.base = heap_base;',
         'if (!want) {\n        reply.status = PROGRAM_MEMORY_OK;\n        reply.base = state->base;'),
        ('rollback_lost', 'if (!program_frame_release(leases[i - 1])) state->phase = PROGRAM_MEMORY_QUARANTINE;',
         'if (false) state->phase = PROGRAM_MEMORY_QUARANTINE;'),
        ('end_gate', '(state->main_alive || state->workers)', 'false'),
        ('run_identity', '!binding->present || binding->lease.holder != target || binding->run != run',
         '!binding->present || binding->lease.holder != target || (binding->run != run && run == 0)'),
        ('map_failure', 'if (lease.status != FRAME_LEASE_OK) return false;',
         'if (lease.status != FRAME_LEASE_OK) return true;'),
        ('pressure_hysteresis', 'uint64_t falling = program_pressure_level(free - free / 5, base);',
         'uint64_t falling = current;'),
    ]
    caught = []
    if args.mutations:
        original = LayoutText((work / 'program_memory.c').read_text())
        kernel_original = LayoutText((work / 'kernel_memory.c').read_text())
        for name, before, after in mutations:
            path = work / ('kernel_memory.c' if name in ('run_identity', 'pressure_hysteresis') else 'program_memory.c')
            text = kernel_original if path.name == 'kernel_memory.c' else original
            assert text.count(before) == 1, (name, 'anchor')
            try:
                path.write_text(text.replace(before, after, 1))
                binary = build('gcc', ['-O2'], name)
                result = subprocess.run([binary], cwd=HERE, capture_output=True)
                assert result.returncode != 0, ('mutation survived', name)
                caught.append(name)
            finally:
                path.write_text(text)
    admission = None
    if args.lean_root:
        if not args.cesta_source:
            parser.error('--lean-root requires --cesta-source')
        import sys
        sys.path.insert(0, str(args.cesta_source.resolve() / 'tests'))
        import ownership_admission
        record, = ownership_admission.parse(plan.read_bytes())
        admission = ownership_admission.admit_record(
            record, work / 'ProgramMemoryOwnership.lean', args.lean_root.resolve())
    names = ['src/frame_lease.cesta', 'src/kernel_memory.cesta', 'src/kernel_interface.cesta',
             'src/program_memory.cesta', 'src/program_memory_interface.cesta',
             'tests/program_memory.py', 'tests/program_memory/host.c']
    summary = {
        'builds': builds,
        'mutations_caught': caught,
        'admission': admission,
        'source_sha256': {name: hashlib.sha256((HERE / name).read_bytes()).hexdigest()
                          for name in names},
        'scope': ('production program-memory growth, rollback and exact run cleanup on RAM; '
                  'no scheduler task objects, restart protocol, CPU execution or boot-lifecycle claim')
    }
    (work / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n')
    print('program-memory qualification passed:', len(caught), 'mutations caught', flush=True)


if __name__ == '__main__':
    main()
