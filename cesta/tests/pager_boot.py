#!/usr/bin/env python3
"""Real page-fault/retry/reclaim/idle-timer probes with the production Sext ELF."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys
import time

HERE=Path(__file__).resolve().parent.parent


def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()


def idle_observations(commands,w,serial,kernel):
    """Observe a pending timed wait and final idle on the dedicated kernel stack."""
    symbols={}
    for line in subprocess.check_output(['nm','-S',str(kernel)],text=True).splitlines():
        words=line.split()
        if words and words[-1] in ('kernel_idle_stack','kernel_idle_stack_top','kernel_idle_loop'):
            symbols[words[-1]]=(int(words[0],16),int(words[1],16) if len(words)==4 else 0)
    low=symbols['kernel_idle_stack'][0];high=symbols['kernel_idle_stack_top'][0]
    start,size=symbols['kernel_idle_loop'];assert low<high and size>0
    sys.path.insert(0,str(HERE.parent/'scripts'));from desktop_close_gate import QMP
    sock=w/'control.sock';cmd=commands[:6]+['--qmp-socket',str(sock)]
    # Keep the guest alive for observation. The controller quits its own QEMU
    # after checking the final parked state; raw serial evidence is unchanged.
    with (w/'qemu.log').open('wb') as log:
        proc=subprocess.Popen(cmd,cwd=HERE,stdout=log,stderr=subprocess.STDOUT)
        q=None;observations=[]
        try:
            deadline=time.monotonic()+120
            while not sock.exists():
                if proc.poll() is not None or time.monotonic()>deadline:raise RuntimeError('QMP startup failed')
                time.sleep(.05)
            q=QMP(sock)
            while time.monotonic()<deadline:
                text=serial.read_text(errors='replace') if serial.exists() else ''
                if re.search(r'KERNEL PANIC|fault\.kill|\[sext.fatal\]|\[pager.probe.failed\]',text):raise AssertionError(text[-3000:])
                pending=set()
                for match in re.finditer(r'\[scheduler.self_wait\.(park|expired)\] task=(\d+)',text):
                    if match[1]=='park':pending.add(int(match[2]))
                    else:pending.discard(int(match[2]))
                done=all('[paging.target.cleanup] target='+str(t) in text and '[pager.probe.done] target='+str(t) in text for t in (10,15))
                phase='final' if done else 'timed' if pending else None
                if phase and not any(o['phase']==phase for o in observations):
                    q.cmd('stop');regs=q.cmd('human-monitor-command',{'command-line':'info registers'})
                    sp=re.search(r'\bRSP\s*=\s*([0-9a-f]+)',regs,re.I);ip=re.search(r'\bRIP\s*=\s*([0-9a-f]+)',regs,re.I)
                    # Re-read evidence after stop; a pre-stop pending set may
                    # have expired while QMP was acquiring the CPU snapshot.
                    stopped=serial.read_text(errors='replace')
                    pending=set()
                    for event in re.finditer(r'\[scheduler.self_wait\.(park|expired)\] task=(\d+)',stopped):
                        if event[1]=='park':pending.add(int(event[2]))
                        else:pending.discard(int(event[2]))
                    stopped_done=all('[paging.target.cleanup] target='+str(t) in stopped and '[pager.probe.done] target='+str(t) in stopped for t in (10,15))
                    phase_current=(phase=='timed' and bool(pending) and not stopped_done) or (phase=='final' and stopped_done)
                    valid=phase_current and sp and ip and low<=int(sp[1],16)<high and start<=int(ip[1],16)<start+size and 'CPL=0' in regs and 'HLT=1' in regs
                    if valid:
                        observations.append({'phase':phase,'rsp':int(sp[1],16),'rip':int(ip[1],16),'pending_tasks':sorted(pending),'cpl':0,'halted':True})
                        (w/('idle-'+phase+'.registers')).write_text(regs)
                    q.cmd('cont')
                    if valid and phase=='final':break
                time.sleep(.02)
            assert any(o['phase']=='final' for o in observations),observations
            timed=re.findall(r'\[scheduler.idle.timed\] task=(\d+) deadline=(\d+) now=(\d+) rsp=(0x[0-9a-f]+)',serial.read_text(errors='replace'))
            assert len(timed)==1,timed
            task,at,now,sp=timed[0];assert int(task) in (10,15) and int(now)<int(at) and low<=int(sp,16)<high,timed
            observations.append({'phase':'timed-entry','rsp':int(sp,16),'task':int(task),'deadline':int(at),'now':int(now),'provider':'guest CPU stack register and live task state'})
            q.cmd('quit');assert proc.wait(timeout=10)==0
            return {'stack_start':low,'stack_end':high,'loop_start':start,'loop_size':size,'observations':observations}
        finally:
            if q:q.file.close();q.sock.close()
            if proc.poll() is None:proc.terminate();proc.wait(timeout=10)

def main():
    ap=argparse.ArgumentParser();ap.add_argument('--reference-tree',type=Path,required=True)
    ap.add_argument('--kernel',type=Path,required=True);ap.add_argument('--sext',type=Path,required=True)
    ap.add_argument('--idle-snapshot',action='store_true');ap.add_argument('--probe',type=Path,required=True);ap.add_argument('--limine',type=Path,required=True);ap.add_argument('--work',type=Path,required=True);args=ap.parse_args()
    w=args.work.resolve();w.mkdir(parents=True,exist_ok=False)
    before={n:digest(getattr(args,n).resolve()) for n in ('kernel','sext','probe')}
    commands=['python3',str(HERE/'tools/package_client.py'),'--reference-tree',str(args.reference_tree.resolve()),'--kernel',str(args.kernel.resolve()),'--limine',str(args.limine.resolve()),'--out',str(w/'package')]
    programs=[*sorted((args.reference_tree/'servers').iterdir()),*sorted((args.reference_tree/'apps').iterdir())]
    for p in programs:
        if p.is_file():commands+=['--replace',str(p.relative_to(args.reference_tree))+'='+str((args.sext if p.name=='sext' else args.probe).resolve())]
    with (w/'package.log').open('wb') as log:subprocess.run(commands,cwd=HERE,check=True,stdout=log,stderr=subprocess.STDOUT)
    serial=w/'serial.log';image=next((w/'package').glob('*.iso'));start=time.monotonic()
    commands=['bash',str(HERE/'oracle/run.sh'),str(image),str(serial),'120','--fixed-clock',
              '--until','[paging.target.cleanup] target=10','--until','[paging.target.cleanup] target=15',
              '--until','[pager.probe.done] target=10','--until','[pager.probe.done] target=15']
    idle=idle_observations(commands,w,serial,args.kernel.resolve()) if args.idle_snapshot else None
    if not args.idle_snapshot:
        with (w/'qemu.log').open('wb') as log:subprocess.run(commands,cwd=HERE,check=True,stdout=log,stderr=subprocess.STDOUT)
    text=serial.read_text(errors='replace')
    for bad in ('KERNEL PANIC','fault.kill','[sext.fatal]','[pager.probe.failed]'):assert bad not in text,(bad,text[-6000:])
    resumed=[tuple(map(int,x)) for x in re.findall(r'\[pager.probe.resumed\] target=(10|15) cycle=(\d+) zero=1 registers=1 write=1',text)]
    assert sorted(resumed)==[(10,1),(10,2),(15,1),(15,2)],resumed
    assert set(map(int,re.findall(r'\[pager.probe.released\] target=(10|15) request=1 correlated=1',text)))=={10,15}
    done={(int(target),int(timed)) for target,timed in re.findall(r'\[pager.probe.done\] target=(10|15) cycles=2 timed_wake=([01])',text)}
    assert done=={(10,0),(15,1)},done
    commits=re.findall(r'\[paging.leaf.commit\] token=(\d+) target=(10|15) page=(0x[0-9a-f]+) frame=(\d+) rights=6 pkey=(\d+) zero=1 epoch=(\d+) runtime_alloc=0',text)
    assert len(commits)==4 and len({x[0] for x in commits})==4 and all(x[1]==x[4] and int(x[5])>0 for x in commits),commits
    partitions={10:range(4,8),15:range(0,4)}
    assert all(int(row[3]) in partitions[int(row[1])] for row in commits),commits
    assert all(len({int(row[3]) for row in commits if int(row[1])==target})==1 for target in partitions),commits
    completed=re.findall(r'\[paging.fault.completed\] target=(10|15) task=(\d+) token=(\d+) resume=1 saved_rip=(0x[0-9a-f]+)',text)
    assert len(completed)==4 and {c[2] for c in completed}=={c[0] for c in commits},completed
    reclaims=re.findall(r'\[paging.leaf.reclaim\] token=(\d+) target=(10|15) page=(0x[0-9a-f]+) frame=(\d+) cleared=1 tlb=1 epoch=(\d+) runtime_alloc=0',text)
    assert len(reclaims)==2 and len({r[0] for r in reclaims})==2 and all(int(r[4])>0 for r in reclaims),reclaims
    assert all(int(row[3]) in partitions[int(row[1])] for row in reclaims),reclaims
    cleanup=re.findall(r'\[paging.target.cleanup\] target=(10|15) task=(\d+) frames=(0x[0-9a-f]+) closed=1 detached=1 all_flushed=1 epoch=(\d+) runtime_alloc=0',text)
    assert sorted(int(row[0]) for row in cleanup)==[10,15] and all(int(row[3])>0 for row in cleanup),cleanup
    assert '[scheduler.self_wait.expired]' in text
    assert all(digest(getattr(args,n).resolve())==h for n,h in before.items())
    summary={'idle':idle,'inputs':before,'image_sha256':digest(image),'resumed':resumed,'commits':commits,'completed':completed,'reclaims':reclaims,'partitions':{'10':[4,8],'15':[0,4]},'done':sorted(done),'cleanup':cleanup,'seconds':time.monotonic()-start,'scope':'actual CPU faults/resumption, deterministic per-range frame allocation, epoch-recorded release/reuse and terminal range cleanup, one unambiguous syscall deadline receipt, through the production kernel and Sext. Domain 10 is also a lifecycle service and is not claimed to become timer-idle in this probe. Other programs are inert probes; this does not qualify the ordinary full userland.'}
    (w/'summary.json').write_text(json.dumps(summary,indent=2)+'\n');print('hardware pager probe passed:4 partitioned faults,2 releases/reuses,1 deadline receipt,2 closed targets')


if __name__=='__main__':main()
