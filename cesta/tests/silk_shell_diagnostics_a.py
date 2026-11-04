"""Pinned live diagnostic helpers: successful sends and independent failure controls."""
import argparse
import hashlib
import json
import itertools
import os
from pathlib import Path
import re
import subprocess

from silk_shell_input_differential import declarations, identifiers, item_end

HERE = Path(__file__).resolve().parent.parent
FIX = HERE / 'tests/silk_shell_diagnostics_a'
FLAGS = ('INPUT_NEGATIVE_PROOF', 'SURFACE_ID_LIFETIME_INPUT_SAFETY_PROOF',
         'SHELL_INTERACTION_CONTRACT_PROOF', 'CLICK_DRAG_PROOF', 'SILKBAR_PHASE2_SHELL_PROOF')
FIELDS = ('INPUT_NEGATIVE_BEGIN_EMITTED', 'INPUT_NEGATIVE_UNKNOWN_CLASS_SEEN',
          'INPUT_NEGATIVE_BAD_BUTTON_SEEN', 'INPUT_NEGATIVE_BUTTON_UP_NO_CAPTURE_SEEN',
          'INPUT_NEGATIVE_DONE_EMITTED', 'SURFACE_ID_LIFETIME_PROOF_BEGIN_EMITTED',
          'SURFACE_ID_LIFETIME_PROOF_FOCUS_LIVE_SEEN', 'SURFACE_ID_LIFETIME_PROOF_KEY_ROUTE_SEEN',
          'SURFACE_ID_LIFETIME_PROOF_CLICK_TARGET_SEEN', 'SURFACE_ID_LIFETIME_PROOF_DRAG_TARGET_SEEN',
          'SURFACE_ID_LIFETIME_PROOF_DEAD_CLEAR_SEEN', 'SHELL_INTERACTION_CONTRACT_BEGIN_EMITTED',
          'SHELL_INTERACTION_CONTRACT_DONE_EMITTED', 'CLICK_FOCUS_PROOF_BEGIN_EMITTED',
          'DRAG_PROOF_BEGIN_EMITTED')
LABELS = ('begin', 'move', 'release', 'shell', '')
MASK = (1 << 64) - 1


def run(argv, data=None, env=None):
    result = subprocess.run(list(map(str, argv)), cwd=HERE, input=data, capture_output=True, env=env)
    assert result.returncode == 0, (argv, result.returncode, result.stdout[-1000:], result.stderr[-5000:])
    return result.stdout


