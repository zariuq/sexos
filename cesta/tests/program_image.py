"""Pinned Rust image parsing, independent kernel policy and retained byte views."""

import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import random
import struct
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
MAX = (1 << 64) - 1
WINDOW = 16 * 1024 * 1024


def run(command, *, data=None, cwd=ROOT):
    result = subprocess.run(list(map(str, command)), cwd=cwd, input=data, capture_output=True)
    if result.returncode:
        raise RuntimeError((command, result.returncode, result.stdout[-1000:], result.stderr[-5000:]))
    return result.stdout


def put(data, offset, size, value):
    data[offset:offset+size] = value.to_bytes(size, 'little')


def elf(segments, bias=0, dynamic=False, stack=0):
    # Segment tuples are (window offset, file offset, file bytes, memory, flags).
    headers = [(1, flags, offset, bias+start, file, memory)
               for start, offset, file, memory, flags in segments]
    if dynamic:
        headers.append((2, 6, 8192, bias+4096, 64, 64))
    if stack:
        headers.append((0x6474e551, 6, 0, 0, 0, stack))
    size = max([64+56*len(headers), *(offset+file for _, offset, file, _, _ in segments)])
    data = bytearray(size)
    data[:7] = b'\x7fELF\x02\x01\x01'
    put(data, 16, 2, 3); put(data, 18, 2, 62); put(data, 20, 4, 1)
    put(data, 24, 8, bias+segments[0][0]); put(data, 32, 8, 64)
    put(data, 52, 2, 64); put(data, 54, 2, 56); put(data, 56, 2, len(headers))
    for i, (kind, flags, offset, address, file, memory) in enumerate(headers):
        struct.pack_into('<IIQQQQQQ', data, 64+i*56, kind, flags, offset, address, 0, file, memory, 4096)
    for _, offset, file, _, flags in segments:
        data[offset:offset+file] = bytes([0x90 if flags & 1 else 0]) * file
    if dynamic:
        for i, (tag, value) in enumerate(((7, bias+4096+64), (8, 48), (9, 24), (0, 0))):
            struct.pack_into('<QQ', data, 8192+16*i, tag, value)
        # The third valid-looking entry is deliberately outside DT_RELASZ.
        # Accessor index refusal must not rely on incidental bytes after it.
        for i, (target, addend) in enumerate(((4096+160, 8), (8190, 16), (4096+192, 32))):
            struct.pack_into('<QQQ', data, 8192+64+24*i, bias+target, 8, bias+addend)
    return data


