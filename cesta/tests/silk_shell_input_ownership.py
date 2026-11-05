"""Check the input unit's real reference fields and emitted permission plans."""
import argparse
import hashlib
import importlib
import json
from pathlib import Path
import re
import subprocess
import sys
from layout_text import LayoutText

HERE=Path(__file__).resolve().parent.parent


def run(command, accepted=True):
    result=subprocess.run(list(map(str,command)),cwd=HERE,capture_output=True,text=True)
    assert (result.returncode==0)==accepted,(command,result.stdout[-1000:],result.stderr[-6000:])
    if not accepted:
        assert 'ownership' in result.stderr,result.stderr
    return result


def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('--cesta',default='cesta')
    parser.add_argument('--cesta-source',type=Path)
    parser.add_argument('--lean-root',type=Path)
    parser.add_argument('--output',type=Path,default=HERE/'build/silk-shell-input-ownership')
    args=parser.parse_args()
    work=args.output.resolve();work.mkdir(parents=True,exist_ok=True)
    source=HERE/'src/silk_shell_input.cesta'
    app_source=HERE/'src/silk_shell_input_apps.cesta'
    inputs=[source,app_source,HERE/'src/silk_shell_input_interface.cesta',*sorted((HERE/'generated/include').glob('*.h'))]
    frozen={path:path.read_bytes() for path in inputs}
    include='-I'+str(HERE/'generated/include')
    inventory=json.loads((HERE/'interfaces/silk_shell_input.source.json').read_text())
    expected={'shell_input_'+item['name'] for item in inventory['functions']}
    plans=[]
    for unit in (source,app_source):
        required=expected if unit==source else {'shell_input_peer_quil_has_focus','shell_input_peer_spindle_has_focus'}
        for compiler in ('gcc','clang'):
            base=unit.stem+'-'+compiler
            exported=work/(base+'.c');plan=work/(base+'.plan')
            run([args.cesta,'export-c',unit,'--cc',compiler,'--std=c11','-o',exported,'--plan',plan,'--',include])
            run([compiler,'-std=c11','-Wall','-Wextra','-Wpedantic','-Werror','-fsyntax-only',include,exported])
            names=set(re.findall(r'\(function ([^\s()]+) ',plan.read_text()))
            assert required <= names,('input functions missing from ownership plan',sorted(required-names))
            plans.append(dict(unit=unit.stem,compiler=compiler,sha256=hashlib.sha256(plan.read_bytes()).hexdigest(),plan=plan.name,functions=len(names)))
    original=LayoutText(frozen[source].decode())
    opening='void shell_input_input_trace_shell_summary(ShellInputContext *ctx)\n{'
    assert original.count(opening)==1
    changes=[
        ('read_only_input', '(field input (borrow write))', '(field input (borrow read))'),
        ('dangling_input_field', opening, opening+'\n    SilkShellInputState local={0}; ctx->shell.input=&local;'),
        ('field_under_exclusive_borrow', opening, opening+'\n    SilkShellInputState *alias=ctx->shell.input; POINTER_X=1; (void)alias;'),
        ('uninitialized_context', opening, opening+'\n    ShellInputContext blank; shell_input_flush_pending_cursor_send(&blank);'),
    ]
    for name,old,new in changes:
        assert original.count(old)==1,(name,original.count(old))
        mutant=work/(name+'.cesta');mutant.write_text(original.replace(old,new))
        for compiler in ('gcc','clang'):
            run([args.cesta,'cc','--cc',compiler,'-std=c11','-fsyntax-only',include,mutant],False)
    original_apps=LayoutText(frozen[app_source].decode())
    view='ShellSurfacesContext view={.shell=ctx->shell,.surfaces=ctx->surfaces};'
    app_changes=[
        ('uninitialized_subview',view,'ShellSurfacesContext view;'),
        ('write_beneath_subview',view,view+'\n     ctx->shell.chrome->FOCUSED_SURFACE_ID=0;'),
        ('escape_models_field','uint64_t focused=ctx->shell.chrome->FOCUSED_SURFACE_ID;','{SilkShellModelsState local={0};ctx->shell.models=&local;}\n    uint64_t focused=ctx->shell.chrome->FOCUSED_SURFACE_ID;'),
    ]
    for name,old,new in app_changes:
        assert original_apps.count(old)==2,(name,original_apps.count(old))
        mutant=work/(name+'.cesta');mutant.write_text(original_apps.replace(old,new))
        for compiler in ('gcc','clang'):
            run([args.cesta,'cc','--cc',compiler,'-std=c11','-fsyntax-only',include,mutant],False)
    admission=[]
    if args.lean_root:
        if not args.cesta_source:parser.error('--lean-root requires --cesta-source')
        sys.path.insert(0,str(args.cesta_source.resolve()/'tests'))
        native=importlib.import_module('ownership_admission')
        for digest in sorted({item['sha256'] for item in plans}):
            item=next(item for item in plans if item['sha256']==digest)
            record,=native.parse((work/item['plan']).read_bytes())
            admission.append(native.admit_record(record,work/f'Input{len(admission)}.lean',args.lean_root.resolve()))
    assert all(path.read_bytes()==data for path,data in frozen.items()),'ownership inputs changed during qualification'
    summary=dict(plans=plans,reference_functions=len(expected)+2,source_mutants_refused=len(changes)+len(app_changes),
                 admission=admission,source_sha256=hashlib.sha256(frozen[source]).hexdigest())
    (work/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
    print('input ownership:',len(expected)+2,'pinned functions checked;',len(changes)+len(app_changes),'source mutations refused;',
          sum(item['functions'] for item in admission),'actual function plans admitted')


if __name__=='__main__':main()
