#!/usr/bin/env python3
"""Compare the real kernel VA cursor with the pinned Rust allocator."""
import argparse
import hashlib
import json
from pathlib import Path
import random
import subprocess
import tempfile

from layout_text import LayoutText

HERE = Path(__file__).resolve().parent.parent
PIN = '8019f72d617927949545bde1fb5fc6fc56acd939'
REFERENCE = 'kernel/src/memory/va_allocator.rs'
START, END, PAGE = 0x400000000000, 0x444444440000, 4096
MAX = (1 << 64) - 1

RUST_OBSERVER = r'''
pub fn observe(cursor:u64,size:u64)->(u64,u64) {
    let cursor=AtomicU64::new(cursor);
    let result=usize::try_from(size).ok().and_then(|size|allocate_from(&cursor,size));
    (result.unwrap_or(u64::MAX),cursor.load(Ordering::SeqCst))
}
'''
RUST_MAIN = r'''
#[path="va_allocator.rs"] mod implementation;
use std::io::{self,Read};
fn main(){
    let mut input=String::new();io::stdin().read_to_string(&mut input).unwrap();
    let mut words=input.split_whitespace();
    while let Some(cursor)=words.next(){
        let bytes=words.next().unwrap().parse().unwrap();
        let (result,next)=implementation::observe(cursor.parse().unwrap(),bytes);
        println!("{result} {next}");
    }
}
'''


def run(command, *, data=None):
    result = subprocess.run(list(map(str, command)), cwd=HERE, input=data, capture_output=True)
    if result.returncode:
        raise RuntimeError((command, result.returncode, result.stdout[-1000:], result.stderr[-4000:]))
    return result.stdout


def cases():
    cursors = [0, START-PAGE, START-1, START, START+1, START+PAGE,
               END-2*PAGE, END-PAGE, END-1, END, END+PAGE, MAX-PAGE+1, MAX]
    sizes = [0, 1, PAGE-1, PAGE, PAGE+1, 2*PAGE, 2*PAGE+1,
             END-START, END-START+1, MAX-PAGE+1, MAX-PAGE+2, MAX]
    result = [(c, n) for c in cursors for n in sizes]
    rng = random.Random(0x8019F72D)
    for _ in range(2048):
        cursor = START + rng.randrange((END-START)//PAGE)*PAGE
        if rng.randrange(4) == 0:
            cursor += rng.choice((-1, 1))
        count = rng.choice((rng.randrange(1, 1024*1024), END-cursor,
                            END-cursor+1, rng.getrandbits(64)))
        result.append((cursor, count))
    return result


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cesta', type=Path, required=True)
    parser.add_argument('--work', type=Path)
    parser.add_argument('--mutations', action='store_true')
    args = parser.parse_args()
    work = (args.work.resolve() if args.work else
            Path(tempfile.mkdtemp(prefix='va-reservation-', dir=HERE/'build')))
    work.mkdir(parents=True, exist_ok=True)
    reference = run(['git', 'show', PIN+':'+REFERENCE])
    (work/'va_allocator.rs').write_bytes(reference+RUST_OBSERVER.encode())
    (work/'oracle.rs').write_text(RUST_MAIN)
    run(['rustc', '--edition=2021', '-A', 'warnings', work/'oracle.rs', '-o', work/'oracle'])
    data = ''.join(f'{cursor} {size}\n' for cursor, size in cases()).encode()
    (work/'cases.txt').write_bytes(data)
    expected = run([work/'oracle'], data=data)
    (work/'expected.txt').write_bytes(expected)
    source = work/'kernel_memory.c'
    run([args.cesta, 'export-c', 'src/kernel_memory.cesta', '--std=c11', '-o', source,
         '--', '-Igenerated/include', '-Iruntime/kernel'])
    flags = ['-std=c11', '-g0', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
             '-ffreestanding', '-fno-builtin', '-ffunction-sections', '-fdata-sections',
             '-I'+str(work), '-Itests/kernel_memory', '-Igenerated/include',
             '-Iruntime/kernel', '-Wl,--gc-sections']

    def build(compiler, extra, tag):
        binary = work/tag
        run([compiler, *flags, *extra, 'tests/va_reservation/host.c', '-o', binary])
        return binary

    builds = []
    for compiler in ('gcc', 'clang'):
        for mode in ('O0', 'O2', 'sanitize'):
            extra = (['-O1', '-fsanitize=address,undefined', '-fno-sanitize-recover=all']
                     if mode == 'sanitize' else ['-'+mode])
            binary = build(compiler, extra, compiler+'-'+mode)
            actual = run([binary], data=data)
            assert actual == expected, (compiler, mode, 'VA trace differs')
            builds.append({'compiler': compiler, 'mode': mode,
                           'binary_sha256': hashlib.sha256(binary.read_bytes()).hexdigest()})
            print(compiler, mode, 'VA cursor matches pinned Rust', flush=True)

    mutations = [
        ('lower_bound', 'frames.va_next < USER_VA_BASE', 'false'),
        ('upper_bound', 'frames.va_next >= USER_VA_END', 'false'),
        ('alignment', 'frames.va_next % KERNEL_PAGE_SIZE != 0', 'false'),
        ('extent', 'rounded > USER_VA_END - frames.va_next', 'false'),
        ('rounding', 'align_up(size, KERNEL_PAGE_SIZE)', 'size'),
        ('zero_length', 'size == 0 || ', ''),
        ('rounding_overflow', 'size > UINT64_MAX - (KERNEL_PAGE_SIZE - 1)', 'false'),
        ('cursor_commit', 'frames.va_next += rounded;', '(void)rounded;'),
        ('unrelated_state', 'frames.va_next += rounded;', 'frames.va_next += rounded; ++frames.allocated;'),
    ]
    caught = []
    if args.mutations:
        original = LayoutText(source.read_text())
        for name, before, after in mutations:
            assert original.count(before) == 1, (name, 'mutation anchor')
            try:
                source.write_text(original.replace(before, after, 1))
                binary = build('gcc', ['-O2'], name)
                result = subprocess.run([binary], cwd=HERE, input=data, capture_output=True)
                assert result.returncode != 0 or result.stdout != expected, ('mutation survived', name)
                caught.append(name)
            finally:
                source.write_text(original)
    names = ['src/kernel_memory.cesta', 'tests/va_reservation.py',
             'tests/va_reservation/host.c', 'generated/include/sexos_kernel.h']
    summary = {'pin': PIN, 'reference': REFERENCE,
               'reference_sha256': hashlib.sha256(reference).hexdigest(),
               'cases_per_route': len(cases()), 'builds': builds, 'mutations_caught': caught,
               'source_sha256': {n: hashlib.sha256((HERE/n).read_bytes()).hexdigest() for n in names},
               'scope': 'serialized VA reservation and other allocator metadata preservation; no page mapping, access grant, concurrent allocator or whole-C refinement claim'}
    (work/'summary.json').write_text(json.dumps(summary, indent=2)+'\n')
    print('VA reservation qualification passed:', len(caught), 'mutations caught', flush=True)


if __name__ == '__main__':
    main()
