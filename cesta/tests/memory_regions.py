#!/usr/bin/env python3
"""Immutable region policy against pinned Rust bodies and the kernel ABI encoder."""
import argparse
import hashlib
import json
from pathlib import Path
import random
import subprocess
import sys
import tempfile
from layout_text import LayoutText

HERE = Path(__file__).resolve().parent.parent
INVENTORY = HERE/'interfaces/memory_regions.source.json'
MASK = (1 << 64) - 1


def run(argv, data=None):
    p = subprocess.run(list(map(str, argv)), cwd=HERE, input=data, capture_output=True)
    if p.returncode:
        raise RuntimeError((argv, p.returncode, p.stdout[-2000:], p.stderr[-5000:]))
    return p.stdout


def cases(seed, count):
    base = [1, 7, 0x400000, 8192, 7, 0, 0x123000, 1, 15, 1, 0, 64, 3, 15]
    rows = []

    def add(g):
        for op in (0, 1, 2, 3, 4, 54, 55, 999):
            v = g[:]; v[9] = op; rows.append(v)

    add(base)
    grant_values = {
        0: [0, 1], 1: [0, 1, (1 << 32)-1],
        2: [0, 1, 4096, (1 << 47)-8192, (1 << 47)-4096, 1 << 47, MASK-4095, MASK],
        3: [0, 1, 4095, 4096, 8191, 8192, 1 << 47, MASK],
        4: list(range(32)) + [127, 255], 5: [0, 1],
        6: [0, 1, 4096, (1 << 52)-8192, (1 << 52)-4096, 1 << 52, MASK],
        7: [0, 1], 8: [0, 1, 15, (1 << 32)-1],
    }
    for index, values in grant_values.items():
        for value in values:
            g = base[:]; g[index] = value; add(g)
    for dma, grant_rights in ((0, 7), (1, 15), (0, 3), (0, 2), (0, 1)):
        for offset in (0, 1, 4095, 4096, 8191, 8192, 8193, MASK-1, MASK):
            for length in (0, 1, 2, 64, 4096, 8192, MASK):
                for rights in (*range(16), 16, 127, 255, 256, 259, 1 << 32, MASK):
                    g = base[:]; g[4] = grant_rights; g[5] = dma
                    g[10:13] = [offset, length, rights]; add(g)
    for caller in (0, 1, 14, 15, 16, (1 << 32)-1):
        g = base[:]; g[13] = caller; add(g)
    rng = random.Random(seed)
    for _ in range(count):
        g = base[:]
        for _ in range(rng.randrange(1, 5)):
            i = rng.choice(list(grant_values) + [10, 11, 12, 13])
            g[i] = rng.choice(grant_values[i]) if i in grant_values else rng.getrandbits(64)
        add(g)
    return rows


