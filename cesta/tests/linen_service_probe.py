"""Join the real correlated Linen client, server, kernel and display adapters."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess

HERE = Path(__file__).resolve().parent.parent


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cesta', type=Path, required=True)
    parser.add_argument('--output', type=Path, default=HERE / 'build/linen-service-probe')
    args = parser.parse_args()
    work = args.output.resolve(); work.mkdir(parents=True, exist_ok=True)

    def run(command):
        result = subprocess.run(list(map(str, command)), cwd=HERE, capture_output=True, text=True)
        assert result.returncode == 0, (command, result.stdout[-2000:], result.stderr[-6000:])
        return result.stdout

    units = ('kernel_domains', 'kernel_syscalls', 'kernel_rpc', 'memory_regions', 'kernel_task_wait',
             'kernel_sources', 'source_policy', 'kernel_pci', 'pci_policy', 'network_config',
             'kernel_history', 'history_recovery', 'linen', 'linen_capture', 'linen_capture_client',
             'linen_capture_transport', 'linen_capture_service', 'shell_linen_snapshot',
             'shell_text_delivery', 'shell_text_delivery_native', 'shell_text_delivery_display',
             'shell_linen_presentation', 'silk_shell_models_projection', 'silk_shell_models_publish',
             'silk_shell_models_publish_native', 'silk_shell_models_linen')
    run([args.cesta, 'export-c', 'src/kernel_interface.cesta', '-o', work / 'sexos_kernel.h',
         '--', '-Iruntime/kernel', '-Igenerated/include'])
    for unit in units:
        run([args.cesta, 'export-c', 'src/' + unit + '.cesta', '--std=c11', '-o', work / (unit + '.c'),
             '--', '-I' + str(work), '-Iruntime/kernel', '-Igenerated/include'])

    linked = ('memory_regions', 'source_policy', 'pci_policy', 'network_config', 'history_recovery',
              'linen_capture', 'linen_capture_client', 'linen_capture_transport', 'linen_capture_service',
              'shell_linen_snapshot', 'shell_text_delivery', 'shell_text_delivery_native',
              'shell_text_delivery_display', 'shell_linen_presentation', 'silk_shell_models_projection',
              'silk_shell_models_publish', 'silk_shell_models_publish_native', 'silk_shell_models_linen')
    records = []
    for compiler in ('gcc', 'clang'):
        for mode, flags in (('O0', ['-O0']), ('O2', ['-O2']),
                            ('SAN', ['-O1', '-fsanitize=address,undefined',
                                     '-fno-sanitize-recover=all', '-fno-pie', '-no-pie'])):
            binary = work / (compiler + '-' + mode)
            run([compiler, '-std=c11', '-Wall', '-Wextra', '-Wpedantic', '-Werror', *flags,
                 '-DSEXOS_KERNEL_CURRENT_PDX=1', '-DSEXOS_LINEN_CORRELATED_REQUIRED=1',
                 '-ffunction-sections', '-fdata-sections', '-Wl,--gc-sections', '-I' + str(work),
                 '-Igenerated/include', '-Iruntime/kernel', '-Itests/linen_service_probe',
                 'tests/linen_service_probe/host.c', 'tests/linen_service_probe/linen_adapter.c',
                 *(work / (unit + '.c') for unit in linked), '-o', binary])
            output = run([binary]).strip()
            records.append({'compiler': compiler, 'mode': mode, 'output': output,
                            'sha256': hashlib.sha256(binary.read_bytes()).hexdigest()})
    summary = {'runtime': records, 'configurations': len(records),
               'scope': 'Protocol probe: actual Cesta shell capture/publication/presentation, Linen dispatch, current kernel RPC and display admission adapters; host CPU boundary and no pixel claim.'}
    (work / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n')
    print('Linen service protocol probe:', len(records), 'configurations;', records[0]['output'])


if __name__ == '__main__':
    main()
