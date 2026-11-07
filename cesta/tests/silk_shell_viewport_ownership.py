"""Check the actual viewport receivers and admit their emitted permission plans."""
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
    assert (result.returncode==0)==accepted,(command,result.stdout[-1000:],result.stderr[-5000:])
    if not accepted:assert 'ownership' in result.stderr,result.stderr
    return result


def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('--cesta',default='cesta')
    parser.add_argument('--cesta-source',type=Path)
    parser.add_argument('--lean-root',type=Path)
    parser.add_argument('--output',type=Path,default=HERE/'build/silk-shell-viewport-ownership')
    args=parser.parse_args();work=args.output.resolve();work.mkdir(parents=True,exist_ok=True)
    source=HERE/'src/silk_shell_viewport.cesta'
    frozen={p:p.read_bytes() for p in [source,HERE/'src/silk_shell_viewport_interface.cesta',HERE/'generated/include/silk_shell_viewport.h',HERE/'generated/include/shell_app_rows.h']}
    expected=set(re.findall(r'\bshell_\w+(?=\()',frozen[HERE/'generated/include/silk_shell_viewport.h'].decode()))
    plans=[];include='-I'+str(HERE/'generated/include')
    for compiler in ('gcc','clang'):
        exported=work/(compiler+'.c');plan=work/(compiler+'.plan')
        run([args.cesta,'export-c',source,'--cc',compiler,'--std=c11','-o',exported,'--plan',plan,'--',include])
        run([compiler,'-std=c11','-Wall','-Wextra','-Wpedantic','-Werror','-fsyntax-only',include,exported])
        names=set(re.findall(r'\(function ([^\s()]+) ',plan.read_text()))
        assert expected<=names,(expected-names)
        plans.append(dict(compiler=compiler,sha256=hashlib.sha256(plan.read_bytes()).hexdigest(),plan=plan.name,functions=len(names)))
    original=frozen[source].decode();opening='static void view_delivery_queue(ShellViewportDelivery *delivery,uint8_t kind,uint32_t target,uint32_t task,uint32_t w,uint32_t h)\n{'
    assert LayoutText(original).count(opening)==1
    changes=[
        ('read_only_receiver',opening,opening+'\n const ShellViewportDelivery *read=delivery; view_delivery_retire(read);'),
        ('write_during_live_loan',opening,opening+'\n ShellViewportDelivery *held=delivery; delivery->dead=true; (void)held;'),
        ('read_during_exclusive_loan',opening,opening+'\n ShellViewportDelivery *held=delivery; const ShellViewportDelivery *second=delivery; (void)second; (void)held;'),
    ]
    for name,old,new in changes:
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
            admission.append(native.admit_record(record,work/f'Viewport{len(admission)}.lean',args.lean_root.resolve()))
    assert all(p.read_bytes()==b for p,b in frozen.items()),'viewport sources changed during qualification'
    summary=dict(plans=plans,source_mutants_refused=len(changes),admission=admission,
                 source_sha256=hashlib.sha256(frozen[source]).hexdigest())
    (work/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
    print('viewport ownership:',len(expected),'functions;',len(changes),'source mutants refused;',
          sum(item['functions'] for item in admission),'actual plans admitted')


if __name__=='__main__':main()
