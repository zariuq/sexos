#!/usr/bin/env python3
"""Qualify kernel-visible device reset generation policy."""
import argparse,hashlib,json,subprocess,sys,tempfile
from pathlib import Path

HERE=Path(__file__).resolve().parent.parent
def run(argv):
    result=subprocess.run(list(map(str,argv)),cwd=HERE,capture_output=True)
    if result.returncode:raise RuntimeError((argv,result.returncode,result.stdout[-1000:],result.stderr[-5000:]))
    return result.stdout
def main():
    p=argparse.ArgumentParser();p.add_argument('--cesta',type=Path,required=True);p.add_argument('--work',type=Path)
    p.add_argument('--mutations',action='store_true');p.add_argument('--cesta-source',type=Path);p.add_argument('--lean-root',type=Path);a=p.parse_args()
    w=a.work or Path(tempfile.mkdtemp(prefix='device-reset-',dir=HERE/'build'));w.mkdir(parents=True,exist_ok=True)
    src=w/'device_reset.c';plan=w/'device_reset.plan'
    run([a.cesta,'export-c','src/device_reset.cesta','--std=c11','--plan',plan,'-o',src,'--','-Igenerated/include'])
    flags=['-std=c11','-g0','-Wall','-Wextra','-Wpedantic','-Werror','-Igenerated/include']
    def build(cc,extra,name):
        out=w/name;run([cc,*flags,*extra,'tests/device_reset/host.c',src,'-o',out]);return out
    builds=[]
    for cc in ('gcc','clang'):
        for mode in ('O0','O2','sanitize'):
            extra=['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all']if mode=='sanitize'else['-'+mode]
            run([build(cc,extra,cc+'-'+mode)]);builds.append({'compiler':cc,'mode':mode})
    changes=[
      ('valid_owner','s->owner &&','true &&'),('valid_identity','s->identity &&','true &&'),
      ('query_live_interrupt','receipt.interrupt_revision=q.interrupt_revision','receipt.interrupt_revision=state->interrupt_revision'),
      ('query_live_ack','receipt.acknowledged_revision=q.acknowledged_revision','receipt.acknowledged_revision=state->acknowledged_revision'),
      ('stale_generation','q.expected_generation!=state->generation','false'),
      ('begin_active','state->phase!=DEVICE_RESET_ACTIVE','false'),
      ('begin_exhaustion','state->generation==UINT64_MAX','false'),
      ('begin_pending_interrupt','q.interrupt_revision!=q.acknowledged_revision','false'),
      ('ack_in_order','q.acknowledged_revision!=state->acknowledged_revision+1','false'),
      ('ack_produced','q.acknowledged_revision>q.interrupt_revision','false'),
      ('interrupt_revision','q.interrupt_revision!=q.acknowledged_revision','false'),
      ('quiescent_phase','state->phase!=DEVICE_RESET_RESETTING','false'),
      ('quiescent_hardware','(q.cc&1u)||(q.csts&1u)','false'),
      ('resume_phase','state->phase!=DEVICE_RESET_QUIESCENT','false'),
      ('resume_queues','!q.queues_ready','(q.queues_ready&&false)'),
      ('resume_cc','!(q.cc&1u)','false'),('resume_csts','!(q.csts&1u)','false'),
      ('resume_same_snapshot','q.interrupt_revision!=state->interrupt_revision','false'),
      ('resume_generation','++state->generation','state->generation+=2'),
      ('owner_death_orphans','state->phase=DEVICE_RESET_ORPHANED','state->phase=DEVICE_RESET_ACTIVE'),
    ];caught=[]
    if a.mutations:
        original=src.read_text()
        for name,before,after in changes:
            assert original.count(before)>=1,(name,'anchor')
            try:
                src.write_text(original.replace(before,after,1));binary=build('gcc',['-O2'],name)
                result=subprocess.run([binary],cwd=HERE,capture_output=True);assert result.returncode!=0,('survived',name);caught.append(name)
            finally:src.write_text(original)
    admission=None
    if a.lean_root:
        assert a.cesta_source;sys.path.insert(0,str(a.cesta_source/'tests'));import ownership_admission
        record,=ownership_admission.parse(plan.read_bytes());admission=ownership_admission.admit_record(record,w/'DeviceResetOwnership.lean',a.lean_root)
    names=['src/device_reset.cesta','src/device_reset_interface.cesta','tests/device_reset.py','tests/device_reset/host.c']
    (w/'summary.json').write_text(json.dumps({'builds':builds,'mutations':caught,'admission':admission,
      'source_sha256':{n:hashlib.sha256((HERE/n).read_bytes()).hexdigest()for n in names},
      'scope':'pure device reset generation policy; kernel capability/MMIO adapter and hardware remain separate'},indent=2)+'\n')
    print('device reset qualification passed:',len(builds),'builds,',len(caught),'mutants')
if __name__=='__main__':main()
