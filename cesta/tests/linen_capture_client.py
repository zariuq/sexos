"""Exercise the real capture client/server across adverse transport schedules."""
import argparse
import hashlib
import importlib
import json
import os
from pathlib import Path
import subprocess
import sys

HERE = Path(__file__).resolve().parent.parent


def main():
    ap = argparse.ArgumentParser(); ap.add_argument('--cesta', required=True)
    ap.add_argument('--lean-root', type=Path); ap.add_argument('--cesta-source', type=Path)
    ap.add_argument('--output', type=Path, default=HERE / 'build/linen-capture-client'); args = ap.parse_args()
    w = args.output.resolve(); w.mkdir(parents=True, exist_ok=True); records = []; plans = {}
    include = '-I' + str(HERE / 'generated/include'); units = ['linen_capture', 'linen_capture_client']
    originals = {unit: (HERE / ('src/' + unit + '.cesta')).read_bytes() for unit in units}
    for cc in ('gcc', 'clang'):
        sources = []
        for unit in units:
            c = w / (unit + '-' + cc + '.c'); plan = c.with_suffix('.plan'); sources.append(str(c))
            subprocess.run([args.cesta, 'export-c', str(HERE / ('src/' + unit + '.cesta')), '--cc', cc, '--std=c11', '-o', str(c), '--plan', str(plan), '--', include], check=True)
            if unit == 'linen_capture_client': plans[hashlib.sha256(plan.read_bytes()).hexdigest()] = plan
        for mode, flags in [('O0', ['-O0']), ('O2', ['-O2']), ('SAN', ['-O1', '-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-fno-pie', '-no-pie'])]:
            binary = w / (cc + '-' + mode)
            subprocess.run([cc, '-std=c11', '-Wall', '-Wextra', '-Wpedantic', '-Werror', *flags, include, *sources, str(HERE / 'tests/linen_capture/client_host.c'), '-o', str(binary)], check=True)
            result = subprocess.run([str(binary)], capture_output=True, env={**os.environ, 'ASAN_OPTIONS': 'detect_leaks=1:abort_on_error=1'})
            assert result.returncode == 0, result.stderr.decode()[-4000:]
            records.append(dict(compiler=cc, mode=mode, output=result.stdout.decode().strip()))
    admission = []
    if args.lean_root:
        assert args.cesta_source; sys.path.insert(0, str(args.cesta_source.resolve() / 'tests'))
        native = importlib.import_module('ownership_admission')
        for i, plan in enumerate(plans.values()):
            record, = native.parse(plan.read_bytes()); admission.append(native.admit_record(record, w / f'Client{i}.lean', args.lean_root.resolve()))
    for unit, original in originals.items(): assert (HERE / ('src/' + unit + '.cesta')).read_bytes() == original
    (w / 'summary.json').write_text(json.dumps(dict(runtime=records, admission=admission), indent=2) + '\n')
    print('Correlated Linen client:', len(records), 'configurations;', records[0]['output'])


if __name__ == '__main__': main()
