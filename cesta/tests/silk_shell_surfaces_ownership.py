"""Check the Surfaces unit's nested reference fields and emitted permission plans."""
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
    parser.add_argument('--parts-only',action='store_true')
    parser.add_argument('--output',type=Path,default=HERE/'build/silk-shell-surfaces-ownership')
    args=parser.parse_args()
    work=args.output.resolve();work.mkdir(parents=True,exist_ok=True)
    source=HERE/'src/silk_shell_surfaces.cesta'
    chromes=[source,HERE/'src/silk_shell_surfaces_interface.cesta',*sorted((HERE/'generated/include').glob('*.h'))]
    frozen={path:path.read_bytes() for path in chromes}
    include='-I'+str(HERE/'generated/include')
    inventory=json.loads((HERE/'interfaces/silk_shell_surfaces.source.json').read_text())
    expected={'shell_surfaces_'+item['name'] for item in inventory['functions']}
    plans=[]
    for compiler in ('gcc','clang'):
        exported=work/(compiler+'.c');plan=work/(compiler+'.plan')
        run([args.cesta,'export-c',source,'--cc',compiler,'--std=c11','-o',exported,'--plan',plan,'--',include])
        run([compiler,'-std=c11','-Wall','-Wextra','-Wpedantic','-Werror','-fsyntax-only',include,exported])
        names=set(re.findall(r'\(function ([^\s()]+) ',plan.read_text()))
        assert expected <= names,('Surfaces functions missing from ownership plan',sorted(expected-names))
        plans.append(dict(compiler=compiler,sha256=hashlib.sha256(plan.read_bytes()).hexdigest(),plan=plan.name,functions=len(names)))
    original=frozen[source].decode()
    opening='static bool surface_ensure(ShellSurfacesContext *ctx,uint8_t family,uint32_t *out)\n{'
    assert LayoutText(original).count(opening)==1
    changes=[
        ('read_only_frames','(field chrome (borrow write))','(field chrome (borrow read))'),
        ('read_only_surfaces','(field surfaces (borrow write))','(field surfaces (borrow read))'),
        ('uninitialized_nested_context',opening,opening+'\n    ShellSurfacesContext blank; shell_surfaces_ensure_linen_frame(&blank,out);'),
        ('live_frame_during_chrome_call',opening,opening+'\n    ShellFrame *held=&ctx->shell.chrome->FRAMES[0]; shell_chrome_send_frame_tab_info(&ctx->shell,2); (void)held->frame_id;'),
        ('escaped_surface_state',opening,opening+'\n    SilkShellSurfacesState temporary={0}; ctx->surfaces=&temporary;'),
    ]
    dispatch='void shell_surfaces_spindle_dispatch(ShellSurfacesContext *ctx)\n{'
    changes.append(('yarn_borrow_across_dispatch',dispatch,dispatch+'\n    const YarnSession *held=&ctx->surfaces->YARN; shell_surfaces_yarn_cmd_clear(ctx); (void)held->cmd_len;'))
    for name,old,new in changes:
        assert LayoutText(original).count(old)==1,(name,LayoutText(original).count(old))
        mutant=work/(name+'.cesta');mutant.write_text(LayoutText(original).replace(old,new))
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
            admission.append(native.admit_record(record,work/f'Surfaces{len(admission)}.lean',args.lean_root.resolve(),assemble=not args.parts_only))
    assert all(path.read_bytes()==data for path,data in frozen.items()),'ownership chromes changed during qualification'
    summary=dict(plans=plans,reference_functions=len(expected),source_mutants_refused=len(changes),
                 admission=admission,module_assembly_requested=not args.parts_only,
                 source_sha256=hashlib.sha256(frozen[source]).hexdigest())
    (work/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
    print('surfaces ownership:',len(expected),'current entry functions checked;',len(changes),'source mutations refused;',
          sum(item['functions'] for item in admission),'actual function plans admitted;',
          'module assembly deferred'if args.parts_only else 'module assembly required')


if __name__=='__main__':main()
