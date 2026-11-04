#!/usr/bin/env python3
"""Execute the actual shared history decoder through real held kernel queries."""
import argparse,hashlib,json,re,subprocess,sys,time
from pathlib import Path
HERE=Path(__file__).resolve().parent.parent
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--reference-tree',type=Path,required=True);ap.add_argument('--kernel',type=Path,required=True)
 ap.add_argument('--probe',type=Path,required=True);ap.add_argument('--limine',type=Path,required=True);ap.add_argument('--work',type=Path,required=True);args=ap.parse_args()
 w=args.work.resolve();w.mkdir(parents=True,exist_ok=False);sys.path.insert(0,str(HERE.parent/'scripts'));from desktop_close_gate import QMP
 expected=json.loads((HERE/'interfaces/kernel_current.source.json').read_text())['expected_boot']
 # Compile the real descriptive catalog independently;it never supplies authority.
 (w/'catalog.rs').write_bytes((HERE.parent/'crates/sex-pdx/src/app_catalog.rs').read_bytes())
 (w/'catalog_probe.rs').write_text('#[path="catalog.rs"]mod catalog;fn main(){for r in catalog::spindle_views(){println!("{}",r.target);}}\n')
 subprocess.run(['rustc','--edition=2021','--cap-lints','allow',str(w/'catalog_probe.rs'),'-o',str(w/'catalog_probe')],cwd=HERE,check=True)
 targets=list(map(int,subprocess.check_output([w/'catalog_probe']).split()))
 command=['python3',str(HERE/'tools/package_client.py'),'--reference-tree',str(args.reference_tree.resolve()),'--kernel',str(args.kernel.resolve()),'--limine',str(args.limine.resolve()),'--out',str(w/'package')]
 for sub in ('servers','apps'):
  for p in sorted((args.reference_tree/sub).iterdir()):
   if p.is_file():command+=['--replace',str(p.relative_to(args.reference_tree))+'='+str(args.probe.resolve())]
 with (w/'package.log').open('wb') as f:subprocess.run(command,cwd=HERE,stdout=f,stderr=subprocess.STDOUT,check=True)
 image=next((w/'package').glob('*.iso'));serial=w/'serial.log';sock=w/'control.sock'
 cmd=['bash',str(HERE/'oracle/run.sh'),str(image),str(serial),'90','--fixed-clock','--qmp-socket',str(sock)]
 with (w/'qemu.log').open('wb') as f:
  proc=subprocess.Popen(cmd,cwd=HERE,stdout=f,stderr=subprocess.STDOUT);q=None
  try:
   deadline=time.monotonic()+90
   while not sock.exists():
    if proc.poll() is not None or time.monotonic()>deadline:raise RuntimeError('QMP startup failed')
    time.sleep(.05)
   q=QMP(sock)
   while time.monotonic()<deadline:
    text=serial.read_text(errors='replace') if serial.exists() else ''
    if re.search(r'\[history.probe.failed\]|KERNEL PANIC|panicked at|fault.kill',text):raise AssertionError(text[-5000:])
    if '[history.probe.done]' in text and text.count('[history.probe.unheld]')==expected['initial']-1:break
    if proc.poll() is not None:raise RuntimeError('QEMU stopped before all roles')
    time.sleep(.05)
   q.cmd('quit');assert proc.wait(timeout=10)==0
  finally:
   if q:
    if proc.poll() is None:
     try:q.cmd('quit')
     except (OSError,EOFError):pass
    q.file.close();q.sock.close()
   if proc.poll() is None:proc.terminate();proc.wait(timeout=10)
 text=serial.read_text(errors='replace')
 held=[tuple(map(int,r)) for r in re.findall(r'\[boot.history.participant\] index=(\d+) target=(\d+) task=(\d+) holder=11 immutable=1 runtime_mint=0',text)]
 observed=[tuple(map(int,r)) for r in re.findall(r'\[history.probe.participant\] index=(\d+) target=(\d+) task=(\d+)',text)]
 assert held==observed and [r[0] for r in observed]==list(range(len(targets))) and [r[1] for r in observed]==targets
 assert all(r[2]>0 for r in observed) and len({r[2] for r in observed})==len(targets)
 assert text.count('[history.probe.unheld] rejected=1 partial_published=0')==expected['initial']-1
 assert text.count('[history.probe.done] holder=11 logical=12 participants='+str(len(targets))+' extent=1808:18 alias_denied=1')==1
 (w/'summary.json').write_text(json.dumps({'inputs':{'kernel':digest(args.kernel),'probe':digest(args.probe),'iso':digest(image)},'held_roster':held,'decoded_roster':observed,'unheld_callers':expected['initial']-1,
   'scope':'actual BOOT held roster/native query/client asm/shared decoder with14 unheld callers. Inert payloads;no disk recovery,retirement or task-inequality CPU claim.'},indent=2)+'\n')
 print('history CPU probe passed:',len(targets),'held participants,full client decode,14 unheld callers',flush=True)
if __name__=='__main__':main()
