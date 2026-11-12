#!/usr/bin/env python3
"""Differential and mutation controls for bounded physical-frame leases."""
import argparse
from dataclasses import dataclass, replace
import hashlib
import json
from pathlib import Path
import random
import subprocess
import sys
import tempfile
from layout_text import LayoutText

HERE = Path(__file__).resolve().parent.parent
PAGE = 4096
CAPACITY = 17
OK, INVALID, RETRY, DENIED, FULL = range(5)
EMPTY, LIVE, DETACHED, FREE = range(4)
PINNED, PROGRAM, TRANSIENT = range(3)
MASK = (1 << 64) - 1


def run(argv, data=None):
    result = subprocess.run(list(map(str, argv)), cwd=HERE, input=data, capture_output=True)
    if result.returncode:
        raise RuntimeError((argv, result.returncode, result.stdout[-2000:], result.stderr[-6000:]))
    return result.stdout


@dataclass
class Entry:
    base: int = 0
    pages: int = 0
    order: int = 0
    phase: int = EMPTY
    kind: int = PINNED
    holder: int = 0
    incarnation: int = 0


@dataclass
class Lease:
    base: int = 0
    order: int = 0
    kind: int = PINNED
    holder: int = 0
    incarnation: int = 0


class Model:
    def __init__(self, reserve=0):
        self.entries = []
        self.free_pages = 0
        self.lent = 0
        self.reserve = reserve
        self.next_incarnation = 1

    def init(self, reserve):
        self.__init__(reserve)

    def coalesce(self):
        merged = []
        for entry in sorted((e for e in self.entries if e.phase == FREE),
                            key=lambda e: e.base):
            if merged and merged[-1].base + merged[-1].pages * PAGE == entry.base:
                merged[-1].pages += entry.pages
            else:
                merged.append(entry)
        others = [e for e in self.entries if e.phase != FREE]
        self.entries = merged + others

    def region(self, base, pages):
        if not base or base % PAGE or not pages or pages > (MASK - base) // PAGE:
            return INVALID
        for entry in self.entries:
            if entry.phase != FREE:
                return DENIED
            if base < entry.base + entry.pages * PAGE and entry.base < base + pages * PAGE:
                return DENIED
        if len(self.entries) == CAPACITY:
            return FULL
        self.entries.append(Entry(base, pages, phase=FREE))
        self.free_pages += pages
        self.coalesce()
        return OK

    def take(self, order, kind, holder):
        if order > 18 or kind > TRANSIENT or (kind == PINNED) != (holder == 0):
            return INVALID, Lease()
        if not self.next_incarnation:
            return FULL, Lease()
        pages = 1 << order
        size = pages * PAGE
        if kind == PROGRAM and (self.free_pages < pages or
                                self.free_pages - pages < self.reserve):
            return FULL, Lease()
        choices = []
        for index, entry in enumerate(self.entries):
            if entry.phase != FREE or entry.pages < pages or entry.base > MASK - (size - 1):
                continue
            base = (entry.base + size - 1) & ~(size - 1)
            prefix = (base - entry.base) // PAGE
            if prefix > entry.pages - pages:
                continue
            suffix = entry.pages - prefix - pages
            choices.append((base, index, prefix, suffix))
        if not choices:
            return FULL, Lease()
        base, index, prefix, suffix = min(choices)
        extras = bool(prefix) + bool(suffix)
        if len(self.entries) + extras > CAPACITY:
            return FULL, Lease()
        original = self.entries.pop(index)
        if prefix:
            self.entries.append(Entry(original.base, prefix, phase=FREE))
        lease = Lease(base, order, kind, holder, self.next_incarnation)
        self.entries.append(Entry(base, pages, order, LIVE, kind, holder, self.next_incarnation))
        if suffix:
            self.entries.append(Entry(base + size, suffix, phase=FREE))
        self.free_pages -= pages
        if kind == PROGRAM:
            self.lent += pages
        self.next_incarnation = (self.next_incarnation + 1) & 0xffffffff
        return OK, lease

    def detach(self, lease):
        if (lease.kind not in (PROGRAM, TRANSIENT) or not lease.holder or not lease.incarnation or
                not lease.base or lease.base % PAGE or lease.order > 18):
            return INVALID
        for entry in self.entries:
            if (entry.phase == LIVE and entry.base == lease.base and
                    entry.order == lease.order and entry.kind == lease.kind and
                    entry.holder == lease.holder and entry.incarnation == lease.incarnation):
                entry.phase = DETACHED
                return OK
        return DENIED

    def give(self, lease):
        if (lease.kind not in (PROGRAM, TRANSIENT) or not lease.holder or not lease.incarnation or
                not lease.base or lease.base % PAGE or lease.order > 18):
            return INVALID
        for entry in self.entries:
            if (entry.base == lease.base and entry.order == lease.order and
                    entry.kind == lease.kind and entry.holder == lease.holder and
                    entry.incarnation == lease.incarnation):
                if entry.phase == LIVE:
                    return RETRY
                if entry.phase != DETACHED:
                    return DENIED
                self.entries.remove(entry)
                self.entries.append(Entry(entry.base, entry.pages, phase=FREE))
                self.free_pages += entry.pages
                if entry.kind == PROGRAM:
                    self.lent -= entry.pages
                self.coalesce()
                return OK
        return DENIED

    def commit(self, lease):
        if (lease.kind != TRANSIENT or not lease.holder or not lease.incarnation or
                not lease.base or lease.base % PAGE or lease.order > 18):
            return INVALID
        for entry in self.entries:
            if (entry.phase == LIVE and entry.base == lease.base and
                    entry.order == lease.order and entry.kind == lease.kind and
                    entry.holder == lease.holder and entry.incarnation == lease.incarnation):
                entry.kind = PINNED
                entry.holder = 0
                return OK
        return DENIED

    def available(self):
        return max(0, self.free_pages - self.reserve)

    def audit(self):
        free = sum(e.pages for e in self.entries if e.phase == FREE)
        lent = sum(e.pages for e in self.entries if e.phase in (LIVE, DETACHED) and e.kind == PROGRAM)
        if free != self.free_pages or lent != self.lent or len(self.entries) > CAPACITY:
            return False
        for i, a in enumerate(self.entries):
            if (not a.base or a.base % PAGE or not a.pages or
                    a.pages > (MASK - a.base) // PAGE or a.kind > TRANSIENT or
                    a.phase not in (FREE, LIVE, DETACHED)):
                return False
            if a.phase == FREE and (a.holder or a.incarnation or a.kind != PINNED):
                return False
            if a.phase != FREE:
                if (a.order > 18 or a.pages != 1 << a.order or not a.incarnation or
                        (a.kind == PINNED) != (a.holder == 0)):
                    return False
            for b in self.entries[i + 1:]:
                if a.base < b.base + b.pages * PAGE and b.base < a.base + a.pages * PAGE:
                    return False
                if a.phase != FREE and b.phase != FREE and a.incarnation == b.incarnation:
                    return False
                if a.phase == b.phase == FREE and (
                        a.base + a.pages * PAGE == b.base or b.base + b.pages * PAGE == a.base):
                    return False
        return True

    def snapshot(self):
        entries = tuple(sorted((e.base, e.pages, e.order, e.phase, e.kind, e.holder, e.incarnation)
                               for e in self.entries))
        return (int(self.audit()), len(self.entries), self.free_pages, self.lent,
                self.reserve, self.next_incarnation, entries)


