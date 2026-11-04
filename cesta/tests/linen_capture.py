"""Independent canonical encoding and capture-lifecycle controls."""
import argparse
import hashlib
import importlib
import json
import os
from pathlib import Path
import random
import struct
import subprocess
import sys

HERE = Path(__file__).resolve().parent.parent
MAX = (1 << 64) - 1


def encode(rows, revision):
    if revision == 0: return None
    present = [(i, r) for i, r in enumerate(rows) if r is not None]
    ids = [r[0] for _, r in present]
    if len(set(ids)) != len(ids) or any(i in (0, MAX) for i in ids): return None
    payload = b''.join(struct.pack('<QBBB', r[0], r[1], r[2], len(r[3])) + r[3] for _, r in present)
    return struct.pack('<HHQH', 1, 14 + len(payload), revision, sum(1 << i for i, _ in present)) + payload


def canonical(data):
    if len(data) < 14 or len(data) > 574: return False
    version, length, revision, mask = struct.unpack_from('<HHQH', data)
    if version != 1 or length != len(data) or revision == 0: return False
    at = 14; ids = set()
    for i in range(16):
        if mask & (1 << i):
            if at + 11 > len(data): return False
            ident, kind, disk, count = struct.unpack_from('<QBBB', data, at); at += 11
            if ident in (0, MAX) or ident in ids or count > 24 or at + count > len(data): return False
            ids.add(ident); at += count
    return at == len(data)


def vectors():
    rng = random.Random(0x2C0); commands = []; expected = []; frames = []
    def emit(command, result=None):
        commands.append(command)
        if result is not None: expected.append(result)
    def frame_text(data): return str(len(data)) + ' ' + data.hex()
    def word(value): return f'{value:016x}'
    def error(code): return (0x80 | code) << 56
    for trial in range(160):
        rows = [None] * 16; emit('clear')
        for i in range(16):
            if trial == 0 or rng.randrange(3):
                ident = (1 << 63) + trial * 16 + i + 1
                name = bytes(rng.randrange(256) for _ in range(24 if trial == 0 else rng.randrange(25)))
                r = (ident, rng.randrange(256), rng.randrange(256), name); rows[i] = r
                emit(' '.join(map(str, ('row', i, 1, *r[:3], len(name), *name))))
        revision = rng.randrange(1, MAX + 1); data = encode(rows, revision); assert data is not None
        frames.append(data); emit(f'encode {revision}', frame_text(data))
        emit('decode ' + str(len(data)) + ' ' + ' '.join(map(str, data)), frame_text(data))
        # Every truncated prefix, trailing bytes and random bit alterations.
        altered = [data[:i] for i in range(len(data))]
        if len(data) < 574: altered.append(data + b'\x00')
        for _ in range(12):
            changed = bytearray(data); j = rng.randrange(len(changed)); changed[j] ^= 1 << rng.randrange(8); altered.append(bytes(changed))
        for candidate in altered:
            emit('decode ' + str(len(candidate)) + ' ' + ' '.join(map(str, candidate)),
                 frame_text(candidate) if canonical(candidate) else 'invalid')
        emit('reset'); emit('begin 13 13 1 1 0', word(0))
        captured = encode(rows, 1); assert captured is not None
        # Mutate the live table after capture. Repeating begin must retain it.
        emit('clear'); emit('begin 13 13 1 1 0', word(0))
        emit('begin 13 13 2 1 0', word(error(1)))
        for offset in range(0, len(captured) + 1, 7):
            part = captured[offset:offset + 7]
            result = len(part) << 56 | int.from_bytes(part, 'little')
            emit(f'read 13 13 1 {offset} 0', word(result))
        emit(f'read 13 13 1 {len(captured)} 0', word(0))
        emit(f'read 13 13 1 {MAX} 0', word(error(7)))
        emit('read 14 13 1 0 0', word(error(5)))
        emit('read 13 14 1 0 0', word(error(5)))
        emit('read 13 13 2 0 0', word(error(2)))
        emit('end 13 13 2 0 0', word(error(2)))
        emit('end 13 13 1 0 0', word(0)); emit('end 13 13 1 0 0', word(0))
        emit('begin 13 13 1 1 0', word(error(2)))
        emit('end 13 13 3 0 0', word(error(2)))
        emit('begin 13 13 2 1 0', word(0))
        emit('end 13 13 1 0 0', word(0))  # A delayed duplicate cannot end the new capture.
        emit('read 13 13 2 0 0', word(7 << 56 | int.from_bytes(encode([None] * 16, 2)[:7], 'little')))
        emit('end 13 13 2 0 0', word(0))
        emit(f'counter {MAX}'); emit('begin 13 13 3 1 0', word(error(3)))
    emit('reset'); assembled = b''
    for frame in frames[:1]:
        for offset in range(0, len(frame), 7):
            part = frame[offset:offset + 7]; assembled += part
            emit('append ' + str(len(part) << 56 | int.from_bytes(part, 'little')), '1 ' + frame_text(assembled))
    for value in [0, 1 << 56 | 1 << 8, 8 << 56, 255 << 56]:
        emit('append ' + str(value), str(int(value == 0)) + ' ' + frame_text(assembled))
    return ('\n'.join(commands) + '\n').encode(), ('\n'.join(expected) + '\n').encode(), len(expected)


