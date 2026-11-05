#!/usr/bin/env python3
"""Current upstream Sext allocator/pager bodies against the Cesta translation."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import random
import re
import subprocess
import tempfile
from layout_text import LayoutText

HERE=Path(__file__).resolve().parent.parent
INVENTORY=HERE/'interfaces/sext.source.json'
MASK=(1<<64)-1

def run(command, *, data=None, env=None):
    p=subprocess.run(list(map(str,command)),cwd=HERE,input=data,capture_output=True,env=env)
    if p.returncode:
        raise AssertionError((command,p.returncode,p.stdout[-1500:],p.stderr[-7000:]))
    return p.stdout

def vectors(count,seeds,cases):
    rows=[]
    def add(op,*args):rows.append((op,*args,*([0]*(8-len(args)))))
    def statuses(commit=0,reject=0,reclaim=0,cleanup=5,bitmap=0,task=0):
        add(21,commit,reject,reclaim,cleanup,bitmap,task)
    def reset():
        add(11,0x400000,count*4096,0,6)
        add(12,0,1,36,0x800000,32768,15,6)
        add(12,1,1,37,0x900000,32768,10,6)
        statuses();add(23);add(13,max(1,min(count,4)))
    def fault(page=0,token=1,target=15,error=4,query_status=0,rights=6):
        base=0x900000 if target==10 else 0x800000
        add(20,query_status,base+page*4096,error,target,rights)
        add(14,base+page*4096+19,error,token,target)
    add(0,2)
    for token,target,page in ((0,15,4096),(1,0,4096),(1,15,0),(1,15,4097),(1,15,4096),(1,15,4096)):
        add(1,token,target,page)
    for op,index,token in ((3,0,1),(2,count,1),(2,0,2),(4,0,2),(2,0,1),(1,1,15),(3,0,1),(4,0,1)):
        if op==1:add(1,1,15,4096)
        else:add(op,index,token)
    add(5,15,4096)
    for index,token,target,page in ((count,1,15,4096),(0,2,15,4096),(0,1,10,4096),(0,1,15,8192),(0,1,15,4096)):
        add(6,index,token,target,page)
    add(0,count)
    for i in range(count):add(1,i+1,1+i%2,4096+i*4096)
    add(1,count+1,99,4096)
    for target in (0,1,2,99):
        add(8,target);add(9,target);add(10,target)
    add(7,1);add(8,2);add(10,2);add(7,2)
    # Wire widths and invalid forms, independently of initialized pager state.
    valid=[(0,0x103,15,0x800013,4,1),(1,0x104,15,0x800000,1,0),
           (2,0x105,0,15,77,0),(3,0x106,10,15,1,0)]
    for kind,op,caller,a,b,c in valid:
        add(19,kind,op,caller,a,b,c)
        for index in range(5):
            for value in (0,1,(1<<32)+15,(1<<47),MASK):
                fields=[op,caller,a,b,c];fields[index]=value
                add(19,kind,*fields)
    for request in (0,1,MASK>>8,(MASK>>8)+1,MASK):
        for status in (0,1,5,255,256,MASK):
            add(19,4,0,0,request,status,0)
            add(19,5,0,0,((request<<8)|status)&MASK,request,0)
    reset();fault();add(18,15)
    add(15,0x800000,1,15);add(15,0x800000,1,15)
    fault(token=2);add(15,0x800000,1,15);add(15,0x800000,2,15);add(15,0x800000,1,15)
    # An uncertain commit must retain backing and never re-zero client bytes.
    reset();statuses(commit=2);fault();add(22,0,119)
    add(14,0x800013,4,1,15);fault(1,2)
    statuses();add(14,0x800013,4,1,15)
    for error in (1,3,4,999):
        reset();statuses(commit=error);fault();add(14,0x800013,4,1,15)
        reset();statuses(commit=2);fault();statuses(commit=error);add(14,0x800013,4,1,15)
    # Query authentication and mapping policy.
    for status in (1,2,3,4,999):
        reset();fault(query_status=status)
    for delta in (0,4096):
        for target,error,rights in ((10,4,6),(15,6,6),(15,4,0),(15,4,2),(15,4,255)):
            reset();add(20,0,0x800000+delta,error,target,rights);add(14,0x800013,4,1,15)
    reset();fault(error=6,rights=2);reset();fault(page=8)
    reset();add(12,0,1,36,0x800000,32768,15,2);add(13,max(1,min(count,4)));fault(error=6)
    reset()
    for i in range(6):fault(page=i,token=i+1)
    # Reclaim failure keeps exact custody; release correlation never rewinds.
    for status in (0,2,3,4,999):
        reset();fault();statuses(reclaim=status)
        add(15,0x800000,1,15);add(15,0x800000,2,15);add(15,0x800000,1,15)
        statuses();add(15,0x800000,1,15)
    # Permanent cleanup: old cached bitmaps must not release another owner.
    reset();statuses(commit=2);fault()
    add(17,15,1,10)
    for status,bitmap,task in ((2,1,77),(5,1,77),(0,1,78),(0,1<<63,77),(0,1,77)):
        statuses(cleanup=status,bitmap=bitmap,task=task);add(16,15,77)
    add(17,15,1,10);fault(target=10,token=91);add(16,15,77);add(18,10)
    add(16,15,78);add(16,999,77);add(16,10,0)
    for target,caller,request in ((15,15,1),(15,999,1),(999,10,1),(15,10,0)):
        add(17,target,request,caller)
    # Bad BOOT views must leave a previously constructed pager unchanged.
    for base,length,target,rights in ((0,4096,0,6),(0x400001,4096,0,6),(MASK-4095,8192,0,6),(0x400000,4095,0,6),(0x400000,count*4096,15,6),(0x400000,count*4096,0,2)):
        reset();add(11,base,length,target,rights);add(13,1)
    reset();add(13,0);add(13,count+1)
    for slot,base,target in ((36,0x900000,10),(37,0x800000,10),(37,0x900000,15),(0,0x900000,10)):
        reset();add(12,1,1,slot,base,32768,target,6);add(13,1)
    boundary=[0,1,2,3,4,5,6,15,16,65,255,256,4095,4096,4097,1<<32,(1<<32)+15,1<<47,MASK]
    for seed in range(seeds):
        rng=random.Random(0x5e87+seed);reset();add(0,max(count,1))
        for _ in range(cases):
            op=rng.choice([1,2,3,4,5,6,7,8,9,10,14,14,14,15,16,17,18,19,20,21,22,23,24])
            if op==24:reset()
            elif op==1:add(op,rng.randrange(1,20),rng.choice([0,10,15,99]),rng.choice([0,4096,8192,4097]))
            elif op in (2,3,4):add(op,rng.choice([0,1,count,count-1 if count else 0,MASK]),rng.randrange(20))
            elif op==5:add(op,rng.choice([10,15,99]),rng.choice([4096,8192,0x800000]))
            elif op==6:add(op,rng.choice([0,count,MASK]),rng.randrange(20),rng.choice([10,15,99]),rng.choice([4096,8192,0x800000]))
            elif op in (7,8,9,10,18):add(op,rng.choice([0,10,15,99,MASK]))
            elif op==14:
                target=rng.choice([0,10,15,99]);base=0x900000 if target==10 else 0x800000
                page=rng.randrange(10);error=rng.choice([4,6,7]);token=rng.randrange(20)
                if rng.randrange(3):add(20,0,base+page*4096,error,target,rng.choice([0,2,4,6,255]))
                add(op,base+page*4096+rng.choice([0,19,4095]),error,token,target)
            elif op==15:add(op,0x800000+4096*rng.randrange(10),rng.randrange(20),rng.choice([10,15,99]))
            elif op==16:add(op,rng.choice([0,10,15,99]),rng.choice([0,77,78,MASK]))
            elif op==17:add(op,rng.choice([10,15,99]),rng.randrange(20),rng.choice([10,15,99]))
            elif op==19:add(op,rng.randrange(6),rng.choice([0x103,0x104,0x105,0x106]),rng.choice(boundary),*[rng.choice(boundary) for _ in range(3)])
            elif op==20:add(op,rng.choice([0,1,2,3,999]),rng.choice([0x800000,0x900000]),rng.choice([4,6]),rng.choice([10,15]),rng.choice([2,6]))
            elif op==21:statuses(*[rng.choice([0,1,2,3,4,5,999]) for _ in range(4)],rng.choice([0,1,3,15,MASK]),rng.choice([77,78,MASK]))
            elif op==22:add(op,rng.choice([0,4095,4096,count*4096]),rng.randrange(256))
            else:add(op)
    return (''.join(' '.join(str(x&MASK) for x in row)+'\n' for row in rows).encode(),len(rows))

def prepare(work,count):
    inventory=json.loads(INVENTORY.read_text())
    originals={}
    for item in inventory['sources']:
        data=run(['git','show',inventory['pin']+':'+item['path']])
        assert hashlib.sha256(data).hexdigest()==item['sha256'],item['path']
        originals[item['path']]=data.decode()
    oracle=(HERE/'tests/sext/oracle.rs').read_text().replace('@COUNT@',str(count))
    pattern=r'include!\(concat!\(env!\("SEXT_REF_DIR"\),"/([^"]+)"\)\);'
    def inline(match):
        # Inner module documentation cannot appear inside include! expansion.
        # All executable Rust bodies remain byte-for-byte from the pinned file.
        return re.sub(r'(?m)^//!.*\n','',originals[match[1]])
    oracle,n=re.subn(pattern,inline,oracle)
    assert n==4,n
    destination=work/'oracle.rs';destination.write_text(oracle)
    return destination,inventory

def compare(expected,actual,work,label):
    if expected==actual:return
    (work/(label+'.rust.out')).write_bytes(expected)
    (work/(label+'.cesta.out')).write_bytes(actual)
    a,b=expected.splitlines(),actual.splitlines()
    i=next((i for i,(x,y) in enumerate(zip(a,b)) if x!=y),min(len(a),len(b)))
    raise AssertionError((label,'first differing line',i+1,a[i:i+1],b[i:i+1]))

def mutations(work,source,data,expected):
    original=LayoutText(source.read_text())
    changes=[
        ('quota', 'sext_allocator_reserve', '>= a->quota', '> a->quota'),
        ('unzeroed_commit', 'sext_allocator_commit', '|| !a->frames[index].zeroed', ''),
        ('reclaim_target', 'sext_allocator_finish_reclaim', '|| f.target != b.target', ''),
        ('zero_token', 'sext_allocator_mark_zeroed', '|| a->frames[index].token != token', ''),
        ('foreign_cleanup', 'sext_allocator_finish_cleanup', '&& a->frames[i].target == target', ''),
        ('fault_page_identity', 'sext_pager_step', 'query.page_va != (notice.fault_addr & ~(SEXT_PAGE_BYTES-1))', 'false'),
        ('mapping_rights', 'sext_pager_step', 'query.allowed_rights & mapping.info.rights &', 'query.allowed_rights &'),
        ('uncertain_commit', 'sext_pager_step', 'if (active.may_have_published)', 'if (false)'),
        ('retained_notice', 'sext_pager_step', 'if (active.present)', 'if (false)'),
        ('cleanup_replay', 'sext_pager_cleanup', 'if (p->closed[client].present)', 'if (false)'),
    ]
    results=[]
    for name,function,before,after in changes:
        match=re.search(r'(?m)^.*\b'+function+r'\([^;{}]*\)\s*\{',original)
        assert match,(name,'function anchor')
        end=original.index('\n}',match.end())+2
        body=LayoutText(original[match.start():end])
        assert body.count(before)==1,(name,'mutation anchor',body.count(before))
        candidate=work/(name+'.c')
        candidate.write_text(original[:match.start()]+body.replace(before,after)+original[end:])
        binary=work/name
        run(['gcc','-std=c11','-O2','-Igenerated/include',HERE/'tests/sext/host.c',candidate,'-o',binary])
        changed=run([binary],data=data)
        assert changed!=expected,('semantic mutation survived',name)
        results.append(name)
    return results

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('--cesta',type=Path,required=True)
    ap.add_argument('--counts',default='8')
    ap.add_argument('--seeds',type=int,default=4)
    ap.add_argument('--cases',type=int,default=1000)
    ap.add_argument('--work',type=Path)
    ap.add_argument('--quick',action='store_true')
    ap.add_argument('--mutations',action='store_true')
    ap.add_argument('--cesta-source',type=Path)
    ap.add_argument('--lean-root',type=Path)
    args=ap.parse_args()
    work=args.work or Path(tempfile.mkdtemp(prefix='sext-',dir=HERE/'build'))
    work.mkdir(parents=True,exist_ok=True)
    frozen={p:p.read_bytes() for p in (HERE/'src/sext.cesta',HERE/'src/sext_interface.cesta',
                                       HERE/'generated/include/sext.h',INVENTORY,HERE/'tests/sext/host.c',HERE/'tests/sext/oracle.rs')}
    rows=[];mutation_results=[];admission=[]
    for count in map(int,args.counts.split(',')):
        case=work/str(count);case.mkdir(exist_ok=True)
        reference,inventory=prepare(case,count)
        run(['rustc','--edition=2021','-O','--cap-lints','allow',reference,'-o',case/'rust'])
        data,commands=vectors(count,args.seeds,args.cases)
        (case/'commands.txt').write_bytes(data)
        expected=run([case/'rust'],data=data)
        source=case/'sext.c'
        run([args.cesta,'export-c','src/sext.cesta','--std=c11','-o',source,'--plan',case/'sext.plan',
             '--','-Igenerated/include','-DSEXT_FRAME_COUNT='+str(count)])
        if args.mutations and count==8:
            mutation_results=mutations(case,source,data,expected)
        if args.lean_root:
            if not args.cesta_source:ap.error('--lean-root requires --cesta-source')
            import sys
            sys.path.insert(0,str(args.cesta_source.resolve()/'tests'))
            import ownership_admission
            record,=ownership_admission.parse((case/'sext.plan').read_bytes())
            admission.append(ownership_admission.admit_record(record,case/'Ownership.lean',args.lean_root.resolve()))
        for compiler in ('gcc',) if args.quick else ('gcc','clang'):
            for mode in ('O2',) if args.quick else ('O0','O2','sanitize'):
                flags=['-std=c11','-g0','-Wall','-Wextra','-Wpedantic','-Werror','-Igenerated/include','-DSEXT_FRAME_COUNT='+str(count)]
                flags+=['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all'] if mode=='sanitize' else ['-'+mode]
                binary=case/(compiler+'-'+mode)
                run([compiler,*flags,HERE/'tests/sext/host.c',source,'-o',binary])
                compare(expected,run([binary],data=data),case,binary.name)
                if mode=='O2':
                    authored=case/(binary.name+'-authored')
                    run([args.cesta,'cc','--cc',compiler,*flags,HERE/'tests/sext/host.c',HERE/'src/sext.cesta','-o',authored])
                    assert binary.read_bytes()==authored.read_bytes(),('authored/export mismatch',count,compiler)
                    compare(expected,run([authored],data=data),case,authored.name)
                rows.append(dict(count=count,compiler=compiler,mode=mode,commands=commands,lines=len(expected.splitlines())))
        print('sext frames',count,commands,'commands match the actual Rust core;',len(expected.splitlines()),'trace lines',flush=True)
    assert all(p.read_bytes()==s for p,s in frozen.items()),'qualification inputs changed'
    summary=dict(pin=inventory['pin'],builds=rows,semantic_mutations=mutation_results,admission=admission,source_sha256=hashlib.sha256(frozen[HERE/'src/sext.cesta']).hexdigest(),
                 scope='allocator/pager/codec bodies, full declared state projection and scripted backend effects; main/workspace/native syscall boot not yet covered')
    (work/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')

if __name__=='__main__':main()

