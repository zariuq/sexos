#!/usr/bin/env python3
"""Qualify the held-capability/MMIO adapter for kernel device reset epochs."""
import argparse,hashlib,json,subprocess,tempfile
from pathlib import Path
from layout_text import LayoutText
HERE=Path(__file__).resolve().parent.parent
def run(argv):
    result=subprocess.run(list(map(str,argv)),cwd=HERE,capture_output=True)
    if result.returncode:raise RuntimeError((argv,result.returncode,result.stdout[-1000:],result.stderr[-5000:]))
    return result.stdout
def main():
    p=argparse.ArgumentParser();p.add_argument('--cesta',type=Path,required=True);p.add_argument('--work',type=Path);p.add_argument('--mutations',action='store_true');a=p.parse_args()
    w=a.work or Path(tempfile.mkdtemp(prefix='kernel-device-reset-',dir=HERE/'build'));w.mkdir(parents=True,exist_ok=True)
    adapter=w/'kernel_device_reset.c';interrupts=w/'kernel_device_interrupt.c';policy=w/'device_reset.c';regions=w/'memory_regions.c'
    run([a.cesta,'export-c','src/kernel_device_reset.cesta','-o',adapter,'--','-Itests/kernel_device_reset','-Igenerated/include'])
    run([a.cesta,'export-c','src/kernel_device_interrupt.cesta','-o',interrupts,'--','-Itests/kernel_device_reset','-Igenerated/include'])
    run([a.cesta,'export-c','src/device_reset.cesta','--std=c11','-o',policy,'--','-Igenerated/include'])
    run([a.cesta,'export-c','src/memory_regions.cesta','--std=c11','-o',regions,'--','-Igenerated/include'])
    flags=['-std=gnu11','-g0','-DSEXOS_KERNEL_CURRENT_PDX=1','-Wall','-Wextra','-Wpedantic','-Werror','-Itests/kernel_device_reset','-Igenerated/include']
    def build(cc,extra,name):
        out=w/name;run([cc,*flags,*extra,'tests/kernel_device_reset/host.c',adapter,interrupts,policy,regions,'-o',out]);return out
    builds=[]
    for cc in ('gcc','clang'):
        for mode in ('O0','O2','sanitize'):
            extra=['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all']if mode=='sanitize'else['-'+mode]
            run([build(cc,extra,cc+'-'+mode)]);builds.append({'compiler':cc,'mode':mode})
    changes=[
      ('scheduled_subject','kernel_current_task(d)!=d->task_id','false'),
      ('nvme_slot','slot!=KERNEL_NVME_SLOT','false'),
      ('vector_owner','kernel.vector_owner[KERNEL_NVME_VECTOR]!=d->id','false'),
      ('nvme_class','(class_revision>>24)!=1','false'),
      ('nvme_identity','observed!=expected','((observed^expected)>1u)'),
      ('cc_offset','base+0x14','base+0x10'),('csts_offset','base+0x1c','base+0x18'),
      ('epoch_capacity','action!=DEVICE_RESET_QUERY && !kernel_epoch_current(epoch)','action!=DEVICE_RESET_QUERY && false'),
      ('heard_receipt','argument!=kernel.vector_heard_revision[KERNEL_NVME_VECTOR]','false'),
      ('ack_publication','kernel.vector_ack_revision[KERNEL_NVME_VECTOR]=receipt.acknowledged_revision','kernel.vector_ack_revision[KERNEL_NVME_VECTOR]+=0'),
      ('ack_event_revision','action==DEVICE_RESET_ACK_INTERRUPT?\n        receipt.acknowledged_revision:receipt.interrupt_revision','false?\n        receipt.acknowledged_revision:receipt.interrupt_revision'),
      ('dead_owner_vector_custody','kernel.vector_owner[state->vector]=0','kernel.vector_owner[state->vector]+=0'),
      ('reset_masks_msix','action==DEVICE_RESET_BEGIN&&','false&&'),
      ('resume_unmasks_msix','action==DEVICE_RESET_RESUME&&','false&&'),
      ('dead_owner_masks_msix','if(!kernel_nvme_msix_set_mask(cap,true))return false;','if(false)return false;'),
      ('interrupt_revision','.interrupt_revision=kernel.vector_revision[KERNEL_NVME_VECTOR]','.interrupt_revision=0'),
      ('pending_revised_again','if (!revision) {\n        if (kernel.vector_revision[vector] == UINT64_MAX)','if (true) {\n        if (kernel.vector_revision[vector] == UINT64_MAX)'),
      ('pending_not_retried','if (!domain_enqueue(d, kernel_device_interrupt_message(vector, revision)))\n        return false;','if (true) {\n        (void)revision;\n        return false;\n    }'),
      ('epoch_record','receipt.status==DEVICE_RESET_OK && action!=DEVICE_RESET_QUERY','receipt.status==DEVICE_RESET_OK && false'),
    ];caught=[]
    if a.mutations:
        for name,before,after in changes:
            changed = interrupts if name in ('pending_revised_again', 'pending_not_retried') else adapter
            original=LayoutText(changed.read_text())
            assert original.count(before)>=1,(name,'anchor')
            try:
                changed.write_text(original.replace(before,after,1));binary=build('gcc',['-O2'],name)
                result=subprocess.run([binary],cwd=HERE,capture_output=True);assert result.returncode!=0,('survived',name);caught.append(name)
            finally:changed.write_text(original)
    names=['src/kernel_device_reset.cesta','src/kernel_device_interrupt.cesta','tests/kernel_device_reset.py','tests/kernel_device_reset/host.c']
    (w/'summary.json').write_text(json.dumps({'builds':builds,'mutations':caught,
      'source_sha256':{n:hashlib.sha256((HERE/n).read_bytes()).hexdigest()for n in names},
      'scope':'actual kernel adapter with host MMIO/capability/epoch controls; real QEMU NVMe is qualified separately'},indent=2)+'\n')
    print('kernel device reset qualification passed:',len(builds),'builds,',len(caught),'mutants')
if __name__=='__main__':main()
