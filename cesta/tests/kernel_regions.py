#!/usr/bin/env python3
"""Execute current kernel region dispatch and BOOT stores with counted providers."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile
from layout_text import LayoutText

HERE = Path(__file__).resolve().parent.parent


def run(argv):
    p = subprocess.run(list(map(str, argv)), cwd=HERE, capture_output=True)
    if p.returncode:
        raise RuntimeError((argv, p.returncode, p.stdout[-2000:], p.stderr[-5000:]))
    return p.stdout


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--cesta', type=Path, required=True)
    ap.add_argument('--work', type=Path)
    ap.add_argument('--mutations', action='store_true')
    args = ap.parse_args()
    work = args.work or Path(tempfile.mkdtemp(prefix='kernel-regions-', dir=HERE/'build'))
    work.mkdir(parents=True, exist_ok=True)
    units = ('kernel_domains', 'kernel_syscalls', 'kernel_rpc', 'memory_regions', 'kernel_task_wait', 'kernel_sources', 'source_policy','kernel_pci','pci_policy','network_config','kernel_history','history_recovery',
             'region_writer', 'kernel_region_writer', 'device_reset', 'kernel_device_reset')
    for unit in units:
        (work/(unit+'.c')).write_bytes(run([args.cesta, 'export-c', 'src/'+unit+'.cesta',
            '--std=c11', '--', '-Igenerated/include', '-Iruntime/kernel']))
    flags = ['-std=c11', '-g0', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
             '-DSEXOS_KERNEL_CURRENT_PDX=1', '-ffunction-sections', '-fdata-sections',
             '-Wl,--gc-sections', '-I'+str(work), '-Igenerated/include', '-Iruntime/kernel']

    def compile(cc, extra, name):
        binary = work/name
        run([cc, *flags, *extra, HERE/'tests/kernel_regions/host.c', work/'memory_regions.c', work/'source_policy.c',work/'pci_policy.c',work/'network_config.c',work/'history_recovery.c',
             work/'region_writer.c', work/'kernel_region_writer.c', work/'device_reset.c', work/'kernel_device_reset.c', '-o', binary])
        return binary

    builds = []
    for cc in ('gcc', 'clang'):
        for mode in ('O0', 'O2', 'sanitize'):
            extra = ['-O1', '-fsanitize=address,undefined', '-fno-sanitize-recover=all'] if mode == 'sanitize' else ['-'+mode]
            name = cc+'-'+mode
            run([compile(cc, extra, name)])
            builds.append(name)
            print(name, 'kernel region and sparse-store controls passed', flush=True)
    mutants = [
        ("unknown_call_residue", "kernel_syscalls", "f->r8 = 0;", "f->r8 = 1;"),
        ('ambient_display_bridge', 'kernel_syscalls', '!SEXOS_KERNEL_CURRENT_PDX && operation == 0xE3', 'operation == 0xE3'),
        ('display_address_scope', 'kernel_syscalls', 'arg0 == 1 && d->id == kernel.display_pd', 'arg0 == 1'),
        ('selector_alias', 'kernel_domains',
         'const Cap *cap_get(Domain *d, uint64_t slot)\n{',
         'const Cap *cap_get(Domain *d, uint64_t slot)\n{ slot=(uint32_t)slot;'),
        ('unsealed_runtime_install', 'kernel_domains',
         'if (kernel.genesis_sealed || !slot', 'if (false || !slot'),
        ('wrong_capability_kind', 'kernel_syscalls',
         'held->kind == CAP_REGION || held->kind == CAP_PCI', 'held->kind != CAP_NONE'),
        ('failed_register_residue', 'kernel_syscalls',
         'f->rsi=r.value0;', 'if (!r.status) f->rsi=r.value0;'),
        ('runtime_physical_allocation', 'kernel_syscalls',
         'SEXOS_KERNEL_CURRENT_PDX ? NO_ADDRESS : allocate_memory', 'false ? NO_ADDRESS : allocate_memory'),
        ('runtime_bar_mapping', 'kernel_syscalls',
         'SEXOS_KERNEL_CURRENT_PDX ? current_bar_view', 'false ? current_bar_view'),
        ('grant_store_loses_value', 'kernel_domains', '*stored=c;', 'stored->kind=CAP_NONE;'),
    ]
    caught = []
    if args.mutations:
        for name, unit, before, after in mutants:
            path = work/(unit+'.c'); original = LayoutText(path.read_text())
            assert original.count(before) == 1, (name, 'mutation anchor changed')
            try:
                path.write_text(original.replace(before, after))
                binary = compile('gcc', ['-O2'], name)
                result = subprocess.run([binary], cwd=HERE, capture_output=True)
                assert result.returncode != 0, ('mutation survived', name)
                caught.append(name)
            finally:
                path.write_text(original)
    names = ['src/'+n+'.cesta' for n in units] + ['src/kernel_interface.cesta',
             'src/memory_regions_interface.cesta', 'tests/kernel_regions.py', 'tests/kernel_regions/host.c',
             'tests/kernel_model/host.c', 'generated/include/sexos_kernel.h', 'generated/include/sexos_memory_regions.h']
    summary = {'builds': builds, 'mutations_caught': caught,
               'source_sha256': {n: hashlib.sha256((HERE/n).read_bytes()).hexdigest() for n in names},
               'scope': 'actual compiled kernel dispatch and finite BOOT stores under counted memory/CPU providers; no native mappings, hardware isolation, scheduler or whole-C refinement claim'}
    (work/'summary.json').write_text(json.dumps(summary, indent=2)+'\n')
    print('kernel regions passed;', len(caught), 'semantic mutations caught', flush=True)


if __name__ == '__main__':
    main()