def corpus(images):
    cases = []
    def add(name, data, window=WINDOW, error=None, policy=None):
        cases.append((name, bytes(data), window, error, policy))
    plain = elf([(0, 4096, 16, 4096, 5)])
    relocated = elf([(0, 4096, 16, 4096, 5), (4096, 8192, 256, 8192, 6)], dynamic=True)
    add('plain', plain, error=0, policy=True)
    add('relocations-and-cross-page-target', relocated, error=0, policy=True)
    add('nonzero-bias', elf([(0, 4096, 16, 4096, 5), (4096, 8192, 256, 8192, 6)],
                          bias=0x200000, dynamic=True, stack=65536), error=0, policy=True)
    for name, offset, size, value, expected in (
            ('magic', 0, 1, 0, 1), ('machine', 18, 2, 183, 1),
            ('writable-executable', 68, 4, 7, 3), ('entry-gap', 24, 8, 4096, 5),
            ('file-larger-than-memory', 64+32, 8, 4097, 2),
            ('unterminated-dynamic', 8192+48, 8, 1, 6),
            ('rela-size', 8192+24, 8, 47, 6), ('rela-width', 8192+40, 8, 16, 6),
            ('rela-outside-image', 8192+64, 8, 0x100000, 6)):
        data = bytearray(relocated if offset >= 8192 else plain)
        put(data, offset, size, value); add(name, data, error=expected, policy=False)
    add('small-window', plain, window=4095, error=4, policy=False)
    for label, offset, value in (('relocation-to-code', 8192+64, 8),
                                 ('relocation-symbol', 8192+72, (1 << 32)+8)):
        data = bytearray(relocated); put(data, offset, 8, value)
        add(label, data, error=0, policy=False)
    data = bytearray(plain); data[4096:4099] = b'\x0f\x01\xef'
    add('key-write-in-code', data, error=0, policy=False)
    data = bytearray(relocated); data[8192+220:8192+223] = b'\x0f\x01\xef'
    add('key-pattern-in-data', data, error=0, policy=True)
    data = elf([(0, 4096, 4096, 4096, 5), (4096, 8192, 1, 4096, 5)])
    data[8190:8193] = b'\x0f\x01\xef'
    add('key-write-across-segments', data, error=0, policy=False)
    for n in range(1, 10):
        add(f'segment-count-{n}', elf([(i*4096, (i+1)*4096, 16, 4096, 5) for i in range(n)]),
            error=0 if n <= 8 else 2, policy=n <= 8)
    for template in (plain, relocated):
        cuts = set(range(0, 256)) | set(range(max(0, len(template)-160), len(template)+1))
        for n in sorted(cuts):
            add(f'truncate-{len(template)}-{n}', template[:n])
    rng = random.Random(0x845DE950)
    header_fields = [(16, 2), (18, 2), (24, 8), (32, 8), (54, 2), (56, 2)]
    header_fields += [(64+i*56+o, s) for i in range(3)
                      for o, s in ((0, 4), (4, 4), (8, 8), (16, 8), (32, 8), (40, 8))]
    for i in range(2500):
        data = bytearray(relocated)
        at, width = rng.choice(header_fields + [(8192+j*8, 8) for j in range(14)])
        value = rng.choice([0, 1, 2, 3, 8, 24, 48, 56, 64, 4095, 4096, 4097,
                            len(data)-1, len(data), len(data)+1, (1 << (8*width))-1,
                            rng.getrandbits(8*width)])
        put(data, at, width, value)
        add(f'field-{i}', data, window=rng.choice([4095, 4096, 8192, WINDOW, MAX]))
    for i in range(400):
        count = rng.randrange(1, 9)
        segments = [(j*8192, (j+1)*4096, rng.randrange(0, 65), rng.randrange(65, 4097),
                     5 if not j else rng.choice([4, 5, 6])) for j in range(count)]
        add(f'valid-layout-{i}', elf(segments, bias=rng.randrange(0, 256)*4096), error=0, policy=True)
    for image in images:
        add('image-'+image.name, image.read_bytes(), error=0, policy=True)
    return cases


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cesta', type=Path, required=True)
    parser.add_argument('--work', type=Path)
    parser.add_argument('--image', type=Path, action='append', default=[])
    parser.add_argument('--admit', action='store_true')
    parser.add_argument('--cesta-source', type=Path)
    parser.add_argument('--lean-root', type=Path)
    args = parser.parse_args()
    work = args.work.resolve() if args.work else Path(tempfile.mkdtemp(prefix='program-image-', dir=ROOT/'build'))
    work.mkdir(parents=True, exist_ok=True)
    if any(work.iterdir()):
        parser.error('--work must be empty; previous evidence is retained')
    pin = json.loads((ROOT/'interfaces/program_image.source.json').read_text())
    oracle = (ROOT/'tests/program_image/reference.rs').read_bytes()
    assert hashlib.sha256(oracle).hexdigest() == pin['sha256'], 'Rust reference drift'
    (work/'program_image.rs').write_bytes(oracle)
    (work/'oracle.rs').write_bytes((ROOT/'tests/program_image/oracle.rs').read_bytes())
    run(['rustc', '--edition=2021', '-O', work/'oracle.rs', '-o', work/'oracle'])
    cases = corpus(args.image)
    stream = b''.join(struct.pack('<QI', window, len(data))+data for _, data, window, _, _ in cases)
    (work/'cases.bin').write_bytes(stream)
    reference = run([work/'oracle'], data=stream).decode().splitlines()
    assert len(reference) == len(cases)
    counts = Counter(int(line.split()[0]) for line in reference)
    assert set(counts) == set(range(7)), counts
    for (name, _, _, expected, _), line in zip(cases, reference):
        if expected is not None:
            assert int(line.split()[0]) == expected, (name, expected, line)
    (work/'reference.txt').write_text('\n'.join(reference)+'\n')
    builds = []
    for cc in ('gcc', 'clang'):
        target = work/cc; target.mkdir()
        for name in ('program_image', 'program_image_bytes'):
            run([args.cesta, 'export-c', ROOT/f'src/{name}.cesta', '--std=c11', '--cc', cc,
                 '-o', target/f'{name}.c', '--plan', target/f'{name}.plan', '--', '-I'+str(ROOT/'generated/include')])
        for mode in ('O0', 'O2', 'sanitize'):
            extra = ['-O1', '-fsanitize=address,undefined', '-fno-sanitize-recover=all', '-fno-pie', '-no-pie'] if mode == 'sanitize' else ['-'+mode]
            flags = ['-std=c11', '-g0', '-Wall', '-Wextra', '-Werror', '-I'+str(ROOT/'generated/include'), *extra]
            binary = target/mode
            run([cc, *flags, target/'program_image.c', target/'program_image_bytes.c',
                 ROOT/'tests/program_image/host.c', '-o', binary])
            output = run([binary], data=stream).decode().splitlines()
            assert len(output) == len(cases)
            for case, expected, observed in zip(cases, reference, output):
                shape, policy = observed.split(' | ')
                assert shape == expected, (cc, mode, case[0], expected, shape)
                if case[4] is not None:
                    assert bool(int(policy)) == case[4], (cc, mode, case[0], 'kernel policy')
            if mode == 'O2':
                authored = target/'authored'
                run([args.cesta, 'cc', '--cc', cc, *flags, ROOT/'src/program_image.cesta',
                     ROOT/'src/program_image_bytes.cesta', ROOT/'tests/program_image/host.c', '-o', authored])
                assert authored.read_bytes() == binary.read_bytes(), (cc, 'readable-C identity')
            builds.append(cc+'-'+mode)
            print(cc, mode, len(cases), 'parse/relocation observations and independent policy controls passed', flush=True)
    if args.admit:
        if not args.cesta_source or not args.lean_root:
            parser.error('--admit needs --cesta-source and --lean-root')
        sys.path.insert(0, str(args.cesta_source/'tests'))
        import ownership_admission
        record, = ownership_admission.parse((work/'gcc/program_image.plan').read_bytes())
        ownership_admission.admit_record(record, work/'ImageAdmission.lean', args.lean_root)
    (work/'summary.json').write_text(json.dumps({'cases': len(cases), 'outcomes': counts,
        'builds': builds, 'oracle': pin, 'real_images': [str(p) for p in args.image],
        'source_sha256': {name: hashlib.sha256((ROOT/name).read_bytes()).hexdigest() for name in
            ('src/program_image.cesta', 'src/program_image_bytes.cesta', 'src/program_image_interface.cesta',
             'tests/program_image.py', 'tests/program_image/host.c', 'tests/program_image/oracle.rs')},
        'scope': 'pure parsing and policy; no page effects, boot execution or whole-C refinement'}, indent=2)+'\n')


if __name__ == '__main__':
    main()
