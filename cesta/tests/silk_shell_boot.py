#!/usr/bin/env python3
"""Qualify the joined all-Cesta current userland image in QEMU."""
import argparse,hashlib,json,re,subprocess,time
from pathlib import Path
HERE=Path(__file__).resolve().parent.parent
REPLACEMENTS={
 'apps/kaleidoscope':'kaleidoscope','servers/silkbar':'silkbar','servers/sexinput':'sexinput','servers/sexstore':'sexstore',
 'servers/sexdisplay':'sexdisplay','servers/sexbell':'sexbell','servers/linen':'linen','servers/quil':'quil','apps/spindle':'spindle',
 'apps/purple-scanout':'purple-scanout','servers/sexfiles':'sexfiles','apps/sexdrive':'sexdrive','servers/sexusb':'sexusb',
 'servers/sexnet':'sexnet','servers/sext':'sext','servers/silk-shell':'silk-shell'}
MARKERS=('[sexdisplay.init.start]','[sexinput.ready]','[silkbar.ready]','[sexbell.ready]','[sexnet.passive.ready]','[sext.ready]',
 '[quil.ready]','[sexstore.ready]','[spindle.ready]','[sexfiles.ready]','[linen.ready]','[sexdisplay.ready]',
 '[display.ready.sent] to=silk-shell version=1','[display.ready.sent] to=silkbar version=1',
 '[display.ready.accepted] client=silk-shell caller=1 version=1','[display.ready.accepted] client=silkbar caller=1 version=1',
 '[silk-shell.boot.ui.ready] surfaces=2 focus=201','[silk-shell.ready]')
def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--reference-tree',type=Path,required=True);ap.add_argument('--limine',type=Path,required=True);ap.add_argument('--work',type=Path,required=True);ap.add_argument('--kernel',type=Path);ap.add_argument('--seconds',type=int,default=45);ap.add_argument('--nvme-image',type=Path);ap.add_argument('--smp',type=int);ap.add_argument('--accel',choices=('tcg','kvm'));ap.add_argument('--until-ready',action='store_true');ap.add_argument('--post-ready',type=int,default=0);args=ap.parse_args();assert args.post_ready>=0;assert args.until_ready or args.post_ready==0;work=args.work.resolve();work.mkdir(parents=True,exist_ok=False);command=['python3',str(HERE/'tools/package_client.py'),'--reference-tree',str(args.reference_tree.resolve()),'--limine',str(args.limine.resolve()),'--out',str(work/'package')]
 binaries={}
 for path,target in REPLACEMENTS.items():binary=(HERE/'build'/target/target).resolve();assert binary.is_file(),binary;binaries[path]=digest(binary);command+=['--replace',path+'='+str(binary)]
 if args.kernel:
  kernel=args.kernel.resolve();assert kernel.is_file(),kernel;binaries['sexos-kernel']=digest(kernel);command+=['--kernel',str(kernel)]
 package=subprocess.run(command,cwd=HERE,capture_output=True,text=True)
 if package.returncode:raise RuntimeError(package.stderr.strip() or package.stdout.strip())
 image=Path(package.stdout.strip());serial=work/'serial.log';started=time.monotonic();run=['bash',str(HERE/'oracle/run.sh'),str(image),str(serial),str(args.seconds)];
 if args.accel=='kvm':run+=['--accel','kvm']
 else:
  run+=['--fixed-clock']
  if args.accel=='tcg':run+=['--accel','tcg']
 if args.until_ready:
  for marker in MARKERS:run+=['--until',marker]
  if args.post_ready:run+=['--post-ready',str(args.post_ready)]
 if args.smp:
  assert args.kernel and 1<=args.smp<=128
  run+=['--smp',str(args.smp)]
 if args.nvme_image:run+=['--nvme-image',str(args.nvme_image.resolve())]
 subprocess.run(run,cwd=HERE,check=True,capture_output=True);text=serial.read_text(errors='replace');missing=[m for m in MARKERS if m not in text];assert not missing,missing;ready=text.index('[silk-shell.ready]');after=text[ready:]
 forbidden=('fault.kill','paging.fault.terminal','KERNEL PANIC','#PF','#GP','pdx.opcode.unknown');bad={term:after.count(term) for term in forbidden if term in after};assert not bad,bad;whole_unknown=text.count('pdx.opcode.unknown');assert not args.kernel or whole_unknown==0,whole_unknown;epochs=[tuple(map(int,row)) for row in re.findall(r'\[kernel\.epoch\.commit\] epoch=(\d+) count=(\d+) kind=(\d+) subject=(\d+) revision=(\d+)',text)];assert not args.kernel or (epochs and all(epoch==count for epoch,count,_,_,_ in epochs) and max(count for _,count,_,_,_ in epochs)>=512),epochs;manifest=json.loads((work/'package/manifest.json').read_text())
 smp=re.search(r'\[smp\.parked\] supported=(\d+) cpus=(\d+) aps=(\d+) bsp_lapic=(\d+) generation=(\d+) x2apic=(\d+)',text)
 if args.smp:assert smp and tuple(map(int,smp.groups()[:3]))==(1,args.smp,args.smp-1) and int(smp[5])==0,smp.groups() if smp else None
 expected=set(REPLACEMENTS)|({'sexos-kernel'} if args.kernel else set());assert set(manifest['replacements'])==expected;assert set(manifest['changed_files'])<=expected
 elapsed=time.monotonic()-started;summary={'scope':('current all-Cesta kernel and userland image: 17 replacements, permanently parked APs, BSP-only epoch execution, joined shell readiness and sustained main loop' if args.kernel else 'current all-Cesta userland image: 16 replacements, joined shell readiness and sustained main loop; Rust kernel remains the compatibility cohort'),'image_sha256':digest(image),'binaries':binaries,'markers':list(MARKERS),'seconds':elapsed,'requested_seconds':args.seconds,'termination':('ready-plus-'+str(args.post_ready)+'s' if args.until_ready else 'full-soak'),'accelerator':args.accel or 'default-tcg','bytes_after_ready':len(after),'forbidden_after_ready':bad,'unknown_opcodes_whole_boot':whole_unknown,'epoch_checkpoints':epochs,'smp':tuple(map(int,smp.groups())) if smp else None,'nvme':bool(args.nvme_image)}
 (work/'summary.json').write_text(json.dumps(summary,indent=2)+'\n');print('all-Cesta current boot passed:',len(expected),'replacements;',len(MARKERS),'markers;',f'{elapsed:.2f}','seconds;',args.smp or 1,'cores; 0 terminal faults; 0 unknown opcodes after ready')
if __name__=='__main__':main()
