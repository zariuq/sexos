"""Check the atlas unit's real reference fields and emitted permission plans."""
import argparse
import hashlib
import importlib
import json
from pathlib import Path
import re
import subprocess
import sys

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
    parser.add_argument('--output',type=Path,default=HERE/'build/silk-shell-atlas-ownership')
    args=parser.parse_args()
    work=args.output.resolve();work.mkdir(parents=True,exist_ok=True)
    source=HERE/'src/silk_shell_atlas.cesta'
    atlass=[source,HERE/'src/silk_shell_atlas_interface.cesta',*sorted((HERE/'generated/include').glob('*.h'))]
    frozen={path:path.read_bytes() for path in atlass}
    include='-I'+str(HERE/'generated/include')
    inventory=json.loads((HERE/'interfaces/silk_shell_atlas.source.json').read_text())
    expected={'shell_atlas_'+item['name'] for item in inventory['functions']}
    plans=[]
    for compiler in ('gcc','clang'):
        exported=work/(compiler+'.c');plan=work/(compiler+'.plan')
        run([args.cesta,'export-c',source,'--cc',compiler,'--std=c11','-o',exported,'--plan',plan,'--',include])
        run([compiler,'-std=c11','-Wall','-Wextra','-Wpedantic','-Werror','-fsyntax-only',include,exported])
        names=set(re.findall(r'\(function ([^\s()]+) ',plan.read_text()))
        assert expected <= names,('atlas functions missing from ownership plan',sorted(expected-names))
        plans.append(dict(compiler=compiler,sha256=hashlib.sha256(plan.read_bytes()).hexdigest(),plan=plan.name,functions=len(names)))
    original=frozen[source].decode()
    opening='void shell_atlas_collect_atlas_snapshot(ShellAtlasContext *ctx)\n{'
    assert original.count(opening)==1
    changes=[
        ('read_only_chrome', '(field chrome (borrow write))', '(field chrome (borrow read))'),
        ('dangling_atlas_field', opening, opening+'\n    SilkShellChromeState local={0}; ctx->chrome=&local;'),
        ('field_under_exclusive_borrow', opening, opening+'\n    SilkShellChromeState *alias=ctx->chrome; ACTIVE_SCENE_IDX=1; (void)alias;'),
        ('uninitialized_context', opening, opening+'\n    ShellAtlasContext blank; shell_atlas_scene_init_all(&blank);'),
    ]
    for name,old,new in changes:
        assert original.count(old)==1,(name,original.count(old))
        mutant=work/(name+'.cesta');mutant.write_text(original.replace(old,new))
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
            admission.append(native.admit_record(record,work/f'Atlas{len(admission)}.lean',args.lean_root.resolve()))
    assert all(path.read_bytes()==data for path,data in frozen.items()),'ownership atlass changed during qualification'
    summary=dict(plans=plans,reference_functions=len(expected),source_mutants_refused=len(changes),
                 admission=admission,source_sha256=hashlib.sha256(frozen[source]).hexdigest())
    (work/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
    print('atlas ownership:',len(expected),'current entry functions checked;',len(changes),'source mutations refused;',
          sum(item['functions'] for item in admission),'actual function plans admitted')


if __name__=='__main__':main()