def apply(model, command):
    fields = command.split()
    op = fields[0]
    values = list(map(int, fields[1:]))
    lease = Lease()
    if op == 'I':
        model.init(values[0])
        status = OK
    elif op == 'R':
        status = model.region(*values)
    elif op == 'T':
        status, lease = model.take(*values)
    elif op == 'D':
        status = model.detach(Lease(*values))
    elif op == 'G':
        status = model.give(Lease(*values))
    elif op == 'C':
        status = model.commit(Lease(*values))
    elif op == 'X':
        model.next_incarnation = values[0]
        status = OK
    elif op == 'V':
        status = model.available()
    else:
        raise AssertionError(command)
    lease_values = ((lease.base, lease.order, lease.kind, lease.holder, lease.incarnation)
                    if op == 'T' else ())
    return op, status, lease_values, model.snapshot()


def actual(binary, commands):
    data = ('\n'.join(commands) + '\n').encode()
    lines = run([binary], data).decode().splitlines()
    at = 0
    blocks = []
    for command in commands:
        op = command[0]
        header = lines[at].split()
        at += 1
        assert header[0] == op, (command, header)
        status = int(header[1])
        lease = tuple(map(int, header[2:])) if op == 'T' else ()
        audit = lines[at].split()
        at += 1
        assert audit[0] == 'A' and len(audit) == 7, audit
        count = int(audit[2])
        entries = []
        for _ in range(count):
            entry = tuple(map(int, lines[at].split()[1:]))
            at += 1
            entries.append(entry)
        snapshot = (int(audit[1]), count, *map(int, audit[3:]), tuple(sorted(entries)))
        blocks.append((op, status, lease, snapshot))
    assert at == len(lines), (at, len(lines), lines[at:at + 3])
    return blocks


