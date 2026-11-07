"""Admit the current finite-width SilkBar dispatch adapter's actual plans."""
import argparse
import hashlib
import importlib
import json
from pathlib import Path
import subprocess
import sys
from layout_text import LayoutText

HERE=Path(__file__).resolve().parent.parent


def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('--cesta',default='cesta')
    parser.add_argument('--cesta-source',type=Path)
    parser.add_argument('--lean-root',type=Path)
    parser.add_argument('--output',type=Path,default=HERE/'build/silkbar-geometry-ownership')
    args=parser.parse_args();work=args.output.resolve();work.mkdir(parents=True,exist_ok=True)
    source=HERE/'src/silkbar_geometry.cesta';original=source.read_bytes()
    include='-I'+str(HERE/'generated/include');plans=[]
    def run(command,ok=True):
        result=subprocess.run(list(map(str,command)),cwd=HERE,capture_output=True,text=True)
        assert (result.returncode==0)==ok,(command,result.stderr[-4000:])
        if not ok:assert 'ownership'in result.stderr,result.stderr
    for cc in ('gcc','clang'):
        plan=work/(cc+'.plan');out=work/(cc+'.c')
        run([args.cesta,'export-c',source,'--cc',cc,'--std=c11','-o',out,'--plan',plan,'--',include])
        run([cc,'-std=c11','-Wall','-Wextra','-Wpedantic','-Werror','-fsyntax-only',include,out])
        plans.append(dict(compiler=cc,plan=plan.name,sha256=hashlib.sha256(plan.read_bytes()).hexdigest()))
        mutant=work/'read-only.cesta'
        old='silkbar_geometry_fit(size_t width,SilkBar *out)'
        assert LayoutText(original.decode()).count(old)==1
        mutant.write_text(LayoutText(original.decode()).replace(old,'silkbar_geometry_fit(size_t width,const SilkBar *out)'))
        run([args.cesta,'cc','--cc',cc,'-std=c11','-fsyntax-only',include,mutant],False)
    admission=[]
    if args.lean_root:
        if not args.cesta_source:parser.error('--lean-root requires --cesta-source')
        sys.path.insert(0,str(args.cesta_source.resolve()/'tests'))
        native=importlib.import_module('ownership_admission')
        for digest in sorted({p['sha256']for p in plans}):
            row=next(p for p in plans if p['sha256']==digest)
            record,=native.parse((work/row['plan']).read_bytes())
            admission.append(native.admit_record(record,work/f'Geometry{len(admission)}.lean',args.lean_root.resolve()))
    assert source.read_bytes()==original
    (work/'summary.json').write_text(json.dumps(dict(plans=plans,admission=admission,source_mutants_refused=1),indent=2)+'\n')
    print('SilkBar geometry: GCC/Clang plans checked; read-only output refused;',sum(a['functions']for a in admission),'actual plans admitted')


if __name__=='__main__':main()
