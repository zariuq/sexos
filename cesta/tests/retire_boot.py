#!/usr/bin/env python3
"""Normal kernel+real Sext:all app payloads retire and both BOOT chains run."""
import argparse,hashlib,json,re,subprocess,sys,time
from pathlib import Path
HERE=Path(__file__).resolve().parent.parent
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--reference-tree',type=Path,required=True);ap.add_argument('--kernel',type=Path,required=True)
 ap.add_argument('--probe',type=Path,required=True);ap.add_argument('--sext',type=Path,required=True);ap.add_argument('--limine',type=Path,required=True);ap.add_argument('--work',type=Path,required=True);args=ap.parse_args()
 w=args.work.resolve();w.mkdir(parents=True,exist_ok=False);sys.path.insert(0,str(HERE.parent/'scripts'));from desktop_close_gate import QMP
 expected=json.loads((HERE/'interfaces/kernel_current.source.json').read_text())['expected_boot'];targets=set(expected['apps']);successors=set(expected['dormant'])
 command=['python3',str(HERE/'tools/package_client.py'),'--reference-tree',str(args.reference_tree.resolve()),'--kernel',str(args.kernel.resolve()),'--limine',str(args.limine.resolve()),'--out',str(w/'package')]
 for sub in ('servers','apps'):
  for p in sorted((args.reference_tree/sub).iterdir()):
   if p.is_file():command+=['--replace',str(p.relative_to(args.reference_tree))+'='+str((args.sext if p.name=='sext' else args.probe).resolve())]
 with (w/'package.log').open('wb') as f:subprocess.run(command,cwd=HERE,stdout=f,stderr=subprocess.STDOUT,check=True)
 image=next((w/'package').glob('*.iso'));serial=w/'serial.log';sock=w/'control.sock'
 cmd=['bash',str(HERE/'oracle/run.sh'),str(image),str(serial),'150','--fixed-clock','--qmp-socket',str(sock)]
 observations=[]
 with (w/'qemu.log').open('wb') as f:
  proc=subprocess.Popen(cmd,cwd=HERE,stdout=f,stderr=subprocess.STDOUT);q=None
  try:
   deadline=time.monotonic()+150
   while not sock.exists():
    if proc.poll() is not None or time.monotonic()>deadline:raise RuntimeError('QMP startup failed')
    time.sleep(.05)
   q=QMP(sock)
   while time.monotonic()<deadline:
    text=serial.read_text(errors='replace') if serial.exists() else ''
    if re.search(r'\[retire.probe.failed\]|KERNEL PANIC|panicked at|fault.kill|\[sext.fatal\]',text):raise AssertionError(text[-8000:])
    ready=set(map(int,re.findall(r'\[sext.app.pool.ready\] target=(\d+)[^\n]*ok=1',text)))
    if ready==targets and '[retire.probe.coordinator.done]' in text:break
    if proc.poll() is not None:raise RuntimeError('QEMU stopped before all retirements')
    time.sleep(.05)
   q.cmd('stop');text=serial.read_text(errors='replace')
   apps=[tuple(map(int,r))for r in re.findall(r'\[retire.probe.app\] target=(\d+) task=(\d+) entry=(\d+) data_fresh=1',text)]
   assert len(apps)==len(targets) and {r[0]for r in apps}==targets and len({r[2]for r in apps})==len(apps),apps
   for target,task,address in apps:
    response=q.cmd('human-monitor-command',{'command-line':'x /1gx 0x'+format(address,'x')})
    assert 'Cannot access memory' in response,(target,address,response)
    observations.append({'target':target,'task':task,'private_address':address,'unmapped_response':response})
   q.cmd('quit');assert proc.wait(timeout=10)==0
  finally:
   if q:
    if proc.poll() is None:
     try:q.cmd('quit')
     except (OSError,EOFError):pass
    q.file.close();q.sock.close()
   if proc.poll() is None:proc.terminate();proc.wait(timeout=10)
 text=serial.read_text(errors='replace')
 ready=list(map(int,re.findall(r'\[sext.app.pool.ready\] target=(\d+)[^\n]*ok=1',text)));assert len(ready)==len(targets) and set(ready)==targets,ready
 retired=list(map(int,re.findall(r'\[app.private.retired\] target=(\d+)[^\n]*acked=31 required=31 all_flushed=1 pool_reusable=1',text)));assert len(retired)==len(targets) and set(retired)==targets,retired
 revoked=list(map(int,re.findall(r'\[app.caps.revoked\] target=(\d+)[^\n]*remaining=0',text)));assert len(revoked)==len(targets) and set(revoked)==targets,revoked
 activated=[tuple(map(int,r))for r in re.findall(r'\[retire.probe.activated\] predecessor=(\d+) target=(\d+) task=(\d+)',text)]
 assert len(activated)==len(successors) and {r[1]for r in activated}==successors,activated
 acknowledgements=[tuple(map(int,r))for r in re.findall(r'\[app.service.ack\] target=(\d+) task=(\d+) holder=(\d+) role=(\d+) acked=(\d+) required=31',text)]
 for target in targets:
  a=[r for r in acknowledgements if r[0]==target];assert len(a)==5 and {r[3]for r in a}=={1,2,4,8,16} and a[-1][4]==31,(target,a)
 assert text.count('[retire.probe.coordinator.done] activations='+str(len(successors)))==1
 (w/'summary.json').write_text(json.dumps({'inputs':{'kernel':digest(args.kernel),'probe':digest(args.probe),'sext':digest(args.sext),'iso':digest(image)},'retired':retired,'pool_ready':ready,'activated':activated,'acks':acknowledgements,'hardware_private_absence':observations,
  'scope':'normal kernel+real Cesta Sext;controlled service ACK probes,actual17 app executions/14 successor CPU launches,private virtual absence,revocation andworkspace reuse. No GUI-service cleanup/disk/SMP/full-userland claim.'},indent=2)+'\n')
 print('CPU retirement passed:',len(retired),'app pools,',len(activated),'successor launches,all85 ACKs and17 private addresses absent',flush=True)
if __name__=='__main__':main()
