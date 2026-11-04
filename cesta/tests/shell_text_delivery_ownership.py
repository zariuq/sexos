"""Admit the actual display outbox and fixed-BOOT admission adapter plans."""
import argparse
import hashlib
import importlib
import json
from pathlib import Path
import subprocess
import sys

HERE=Path(__file__).resolve().parent.parent


def run(command,accepted=True):
    p=subprocess.run(list(map(str,command)),cwd=HERE,capture_output=True,text=True)
    assert (p.returncode==0)==accepted,(command,p.returncode,p.stderr[-5000:])
    if not accepted:assert 'ownership'in p.stderr,p.stderr
    return p


def main():
    parser=argparse.ArgumentParser();parser.add_argument('--cesta',default='cesta');parser.add_argument('--cesta-source',type=Path,required=True);parser.add_argument('--lean-root',type=Path,required=True);parser.add_argument('--output',type=Path,default=HERE/'build/shell-text-ownership');args=parser.parse_args()
    work=args.output.resolve();work.mkdir(parents=True,exist_ok=True)
    sys.path.insert(0,str(args.cesta_source.resolve()/'tests'));native=importlib.import_module('ownership_admission')
    include='-I'+str(HERE/'generated/include');rows=[]
    for name in ('shell_text_delivery','shell_text_delivery_display'):
        source=HERE/'src'/(name+'.cesta');frozen=source.read_bytes();plans={}
        for cc in ('gcc','clang'):
            plan=work/(name+'-'+cc+'.plan');exported=work/(name+'-'+cc+'.c')
            run([args.cesta,'export-c',source,'--std=c11','--cc',cc,'--plan',plan,'-o',exported,'--',include]);run([cc,'-std=c11','-Wall','-Wextra','-Wpedantic','-Werror','-fsyntax-only',include,exported])
            plans[hashlib.sha256(plan.read_bytes()).hexdigest()]=plan
        admissions=[]
        for index,plan in enumerate(plans.values()):
            record,=native.parse(plan.read_bytes());admissions.append(native.admit_record(record,work/(name+str(index)+'.lean'),args.lean_root.resolve()))
        assert source.read_bytes()==frozen
        rows.append(dict(unit=name,source_sha256=hashlib.sha256(frozen).hexdigest(),admission=admissions))
    source=HERE/'src/shell_text_delivery.cesta';text=source.read_text()
    opening='ShellTextPoll shell_text_poll(ShellTextOutbox *out,size_t budget)\n{'
    assert opening in text
    mutated=text.replace(opening,opening+'\n    const ShellTextBatch *held=&out->current;shell_text_cancel(out);(void)held->text.len;')
    path=work/'borrowed_batch_during_cancel.cesta';path.write_text(mutated)
    for cc in ('gcc','clang'):run([args.cesta,'cc','--cc',cc,'-std=c11','-fsyntax-only',include,path],False)
    summary=dict(units=rows,source_mutants_refused=1)
    (work/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
    print('text ownership:',sum(a['functions']for row in rows for a in row['admission']),'actual plans admitted; live-batch cancellation refused')


if __name__=='__main__':main()
