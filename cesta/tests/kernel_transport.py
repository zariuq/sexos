#!/usr/bin/env python3
"""Current PDX against an independent accepted-invocation history model.

CPU entry, concurrent scheduling, retirement and paging are separate boundaries.
The current-source profile does not make the historical kernel a current image.
"""
import argparse
from collections import deque
import hashlib
import json
from pathlib import Path
import random
import subprocess
import tempfile
from layout_text import LayoutText

HERE = Path(__file__).resolve().parent.parent
MASK = (1 << 64) - 1
INVALID, BUSY = MASK - 3, MASK - 1


def run(argv, **kwargs):
    result = subprocess.run(list(map(str, argv)), cwd=HERE, capture_output=True,
                            text=True, **kwargs)
    if result.returncode:
        raise RuntimeError((argv, result.returncode, result.stdout[-3000:], result.stderr[-3000:]))
    return result.stdout


def history_trace(commands):
    """Accepted invocations remain obligations even after their messages are read."""
    inbox = {a: deque() for a in range(1, 4)}
    reply = {a: None for a in range(1, 4)}
    obligations = []  # (callee, caller), removed only by a successful reply.
    result = []
    for op, a, selector, value in commands:
        observed = [0] * 6
        if op == 'C':
            target = selector - 5
            if target not in inbox:
                observed[0] = INVALID
            elif len(inbox[target]) == 256:
                observed[0] = BUSY
            else:
                obligations.append((target, a))
                inbox[target].append([0x600, a, a * 101, value,
                                      value ^ 0xabcdef, value ^ MASK])
        elif op == 'R':
            debt = (a, selector)
            if debt not in obligations or selector not in reply or reply[selector] is not None:
                observed[0] = 1
            else:
                reply[selector] = [1, a, a * 101, value, 0, 0]
                obligations.remove(debt)
        elif selector == 0 and reply[a] is not None:
            observed = reply[a]
            reply[a] = None
        elif selector in (0, 2) and inbox[a]:
            observed = inbox[a].popleft()
        for d in range(1, 4):
            pending = reply[d]
            observed += ([1, pending[3], pending[1], pending[2]] if pending else [0, 0, 0, 0])
            observed.append(len(inbox[d]))
            observed += [obligations.count((d, caller)) for caller in range(1, 4)]
        result.append(observed)
    return result


