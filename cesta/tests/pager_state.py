#!/usr/bin/env python3
"""Fault metadata and bounded frame custody against unmodified Rust transitions."""
import argparse
import hashlib
import json
from pathlib import Path
import random
import subprocess
import sys
import tempfile
from layout_text import LayoutText

HERE=Path(__file__).resolve().parent.parent
MASK=(1<<64)-1


def run(argv,data=None):
    p=subprocess.run(list(map(str,argv)),cwd=HERE,input=data,capture_output=True)
    if p.returncode:raise RuntimeError((list(map(str,argv)),p.returncode,p.stdout[-1500:],p.stderr[-5000:]))
    return p.stdout


def vectors(seed,cases):
    rows=[];controls=[]
    def add(op,*args,expect=None):
        assert len(args)<24
        if expect is not None:controls.append((len(rows),expect))
        rows.append((op,*args,*([0]*(23-len(args)))))
    def record(token=1):return [token,10,14,1010,0x4001,0x4000,6,0x1000,0x2000]
    def binding(token=1,frame=0,page=0,range_id=1):return [token,1010 if range_id==1 else 1515,10 if range_id==1 else 15,14,range_id,page,frame,6]
    for last in (0,MASK-1,MASK):add(0,last);add(2,expect=4 if last==MASK else 0);add(2)
    for address in (0,1,0x4000,(1<<47)-1,1<<47,MASK):
        for error in (0,2,4,6,7,14,20,36):
            for cs in (0,3,7):add(1,address,error,cs)
    for field in range(9):
        for value in (0,1,3,4,6,14,0x4000,0x4001,1<<47,MASK):
            add(0);r=record();r[field]=value;add(3,*r);add(6,14,1);add(7,14,1);add(9,14,1)
    add(0);add(3,*record(),expect=0);add(3,*record(2),expect=3)
    add(6,14,1,expect=2);add(7,13,1,expect=3);add(4,1,expect=0)
    add(6,14,2,expect=1);add(7,14,1,expect=0);add(6,14,1,expect=2)
    add(8,14,1,expect=0);add(7,14,1);add(9,14,1,expect=0);add(9,14,1,expect=0)
    add(3,*record(2));add(4,2);add(7,14,2);add(9,14,1,expect=0)
    add(9,14,2);add(9,14,1,expect=1);add(10,10,14,1010);add(3,*record(3),expect=3)
    add(0);add(3,*record());add(5,1);add(3,*record(),expect=1);add(3,*record(2),expect=0)
    add(4,2);add(11,14,2);add(3,*record(3));add(4,3);add(7,14,3);add(11,14,3,expect=2)
    add(10,10,14,999,expect=3);add(10,10,14,1010,expect=0)
    valid=[1,1,10,0x4000,32768,6,8,1,0x10000,32768,14,1010,1,10,14,0x4000,6,0,6]
    add(20,*valid,expect=0)
    for i in range(len(valid)):
        for value in (0,1,2,4,6,8,9,255,256,262,4096,32768,1<<47,MASK):
            v=valid[:];v[i]=value;add(20,*v)
    # Occupancy includes published but unfinished mappings; closure includes them.
    add(0);b=binding();add(21,*b,8,expect=0);add(24,*b,expect=3)
    add(21,*binding(2,1,0),8,expect=3);add(21,*binding(2,0,1),8,expect=3)
    add(23,*b,expect=0);add(24,*b,expect=0);add(21,*b,8,expect=3)
    add(21,*binding(2),8,expect=0);add(27,1,10,14,999,expect=3)
    add(28,1,10,14,1010,expect=0);add(21,*binding(3,1,1),8,expect=3)
    for quota in (1,4,8):
        add(0)
        for frame in range(8):add(21,*binding(frame+1,frame,frame),quota,expect=0 if frame<quota else 4)
    rng=random.Random(seed)
    for _ in range(cases):
        add(0);token=rng.randrange(1,100000);add(3,*record(token));add(4,token)
        for _ in range(4):add(rng.choice((6,7,8,9,11)),rng.choice((13,14)),token+rng.choice((0,0,1)))
        add(7,14,token);add(9,14,token);add(9,14,token)
        add(3,*record(token+1));add(4,token+1);add(7,14,token+1);add(9,14,token+1);add(9,14,token)
        for frame in range(rng.randrange(1,9)):
            b=binding(token+frame,frame,frame,rng.choice((1,2)));add(21,*b,rng.choice((1,4,8)))
            if rng.randrange(2):add(23,*b)
            if rng.randrange(2):add(24,*b)
            changed=b[:];changed[rng.randrange(8)]=rng.randrange(10)
            add(rng.choice((22,23,24)),*changed,8)
        for r,t,task in ((1,10,1010),(2,15,1515)):
            add(27,r,t,14,task);add(28,r,t,14,task);add(28,r,t,14,task);add(21,*binding(token+20,0,0,r),8)
    return rows,controls


