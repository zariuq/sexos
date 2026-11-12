"""Preflight the actual loader before allocator, mapping or manifest effects."""

import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def run(command):
    result = subprocess.run(list(map(str, command)), cwd=ROOT, capture_output=True)
    if result.returncode:
        raise RuntimeError((command, result.returncode, result.stdout[-1500:], result.stderr[-6000:]))
    return result.stdout


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cesta', type=Path, required=True)
    parser.add_argument('--work', type=Path)
    parser.add_argument('--mutations', action='store_true')
    args = parser.parse_args()
    work = args.work.resolve() if args.work else Path(tempfile.mkdtemp(prefix='image-loader-', dir=ROOT/'build'))
    work.mkdir(parents=True, exist_ok=True)
    if any(work.iterdir()):
        parser.error('--work must be empty; previous evidence is retained')
    units = ('frame_lease', 'kernel_domains', 'kernel_memory', 'kernel_apps', 'app_private', 'program_memory', 'program_image', 'program_image_bytes')
    for unit in units:
        run([args.cesta, 'export-c', ROOT/f'src/{unit}.cesta', '--std=c11', '-o', work/f'{unit}.c',
             '--', '-I'+str(ROOT/'generated/include'), '-I'+str(ROOT/'runtime/kernel')])
    flags = ['-std=c11', '-g0', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
             '-ffreestanding', '-fno-builtin', '-ffunction-sections', '-fdata-sections',
             '-DSEXOS_KERNEL_CURRENT_PDX=1', '-I'+str(work), '-I'+str(ROOT/'tests/kernel_memory'),
             '-I'+str(ROOT/'generated/include'), '-I'+str(ROOT/'runtime/kernel'), '-Wl,--gc-sections',
             '-Wl,--wrap=kernel_app_capture_range']
    sources = [ROOT/'tests/program_image/kernel_host.c', ROOT/'tests/kernel_memory/worker_stubs.c',
               work/'frame_lease.c', work/'kernel_apps.c', work/'app_private.c', work/'program_memory.c',
               work/'program_image.c', work/'program_image_bytes.c', ROOT/'runtime/kernel/image_failure.c']
    def compile(cc, mode, name):
        extra = ['-O1', '-fsanitize=address,undefined', '-fno-sanitize-recover=all', '-fno-pie', '-no-pie'] if mode == 'sanitize' else ['-'+mode]
        binary = work/name
        run([cc, *flags, *extra, *sources, '-o', binary])
        return binary
    builds = []
    for cc in ('gcc', 'clang'):
        for mode in ('O0', 'O2', 'sanitize'):
            name = cc+'-'+mode
            output = run([compile(cc, mode, name)])
            (work/(name+'.log')).write_bytes(output)
            builds.append(name)
            print(name, output.decode().strip(), flush=True)
    mutations = [
        ('skip-preflight', 'kernel_domains', 'if (!image_load_preflight(d, &plan)) {', 'if (false && !image_load_preflight(d, &plan)) {'),
        ('incomplete-header-table', 'kernel_domains', 'if (!program_header(image, size, &header, i, &ph)) return 0;', 'if (false && !program_header(image, size, &header, i, &ph)) return 0;'),
        ('overwrite-existing-page', 'kernel_domains', 'if (read_pte(page, &pte)) return false;', 'if (false && read_pte(page, &pte)) return false;'),
        ('late-manifest-exhaustion', 'kernel_domains', 'pages > APP_PRIVATE_MAX_PAGES', '(pages > APP_PRIVATE_MAX_PAGES && false)'),
        ('contiguous-relocation', 'kernel_memory', 'uint64_t chunk = KERNEL_PAGE_SIZE - (address & UINT64_C(4095));', 'uint64_t chunk = count;'),
        ('data-executable', 'kernel_domains', '(s.executable ? 0 : PTE_NO_EXECUTE)', '0'),
        ('stack-underprovisioned', 'kernel_domains', 'plan->stack > USER_STACK_SIZE', 'false'),
        ('unadjusted-bias', 'program_image', 'addend -= image->bias;', '(void)image->bias;'),
    ]
    killed = []
    if args.mutations:
        for name, unit, old, new in mutations:
            path = work/f'{unit}.c'
            original = path.read_text()
            assert original.count(old) == 1, (name, 'mutation anchor changed')
            try:
                path.write_text(original.replace(old, new))
                binary = compile('gcc', 'O2', name)
                observed = subprocess.run([binary], cwd=ROOT, capture_output=True)
                (work/(name+'.log')).write_bytes(observed.stdout+observed.stderr)
                assert observed.returncode != 0, ('mutation survived', name)
                killed.append(name)
            finally:
                path.write_text(original)
    (work/'summary.json').write_text(json.dumps({
        'builds': builds, 'mutations_killed': killed, 'rollback_cases': 45,
        'source_sha256': {name: hashlib.sha256((ROOT/name).read_bytes()).hexdigest() for name in
            [*(f'src/{u}.cesta' for u in units), 'src/kernel_interface.cesta',
             'tests/program_image/kernel_host.c',
             'tests/program_image_kernel.py', 'runtime/kernel/image_failure.c']},
        'scope': 'actual loader/allocator/page tables/private capture and transient rollback on RAM; CPU providers and padding allocations; no boot or whole-C refinement claim'
    }, indent=2)+'\n')


if __name__ == '__main__':
    main()