def prepare(work, joined=False):
    inventory = json.loads((HERE / 'interfaces/silk_shell_diagnostics_a.source.json').read_text())
    source = run(['git', 'show', inventory['pin'] + ':servers/silk-shell/src/main.rs']).decode()
    assert hashlib.sha256(source.encode()).hexdigest() == inventory['source_sha256']
    pdx = run(['git', 'show', inventory['pin'] + ':crates/sex-pdx/src/lib.rs']).decode()
    bodies = []
    for row in inventory['functions']:
        match = re.search(r'(?m)^(?:pub\s+)?(?:const\s+)?(?:unsafe\s+)?fn\s+' + row['name'] + r'\b', source)
        assert match, row['name']
        body = source[match.start():item_end(source, match.start())] + '\n'
        assert hashlib.sha256(body.encode()).hexdigest() == row['sha256'], row['name']
        bodies.append(body)
    catalog = {**declarations(pdx), **declarations(source)}
    needed = set(FIELDS) | {'SILK_TEXT_BUF', 'SILK_TEXT_LEN', 'SILK_TEXT_PROOF_DONE_EMITTED',
                            'FOCUSED_SURFACE_ID', 'SURFACE_100_ALIVE', 'SHELL_INTERACTION_CONTRACT_STAGE_MASK'}
    for body in bodies:
        needed |= identifiers(body) & catalog.keys()
    selected = {}
    while needed:
        name = needed.pop()
        if name in selected:
            continue
        assert name in catalog, name
        selected[name] = catalog[name]
        needed |= (identifiers(catalog[name]) & catalog.keys()) - selected.keys()
    prefix = '''#![allow(dead_code, unused_imports, unused_variables, unused_mut, static_mut_refs)]
extern crate self as sex_pdx;
macro_rules! serial_println { ($($arg:tt)*) => { println!($($arg)*) }; }
'''
    rust = prefix + '\n'.join(selected[name] for name in sorted(selected)) + '\n' + '\n'.join(bodies)
    rust += (FIX / 'provider.rs').read_text()
    rust = rust.replace('/* RESET_FIELDS */', ''.join(f'{name}=false;' for name in FIELDS))
    rust = rust.replace('/* SET_FIELDS */', ''.join(f'{name}=n(1)&(1<<{i})!=0;' for i, name in enumerate(FIELDS)))
    rust = rust.replace('/* OBSERVE_FIELDS */', '|'.join(f'(({name} as u64)<<{i})' for i, name in enumerate(FIELDS)))
    c = (FIX / 'provider.c').read_text()
    c = c.replace('/* SET_FIELDS */', ''.join(f'state.{name}=(n[1]&(UINT64_C(1)<<{i}))!=0;' for i, name in enumerate(FIELDS)))
    c = c.replace('/* OBSERVE_FIELDS */', '|'.join(f'((uint64_t)state.{name}<<{i})' for i, name in enumerate(FIELDS)))
    if joined:
        c = c.replace('#include "silk_shell_diagnostics_a.h"', '#include "silk_shell_input.h"')
        c = c.replace('static uint64_t status,', '''static SilkShellInputState input;
static SilkShellAtlasState atlas;
static SilkShellSurfacesState surfaces;
static SilkShellModelsState models;
static ShellChromeApps apps;
static ShellInputContext input_ctx={{&chrome,&models,&apps,&input,&state},&atlas,&surfaces};
static uint64_t status,''')
        names = set(re.findall(r'\bshell_input_peer_(\w+)\(', (HERE / 'src/silk_shell_input_diagnostics.cesta').read_text()))
        assert len(names) == 13
        for name in names:
            before = 'shell_diagnostics_' + name + '('
            if name == 'silk_text_sink_key':
                c = c.replace(before + '&ctx,', 'shell_input_peer_' + name + '(&input_ctx,')
            elif name.startswith('is_'):
                c = c.replace(before, 'shell_input_peer_' + name + '(&input_ctx,')
            else:
                c = c.replace(before + '&state', 'shell_input_peer_' + name + '(&input_ctx')
        c = c.replace('ShellTextSinkContext separate={&other,&other_chrome};',
                      'ShellInputContext separate=input_ctx;separate.shell.proofs_a=&other;separate.shell.chrome=&other_chrome;')
        c = c.replace('shell_diagnostics_silk_text_sink_key(&separate,', 'shell_input_peer_silk_text_sink_key(&separate,')
    (work / 'provider.rs').write_text(rust)
    (work / 'provider.c').write_text(c)
    return inventory


def corpus():
    rows = []
    def add(*args): rows.append(' '.join(str(x & MASK) for x in args))
    add(0)
    for scan in range(256): add(11, scan)
    for focus in (0, 100, 101):
        for live in (0, 1):
            for value in (0, 1, 2, MASK):
                add(0); add(2, focus, live)
                for scan in range(256): add(1, scan, value)
    for length in (0, 1, 7, 8, 9, 95, 96):
        add(0); add(2, 100, 1); add(3, length); add(4)
        for _ in range(100): add(1, 16, 1)
        for _ in range(100): add(1, 14, 1)
    for length in (0, 1, 7, 8, 9, 255, 256, 257, 511, 512, 513, 1024):
        for color in (0, 0xffffff, 0xffffffff, MASK):
            for sid in (0, 100, MASK): add(5, length, color, sid, length + 37)
    for kind in (0, 7, 8, 9, 10, 11, (1 << 32) - 1):
        for value in (0, 1, MASK): add(6, kind, value, value ^ MASK)
    for label in range(len(LABELS)):
        for event in (0, 1, 255, 256, MASK): add(7, label, event)
    flag_masks = {0, (1 << len(FIELDS)) - 1}
    # Exhaust each marker family's coverage state without multiplying unrelated families.
    for start, width in ((0, 5), (5, 6), (11, 4)):
        flag_masks.update(bits << start for bits in range(1 << width))
    for flags in sorted(flag_masks):
        add(0); add(10, flags)
        for op in (0, 1, 2, 8, 9, 10, 12): add(9, op)
    for live in (0, 1, 2, 255):
        for ok in (0, 1, 2, 255):
            for routed in (0, 1, 255):
                add(0)
                for op in (3, 4, 5, 6): add(9, op, MASK, live, routed, ok, 2)
                add(9, 7, MASK); add(9, 8)
    for mask in range(256):
        add(0); add(9, 11, mask); add(9, 12); add(9, 12)
    for stages in itertools.permutations((1, 2, 4, 8)):
        add(0)
        for stage in stages:
            add(9, 11, stage); add(9, 12)
        add(9, 12)
    return ('\n'.join(rows) + '\n').encode(), len(rows)