def prepare(work):
    inventory = json.loads(INVENTORY.read_text())
    for item in inventory['sources']:
        raw = (HERE.parent/item['path']).read_bytes()
        assert hashlib.sha256(raw).hexdigest() == item['sha256'], ('reference changed', item['path'])
    raw = (HERE.parent/'crates/sex-pdx/src/memory_region.rs').read_bytes()
    (work/'actual_memory_region.rs').write_bytes(raw + b'\npub fn valid_probe(g:&RegionGrant)->bool {valid_grant(g)}\n')
    (work/'actual_capability_slot.rs').write_bytes((HERE.parent/'kernel/src/capability_slot.rs').read_bytes())
    source = (HERE.parent/'kernel/src/syscalls/mod.rs').read_text()
    start = source.index('pub(crate) fn region_query(')
    opening = source.index('{', start)
    depth = 1; end = opening + 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}'); end += 1
    (work/'actual_region_query.rs').write_text(source[start:end]+'\n')
    (work/'oracle.rs').write_bytes((HERE/'tests/memory_regions/oracle.rs').read_bytes())
    return inventory


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--cesta', required=True, type=Path)
    ap.add_argument('--work', type=Path)
    ap.add_argument('--seed', type=int, default=20261031)
    ap.add_argument('--cases', type=int, default=1200)
    ap.add_argument('--mutations', action='store_true')
    ap.add_argument('--cesta-source', type=Path)
    ap.add_argument('--lean-root', type=Path)
    args = ap.parse_args()
    work = args.work or Path(tempfile.mkdtemp(prefix='memory-regions-', dir=HERE/'build'))
    work.mkdir(parents=True, exist_ok=True)
    inventory = prepare(work)
    run(['rustc', '--edition=2021', '--cap-lints', 'allow', '-O', work/'oracle.rs', '-o', work/'rust'])
    vectors = cases(args.seed, args.cases)
    data = ''.join(' '.join(map(str, v))+'\n' for v in vectors).encode()
    (work/'commands.txt').write_bytes(data)
    expected = run([work/'rust'], data)
    (work/'rust.out').write_bytes(expected)
    source = work/'memory_regions.c'
    plan = work/'memory_regions.plan'
    run([args.cesta, 'export-c', 'src/memory_regions.cesta', '--std=c11', '--plan', plan,
         '-o', source, '--', '-Igenerated/include'])
    flags = ['-std=c11', '-g0', '-Wall', '-Wextra', '-Wpedantic', '-Werror', '-Igenerated/include']

    def compare(binary, actual):
        if actual != expected:
            (work/(binary.name+'.out')).write_bytes(actual)
            a, b = actual.splitlines(), expected.splitlines()
            i = next((i for i, pair in enumerate(zip(a, b)) if pair[0] != pair[1]), min(len(a), len(b)))
            raise AssertionError((binary.name, i, vectors[i], a[i:i+1], b[i:i+1]))

    builds = []
    for cc in ('gcc', 'clang'):
        for mode in ('O0', 'O2', 'sanitize'):
            extra = ['-O1', '-fsanitize=address,undefined', '-fno-sanitize-recover=all'] if mode == 'sanitize' else ['-'+mode]
            binary = work/(cc+'-'+mode)
            run([cc, *flags, *extra, HERE/'tests/memory_regions/host.c', source, '-o', binary])
            compare(binary, run([binary], data))
            if mode == 'O2':
                authored = work/(cc+'-authored')
                run([args.cesta, 'cc', '--cc', cc, *flags, *extra,
                     HERE/'tests/memory_regions/host.c', HERE/'src/memory_regions.cesta', '-o', authored])
                assert binary.read_bytes() == authored.read_bytes(), (cc, 'plain-C identity')
                compare(authored, run([authored], data))
            builds.append({'compiler': cc, 'mode': mode, 'cases': len(vectors)})
        print(cc, len(vectors), 'region-policy cases match actual Rust bodies', flush=True)

    mutations = [
        ('register_rights_alias', 'if (rights > UINT8_MAX)', 'if (false)'),
        ('grant_alignment', 'g->length % REGION_PAGE_BYTES', 'false'),
        ('range_wrapping', 'q.offset > c->grant.length || q.length > c->grant.length - q.offset', 'q.offset + q.length > c->grant.length'),
        ('rights_escalation', 'if (q.rights & ~c->grant.rights)', 'if (false)'),
        ('physical_disclosure', 'dma ? c->grant.dma_base+q.offset : 0', 'c->grant.dma_base+q.offset'),
        ('wrong_peer', 'c->grant.peer_pd != caller', 'c->grant.peer_pd == caller'),
        ('grant_virtual_end', 'g->length > REGION_USER_END - g->virtual_base', 'false'),
        ('failed_reply_residue', '.status=UINT64_MAX', '.status=UINT64_MAX, .value0=length'),
    ]
    rejected = []
    if args.mutations:
        original = LayoutText(source.read_text())
        for name, before, after in mutations:
            assert original.count(before) == 1, (name, 'mutation anchor')
            try:
                source.write_text(original.replace(before, after))
                binary = work/name
                # Mutants must compile; a front-end error does not count as detection.
                run(['gcc', *flags, '-O2', HERE/'tests/memory_regions/host.c', source, '-o', binary])
                actual = run([binary], data)
                assert actual != expected, ('mutation survived', name)
                rejected.append(name)
            finally:
                source.write_text(original)
    admission = None
    if args.lean_root:
        if not args.cesta_source: ap.error('--lean-root requires --cesta-source')
        sys.path.insert(0, str(args.cesta_source.resolve()/'tests'))
        import ownership_admission
        record, = ownership_admission.parse(plan.read_bytes())
        admission = ownership_admission.admit_record(record, work/'RegionOwnership.lean', args.lean_root.resolve())
    summary = {'pin': inventory['source_pin'], 'seed': args.seed, 'builds': builds,
               'mutations_rejected': rejected, 'admission': admission,
               'source_sha256': {n: hashlib.sha256((HERE/n).read_bytes()).hexdigest() for n in
                                 ('src/memory_regions.cesta', 'src/memory_regions_interface.cesta',
                                  'tests/memory_regions.py', 'tests/memory_regions/host.c',
                                  'tests/memory_regions/oracle.rs', 'interfaces/memory_regions.source.json')},
               'scope': 'actual pure Rust region functions and kernel reply encoder; checked Cesta resolver; no mapping, DMA transfer or hardware isolation claim'}
    (work/'summary.json').write_text(json.dumps(summary, indent=2)+'\n')
    print('region-policy qualification passed;', len(rejected), 'semantic controls caught', flush=True)


if __name__ == '__main__':
    main()
