#!/usr/bin/env python3
"""Exercise the actual driver through interrupted reset, IRQ receipt and retry.

The host fixture uses the real DMA/reset cores. It stops at accepted reset
entry; real controller quiescence/rebuild is qualified by the device boot gate.
"""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile
from layout_text import LayoutText

HERE=Path(__file__).resolve().parent.parent

def run(command):
    p=subprocess.run(list(map(str,command)),cwd=HERE,capture_output=True,text=True)
    if p.returncode:
        raise RuntimeError((command,p.returncode,p.stdout[-1500:],p.stderr[-4000:]))
    return p.stdout

def main():
    p=argparse.ArgumentParser()
    p.add_argument('--cesta',type=Path,required=True)
    p.add_argument('--work',type=Path)
    p.add_argument('--mutations',action='store_true')
    args=p.parse_args()
    work=args.work.resolve() if args.work else Path(tempfile.mkdtemp(prefix='nvme-recovery-',dir=HERE/'build'))
    work.mkdir(parents=True,exist_ok=True)
    source=work/'sexdrive.c'
    run([args.cesta,'export-c','src/sexdrive.cesta','-o',source,'--','-Igenerated/include'])
    for name in ('dma_lease','device_reset'):
        run([args.cesta,'export-c','src/'+name+'.cesta','-o',work/(name+'.c'),'--','-Igenerated/include'])
    def build(cc,extra,name):
        binary=work/name
        run([cc,'-std=c11','-g0','-Wall','-Wextra','-Wpedantic','-Werror',*extra,
             '-DSEXOS_CURRENT_REGION_WRITER=1','-DSEXOS_NVME_INTERRUPT_MODE=1','-DSEXOS_PDX_SENDER_TASK=1',
             '-ffunction-sections','-fdata-sections','-Igenerated/include','-I'+str(work),
             'tests/nvme_recovery/host.c',work/'dma_lease.c',work/'device_reset.c','-Wl,--gc-sections','-o',binary])
        return binary
    controls=[]
    for cc in ('gcc','clang'):
        for mode,extra in [('O0',['-O0']),('O2',['-O2']),('sanitize',['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-pie','-no-pie'])]:
            binary=build(cc,extra,cc+'-'+mode)
            controls.append({'compiler':cc,'mode':mode,'output':run([binary])})
    original=LayoutText(source.read_text())
    mutations=[
        ('clock_deadline', 'now - started < NVME_CQE_TIMEOUT_TICKS', 'now - started <= NVME_CQE_TIMEOUT_TICKS'),
        ('clock_reversal', 'now >= started', 'true'),
        ('poll_wrap', 'poll == UINT32_MAX', 'false'),
        ('iteration_cutoff', 'if (!NVME_TIMED_WAIT_ENABLED)', 'if (true)'),
        ('begin_interrupted','kernel_begin.status==DEVICE_RESET_INTERRUPTED','false'),
        ('quiescence_interrupted','kernel_quiescent.status==DEVICE_RESET_INTERRUPTED','false'),
        ('status_is_hardware','kernel_begin.status==DEVICE_RESET_INTERRUPTED','kernel_begin.status==DEVICE_RESET_HARDWARE'),
        ('retry_status','sexdrive_state.reset_recovery_pending?BLOCK_ERR_TIMEOUT:BLOCK_ERR_NO_DEVICE','BLOCK_ERR_NO_DEVICE'),
        ('flush_needs_no_data','!sexdrive_state.reset_recovery_pending&&','true&&'),
        ('flush_wrong_kind','return nvme_recover_controller(dma_lease_is_quarantined(&sexdrive_state.read_lease)||dma_lease_is_quarantined(&sexdrive_state.write_lease));','return nvme_recover_controller(true);'),
        ('irq_sender','msg.caller_pd!=1u','false'),
        ('irq_task','msg.caller_task!=0u','false'),
        ('irq_generation','_buf_cap==generation','true'),
        ('skip_queued_receipt','snapshot.interrupt_revision!=snapshot.acknowledged_revision','false'),
        ('premature_generation','sexdrive_state.reset_recovery_pending=true;','++sexdrive_state.dma_device.generation;sexdrive_state.reset_recovery_pending=true;'),
        ('premature_reuse','sexdrive_state.reset_recovery_pending=true;','sexdrive_state.read_lease.phase=DMA_AVAILABLE;sexdrive_state.write_lease.phase=DMA_AVAILABLE;sexdrive_state.reset_recovery_pending=true;'),
    ]
    killed=[]
    if args.mutations:
        for name,before,after in mutations:
            assert original.count(before)>=1,(name,'missing mutation anchor')
            try:
                source.write_text(original.replace(before,after,1))
                binary=build('gcc',['-O2'],'mutant-'+name)
                result=subprocess.run([binary],cwd=HERE,capture_output=True)
                assert result.returncode!=0,('survived',name,result.stdout,result.stderr)
                killed.append(name)
            finally:
                source.write_text(original)
    names=['src/sexdrive.cesta','src/device_reset.cesta','src/dma_lease.cesta','tests/nvme_recovery.py','tests/nvme_recovery/host.c']
    (work/'summary.json').write_text(json.dumps({'builds':controls,'mutations':killed,'source_sha256':{n:hashlib.sha256((HERE/n).read_bytes()).hexdigest() for n in names},
       'scope':'actual exported driver with real DMA/reset policy; host IRQ delivery; stops at reset entry; hardware rebuild separately qualified'},indent=2)+'\n')
    print('NVMe deferred recovery controls passed:',len(controls),'GCC/Clang routes,',len(killed),'mutants')

if __name__=='__main__':main()
