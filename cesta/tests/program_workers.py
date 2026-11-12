#!/usr/bin/env python3
"""Worker-record lifecycle and mutation controls."""
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
    work = args.work or Path(tempfile.mkdtemp(prefix='program-workers-', dir=HERE / 'build'))
    work.mkdir(parents=True, exist_ok=True)
    (work / 'program_record_tasks.h').write_bytes(run([
        args.cesta, 'export-c', 'src/program_record_tasks_interface.cesta', '--']))
    (work / 'sexos_kernel.h').write_bytes(run([
        args.cesta, 'export-c', 'src/kernel_interface.cesta', '--',
        '-Igenerated/include', '-Iruntime/kernel']))
    (work / 'program_workers.h').write_bytes(run([
        args.cesta, 'export-c', 'src/program_workers_interface.cesta', '--']))
    (work / 'program_record_tasks.c').write_bytes(run([
        args.cesta, 'export-c', 'src/program_record_tasks.cesta', '--std=c11', '--',
        '-I' + str(work), '-Igenerated/include', '-Iruntime/kernel']))
    source = work / 'program_workers.c'
    plan = work / 'program_workers.plan'
    run([args.cesta, 'export-c', 'src/program_workers.cesta', '--std=c11', '--plan', plan,
         '-o', source, '--', '-I' + str(work), '-Igenerated/include', '-Iruntime/kernel'])
    flags = ['-std=c11', '-g0', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
             '-ffreestanding', '-fno-builtin', '-ffunction-sections', '-fdata-sections',
             '-DSEXOS_KERNEL_CURRENT_PDX=1', '-I' + str(work), '-Itests/kernel_memory',
             '-Igenerated/include', '-Iruntime/kernel', '-Wl,--gc-sections']

    def build(cc, extra, name):
        output = work / name
        run([cc, *flags, *extra, 'tests/program_workers/host.c', source,
             work / 'program_record_tasks.c', '-o', output])
        return output

    builds = []
    for cc in ('gcc', 'clang'):
        for mode in ('O0', 'O2', 'sanitize'):
            extra = (['-O1', '-fsanitize=address,undefined', '-fno-sanitize-recover=all']
                     if mode == 'sanitize' else ['-' + mode])
            output = run([build(cc, extra, cc + '-' + mode)])
            (work / (cc + '-' + mode + '.log')).write_bytes(output)
            builds.append({'compiler': cc, 'mode': mode})
            print(cc, mode, 'worker-record controls passed', flush=True)

    mutations = [
        ('grace_guard', 'now - worker->dead_since >= PROGRAM_WORKER_REUSE_TICKS', 'true'),
        ('rearm_fresh', 'memset(storage->stack, 0, PROGRAM_WORKER_STACK_BYTES);',
         '(void)storage->stack;'),
        ('exit_gate', '(worker->phase != PROGRAM_WORKER_DEAD || worker->owner_core != PROGRAM_WORKER_NO_CORE)',
         'false'),
        ('id_lower_bound', 'table->next_id < PROGRAM_WORKER_TASK_FIRST', 'false'),
        ('id_upper_bound', 'table->next_id >= PROGRAM_WORKER_TASK_END', 'false'),
        ('id_last_valid', 'table->next_id >= PROGRAM_WORKER_TASK_END',
         'table->next_id >= PROGRAM_WORKER_TASK_END - 1'),
        ('id_permanent_exhaustion', '++table->next_id;',
         'table->next_id = table->next_id + 1 == PROGRAM_WORKER_TASK_END ? '
         'PROGRAM_WORKER_TASK_FIRST : table->next_id + 1;'),
        ('park_pending', 'if (worker->pending) {', 'if (false) {'),
        ('detach_owner', 'worker->phase != PROGRAM_WORKER_SWITCHING_OUT || worker->owner_core != core',
         'worker->phase != PROGRAM_WORKER_SWITCHING_OUT || worker->owner_core != (core + 1)'),
        ('queue_handoff_admission', 'program_record_queue_admit(ring, (uintptr_t)worker)',
         'program_record_queue_push(ring, (uintptr_t)worker)'),
        ('queue_phase', 'worker->admission_pending = false;\n    worker->phase = PROGRAM_WORKER_QUEUED;\n    return true;',
         'worker->admission_pending = false;\n    worker->phase = PROGRAM_WORKER_DETACHED;\n    return true;'),
        ('queue_selection', 'program_record_queue_select(ring)',
         'program_record_queue_pop(ring)'),
        ('queue_core_guard', 'if (core >= PROGRAM_WORKER_MAX_CORES) return NULL;',
         'if (false) return NULL;'),
        ('queue_retire', 'return worker && program_worker_begin_dispatch(worker, core) ? worker : NULL;',
         '(void)program_worker_begin_dispatch(worker, core); return worker;'),
        ('wake_backpressure', 'return PROGRAM_WORKER_RETRY;', 'return status;'),
        ('poll_retry', '!program_worker_admit_queue(ring, worker))', '((void)ring, false))'),
        ('fault_admission', 'worker->exited || worker->fault_blocked)',
         'worker->exited || false)'),
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
    admission = None
    if args.lean_root:
        if not args.cesta_source:
            parser.error('--lean-root requires --cesta-source')
        import sys
        sys.path.insert(0, str(args.cesta_source.resolve() / 'tests'))
        import ownership_admission
        record, = ownership_admission.parse(plan.read_bytes())
        admission = ownership_admission.admit_record(
            record, work / 'ProgramWorkersOwnership.lean', args.lean_root.resolve())
    names = ['src/program_workers.cesta', 'src/program_workers_interface.cesta',
             'tests/program_workers.py', 'tests/program_workers/host.c']
    summary = {
        'builds': builds,
        'mutations_caught': caught,
        'admission': admission,
        'source_sha256': {name: hashlib.sha256((HERE / name).read_bytes()).hexdigest()
                          for name in names},
        'scope': ('pointer-free worker metadata and delayed rearm; native storage, queues '
                  'and machine execution remain separate evidence layers')
    }
    (work / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n')
    print('program-worker qualification passed:', len(caught), 'mutations caught', flush=True)


if __name__ == '__main__':
    main()
