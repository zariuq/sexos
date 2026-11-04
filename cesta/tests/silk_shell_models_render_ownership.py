"""Admit the actual checked list/render plans; refuse conflicting outbox loans."""
import argparse
import hashlib
import importlib
import json
from pathlib import Path
import subprocess
import sys

HERE=Path(__file__).resolve().parent.parent


def run(args,accepted=True):
    r=subprocess.run(list(map(str,args)),cwd=HERE,capture_output=True,text=True)
    assert (r.returncode==0)==accepted,(args,r.returncode,r.stderr[-6000:])
    if not accepted:assert 'ownership'in r.stderr,r.stderr


def main():
    parser=argparse.ArgumentParser();parser.add_argument('--cesta',default='cesta');parser.add_argument('--cesta-source',type=Path);parser.add_argument('--lean-root',type=Path);parser.add_argument('--output',type=Path,default=HERE/'build/models-render-ownership');args=parser.parse_args()
    w=args.output.resolve();w.mkdir(parents=True,exist_ok=True);include='-I'+str(HERE/'generated/include');plans={};sources={};records=[]
    for unit in ('shell_linen_list','silk_shell_models_render','silk_shell_models_paint','silk_shell_models_workspace','silk_shell_models_projection','silk_shell_models_publish'):
        source=HERE/('src/'+unit+'.cesta');sources[unit]=source.read_bytes()
        for cc in ('gcc','clang'):
            out=w/(unit+'-'+cc+'.c');plan=w/(unit+'-'+cc+'.plan')
            run([args.cesta,'export-c',source,'--cc',cc,'--std=c11','-o',out,'--plan',plan,'--',include])
            run([cc,'-std=c11','-Wall','-Wextra','-Wpedantic','-Werror','-fsyntax-only',include,out])
            digest=hashlib.sha256(plan.read_bytes()).hexdigest();plans[digest]=plan;records.append(dict(unit=unit,compiler=cc,sha256=digest))
    text=sources['silk_shell_models_render'].decode()
    changes=(
        ('read_only_outbox','(field linen (borrow write))','(field linen (borrow read))'),
        ('outbox_loan_across_render','ShellLinenItems items=shell_models_peer_list_items(ctx);','ShellTextOutbox *held=&ctx->linen->text;shell_models_render_static_ui(ctx);(void)held->sequence;ShellLinenItems items=shell_models_peer_list_items(ctx);'),
    )
    for name,old,new in changes:
        assert text.count(old)==1,(name,text.count(old));path=w/(name+'.cesta');path.write_text(text.replace(old,new))
        for cc in ('gcc','clang'):run([args.cesta,'cc','--cc',cc,'-std=c11','-fsyntax-only',include,path],False)
    workspace=sources['silk_shell_models_workspace'].decode()
    workspace_changes=(
        ('workspace_read_only_runtime','(field linen (borrow write))','(field linen (borrow read))'),
        ('snapshot_loan_across_refresh','bool found=false;','const ShellLinenSnapshot *held=&ctx->linen->snapshot;shell_models_workspace_poll(ctx);(void)held->count;bool found=false;'),
    )
    for name,old,new in workspace_changes:
        assert workspace.count(old)==1,(name,workspace.count(old));path=w/(name+'.cesta');path.write_text(workspace.replace(old,new))
        for cc in ('gcc','clang'):run([args.cesta,'cc','--cc',cc,'-std=c11','-fsyntax-only',include,path],False)
    admission=[]
    if args.lean_root:
        assert args.cesta_source
        sys.path.insert(0,str(args.cesta_source.resolve()/'tests'));native=importlib.import_module('ownership_admission')
        for i,plan in enumerate(plans.values()):
            record,=native.parse(plan.read_bytes());admission.append(native.admit_record(record,w/f'Render{i}.lean',args.lean_root.resolve()))
    for unit,original in sources.items():assert (HERE/('src/'+unit+'.cesta')).read_bytes()==original
    summary=dict(plans=records,source_mutants_refused=len(changes)+len(workspace_changes),admission=admission,native_boundary=None,publication_array_admitted=True)
    (w/'summary.json').write_text(json.dumps(summary,indent=2)+'\n');print('Linen renderer ownership:',len(changes)+len(workspace_changes),'source mutations refused;',sum(row['functions']for row in admission),'actual plans admitted')


if __name__=='__main__':main()
