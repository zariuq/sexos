"""Admit the real diagnostic/state footprints and reject unsafe source changes."""
import argparse
import hashlib
import importlib
import json
from pathlib import Path
import re
import subprocess
import sys

from silk_shell_diagnostics_a import FLAGS, HERE
from layout_text import LayoutText


def run(argv, accepted=True):
    result = subprocess.run(list(map(str, argv)), cwd=HERE, capture_output=True, text=True)
    assert (result.returncode == 0) == accepted, (argv, result.stdout[-1000:], result.stderr[-5000:])
    if not accepted:
        assert 'ownership' in result.stderr, result.stderr


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cesta', default='cesta')
    parser.add_argument('--cesta-source', type=Path)
    parser.add_argument('--lean-root', type=Path)
    parser.add_argument('--output', type=Path, default=HERE / 'build/shell-diagnostics-ownership')
    args = parser.parse_args()
    work = args.output.resolve(); work.mkdir(parents=True, exist_ok=True)
    include = '-I' + str(HERE / 'generated/include')
    inventory = json.loads((HERE / 'interfaces/silk_shell_diagnostics_a.source.json').read_text())
    expected = {'shell_diagnostics_' + row['name'] for row in inventory['functions']}
    files = [*sorted((HERE / 'src').glob('silk_shell_diagnostics_a*.cesta')),
             HERE / 'src/silk_shell_input_diagnostics.cesta',
             *sorted((HERE / 'generated/include').glob('*.h'))]
    frozen = {path: path.read_bytes() for path in files}
    plans = []
    for unit in ('silk_shell_diagnostics_a', 'silk_shell_diagnostics_a_native', 'silk_shell_input_diagnostics'):
        for profile in ('default', 'all'):
            defines = [] if profile == 'default' else ['-DSEXOS_' + flag + '=1' for flag in FLAGS]
            for cc in ('gcc', 'clang'):
                base = f'{unit}-{profile}-{cc}'
                out, plan = work / (base + '.c'), work / (base + '.plan')
                run([args.cesta, 'export-c', HERE / 'src' / (unit + '.cesta'), '--cc', cc,
                     '--std=c11', '-o', out, '--plan', plan, '--', include, *defines])
                run([cc, '-std=c11', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
                     '-fsyntax-only', include, *defines, out])
                names = set(re.findall(r'\(function ([^\s()]+) ', plan.read_text()))
                required = expected if unit == 'silk_shell_diagnostics_a' else {'shell_diagnostics_pdx_call'}
                if unit == 'silk_shell_input_diagnostics':
                    required = set(re.findall(r'\b(shell_input_peer_\w+)\(', frozen[HERE / 'src' / (unit + '.cesta')].decode()))
                    assert len(required) == 13
                assert required <= names, sorted(required - names)
                plans.append(dict(unit=unit, profile=profile, compiler=cc, functions=len(names),
                                  sha256=hashlib.sha256(plan.read_bytes()).hexdigest(), plan=plan.name))
    source = HERE / 'src/silk_shell_diagnostics_a.cesta'
    original = frozen[source].decode()
    opening = 'bool shell_diagnostics_silk_text_sink_key(ShellTextSinkContext *ctx,uint8_t scan,uint64_t value) {'
    assert LayoutText(original).count(opening) == 1
    changes = (
        ('read_only_sink', '(field proofs_a (borrow write))', '(field proofs_a (borrow read))'),
        ('uninitialized_context', opening, opening + '\\n    ShellTextSinkContext blank; shell_diagnostics_silk_text_sink_redraw(&blank);'),
        ('escaped_sink', opening, opening + '\\n    SilkShellProofsAState local={0}; ctx->proofs_a=&local;'),
        ('mutate_shared_chrome', opening, opening + '\\n    ctx->chrome->FOCUSED_SURFACE_ID=0;'),
        ('live_buffer_across_mutation', opening, opening + '\\n    uint8_t *held=&ctx->proofs_a->SILK_TEXT_BUF[0]; shell_diagnostics_silk_text_sink_redraw(ctx); (void)*held;'),
        ('dangling_label', 'if(label->len>=sizeof(label->bytes)', 'char local[128]={0}; return local; if(label->len>=sizeof(label->bytes)'),
    )
    for name, old, new in changes:
        assert LayoutText(original).count(old) == 1, (name, LayoutText(original).count(old))
        mutant = work / (name + '.cesta'); mutant.write_text(LayoutText(original).replace(old, new))
        for cc in ('gcc', 'clang'):
            run([args.cesta, 'cc', '--cc', cc, '-std=c11', '-fsyntax-only', include, mutant], False)
    bridge = frozen[HERE / 'src/silk_shell_input_diagnostics.cesta'].decode()
    anchor = '    return shell_diagnostics_silk_text_sink_key(&sink,scancode,value);'
    assert LayoutText(bridge).count(anchor) == 1
    bridge_changes = (
        ('write_under_sink_view', anchor, '    ctx->shell.chrome->FOCUSED_SURFACE_ID=0;\n' + anchor),
        ('escape_sink_view', anchor, '    {SilkShellProofsAState local={0};sink.proofs_a=&local;}\n' + anchor),
        ('live_state_during_marker',
         '{shell_diagnostics_input_negative_begin_once(ctx->shell.proofs_a);}',
         '{SilkShellProofsAState *held=ctx->shell.proofs_a;shell_diagnostics_input_negative_begin_once(ctx->shell.proofs_a);(void)held->INPUT_NEGATIVE_BEGIN_EMITTED;}'),
    )
    for name, old, new in bridge_changes:
        assert LayoutText(bridge).count(old) == 1, (name, LayoutText(bridge).count(old))
        mutant = work / (name + '.cesta'); mutant.write_text(LayoutText(bridge).replace(old, new))
        for cc in ('gcc', 'clang'):
            run([args.cesta, 'cc', '--cc', cc, '-std=c11', '-fsyntax-only', include, mutant], False)
    admission = []
    if args.lean_root:
        assert args.cesta_source, '--lean-root requires --cesta-source'
        sys.path.insert(0, str(args.cesta_source.resolve() / 'tests'))
        native = importlib.import_module('ownership_admission')
        for digest in sorted({row['sha256'] for row in plans}):
            row = next(row for row in plans if row['sha256'] == digest)
            record, = native.parse((work / row['plan']).read_bytes())
            admission.append(native.admit_record(record, work / f'Diagnostics{len(admission)}.lean', args.lean_root.resolve()))
    assert all(path.read_bytes() == data for path, data in frozen.items()), 'source changed during qualification'
    summary = dict(plans=plans, reference_functions=len(expected), source_mutants_refused=len(changes)+len(bridge_changes), admission=admission)
    (work / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n')
    print('diagnostics ownership:', len(expected), 'entry functions;', len(changes)+len(bridge_changes), 'source mutations refused;',
          sum(row['functions'] for row in admission), 'actual plans admitted')


if __name__ == '__main__':
    main()
