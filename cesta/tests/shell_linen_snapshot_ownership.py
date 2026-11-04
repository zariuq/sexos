"""Source refusals and kernel admission for the actual bounded snapshot client."""
import argparse
import hashlib
import importlib
import json
from pathlib import Path
import re
import subprocess
import sys

HERE=Path(__file__).resolve().parent.parent


def run(args,accepted=True):
    result=subprocess.run(list(map(str,args)),cwd=HERE,capture_output=True,text=True)
    assert (result.returncode==0)==accepted,(args,result.returncode,result.stderr[-6000:])
    if not accepted:assert 'ownership'in result.stderr,result.stderr


def main():
    parser=argparse.ArgumentParser();parser.add_argument('--cesta',default='cesta');parser.add_argument('--cesta-source',type=Path);parser.add_argument('--lean-root',type=Path);parser.add_argument('--output',type=Path,default=HERE/'build/linen-snapshot-ownership');args=parser.parse_args()
    work=args.output.resolve();work.mkdir(parents=True,exist_ok=True);include='-I'+str(HERE/'generated/include')
    source=HERE/'src/shell_linen_snapshot.cesta';original=source.read_bytes();plans={};rows=[]
    for cc in ('gcc','clang'):
        out=work/(cc+'.c');plan=work/(cc+'.plan')
        run([args.cesta,'export-c',source,'--cc',cc,'--std=c11','-o',out,'--plan',plan,'--',include])
        run([cc,'-std=c11','-Wall','-Wextra','-Wpedantic','-Werror','-fsyntax-only',include,out])
        names=set(re.findall(r'\(function ([^\s()]+)',plan.read_text()));assert len(names)==14,names
        digest=hashlib.sha256(plan.read_bytes()).hexdigest();plans[digest]=plan;rows.append(dict(compiler=cc,plan=plan.name,sha256=digest,functions=len(names)))
    text=original.decode();opening='bool shell_linen_snapshot_queue_open(ShellLinenSnapshot *s,uint64_t id,uint64_t choice)\n{'
    changes=(
        ('read_only_state','shell_linen_snapshot_refresh(ShellLinenSnapshot *s','shell_linen_snapshot_refresh(const ShellLinenSnapshot *s'),
        ('read_only_request','ShellLinenRequest *out)','const ShellLinenRequest *out)'),
        ('live_current_across_request',opening,opening+'\n ShellLinenObject *held=&s->current;ShellLinenRequest request={0};shell_linen_snapshot_request(s,&request);(void)held->id;'),
        ('uninitialized_snapshot','void sexos_fail(void);','void sexos_fail(void);\nvoid bad_initialization(void){ShellLinenSnapshot s;shell_linen_snapshot_refresh(&s,false);}'),
    )
    for name,old,new in changes:
        assert text.count(old)==1,(name,text.count(old));mutant=work/(name+'.cesta');mutant.write_text(text.replace(old,new))
        for cc in ('gcc','clang'):run([args.cesta,'cc','--cc',cc,'-std=c11','-fsyntax-only',include,mutant],False)
    admission=[]
    if args.lean_root:
        assert args.cesta_source
        sys.path.insert(0,str(args.cesta_source.resolve()/'tests'));native=importlib.import_module('ownership_admission')
        for i,plan in enumerate(plans.values()):
            record,=native.parse(plan.read_bytes());admission.append(native.admit_record(record,work/f'Snapshot{i}.lean',args.lean_root.resolve()))
    assert source.read_bytes()==original
    summary=dict(source_sha256=hashlib.sha256(original).hexdigest(),plans=rows,source_mutants_refused=len(changes),admission=admission)
    (work/'summary.json').write_text(json.dumps(summary,indent=2)+'\n');print('snapshot ownership:',len(changes),'source mutations refused;',sum(row['functions']for row in admission),'actual plans admitted')


if __name__=='__main__':main()