def main():
    ap = argparse.ArgumentParser(); ap.add_argument('--cesta', required=True); ap.add_argument('--quick', action='store_true')
    ap.add_argument('--lean-root', type=Path); ap.add_argument('--cesta-source', type=Path)
    ap.add_argument('--output', type=Path, default=HERE / 'build/linen-capture'); args = ap.parse_args()
    w = args.output.resolve(); w.mkdir(parents=True, exist_ok=True)
    data, expected, count = vectors(); records = []; plans = {}
    source = HERE / 'src/linen_capture.cesta'; original = source.read_bytes()
    for cc in ('gcc', 'clang'):
        c = w / (cc + '.c'); plan = w / (cc + '.plan')
        include = '-I' + str(HERE / 'generated/include')
        subprocess.run([args.cesta, 'export-c', str(HERE / 'src/linen_capture.cesta'), '--cc', cc, '--std=c11', '-o', str(c), '--plan', str(plan), '--', include], check=True)
        plans[hashlib.sha256(plan.read_bytes()).hexdigest()] = plan
        modes = [('O0', ['-O0']), ('O2', ['-O2'])]
        if not args.quick: modes.append(('SAN', ['-O1', '-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-fno-pie', '-no-pie']))
        for label, flags in modes:
            binary = w / (cc + '-' + label)
            subprocess.run([cc, '-std=c11', '-Wall', '-Wextra', '-Wpedantic', '-Werror', *flags, include,
                            str(c), str(HERE / 'tests/linen_capture/host.c'), '-o', str(binary)], check=True)
            result = subprocess.run([str(binary)], input=data, capture_output=True,
                                    env={**os.environ, 'ASAN_OPTIONS': 'detect_leaks=1:abort_on_error=1'})
            assert result.returncode == 0, result.stderr.decode()[-4000:]
            if result.stdout != expected:
                actual = result.stdout.splitlines(); wanted = expected.splitlines()
                mismatch = next((i for i, (a, b) in enumerate(zip(actual, wanted)) if a != b), min(len(actual), len(wanted)))
                raise AssertionError((cc, label, mismatch, actual[mismatch:mismatch + 1], wanted[mismatch:mismatch + 1]))
            records.append(dict(compiler=cc, mode=label, controls=count, trace_sha256=hashlib.sha256(result.stdout).hexdigest(), plan_sha256=hashlib.sha256(plan.read_bytes()).hexdigest()))
    mutants = (
        ('wrong_caller', 'caller==state->owner && task==state->task', 'caller!=0 && task!=0'),
        ('recapture_same_job', 'if(job==state->job)return 0;', 'if(job==state->job){state->active=false;return linen_capture_begin(state,caller,task,job,version,reserved,table);}'),
        ('revive_retired', 'if(job<=state->retired)return linen_capture_error(LINEN_CAPTURE_STALE);', 'if(job<state->retired)return linen_capture_error(LINEN_CAPTURE_STALE);'),
        ('truncate_id', 'capture_put64(&next,at,table->rows[i].id);', 'capture_put64(&next,at,table->rows[i].id&UINT64_C(0xffffffff));'),
        ('reject_full_name', 'next.rows[i].len>LINEN_CAPTURE_NAME_CAPACITY', 'next.rows[i].len>=LINEN_CAPTURE_NAME_CAPACITY'),
        ('end_wrong_job', 'if(job!=state->job)return linen_capture_error(LINEN_CAPTURE_STALE);', 'if(job==0)return linen_capture_error(LINEN_CAPTURE_STALE);'),
        ('wrong_read_count', '(uint64_t)count<<56', '(uint64_t)count<<55'),
    )
    for name, old, new in mutants:
        text = original.decode(); assert text.count(old) == 1, (name, text.count(old))
        changed = w / (name + '.cesta'); changed.write_text(text.replace(old, new)); c = w / (name + '.c')
        subprocess.run([args.cesta, 'export-c', str(changed), '--cc', 'gcc', '--std=c11', '-o', str(c), '--', include], check=True)
        binary = w / name
        subprocess.run(['gcc', '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror', include, str(c), str(HERE / 'tests/linen_capture/host.c'), '-o', str(binary)], check=True)
        result = subprocess.run([str(binary)], input=data, capture_output=True)
        assert result.returncode != 0 or result.stdout != expected, ('surviving mutation', name)
    admission = []
    if args.lean_root:
        assert args.cesta_source
        sys.path.insert(0, str(args.cesta_source.resolve() / 'tests')); native = importlib.import_module('ownership_admission')
        for i, plan in enumerate(plans.values()):
            record, = native.parse(plan.read_bytes())
            admission.append(native.admit_record(record, w / f'Capture{i}.lean', args.lean_root.resolve()))
    assert source.read_bytes() == original
    summary = dict(runtime=records, runtime_mutants_caught=len(mutants), admission=admission,
                   source_sha256=hashlib.sha256(original).hexdigest())
    (w / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n')
    print('Linen capture:', count, 'independent controls per configuration;', len(records), 'configurations;', len(mutants), 'runtime mutations caught')


if __name__ == '__main__': main()
