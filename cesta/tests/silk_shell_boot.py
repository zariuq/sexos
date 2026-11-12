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
def digest(path):
 h=hashlib.sha256()
 with path.open('rb') as stream:
  for chunk in iter(lambda:stream.read(1024*1024),b''):h.update(chunk)
 return h.hexdigest()
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--reference-tree',type=Path,required=True);ap.add_argument('--limine',type=Path,required=True);ap.add_argument('--work',type=Path,required=True);ap.add_argument('--kernel',type=Path);ap.add_argument('--build-root',type=Path,default=HERE/'build');ap.add_argument('--seconds',type=int,default=45);ap.add_argument('--nvme-image',type=Path);ap.add_argument('--network',action='store_true');ap.add_argument('--usb-keyboard',action='store_true');ap.add_argument('--smp',type=int);ap.add_argument('--accel',choices=('tcg','kvm'));ap.add_argument('--until-ready',action='store_true');ap.add_argument('--post-ready',type=int,default=0);ap.add_argument('--worker-probe',action='store_true');ap.add_argument('--record-probe',action='store_true');args=ap.parse_args();assert args.post_ready>=0;assert args.until_ready or args.post_ready==0;markers=MARKERS+(('[sext.worker.probe]',) if args.worker_probe else ())+(('[spindle.record.probe.done]',) if args.record_probe else ());work=args.work.resolve();work.mkdir(parents=True,exist_ok=False);command=['python3',str(HERE/'tools/package_client.py'),'--reference-tree',str(args.reference_tree.resolve()),'--limine',str(args.limine.resolve()),'--out',str(work/'package')]
 binaries={}
 for path,target in REPLACEMENTS.items():build_target='sext-worker-probe' if args.worker_probe and target=='sext' else ('spindle-record-probe' if args.record_probe and target=='spindle' else target);binary=(args.build_root/build_target/build_target).resolve();assert binary.is_file(),binary;binaries[path]=digest(binary);command+=['--replace',path+'='+str(binary)]
 if args.kernel:
  kernel=args.kernel.resolve();assert kernel.is_file(),kernel;binaries['sexos-kernel']=digest(kernel);command+=['--kernel',str(kernel)]
 nvme_hash=digest(args.nvme_image) if args.nvme_image else None
 package=subprocess.run(command,cwd=HERE,capture_output=True,text=True)
 if package.returncode:raise RuntimeError(package.stderr.strip() or package.stdout.strip())
 image=Path(package.stdout.strip());serial=work/'serial.log';started=time.monotonic();run=['bash',str(HERE/'oracle/run.sh'),str(image),str(serial),str(args.seconds)];
 if args.accel=='kvm':run+=['--accel','kvm']
 else:
  run+=['--fixed-clock']
  if args.accel=='tcg':run+=['--accel','tcg']
 if args.until_ready:
  for marker in markers:run+=['--until',marker]
  if args.post_ready:run+=['--post-ready',str(args.post_ready)]
 if args.smp:
  assert args.kernel and 1<=args.smp<=128
  run+=['--smp',str(args.smp)]
 if args.nvme_image:run+=['--nvme-image',str(args.nvme_image.resolve())]
 if args.network:run+=['--network']
 if args.usb_keyboard:run+=['--usb-keyboard']
 observed=subprocess.run(run,cwd=HERE,capture_output=True)
 (work/'qemu.log').write_bytes(observed.stdout+observed.stderr)
 if args.nvme_image:assert digest(args.nvme_image)==nvme_hash,'snapshot boot changed its NVMe input'
 if observed.returncode:raise RuntimeError(observed.stderr.decode(errors='replace').strip())
 text=serial.read_text(errors='replace');missing=[m for m in markers if m not in text];assert not missing,missing;worker_starts=re.findall(r'\[program\.worker\.start\] target=(\d+) run=(\d+) worker=(\d+) core=0 reused=(\d+) slot=(\d+)',text);assert not args.worker_probe or (len(worker_starts)==2 and worker_starts[0][3]=='0' and worker_starts[1][3]=='1' and worker_starts[0][4]==worker_starts[1][4]),worker_starts;record_starts=re.findall(r'\[record\.start\] pd=(\d+) task=(\d+) slot=(\d+)',text);record_census=re.findall(r'\[records\.census\] live=(\d+) started=(\d+) ended=(\d+) capacity=(\d+) pool_free=(\d+) pool_total=(\d+) switches=(\d+)',text);hum_alive=re.findall(r'\[record\.hum\.alive\] seed=(0x[0-9a-f]+) cycle=(\d+)',text);hum_exits=re.findall(r'\[record\.hum\.exit\] seed=(0x[0-9a-f]+) cycles=(\d+)',text);record_seeds=re.findall(r'\[record\.hum\.start\] seed=(0x[0-9a-f]+)',text);record_live_transitions=[int(row[0]) for i,row in enumerate(record_census) if i==0 or row[0]!=record_census[i-1][0]];record_live_transitions=record_live_transitions[1:] if record_live_transitions and record_live_transitions[0]==0 else record_live_transitions;record_live_milestones=[live for live in record_live_transitions if live in (0,1,15)];record_ok=len(record_starts)==17 and record_starts[0][0]==record_starts[-1][0] and record_starts[0][2]==record_starts[-1][2] and len({row[1] for row in record_starts})==17 and [int(row[2]) for row in record_starts[1:16]]==list(range(2,16))+[0] and len(hum_alive)>=34 and len(hum_exits)==17 and len(record_seeds)==17 and record_seeds[0]==record_seeds[-1] and record_live_milestones==[1,0,15,0,1,0] and record_census[-1][1:3]==('17','17') and record_census[-1][4]=='1024' and 'record.hum.corrupt' not in text;assert not args.record_probe or record_ok,(record_starts,record_census,hum_alive,hum_exits,record_seeds);ready=text.index('[silk-shell.ready]');after=text[ready:]
 forbidden=('fault.kill','paging.fault.terminal','KERNEL PANIC','#PF','#GP','pdx.opcode.unknown','cesta.client.fault');bad={term:after.count(term) for term in forbidden if term in after};assert not bad,bad;whole_unknown=text.count('pdx.opcode.unknown');assert not args.kernel or whole_unknown==0,whole_unknown;epoch_failures=text.count('[sexfiles.epoch.failed]');assert not args.nvme_image or epoch_failures==0,epoch_failures;epochs=[tuple(map(int,row)) for row in re.findall(r'\[kernel\.epoch\.commit\] epoch=(\d+) count=(\d+) kind=(\d+) subject=(\d+) revision=(\d+)',text)];assert not args.kernel or (epochs and all(epoch==count for epoch,count,_,_,_ in epochs) and max(count for _,count,_,_,_ in epochs)>=512),epochs;manifest=json.loads((work/'package/manifest.json').read_text())
 smp=re.search(r'\[smp\.parked\] supported=(\d+) cpus=(\d+) aps=(\d+) bsp_lapic=(\d+) generation=(\d+) x2apic=(\d+)',text)
 if args.smp:assert smp and tuple(map(int,smp.groups()[:3]))==(1,args.smp,args.smp-1) and int(smp[5])==0,smp.groups() if smp else None
 expected=set(REPLACEMENTS)|({'sexos-kernel'} if args.kernel else set());assert set(manifest['replacements'])==expected;assert set(manifest['changed_files'])<=expected;assert all(manifest['candidate_files'][path]==value for path,value in binaries.items()),'packaged bytes differ from the captured inputs'
 elapsed=time.monotonic()-started;summary={'scope':('current all-Cesta kernel and userland image: 17 replacements, permanently parked APs, BSP-only epoch execution, joined shell readiness and sustained main loop' if args.kernel else 'current all-Cesta userland image: 16 replacements, joined shell readiness and sustained main loop; Rust kernel remains the compatibility cohort'),'image_sha256':digest(image),'binaries':binaries,'markers':list(markers),'worker_probe':args.worker_probe,'worker_starts':worker_starts,'record_probe':args.record_probe,'record_starts':record_starts,'record_census':record_census,'record_hum_alive':hum_alive,'record_hum_exits':hum_exits,'record_seeds':record_seeds,'seconds':elapsed,'requested_seconds':args.seconds,'termination':('ready-plus-'+str(args.post_ready)+'s' if args.until_ready else 'full-soak'),'accelerator':args.accel or 'default-tcg','bytes_after_ready':len(after),'forbidden_after_ready':bad,'unknown_opcodes_whole_boot':whole_unknown,'epoch_checkpoints':epochs,'smp':tuple(map(int,smp.groups())) if smp else None,'nvme':bool(args.nvme_image),'nvme_input_sha256':nvme_hash,'network':args.network,'usb_keyboard':args.usb_keyboard}
 (work/'summary.json').write_text(json.dumps(summary,indent=2)+'\n');print('all-Cesta current boot passed:',len(expected),'replacements;',len(markers),'markers;',f'{elapsed:.2f}','seconds;',args.smp or 1,'cores; 0 terminal faults; 0 unknown opcodes after ready')
if __name__=='__main__':main()
