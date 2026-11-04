#!/usr/bin/env python3
"""Private-image policy and complete leaf-effect traces against actual Rust."""
import argparse
import hashlib
import json
from pathlib import Path
import random
import subprocess
import sys
import tempfile

HERE=Path(__file__).resolve().parent.parent
MASK=(1<<64)-1


def run(argv,data=None):
    p=subprocess.run(list(map(str,argv)),cwd=HERE,input=data,capture_output=True)
    if p.returncode:raise RuntimeError((argv,p.returncode,p.stdout[-2000:],p.stderr[-6000:]))
    return p.stdout


def vectors(seed,count):
    rows=[]
    def add(op,*args):rows.append((op,*args,*([0]*(23-len(args)))))
    for n in (0,1,3,8,512,513):
        add(0,n);add(2);add(2);add(3);add(3)
    for field,values in {0:[0,1,4097,8192],1:[0,1,2,3],2:[0,4,5,4096|5,8192|5,12288|7]}.items():
        for value in values:
            add(0,3);page=[12288,3,12288|5];page[field]=value;add(1,2,*page,page[2]);add(2)
    # Every mismatch must be discovered before the first clear/write.
    for i in range(3):
        add(0,3);add(8,i+1,((i+1)*4096)|7);add(2)
        add(0,3);add(2);add(8,i+1,5);add(3)
    for ad in (0,0x20,0x40,0x60):
        add(0,3);add(8,3,12288|5|ad);add(2);add(3)
    for logical in (0,9,(1<<32)-1):
        add(9,logical)
        for ordinal in (0,1,2,(1<<32)-2,(1<<32)-1):add(5,logical,ordinal)
    for flags in (0,5,7,MASK,0x8000000000000005):
        for key in range(16):add(7,flags,key)
    base=[9,901,3,3,1,3,9,901,2]
    for i in range(len(base)):
        for value in (0,1,2,3,4,7,16,31,32,255,(1<<32)-1):
            v=base[:];v[i]=value;add(4,*v)
    activation=[3,9,2,9,901,16,1601,3,9,901,16,1601,9,1,9,2]
    add(6,*activation)
    for i in range(len(activation)):
        for value in (0,1,2,3,9,16,901,1601,(1<<32)-1):
            v=activation[:];v[i]=value;add(6,*v)
    rng=random.Random(seed)
    for _ in range(count):
        n=rng.choice((1,3,8));add(0,n)
        for _ in range(rng.randrange(4)):
            i=rng.randrange(n);va=rng.choice((0,4096,8192,(i+1)*4096,4097));leaf=rng.randrange(n+1)
            expected=rng.choice((0,4,5,((i+1)*4096)|5,8192|5,MASK))
            actual=expected^rng.choice((0,0x20,0x40,0x60,2,4096))
            add(1,i,va,leaf,expected,actual)
        add(2);add(3)
    return rows