def commands(seed, count):
    rng = random.Random(seed)
    stream = []
    # Exercise full rings, failed reservations, self traffic and delayed replies.
    stream += [('C', 1, 7, i) for i in range(260)]
    stream += [('R', 2, 1, i) for i in range(4)]
    stream += [('L', 1, 0, 0), ('R', 2, 1, 88), ('L', 1, 0, 0)]
    for _ in range(count):
        op = rng.choices('CRL', [4, 3, 6])[0]
        a = rng.randrange(1, 4)
        if op == 'C':
            selector = rng.choice([6, 7, 8, 6, 7, 8, 4, (1 << 32) | 7])
        elif op == 'R':
            selector = rng.choice([1, 2, 3, 1, 2, 3, 0, (1 << 32) | 2])
        else:
            selector = rng.choice([0, 0, 0, 2, 2, 7, (1 << 32) | 2])
        stream.append((op, a, selector, rng.getrandbits(64)))
    # Observe every remaining queued payload, including earlier priority replies.
    for a in range(1, 4):
        stream += [('L', a, 0, 0)] * 258
    return stream


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cesta', required=True, type=Path)
    parser.add_argument('--work', type=Path)
    parser.add_argument('--seed', type=int, default=20261031)
    parser.add_argument('--cases', type=int, default=12000)
    parser.add_argument('--mutations', action='store_true')
    args = parser.parse_args()
    work = args.work or Path(tempfile.mkdtemp(prefix='kernel-transport-', dir=HERE/'build'))
    work.mkdir(parents=True, exist_ok=True)
    inputs = json.loads((HERE/'interfaces/kernel_transport.source.json').read_text())
    for entry in inputs['sources']:
        actual = hashlib.sha256((HERE.parent/entry['path']).read_bytes()).hexdigest()
        assert actual == entry['sha256'], ('transport reference changed', entry['path'])
    units = ('kernel_domains', 'kernel_syscalls', 'kernel_rpc', 'memory_regions', 'kernel_task_wait', 'kernel_sources', 'source_policy','kernel_pci','pci_policy','network_config','kernel_history','history_recovery')
    (work/'sexos_kernel.h').write_text(run([args.cesta, 'export-c', 'src/kernel_interface.cesta', '--',
                                          '-Iruntime/kernel', '-Igenerated/include']))
    for unit in units:
        (work/(unit+'.c')).write_text(run([args.cesta, 'export-c', 'src/'+unit+'.cesta', '--std=c11', '--',
                                          '-I'+str(work), '-Iruntime/kernel', '-Igenerated/include']))
    stream = commands(args.seed, args.cases)
    expected = history_trace(stream)
    text = ''.join(f'{op} {a} {target} {value}\n' for op, a, target, value in stream)
    (work/'commands.txt').write_text(text)
    flags = ['-std=c11', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
             '-DSEXOS_KERNEL_CURRENT_PDX=1', '-ffunction-sections', '-fdata-sections',
             '-Wl,--gc-sections', '-I'+str(work), '-Igenerated/include', '-Iruntime/kernel']

    def compile_run(compiler, mode, name):
        extra = ['-O1', '-fsanitize=address,undefined', '-fno-sanitize-recover=all'] if mode == 'sanitize' else [mode]
        binary = work/name
        run([compiler, *flags, *extra, 'tests/kernel_transport/host.c', work/'memory_regions.c', work/'source_policy.c',work/'pci_policy.c',work/'network_config.c',work/'history_recovery.c', '-o', binary])
        actual = [json.loads(line) for line in run([binary], input=text).splitlines()]
        assert len(actual) == len(expected), (name, 'trace length')
        for i, (got, wanted) in enumerate(zip(actual, expected)):
            assert got == wanted, (name, i, stream[i], got, wanted)
        return hashlib.sha256(binary.read_bytes()).hexdigest()

    builds = {}
    for compiler in ('gcc', 'clang'):
        for mode in ('-O0', '-O2', 'sanitize'):
            name = compiler+'-'+mode.lstrip('-')
            builds[name] = compile_run(compiler, mode, name)
            print(name, len(stream), 'history steps and directed controls passed', flush=True)

    mutants = [
        ('publish_without_reply_right', 'kernel_syscalls', 'SEXOS_KERNEL_CURRENT_PDX && target->reply_rights[d->id] == UINT64_MAX', 'false'),

        ('mismatched_domain_identity', 'kernel_domains', 'd->live && d->id == id', 'd->live'),
        ('overwrite_full_reply', 'kernel_syscalls', 'task == 0 || (target->reply_ready || (target->task_state == TASK_EXITED || target->task_state == TASK_DORMANT))', 'task == 0 || (target->task_state == TASK_EXITED || target->task_state == TASK_DORMANT)'),
        ('grant_again_at_hearing', 'kernel_syscalls',
         'if (!SEXOS_KERNEL_CURRENT_PDX && (out->from_call', 'if (1 && (out->from_call'),
        ('forget_reservation_rollback', 'kernel_syscalls',
         'target->reply_rights[d->id] -= 1;', '(void)0;'),
        ('invent_task_from_domain', 'kernel_domains', 'return d->task_id;', 'return d->id;'),
        ('ring_epoch_abort_publishes', 'kernel_domains',
         'c->ring->entries[c->tail] = c->entry;\n    c->swapped = true;',
         'c->ring->entries[c->tail] = c->entry;\n    c->ring->count = c->count + 1u;\n    c->swapped = true;'),
        ('ring_epoch_stale_swap', 'kernel_domains',
         'if (!c->transformed || !kernel_ring_push_current(c)) return false;',
         'if (!c->transformed) return false;'),
        ('ring_epoch_revision_not_committed', 'kernel_domains',
         'c->ring->count = c->count + 1u;\n    c->ring->revision = c->revision + 1u;',
         'c->ring->count = c->count + 1u;\n    c->ring->revision = c->revision;'),
        ('ring_pop_epoch_abort_publishes', 'kernel_domains',
         'static bool kernel_ring_swap(KernelRingContext *c) {\n    if (!kernel_ring_is_copied(c)) return false;\n    c->swapped = true;',
         'static bool kernel_ring_swap(KernelRingContext *c) {\n    if (!kernel_ring_is_copied(c)) return false;\n    c->ring->count = c->count - 1u;\n    c->swapped = true;'),
        ('ring_pop_epoch_stale_swap', 'kernel_domains',
         'if (!kernel_ring_is_copied(c)) return false;',
         'if (!c->copied) return false;'),
        ('ring_pop_epoch_revision_not_committed', 'kernel_domains',
         'c->ring->head = (c->head + 1u) % KERNEL_RING_CAPACITY;\n    c->ring->count = c->count - 1u;\n    c->ring->revision = c->revision + 1u;',
         'c->ring->head = (c->head + 1u) % KERNEL_RING_CAPACITY;\n    c->ring->count = c->count - 1u;\n    c->ring->revision = c->revision;'),
        ('input_push_epoch_abort_publishes', 'kernel_domains',
         'c->state->input[c->tail] = c->scancode;\n    c->swapped = true;',
         'c->state->input[c->tail] = c->scancode;\n    c->state->input_count = c->count + 1u;\n    c->swapped = true;'),
        ('input_push_epoch_stale_swap', 'kernel_domains',
         'if (!kernel_input_push_is_transformed(c)) return false;',
         'if (!c->transformed) return false;'),
        ('input_push_epoch_revision_not_committed', 'kernel_domains',
         'c->state->input_count = c->count + 1u;\n    c->state->input_revision = c->revision + 1u;',
         'c->state->input_count = c->count + 1u;\n    c->state->input_revision = c->revision;'),
        ('input_pop_epoch_abort_publishes', 'kernel_domains',
         'static bool kernel_input_pop_swap(KernelInputPopContext *c) {\n    if (!kernel_input_pop_is_transformed(c)) return false;\n    c->swapped = true;',
         'static bool kernel_input_pop_swap(KernelInputPopContext *c) {\n    if (!kernel_input_pop_is_transformed(c)) return false;\n    c->state->input_count = c->count - 1u;\n    c->swapped = true;'),
        ('input_pop_epoch_stale_swap', 'kernel_domains',
         'if (!kernel_input_pop_is_transformed(c)) return false;',
         'if (!c->transformed) return false;'),
        ('input_pop_epoch_revision_not_committed', 'kernel_domains',
         'c->state->input_head = (c->head + 1u) % KERNEL_INPUT_CAPACITY;\n    c->state->input_count = c->count - 1u;\n    c->state->input_revision = c->revision + 1u;',
         'c->state->input_head = (c->head + 1u) % KERNEL_INPUT_CAPACITY;\n    c->state->input_count = c->count - 1u;\n    c->state->input_revision = c->revision;'),
        ('epoch_unsealed_allowed', 'kernel_domains',
         'return kernel.genesis_sealed && kernel.epoch==epoch',
         'return true && kernel.epoch==epoch'),
        ('epoch_not_advanced', 'kernel_domains',
         'kernel.epoch=next;',
         'kernel.epoch=expected;'),
        ('epoch_event_count_not_advanced', 'kernel_domains',
         'kernel.epoch_event_count=slot+1u;',
         'kernel.epoch_event_count=slot;'),
        ('epoch_event_subject_lost', 'kernel_domains',
         'event.epoch=next;',
         'event.epoch=next;event.subject=0;'),
        ('epoch_ring_payload_lost', 'kernel_domains',
         'if(message)event.message=*message;',
         '(void)message;'),
        ('epoch_input_payload_lost', 'kernel_domains',
         '.kind=kind,.scancode=scancode,.revision=revision',
         '.kind=kind,.scancode=(uint8_t)(scancode+1u),.revision=revision'),
        ('stale_empty_sender', 'kernel_syscalls',
         'f->rdi = m.caller_task;', 'f->rdi = m.caller_task ? m.caller_task : 777;'),
        ('interrupt_receipt_not_heard', 'kernel_syscalls',
         'kernel.vector_heard_revision[vector] = revision;',
         'kernel.vector_heard_revision[vector] += 0;'),
        ('lose_full_slot_obligation', 'kernel_syscalls',
         'if (SEXOS_KERNEL_CURRENT_PDX && (task == 0 || (target->reply_ready || (target->task_state == TASK_EXITED || target->task_state == TASK_DORMANT)))) {',
         'if (SEXOS_KERNEL_CURRENT_PDX && (task == 0 || (target->reply_ready || (target->task_state == TASK_EXITED || target->task_state == TASK_DORMANT)))) { d->reply_rights[target_id] -= 1;'),
    ]
    refused = []
    if args.mutations:
        for name, unit, before, after in mutants:
            source = work/(unit+'.c')
            original = LayoutText(source.read_text())
            assert original.count(before) == 1, (name, 'mutation anchor')
            try:
                source.write_text(original.replace(before, after))
                try:
                    compile_run('gcc', '-O2', name)
                except (AssertionError, RuntimeError) as exc:
                    # Compile failures are not semantic rejection evidence.
                    if isinstance(exc, RuntimeError) and isinstance(exc.args[0][0], list):
                        assert str(exc.args[0][0][0]) == str(work/name), (name, 'mutant failed to compile')
                    refused.append(name)
                else:
                    raise AssertionError(('mutation survived', name))
            finally:
                source.write_text(original)
    source_names = ['src/'+u+'.cesta' for u in units] + [
        'src/kernel_interface.cesta', 'tests/kernel_transport/host.c',
        'tests/kernel_transport.py', 'interfaces/kernel_transport.source.json']
    summary = {'source_pin': inputs['source_pin'], 'seed': args.seed,
               'history_steps_per_build': len(stream), 'builds': builds,
               'mutations_rejected': refused,
               'source_sha256': {n: hashlib.sha256((HERE/n).read_bytes()).hexdigest() for n in source_names},
               'scope': 'actual compiled current-profile kernel transport under host providers; independent history observations; no CPU entry, task scheduling, retirement, paging or whole-C refinement proof'}
    (work/'summary.json').write_text(json.dumps(summary, indent=2)+'\n')
    print('current PDX transport qualified:', len(builds), 'builds;', len(refused), 'semantic controls')


if __name__ == '__main__':
    main()
