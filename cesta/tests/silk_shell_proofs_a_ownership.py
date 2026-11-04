"""Admit actual diagnostic plans and reject widened spans or conflicting rights."""
import argparse,hashlib,importlib,json,re,subprocess,sys
from pathlib import Path
HERE=Path(__file__).resolve().parent.parent
def run(args,accepted=True):
    r=subprocess.run(list(map(str,args)),cwd=HERE,capture_output=True,text=True)
    assert (r.returncode==0)==accepted,(args,r.returncode,r.stderr[-6000:])
    if not accepted:assert 'ownership'in r.stderr,r.stderr

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--cesta',default='cesta');parser.add_argument('--cesta-source',type=Path);parser.add_argument('--lean-root',type=Path);parser.add_argument('--output',type=Path,default=HERE/'build/proofs-a-ownership');args=parser.parse_args();w=args.output.resolve();w.mkdir(parents=True,exist_ok=True)
    source=HERE/'src/silk_shell_proofs_a.cesta';original=source.read_bytes();plans={};records=[];include='-I'+str(HERE/'generated/include')
    for cc in ('gcc','clang'):
        out=w/(cc+'.c');plan=w/(cc+'.plan');run([args.cesta,'export-c',source,'--cc',cc,'--std=c11','-o',out,'--plan',plan,'--',include]);run([cc,'-std=c11','-Wall','-Wextra','-Wpedantic','-Werror','-fsyntax-only',include,out])
        names=set(re.findall(r'\(function ([^\s()]+)',plan.read_text()));assert len(names)==83,names
        digest=hashlib.sha256(plan.read_bytes()).hexdigest();plans[digest]=plan;records.append(dict(compiler=cc,sha256=digest,functions=len(names)))
    text=original.decode();changes=(
        ('read_only_state','shell_proofs_a_maybe_run_browser_stub_proof(SilkShellProofsAState *state)','shell_proofs_a_maybe_run_browser_stub_proof(const SilkShellProofsAState *state)'),
        ('widen_literal_extent','proof_draw_3(uint64_t surface,const uint8_t (*text)[3]','proof_draw_3(uint64_t surface,const uint8_t (*text)[4]'),
        ('mutate_read_view','static ShellDiagnosticDraw proof_draw_17(uint64_t surface,const uint8_t (*text)[17],uint64_t color)\n{','static ShellDiagnosticDraw proof_draw_17(uint64_t surface,const uint8_t (*text)[17],uint64_t color)\n{\n (*text)[0]=0;'),
    )
    for name,old,new in changes:
        assert text.count(old)==1,(name,text.count(old));path=w/(name+'.cesta');path.write_text(text.replace(old,new))
        for cc in ('gcc','clang'):run([args.cesta,'cc','--cc',cc,'-std=c11','-fsyntax-only',include,path],False)
    admission=[]
    if args.lean_root:
        assert args.cesta_source;sys.path.insert(0,str(args.cesta_source.resolve()/'tests'));native=importlib.import_module('ownership_admission')
        for i,plan in enumerate(plans.values()):
            record,=native.parse(plan.read_bytes());admission.append(native.admit_record(record,w/f'ProofsA{i}.lean',args.lean_root.resolve()))
    assert source.read_bytes()==original
    (w/'summary.json').write_text(json.dumps(dict(source_sha256=hashlib.sha256(original).hexdigest(),plans=records,source_mutants_refused=len(changes),admission=admission),indent=2)+'\n');print('Proofs-A ownership:',len(changes),'source controls;',sum(r['functions']for r in admission),'actual plans admitted')

if __name__=='__main__':main()