def main():
    ap=argparse.ArgumentParser();ap.add_argument('--cesta',type=Path,required=True)
    ap.add_argument('--work',type=Path);ap.add_argument('--seed',type=int,default=20261031)
    ap.add_argument('--cases',type=int,default=1500);ap.add_argument('--mutations',action='store_true')
    ap.add_argument('--lean-root',type=Path);ap.add_argument('--cesta-source',type=Path);args=ap.parse_args()
    work=args.work or Path(tempfile.mkdtemp(prefix='app-private-',dir=HERE/'build'));work.mkdir(parents=True,exist_ok=True)
    inventory=json.loads((HERE/'interfaces/kernel_current.source.json').read_text())
    for entry in inventory['sources']:
        assert hashlib.sha256((HERE.parent/entry['path']).read_bytes()).hexdigest()==entry['sha256'],entry['path']
    (work/'actual_catalog.rs').write_bytes((HERE.parent/'crates/sex-pdx/src/app_catalog.rs').read_bytes())
    (work/'actual_state.rs').write_bytes((HERE.parent/'kernel/src/app_resource_state.rs').read_bytes())
    (work/'oracle.rs').write_bytes((HERE/'tests/app_private/oracle.rs').read_bytes())
    run(['rustc','--edition=2021','--cap-lints','allow','-O',work/'oracle.rs','-o',work/'rust'])
    rows=vectors(args.seed,args.cases);data=''.join(' '.join(map(str,r))+'\n' for r in rows).encode()
    (work/'commands.txt').write_bytes(data);expected=run([work/'rust'],data);(work/'rust.out').write_bytes(expected)
    source=work/'app_private.c';plan=work/'private.plan'
    run([args.cesta,'export-c','src/app_private.cesta','--std=c11','--plan',plan,'-o',source,'--','-Igenerated/include'])
    flags=['-std=c11','-g0','-Wall','-Wextra','-Wpedantic','-Werror','-Igenerated/include'];builds=[]
    def check(binary):
        actual=run([binary],data)
        if actual!=expected:
            (work/(binary.name+'.out')).write_bytes(actual)
            a,b=actual.splitlines(),expected.splitlines();i=next((i for i,p in enumerate(zip(a,b)) if p[0]!=p[1]),min(len(a),len(b)))
            raise AssertionError((binary.name,i,rows[i],a[i:i+1],b[i:i+1]))
    for cc in ('gcc','clang'):
        for mode in ('O0','O2','sanitize'):
            options=['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all'] if mode=='sanitize' else ['-'+mode]
            binary=work/(cc+'-'+mode);run([cc,*flags,*options,'tests/app_private/host.c',source,'-o',binary]);check(binary)
            if mode=='O2':
                authored=work/(cc+'-authored');run([args.cesta,'cc','--cc',cc,*flags,*options,'tests/app_private/host.c','src/app_private.cesta','-o',authored])
                assert binary.read_bytes()==authored.read_bytes(),(cc,'plain-C identity');check(authored)
            builds.append({'compiler':cc,'mode':mode,'commands':len(rows)})
        print(cc,len(rows),'private-image policy/effect steps match Rust',flush=True)
    mutants=[
        ('ignore_hardware_changed_leaf','(app_private_leaf_read(page.leaf)&~UINT64_C(0x60))!=(page.expected&~UINT64_C(0x60))','false'),
        ('physical_alias','(old.expected&APP_PRIVATE_PHYSICAL)==(page.expected&APP_PRIVATE_PHYSICAL)','false'),
        ('no_retirement_flush','app_private_leaf_flush(p->pages[i].va);','(void)0;'),
        ('wrong_restore_value','app_private_leaf_write(p->pages[i].leaf,p->pages[i].expected);','app_private_leaf_write(p->pages[i].leaf,0);'),
        ('incarnation_wrap','old.number==UINT32_MAX','false'),
        ('wrong_activation_subject','g.successor_task!=successor_task','g.successor_task==successor_task'),
        ('missing_ack_role','(g.ack_bit&(g.ack_bit-1))','false'),
    ];caught=[]
    if args.mutations:
        original=source.read_text()
        for name,before,after in mutants:
            assert original.count(before)>=1,(name,'mutation anchor')
            try:
                source.write_text(original.replace(before,after));binary=work/name
                run(['gcc',*flags,'-O2','tests/app_private/host.c',source,'-o',binary])
                assert run([binary],data)!=expected,('mutation survived',name);caught.append(name)
            finally:source.write_text(original)
    admission=None
    if args.lean_root:
        if not args.cesta_source:ap.error('--lean-root requires --cesta-source')
        sys.path.insert(0,str(args.cesta_source.resolve()/'tests'));import ownership_admission
        record,=ownership_admission.parse(plan.read_bytes());admission=ownership_admission.admit_record(record,work/'PrivateOwnership.lean',args.lean_root.resolve())
    names=('src/app_private.cesta','src/app_private_interface.cesta','tests/app_private.py','tests/app_private/host.c','tests/app_private/oracle.rs','interfaces/kernel_current.source.json')
    summary={'pin':inventory['pin'],'builds':builds,'mutations':caught,'admission':admission,'source_sha256':{n:hashlib.sha256((HERE/n).read_bytes()).hexdigest() for n in names},'scope':'actual pure Rust authority/lineage and leaf-transition functions with complete provider traces; no hardware/scheduler/refinement claim; valid-key final flag comparison'}
    (work/'summary.json').write_text(json.dumps(summary,indent=2)+'\n');print('private policy qualified',flush=True)


if __name__=='__main__':main()
