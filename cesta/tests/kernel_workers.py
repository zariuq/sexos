#!/usr/bin/env python3
"""Native kernel-worker scheduler and syscall acceptance/refusal controls."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
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
    parser.add_argument('--work', type=Path, required=True)
    parser.add_argument('--mutations', action='store_true')
    args = parser.parse_args()
    work = args.work.resolve()
    work.mkdir(parents=True, exist_ok=True)
    run([args.cesta, 'export-c', 'src/kernel_interface.cesta', '-o', work / 'sexos_kernel.h', '--'])
    run([args.cesta, 'export-c', 'src/program_record_tasks_interface.cesta', '-o', work / 'program_record_tasks.h', '--'])
    run([args.cesta, 'export-c', 'src/program_workers_interface.cesta', '-o', work / 'program_workers.h', '--'])
    sources = []
    for unit in ('kernel_workers', 'program_workers', 'program_memory', 'program_record_tasks'):
        source = work / (unit + '.c')
        run([args.cesta, 'export-c', 'src/' + unit + '.cesta', '--std=c11', '-o', source,
             '--', '-I' + str(work), '-Igenerated/include', '-Iruntime/kernel'])
        sources.append(source)
    flags = ['-std=c11', '-g0', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
             '-ffreestanding', '-fno-builtin', '-ffunction-sections', '-fdata-sections',
             '-DSEXOS_KERNEL_CURRENT_PDX=1', '-DSEXOS_KERNEL_CURRENT_FPU=1',
             '-DSEXOS_KERNEL_CURRENT_SMP=1', '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1',
             '-include', 'tests/kernel_workers/host_cpu.h',
             '-I' + str(work), '-Igenerated/include', '-Iruntime/kernel', '-Wl,--gc-sections']

    def build(cc, extra, name, units=sources):
        output = work / name
        run([cc, *flags, *extra, 'tests/kernel_workers/host.c', *units,
             'tests/kernel_memory/record_stubs.c', '-o', output])
        return output

    builds = []
    for cc in ('gcc', 'clang'):
        for mode in ('O0', 'O2', 'sanitize'):
            extra = (['-O1', '-fsanitize=address,undefined', '-fno-sanitize-recover=all']
                     if mode == 'sanitize' else ['-' + mode])
            output = run([build(cc, extra, cc + '-' + mode)])
            (work / (cc + '-' + mode + '.log')).write_bytes(output)
            builds.append({'compiler': cc, 'mode': mode})
            print(cc, mode, 'kernel-worker controls passed', flush=True)

    mutations = [
        ('exit_run_authority', ' || kernel_current_task(d) != worker->id', ''),
        ('wake_run_authority', 'if (a % 4) return frame;\n        if (!worker_live(d)) {',
         'if (a % 4) return frame;\n        if (false) {'),
        ('deferred_release', 'if (kernel_current_worker) {\n        worker_outgoing(frame);\n    } else if (kernel_current_record) {\n        (void)kernel_record_outgoing(frame);\n    } else {\n        Domain *current = domain_get(kernel.current);\n        if (current) {\n            current->frame = frame;',
         'if (kernel_current_worker) {\n        (void)program_worker_begin_switch_out(kernel_current_worker, 0);\n        (void)program_worker_release_after_stack_switch(kernel_current_worker, 0);\n        worker_outgoing(frame);\n    } else if (kernel_current_record) {\n        (void)kernel_record_outgoing(frame);\n    } else {\n        Domain *current = domain_get(kernel.current);\n        if (current) {\n            current->frame = frame;'),
        ('worker_current', 'kernel_current_worker = worker;', 'kernel_current_worker = NULL;'),
        ('wait_address', 'worker->wait_address = address;', 'worker->wait_address = 0;'),
        ('tls_restore', 'cpu_wrmsr(MSR_FS_BASE, worker->fs_base);', 'cpu_wrmsr(MSR_FS_BASE, 0);'),
        ('fault_begin_flag', 'worker->fault_blocked = true;', 'worker->fault_blocked = false;'),
        ('fault_resume_queue', 'if (!program_worker_admit_queue(&kernel.run_queue, worker))\n        worker->admission_pending = true;',
         'if (true)\n        worker->admission_pending = true;'),
    ]
    caught = []
    if args.mutations:
        original = LayoutText(sources[0].read_text())
        for name, before, after in mutations:
            assert original.count(before) == 1, (name, 'anchor')
            try:
                sources[0].write_text(original.replace(before, after, 1))
                binary = build('gcc', ['-O2'], name)
                result = subprocess.run([binary], cwd=HERE, capture_output=True)
                assert result.returncode != 0, ('mutation survived', name)
                caught.append(name)
            finally:
                sources[0].write_text(original)
    names = ['src/kernel_workers.cesta', 'src/kernel_interface.cesta',
             'src/program_workers.cesta', 'src/program_workers_interface.cesta',
             'tests/kernel_workers.py', 'tests/kernel_workers/host.c', 'tests/kernel_workers/host_cpu.h']
    summary = {
        'builds': builds,
        'mutations_caught': caught,
        'source_sha256': {name: hashlib.sha256((HERE / name).read_bytes()).hexdigest()
                          for name in names},
        'scope': ('actual native scheduler/syscall/pager C with host hardware and domain stubs; '
                  'no real machine fault, active SMP or restart proof')
    }
    (work / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n')
    print('kernel-worker qualification passed:', len(caught), 'mutations caught', flush=True)


if __name__ == '__main__':
    main()