def main():
    ap=argparse.ArgumentParser();ap.add_argument('--cesta',type=Path,required=True);ap.add_argument('--work',type=Path)
    ap.add_argument('--seed',type=int,default=20261031);ap.add_argument('--cases',type=int,default=1500)
    ap.add_argument('--mutations',action='store_true');ap.add_argument('--lean-root',type=Path);ap.add_argument('--cesta-source',type=Path);args=ap.parse_args()
    work=args.work or Path(tempfile.mkdtemp(prefix='pager-state-',dir=HERE/'build'));work.mkdir(parents=True,exist_ok=True)
    inventory=json.loads((HERE/'interfaces/pager.source.json').read_text())
    for row in inventory['sources']:assert hashlib.sha256((HERE.parent/row['path']).read_bytes()).hexdigest()==row['sha256'],row['path']
    for name,src,probe in [('fault','fault_transaction','fault_probe'),('paging','paging_state','paging_probe')]:
        (work/f'actual_{name}.rs').write_bytes((HERE.parent/f'kernel/src/{src}.rs').read_bytes()+b'\n'+(HERE/f'tests/pager_state/{probe}.rs').read_bytes())
    (work/'oracle.rs').write_bytes((HERE/'tests/pager_state/oracle.rs').read_bytes())
    run(['rustc','--edition=2021','--cap-lints','allow','-O',work/'oracle.rs','-o',work/'rust'])
    rows,controls=vectors(args.seed,args.cases);data=''.join(' '.join(map(str,r))+'\n' for r in rows).encode();(work/'commands.txt').write_bytes(data)
    expected=run([work/'rust'],data);(work/'rust.out').write_bytes(expected);lines=expected.splitlines()
    for i,status in controls:assert int(lines[i].split()[1])==status,(i,rows[i],status,lines[i])
    source=work/'pager_state.c';plan=work/'pager.plan'
    run([args.cesta,'export-c','src/pager_state.cesta','--std=c11','--plan',plan,'-o',source,'--','-Igenerated/include'])
    flags=['-std=c11','-g0','-Wall','-Wextra','-Wpedantic','-Werror','-Igenerated/include'];builds=[]
    def check(binary):
        actual=run([binary],data)
        if actual!=expected:
            (work/(binary.name+'.out')).write_bytes(actual);a=actual.splitlines()
            i=next((i for i,p in enumerate(zip(a,lines)) if p[0]!=p[1]),min(len(a),len(lines)))
            raise AssertionError((binary.name,i,rows[i],a[i:i+1],lines[i:i+1]))
    for cc in ('gcc','clang'):
        for mode in ('O0','O2','sanitize'):
            options=['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all'] if mode=='sanitize' else ['-'+mode]
            binary=work/(cc+'-'+mode);run([cc,*flags,*options,'tests/pager_state/host.c',source,'-o',binary]);check(binary)
            if mode=='O2':
                authored=work/(cc+'-authored');run([args.cesta,'cc','--cc',cc,*flags,*options,'tests/pager_state/host.c','src/pager_state.cesta','-o',authored])
                assert binary.read_bytes()==authored.read_bytes(),(cc,'plain-C identity');check(authored)
            builds.append({'compiler':cc,'mode':mode,'commands':len(rows)})
        print(cc,len(rows),'fault/ledger steps and complete logical state match Rust',flush=True)
    mutations=[
        ('token_wrap','if (*last==UINT64_MAX)','if (false)'),
        ('skip_token_floor','r.token<=s->last_token','false'),
        ('query_prepared','s->phase!=PG_PENDING','s->phase==PG_IDLE'),
        ('recomplete','return (PgFaultResult){.status=fault_matches(s->completed,pager,token)};','return (PgFaultResult){.present=true,.record=s->completed};'),
        ('ignore_authorized_pager','r.pager_pd!=pager','false'),
        ('narrow_rights','if (rights>UINT8_MAX','rights=(uint8_t)rights; if (rights>UINT8_MAX'),
        ('duplicate_page','old.target_pd==b.target_pd && old.range_id==b.range_id && old.page_index==b.page_index','false'),
        ('reuse_unfinalized',' || !r.finalized',''),
        ('retired_token_reuse','token<=s->retired_highwater[range-1]','(s->retired_highwater[range-1]==UINT64_MAX && token==0)'),
        ('forget_range_closure','s->closed[b.range_id-1].present','false'),
        ('skip_cleanup_custody','b.task_id!=task','(b.task_id==task && false)'),
        ('lose_cleanup_frames','frames|=(uint8_t)(1u<<b.frame_index)','frames=0'),
    ];caught=[]
    if args.mutations:
        original=LayoutText(source.read_text())
        for name,before,after in mutations:
            assert before in original,(name,'anchor')
            try:
                source.write_text(original.replace(before,after));binary=work/name
                run(['gcc',*flags,'-O2','tests/pager_state/host.c',source,'-o',binary])
                assert run([binary],data)!=expected,('mutation survived',name);caught.append(name)
            finally:source.write_text(original)
    admission=None
    if args.lean_root:
        if not args.cesta_source:ap.error('--lean-root requires --cesta-source')
        sys.path.insert(0,str(args.cesta_source.resolve()/'tests'));import ownership_admission
        record,=ownership_admission.parse(plan.read_bytes());admission=ownership_admission.admit_record(record,work/'PagerOwnership.lean',args.lean_root.resolve())
    names=['src/pager_state.cesta','src/pager_state_interface.cesta','interfaces/pager.source.json','tests/pager_state.py',*[str(p.relative_to(HERE)) for p in sorted((HERE/'tests/pager_state').iterdir())]]
    summary={'pin':inventory['pin'],'seed':args.seed,'builds':builds,'independent_controls':len(controls),'mutations':caught,'admission':admission,'source_sha256':{n:hashlib.sha256((HERE/n).read_bytes()).hexdigest() for n in names},'scope':'exact pure Rust fault/ledger transitions and full normalized logical state; probe functions only expose private state. Serialization, detached task ownership, leaf transitions, invalidation and publication ordering are native obligations, not proved by this comparison.'}
    (work/'summary.json').write_text(json.dumps(summary,indent=2)+'\n');print('pager state qualified',flush=True)


if __name__=='__main__':main()
