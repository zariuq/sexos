#!/usr/bin/env python3
"""Actual kernel RPC against the Lean window model after every operation.

The projection observes every call identity/peer/phase/value and queued payload.
Transport queuing is driven by a small Lean replay adapter; the imported proved
core is CorrelatedReplies. CPU entry, legacy reply slots and hardware are outside
this comparison. This is an implementation test, not a C refinement proof.
"""
import argparse
import hashlib
import json
from pathlib import Path
import random
import re
import subprocess
import tempfile

HERE=Path(__file__).resolve().parent.parent

def run(args,*,cwd=HERE,data=None):
    result=subprocess.run(list(map(str,args)),cwd=cwd,input=data,capture_output=True)
    if result.returncode:
        raise RuntimeError((args,result.returncode,result.stdout[-2000:],result.stderr[-4000:]))
    return result.stdout

def cases(domains,grants,capacity):
    events=[(d,0,0,0,0) for d in domains]
    issued={d:0 for d in domains}
    for holder,slot,peer in grants+[(d,2,d) for d in domains]:
        issued[holder]+=1;token=issued[holder]
        stranger=next(d for d in domains if d!=peer)
        events += [(holder,1,slot,0x600,token),(holder,4,token,0,0),(peer,3,holder,token,99),
                   (peer,2,0,0,0),(stranger,3,holder,token,99),(peer,3,holder,token,77),
                   (peer,3,holder,token,88),(holder,4,token,0,0),(holder,4,token,0,0)]
    holder,slot,peer=grants[0]
    start=issued[holder]
    events += [(holder,1,slot,0x601,i) for i in range(capacity+2)]
    events += [(peer,2,0,0,0) for _ in range(capacity)]
    events += [(peer,3,holder,start+i+1,100+i) for i in reversed(range(capacity))]
    events += [(holder,4,start+i+1,0,0) for i in range(capacity)]
    issued[holder]+=capacity
    # Cancel queued calls until the receiver ring fills with stale envelopes.
    for _ in range(256):
        issued[holder]+=1;token=issued[holder]
        events += [(holder,1,slot,0x602,token),(holder,5,token,0,0)]
    events += [(holder,1,slot,0x603,1),(peer,2,0,0,0)]
    issued[holder]+=1;token=issued[holder]
    events += [(holder,1,slot,0x604,3),(peer,2,0,0,0),(holder,5,token,0,0),
               (peer,3,holder,token,33),(holder,4,token,0,0)]
    rng=random.Random(0xC011E1)
    for _ in range(900):
        actor=rng.choice(domains);operation=rng.choices(range(6),[1,8,5,4,3,3])[0]
        if operation==1:
            possible=[(s,p) for h,s,p in grants if h==actor]+[(2,actor),(31,actor)]
            chosen=rng.choice(possible)
            a,b,c=chosen[0],rng.choice([0,0x610,0x611]),rng.randrange(1000)
        elif operation==3:a,b,c=rng.choice(domains),rng.randrange(1,issued.get(actor,0)+10),rng.randrange(1000)
        else:a,b,c=rng.randrange(0,issued.get(actor,0)+10),0,0
        events.append((actor,operation,a,b,c))
    return events

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--cesta',type=Path,required=True)
    parser.add_argument('--lean-root',type=Path,required=True);parser.add_argument('--lean-memory-mb',type=int)
    args=parser.parse_args();project=args.lean_root.resolve()
    work=Path(tempfile.mkdtemp(prefix='kernel-rpc-model-',dir=HERE/'build'))
    for name in ('kernel_domains','kernel_syscalls','kernel_rpc'):
        (work/(name+'.c')).write_bytes(run([args.cesta,'export-c','src/'+name+'.cesta','--',
                                           '-Iruntime/kernel','-Igenerated/include']))
    run(['lake','build','Mettapedia.Machines.Kernel.CorrelatedReplies'],cwd=project)
    domain_rows=run([args.cesta,'tables','rows','interfaces/kernel.data','Domains','id','name']).decode()
    entries=re.findall(r'\(Row (\d+) (\w+)\)',domain_rows)
    domains=[int(number) for number,_ in entries];names={name:int(number) for number,name in entries}
    grant_rows=run([args.cesta,'tables','rows','interfaces/kernel.data','RingGrants','holder','slot','target']).decode()
    grants=[(names[h],int(slot),names[t]) for h,slot,t in re.findall(r'\(Row (\w+) (\d+) (\w+)\)',grant_rows)]
    records={}
    original={name:(work/(name+'.c')).read_text() for name in ('kernel_domains','kernel_syscalls','kernel_rpc')}
    for capacity in (1,8):
        events=cases(domains,grants,capacity)
        data=''.join(' '.join(map(str,event))+'\n' for event in events).encode()
        (work/f'input-{capacity}.txt').write_bytes(data)
        command=['lake','env','lean']
        if args.lean_memory_mb:command += ['-M',str(args.lean_memory_mb)]
        expected=run([*command,'--run',HERE/'tests/kernel_rpc_model/reference.lean',str(capacity)],cwd=project,data=data)
        (work/f'lean-{capacity}.out').write_bytes(expected)
        for compiler in ('gcc','clang'):
            for mode in ('O0','O2','sanitize'):
                flags=['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all'] if mode=='sanitize' else ['-'+mode]
                binary=work/f'{compiler}-{mode}-{capacity}'
                run([compiler,'-std=c11','-Wall','-Wextra','-Wpedantic','-Werror',*flags,
                     '-ffunction-sections','-fdata-sections','-Wl,--gc-sections',f'-DKERNEL_RPC_CAPACITY={capacity}',
                     '-I',work,'-Igenerated/include','-Iruntime/kernel','tests/kernel_rpc_model/host.c','-o',binary])
                actual=run([binary],data=data)
                if actual!=expected:
                    (work/f'{compiler}-{mode}-{capacity}.out').write_bytes(actual)
                    first=next((i for i,(a,b) in enumerate(zip(actual.splitlines(),expected.splitlines())) if a!=b),0)
                    raise AssertionError((work,compiler,mode,capacity,first+1,actual.splitlines()[first][:500],expected.splitlines()[first][:500]))
        records[str(capacity)]={'operations':len(events),'expected_sha256':hashlib.sha256(expected).hexdigest()}
        print('RPC kernel/model:',len(events),'operations, capacity',capacity,'all compiler modes agree')
    mutations=[
        ('kernel_rpc','c->request->value = c->frame->rdx;','c->request->value = c->frame->rdx + 1;'),
        ('kernel_rpc','caller->rpc_next = id;','caller->rpc_next = id + 1;'),
        ('kernel_rpc','c->request->peer == c->peer->id','true'),
        ('kernel_rpc','c->request->state == RPC_HEARD','true'),
        ('kernel_rpc','c->request->state == RPC_READY','true'),
        ('kernel_rpc','if (opcode == 0) return;','if (false) return;'),
        ('kernel_rpc','.state = RPC_FREE','.state = RPC_HEARD'),
        ('kernel_syscalls','if (kernel_rpc_claim(d, out)) return true;','if (true) return true;'),
    ]
    for index,(name,needle,replacement) in enumerate(mutations):
        assert needle in original[name],('missing mutation point',name,needle)
        path=work/(name+'.c');path.write_text(original[name].replace(needle,replacement))
        binary=work/f'mutant-{index}'
        run(['gcc','-std=c11','-O2','-ffunction-sections','-fdata-sections','-Wl,--gc-sections',
             '-I',work,'-Igenerated/include','-Iruntime/kernel','tests/kernel_rpc_model/host.c','-o',binary])
        result=subprocess.run([binary],cwd=HERE,input=data,capture_output=True)
        assert result.returncode or result.stdout!=expected,('mutation survived',name,needle)
        path.write_text(original[name])
    print('RPC kernel/model:',len(mutations),'semantic code mutations detected')
    records['mutations']=len(mutations)
    (work/'summary.json').write_text(json.dumps(records,indent=2)+'\n')

if __name__=='__main__':main()