def deterministic():
    scenarios = []
    lifecycle = ['I 2', f'R {PAGE} 12', f'R {20 * PAGE} 16',
                 'T 0 1 11', 'T 1 1 22', 'T 0 0 0',
                 'G 4096 0 1 11 1', 'D 4096 0 1 12 1',
                 'D 4096 0 1 11 1', 'G 4096 0 1 11 1',
                 'D 4096 0 1 11 1', 'G 4096 0 1 11 1',
                 'T 0 1 33', 'G 4096 0 1 11 1',
                 'D 4096 0 1 33 4', 'G 4096 0 1 33 4',
                 'D 8192 1 1 22 2', 'G 8192 1 1 22 2', 'V']
    scenarios.append(('lifecycle', lifecycle))
    scenarios.append(('two-sided-coalesce', [
        'I 0', f'R {PAGE} 16', 'T 0 1 1', 'T 0 1 2', 'T 0 1 3',
        'D 8192 0 1 2 2', 'G 8192 0 1 2 2',
        'D 4096 0 1 1 1', 'G 4096 0 1 1 1',
        'D 12288 0 1 3 3', 'G 12288 0 1 3 3']))
    scenarios.append(('reserve', [
        'I 2', f'R {PAGE} 6', 'T 0 1 1', 'T 0 1 2', 'T 0 1 3',
        'T 0 1 4', 'T 0 0 0', 'T 0 1 4', 'V']))
    scenarios.append(('capacity', ['I 0'] +
                      [f'R {(2 * i + 1) * PAGE} 1' for i in range(CAPACITY)] +
                      [f'R {100 * PAGE} 1', 'T 0 1 9', f'R {100 * PAGE} 1']))
    scenarios.append(('incarnation-exhaustion', [
        'I 0', f'R {PAGE} 4', 'X 4294967295', 'T 0 1 9', 'T 0 1 10',
        'D 4096 0 1 9 4294967295', 'G 4096 0 1 9 4294967295', 'T 0 1 10']))
    scenarios.append(('invalid', [
        'I 0', 'R 0 1', f'R {PAGE + 1} 1', f'R {PAGE} 0', 'T 19 1 1',
        'T 0 2 1', 'T 0 0 1', 'T 0 1 0', 'D 0 0 1 1 1', 'G 0 0 1 1 1',
        'D 4096 0 0 0 1', 'G 4096 0 0 0 1']))
    scenarios.append(('transient-commit', [
        'I 2', f'R {PAGE} 6', 'T 0 2 9', 'C 4096 0 2 9 1',
        'C 4096 0 2 9 1', 'G 4096 0 2 9 1', 'D 4096 0 2 9 1',
        'T 0 2 0', 'T 0 2 10', 'D 8192 0 2 10 2', 'G 8192 0 2 10 2',
        'T 0 1 11', 'C 12288 0 1 11 3', 'V']))
    return scenarios


