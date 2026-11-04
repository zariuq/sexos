#!/usr/bin/env python3
"""Paired Rust-legacy and correlated-Cesta Linen protocol-probe boots."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import time

HERE = Path(__file__).resolve().parent.parent


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def run(command, log=None):
    if log is None:
        subprocess.run(list(map(str, command)), cwd=HERE, check=True)
    else:
        with log.open('wb') as stream:
            subprocess.run(list(map(str, command)), cwd=HERE, stdout=stream,
                           stderr=subprocess.STDOUT, check=True)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--reference-tree', type=Path, required=True)
    parser.add_argument('--reference-image', type=Path, required=True)
    parser.add_argument('--kernel', type=Path, required=True)
    parser.add_argument('--linen', type=Path, required=True)
    parser.add_argument('--probe', type=Path, required=True)
    parser.add_argument('--limine', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    work = args.output.resolve()
    if work.exists():
        raise RuntimeError('choose a new output directory; boot evidence is immutable')
    work.mkdir(parents=True)

    reference_serial = work / 'legacy-rust.serial.log'
    start = time.monotonic()
    run(['bash', HERE / 'oracle/run.sh', args.reference_image.resolve(), reference_serial, '180',
         '--fixed-clock', '--until', '[linen.remote.snapshot.ok]'], work / 'legacy-rust.qemu.log')
    reference = reference_serial.read_text(errors='replace')
    assert '[linen.remote.snapshot.ok]' in reference
    assert '[linen.protocol.probe.done]' not in reference

    package = work / 'correlated-package'
    run(['python3', HERE / 'tools/package_client.py', '--reference-tree', args.reference_tree.resolve(),
         '--replace', 'servers/linen=' + str(args.linen.resolve()), '--replace',
         'servers/silk-shell=' + str(args.probe.resolve()), '--kernel', args.kernel.resolve(),
         '--limine', args.limine.resolve(), '--out', package], work / 'package.log')
    image, = package.glob('*.iso')
    strong_serial = work / 'correlated-cesta.serial.log'
    run(['bash', HERE / 'oracle/run.sh', image, strong_serial, '180', '--fixed-clock',
         '--until', '[linen.protocol.probe.done]'], work / 'correlated-cesta.qemu.log')
    strong = strong_serial.read_text(errors='replace')
    assert '[linen.protocol.probe.begin] mode=correlated-required legacy=0' in strong
    assert '[linen.transport.mode] correlated=1 legacy=0 shell_pd=3 shell_task=3' in strong
    assert '[linen.protocol.probe.done] mode=correlated capture=1 published=1 display_ack=1' in strong
    for marker in ('KERNEL PANIC', '[cesta.client.fault]', '[linen.protocol.probe.failed]'):
        assert marker not in strong, marker

    summary = {
        'scope': 'Paired protocol-probe images, not a completed Cesta shell boot.',
        'legacy': {'image_sha256': digest(args.reference_image.resolve()),
                   'marker': '[linen.remote.snapshot.ok]', 'mode': 'Rust-compatible legacy'},
        'correlated': {'image_sha256': digest(image), 'kernel_sha256': digest(args.kernel.resolve()),
                       'linen_sha256': digest(args.linen.resolve()), 'probe_sha256': digest(args.probe.resolve()),
                       'marker': '[linen.protocol.probe.done]', 'mode': 'Cesta correlated-required'},
        'seconds': time.monotonic() - start,
    }
    (work / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n')
    print('Linen paired protocol boots passed: Rust legacy and Cesta correlated-required')


if __name__ == '__main__':
    main()
