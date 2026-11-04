"""Join the actual current kernel RPC and capture adapters on the host boundary."""
import argparse
import hashlib
import importlib
import json
from pathlib import Path
import subprocess
import sys

HERE = Path(__file__).resolve().parent.parent


def main():
    ap = argparse.ArgumentParser(); ap.add_argument('--cesta', required=True)
    ap.add_argument('--lean-root', type=Path); ap.add_argument('--cesta-source', type=Path)
    ap.add_argument('--output', type=Path, default=HERE / 'build/linen-capture-kernel'); args = ap.parse_args()
    w = args.output.resolve(); w.mkdir(parents=True, exist_ok=True)
    def run(command):
        result = subprocess.run(list(map(str, command)), cwd=HERE, capture_output=True, text=True)
        assert result.returncode == 0, (command, result.stdout[-2000:], result.stderr[-5000:])
        return result.stdout
    units = ('kernel_domains', 'kernel_syscalls', 'kernel_rpc', 'memory_regions', 'kernel_task_wait',
             'kernel_sources', 'source_policy', 'kernel_pci', 'pci_policy', 'network_config', 'kernel_history', 'history_recovery',
             'linen_capture', 'linen_capture_client', 'linen_capture_transport', 'linen_capture_service', 'rpc_runtime')
    run([args.cesta, 'export-c', 'src/kernel_interface.cesta', '-o', w / 'sexos_kernel.h', '--', '-Iruntime/kernel', '-Igenerated/include'])
    plans = {}
    for unit in units:
        c = w / (unit + '.c'); plan = c.with_suffix('.plan')
        run([args.cesta, 'export-c', 'src/' + unit + '.cesta', '--std=c11', '-o', c, '--plan', plan,
             '--', '-I' + str(w), '-Iruntime/kernel', '-Igenerated/include'])
        if unit in ('linen_capture_transport', 'linen_capture_service'):
            plans[hashlib.sha256(plan.read_bytes()).hexdigest()] = plan
    linked = ['memory_regions', 'source_policy', 'pci_policy', 'network_config', 'history_recovery',
              'linen_capture', 'linen_capture_client', 'linen_capture_transport', 'linen_capture_service', 'rpc_runtime']
    records = []
    for cc in ('gcc', 'clang'):
        for mode, extra in [('O0', ['-O0']), ('O2', ['-O2']), ('SAN', ['-O1', '-fsanitize=address,undefined', '-fno-sanitize-recover=all', '-fno-pie', '-no-pie'])]:
            binary = w / (cc + '-' + mode)
            run([cc, '-std=c11', '-Wall', '-Wextra', '-Wpedantic', '-Werror', *extra, '-DSEXOS_KERNEL_CURRENT_PDX=1',
                 '-ffunction-sections', '-fdata-sections', '-Wl,--gc-sections', '-I' + str(w), '-Igenerated/include', '-Iruntime/kernel',
                 'tests/linen_capture/kernel_host.c', *(w / (unit + '.c') for unit in linked), '-o', binary])
            result = run([binary]); records.append(dict(compiler=cc, mode=mode, output=result.strip()))
    admission = []
    if args.lean_root:
        assert args.cesta_source; sys.path.insert(0, str(args.cesta_source.resolve() / 'tests'))
        native = importlib.import_module('ownership_admission')
        for i, plan in enumerate(plans.values()):
            record, = native.parse(plan.read_bytes()); admission.append(native.admit_record(record, w / f'Adapters{i}.lean', args.lean_root.resolve()))
    (w / 'summary.json').write_text(json.dumps(dict(runtime=records, admission=admission,
        scope='Actual current kernel units and capture adapters with a three-domain host BOOT fixture; no CPU trap or QEMU execution.'), indent=2) + '\n')
    print('Linen capture/kernel:', len(records), 'configurations;', records[0]['output'])


if __name__ == '__main__': main()