def observe(binary, commands):
    return run([binary], ('\n'.join(commands) + '\n').encode()).decode()


def independent(binary, reference, enabled):
    count = 0
    differences = 0
    def check(commands, expected, *, differs=False, no_calls=False):
        nonlocal count, differences
        out = observe(binary, commands)
        results = [line for line in out.splitlines() if line.startswith('R ')]
        assert results[-1] == expected, (commands, results, out[-1000:])
        if no_calls:
            assert not any(line.startswith('call ') for line in out.splitlines()), out
        if differs:
            old = observe(reference, commands)
            assert old != out, ('source counterexample disappeared', commands)
            differences += 1
        count += 1
        return out
    for status in (1, 2, MASK):
        commands = [f'8 {status} 0', '6 8 17 29']
        check(commands, 'R 0', differs=enabled, no_calls=not enabled)
        check([f'8 {status} 0', '5 0 0 100 1'], 'R 0 0', no_calls=True)
        check([f'8 {status} 0', '5 17 0 100 1'], 'R 17 0', differs=True)
        check(['0', f'8 {status} 0', '4'], 'R 0', differs=True)
        check(['0', '3 9', f'8 {status} 1', '4'], 'R 0', differs=True)
        out = check(['0', '2 100 1', f'8 {status} 0', '1 16 1'], 'R 1', differs=True)
        assert '[silk.text.input.proof.done]' not in out
        assert any(line.startswith('S 1 0 ') for line in out.splitlines()), out
    for at in (0, 1, 31, 63):
        out = check([f'8 2 {at}', '5 512 16777215 100 13'], 'R 512 0', differs=True)
        assert len([line for line in out.splitlines() if line.startswith('call ')]) == 64
    check(['0', '2 100 1', '3 97', '1 16 1'], 'R 0', no_calls=True)
    check(['0', '3 97', '4'], 'R 0', no_calls=True)
    # Empty redraw is a successful clear only when that send was admitted.
    check(['0', '4'], 'R 1')
    check(['0', '2 100 1', '1 16 0'], 'R 1', no_calls=True)
    check(['0', '3 7', '12'], 'R 1 7 1 0 1')
    return count, differences


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cesta', default='cesta')
    parser.add_argument('--output', type=Path, default=HERE / 'build/shell-diagnostics-tests')
    parser.add_argument('--mutations', action='store_true')
    parser.add_argument('--joined', action='store_true')
    args = parser.parse_args()
    work = args.output.resolve(); work.mkdir(parents=True, exist_ok=True)
    inventory = prepare(work, args.joined)
    exports = []
    include = '-I' + str(HERE / 'generated/include')
    units = ['silk_shell_diagnostics_a', 'silk_shell_diagnostics_a_native']
    if args.joined:
        units.append('silk_shell_input_diagnostics')
    for name in units:
        out = work / (name + '.c')
        run([args.cesta, 'export-c', HERE / 'src' / (name + '.cesta'), '--std=c11', '-o', out, '--', include])
        exports.append(out)
    data, count = corpus()
    (work / 'commands.txt').write_bytes(data)
    profiles = [(), *((flag,) for flag in FLAGS), FLAGS]
    configurations = []; controls = 0; differences = 0
    for pi, profile in enumerate(profiles):
        env = {key: value for key, value in os.environ.items() if not key.startswith('SEXOS_')}
        env.update({'SEXOS_' + flag: '1' for flag in profile})
        reference = work / f'reference-{pi}'
        run(['rustc', '--edition=2021', '-C', 'opt-level=1', work / 'provider.rs', '-o', reference], env=env)
        expected = run([reference], data)
        (work / f'expected-{pi}.txt').write_bytes(expected)
        defines = ['-DSEXOS_' + flag + '=1' for flag in profile]
        for cc in ('gcc', 'clang'):
            for mode, options in (('O0', ['-O0']), ('O2', ['-O2']),
                                  ('san', ['-O1', '-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-fno-pie', '-no-pie'])):
                binary = work / f'{cc}-{mode}-{pi}'
                run([cc, '-std=c11', '-Wall', '-Wextra', '-Wpedantic', '-Werror', include, *defines,
                     *options, *exports, work / 'provider.c', '-o', binary])
                actual = run([binary], data)
                if actual != expected:
                    left = expected.splitlines(); right = actual.splitlines()
                    line = next((i for i, pair in enumerate(zip(left, right)) if pair[0] != pair[1]), min(len(left), len(right)))
                    raise AssertionError((cc, mode, profile, line, left[max(0, line-2):line+2], right[max(0, line-2):line+2]))
                controls, differences = independent(binary, reference, 'SILKBAR_PHASE2_SHELL_PROOF' in profile)
                configurations.append(dict(compiler=cc, mode=mode, profile=list(profile), sha256=hashlib.sha256(actual).hexdigest()))
        print('diagnostics profile', pi, ':', count, 'commands, six configurations', flush=True)
    mutants = []
    if args.mutations:
        body = exports[0].read_text()
        for name, old, new in (
            ('spindle_missing_punctuation', 'sc==0x0c ||', 'false ||'),
            ('quil_accepts_tab', 'sc==0x1c || sc==0x0e || sc==1 || sc==0x39', 'sc==0x0f || sc==0x1c || sc==0x0e || sc==1 || sc==0x39'),
            ('draw_wrong_offset', '(offset&255)', '((offset+1)&255)'),
            ('draw_loses_high_offset', '((uint64_t)(offset>>8)&15)', 'UINT64_C(0)'),
            ('draw_ignores_rejection', 'if(reply.status)', 'if(false)'),
            ('bar_ignores_rejection', 'bool admitted=!result.status;', 'bool admitted=true;'),
            ('sink_ignores_clear', 'result.admitted && !clear.status', 'result.admitted'),
            ('sink_skips_focus', 'ctx->chrome->FOCUSED_SURFACE_ID!=SILK_TEXT_SINK_SID', 'false'),
            ('sink_repeats_release', 'if(value!=1)return true;', 'if(false)return true;'),
            ('sink_never_backspaces', 'ctx->proofs_a->SILK_TEXT_LEN=len-1;', 'ctx->proofs_a->SILK_TEXT_LEN=len;'),
            ('marker_repeats_begin', '|| state->INPUT_NEGATIVE_BEGIN_EMITTED', '|| false'),
            ('contract_loses_stages', 'SHELL_INTERACTION_CONTRACT_STAGE_MASK|=stage', 'SHELL_INTERACTION_CONTRACT_STAGE_MASK=stage'),
        ):
            assert old in body, (name, old)
            mutant = work / (name + '.c'); mutant.write_text(body.replace(old, new))
            binary = work / (name + '.bin')
            run(['gcc', '-std=c11', '-O2', include, *['-DSEXOS_' + flag + '=1' for flag in FLAGS],
                 mutant, *exports[1:], work / 'provider.c', '-o', binary])
            killed = run([binary], data) != expected
            if not killed:
                try: independent(binary, reference, True)
                except AssertionError: killed = True
            assert killed, (name, 'survived')
            mutants.append(name)
    summary = dict(pin=inventory['pin'], functions=len(inventory['functions']),
                   remaining=len(inventory['remaining']), commands=count, configurations=configurations,
                   actual_input_diagnostic_peers=13 if args.joined else 0,
                   independent_controls=controls, source_counterexamples=differences, mutants=mutants,
                   boundary='Register-call admission is scripted. Successful admission does not establish application or visible rendering. The original diagnostic markers are observations, not logical proofs.')
    (work / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n')
    print('diagnostics:', len(inventory['functions']), 'entries;', count, 'commands/configuration;',
          len(mutants), 'mutations caught;', controls, 'independent controls')


if __name__ == '__main__':
    main()