def mutate_lease(lease, rng):
    changed = replace(lease)
    field = rng.choice(('base', 'order', 'kind', 'holder', 'incarnation', 'none'))
    if field == 'base':
        changed.base = max(PAGE, changed.base + rng.choice((-PAGE, PAGE, 1)))
    elif field == 'order':
        changed.order = (changed.order + 1) % 20
    elif field == 'kind':
        changed.kind = PINNED if changed.kind == PROGRAM else PROGRAM
    elif field == 'holder':
        changed.holder += 1
    elif field == 'incarnation':
        changed.incarnation += 1
    return changed


def randomized(seed, count):
    rng = random.Random(seed)
    commands = ['I 2', f'R {PAGE} 32', f'R {40 * PAGE} 32']
    model = Model()
    history = []
    for command in commands:
        apply(model, command)
    for _ in range(count):
        choice = rng.random()
        if choice < 0.46:
            order = 19 if rng.random() < 0.03 else rng.randrange(4)
            kind = rng.choices((PROGRAM, TRANSIENT, PINNED, 3), weights=(75, 10, 10, 5))[0]
            holder = rng.randrange(4) if kind in (PROGRAM, TRANSIENT) else int(rng.random() < 0.2)
            command = f'T {order} {kind} {holder}'
            result = apply(model, command)
            if result[1] == OK:
                history.append(Lease(*result[2]))
        elif choice < 0.90 and history:
            lease = rng.choice(history)
            if rng.random() < 0.38:
                lease = mutate_lease(lease, rng)
            op = 'D' if choice < 0.68 else 'G'
            command = f'{op} {lease.base} {lease.order} {lease.kind} {lease.holder} {lease.incarnation}'
            apply(model, command)
        elif choice < 0.94:
            command = 'V'
            apply(model, command)
        else:
            base = rng.choice((0, 1, PAGE, 40 * PAGE, 100 * PAGE))
            pages = rng.choice((0, 1, 3, (MASK // PAGE) + 1))
            command = f'R {base} {pages}'
            apply(model, command)
        commands.append(command)
        assert model.audit(), (seed, command)
    return commands


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cesta', type=Path, required=True)
    parser.add_argument('--work', type=Path)
    parser.add_argument('--mutations', action='store_true')
    parser.add_argument('--cesta-source', type=Path)
    parser.add_argument('--lean-root', type=Path)
    parser.add_argument('--seed', type=int, default=20261110)
    parser.add_argument('--cases', type=int, default=180)
    args = parser.parse_args()
    work = args.work or Path(tempfile.mkdtemp(prefix='frame-lease-', dir=HERE / 'build'))
    work.mkdir(parents=True, exist_ok=True)
    (work / 'frame_lease.h').write_bytes(run([
        args.cesta, 'export-c', 'src/frame_lease_interface.cesta', '--']))
    source = work / 'frame_lease.c'
    plan = work / 'frame_lease.plan'
    run([args.cesta, 'export-c', 'src/frame_lease.cesta', '--std=c11', '--plan', plan,
         '-o', source, '--', f'-DFRAME_LEASE_CAPACITY={CAPACITY}', '-I', work])
    scenarios = deterministic()
    for offset in range(4):
        scenarios.append((f'random-{offset}', randomized(args.seed + offset, args.cases)))
    flags = ['-std=c11', '-g0', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
             f'-DFRAME_LEASE_CAPACITY={CAPACITY}', '-I', work]

    def build(cc, extra, name):
        output = work / name
        run([cc, *flags, *extra, 'tests/frame_lease/host.c', source, '-o', output])
        return output

    def check(binary):
        for name, commands in scenarios:
            expected = []
            model = Model()
            for command in commands:
                expected.append(apply(model, command))
            observed = actual(binary, commands)
            if observed != expected:
                for index, (want, got) in enumerate(zip(expected, observed)):
                    if want != got:
                        raise AssertionError((binary.name, name, index, commands[index], want, got))
                raise AssertionError((binary.name, name, 'length', len(expected), len(observed)))

    builds = []
    for cc in ('gcc', 'clang'):
        for mode in ('O0', 'O2', 'sanitize'):
            extra = (['-O1', '-fsanitize=address,undefined', '-fno-sanitize-recover=all']
                     if mode == 'sanitize' else ['-' + mode])
            binary = build(cc, extra, cc + '-' + mode)
            check(binary)
            if mode == 'O2':
                authored = work / (cc + '-authored')
                run([args.cesta, 'cc', '--cc', cc, *flags, *extra,
                     'tests/frame_lease/host.c', 'src/frame_lease.cesta', '-o', authored])
                assert authored.read_bytes() == binary.read_bytes(), (cc, 'plain-C identity')
                check(authored)
            builds.append({'compiler': cc, 'mode': mode})
        print(cc, 'frame-lease differential cases passed', flush=True)

    mutations = [
        ('still_mapped_return', 'if (entry.phase == FRAME_LEASE_LIVE) return FRAME_LEASE_RETRY;',
         'if (false) return FRAME_LEASE_RETRY;'),
        ('detached_guard', 'if (entry.phase != FRAME_LEASE_DETACHED) return FRAME_LEASE_DENIED;',
         'if (entry.phase == FRAME_LEASE_DETACHED) return FRAME_LEASE_DENIED;'),
        ('reserve_bypass', 'state->free_pages - pages < state->reserve', 'false'),
        ('incarnation_wrap', 'if (state->next_incarnation == UINT32_MAX)\n        state->next_incarnation = 0;',
         'if (state->next_incarnation == UINT32_MAX)\n        state->next_incarnation = 1;'),
        ('coalesce_extent', 'state->entries[i].pages += state->entries[j].pages;',
         'state->entries[i].pages += 0;'),
        ('wrong_extent', 'uint64_t pages = UINT64_C(1) << order;',
         'uint64_t pages = (UINT64_C(1) << order) + 1;'),
        ('commit_class', 'entry->kind = FRAME_LEASE_PINNED;',
         'entry->kind = FRAME_LEASE_TRANSIENT;'),
        ('transient_lent', 'if (entry.kind == FRAME_LEASE_PROGRAM) state->lent -= entry.pages;',
         'state->lent -= entry.pages;'),
    ]
    caught = []
    if args.mutations:
        original = LayoutText(source.read_text())
        for name, before, after in mutations:
            assert original.count(before) >= 1, (name, 'anchor')
            try:
                source.write_text(original.replace(before, after, 1))
                binary = build('gcc', ['-O2'], name)
                try:
                    check(binary)
                except (AssertionError, RuntimeError):
                    caught.append(name)
                else:
                    raise AssertionError(('mutation survived', name))
            finally:
                source.write_text(original)
    admission = None
    if args.lean_root:
        if not args.cesta_source:
            parser.error('--lean-root requires --cesta-source')
        sys.path.insert(0, str(args.cesta_source.resolve() / 'tests'))
        import ownership_admission
        record, = ownership_admission.parse(plan.read_bytes())
        admission = ownership_admission.admit_record(
            record, work / 'FrameLeaseOwnership.lean', args.lean_root.resolve())
    inventory = json.loads((HERE / 'interfaces/frame_lease.source.json').read_text())
    names = ['src/frame_lease.cesta', 'src/frame_lease_interface.cesta',
             'tests/frame_lease.py', 'tests/frame_lease/host.c',
             'interfaces/frame_lease.source.json']
    summary = {
        'pin': inventory['pin'],
        'builds': builds,
        'cases_per_build': sum(len(commands) for _, commands in scenarios),
        'mutations_caught': caught,
        'admission': admission,
        'source_sha256': {name: hashlib.sha256((HERE / name).read_bytes()).hexdigest()
                          for name in names},
        'scope': ('bounded frame metadata transitions, independent sorted-interval model, '
                  'GCC/Clang/sanitizers and plain-C identity; no mapping detachment, '
                  'RAM zeroing, lifecycle integration or boot-reuse claim')
    }
    (work / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n')
    print('frame lease qualification passed:', sum(len(c) for _, c in scenarios),
          'cases per route,', len(caught), 'mutations caught', flush=True)


if __name__ == '__main__':
    main()
