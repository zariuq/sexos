#!/usr/bin/env python3
"""Qualify bounded DMA suballocation and non-reusable uncertain submissions."""
import argparse,hashlib,json,subprocess,sys,tempfile
from pathlib import Path

HERE=Path(__file__).resolve().parent.parent

def run(argv):
    result=subprocess.run(list(map(str,argv)),cwd=HERE,capture_output=True)
    if result.returncode:raise RuntimeError((argv,result.returncode,result.stdout[-1000:],result.stderr[-5000:]))
    return result.stdout

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--cesta',type=Path,required=True)
    parser.add_argument('--work',type=Path);parser.add_argument('--mutations',action='store_true')
    parser.add_argument('--cesta-source',type=Path);parser.add_argument('--lean-root',type=Path)
    args=parser.parse_args();work=args.work or Path(tempfile.mkdtemp(prefix='dma-lease-',dir=HERE/'build'))
    work.mkdir(parents=True,exist_ok=True);source=work/'dma_lease.c';plan=work/'dma_lease.plan'
    run([args.cesta,'export-c','src/dma_lease.cesta','--std=c11','--plan',plan,'-o',source,'--','-Igenerated/include'])
    flags=['-std=c11','-g0','-Wall','-Wextra','-Wpedantic','-Werror','-Igenerated/include']
    def build(cc,extra,name):
        binary=work/name;run([cc,*flags,*extra,'tests/dma_lease/host.c',source,'-o',binary]);return binary
    builds=[]
    for cc in ('gcc','clang'):
        for mode in ('O0','O2','sanitize'):
            extra=['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all'] if mode=='sanitize' else ['-'+mode]
            run([build(cc,extra,cc+'-'+mode)]);builds.append({'compiler':cc,'mode':mode})
    changes=[
        ('region_wrap','region.length<=DMA_LEASE_USER_END-region.virtual_address','true'),
        ('physical_wrap','region.length<=DMA_LEASE_PHYSICAL_END-region.physical_address','true'),
        ('reserve_rounding','bytes+DMA_LEASE_PAGE_BYTES-1','bytes'),
        ('reserve_capacity','length>pool->region.length-pool->used','false'),
        ('prepare_busy','lease->phase!=DMA_AVAILABLE','false'),
        ('prepare_device_active','if(!dma_device_epoch_active(device) || lease->device_id!=device->device_id ||','if(false || lease->device_id!=device->device_id ||'),
        ('prepare_generation','lease->generation!=device->generation)\n        return(DmaLeaseResult)','false)\n        return(DmaLeaseResult)'),
        ('submit_tag','lease->tag!=tag','lease->tag!=(tag^1u)'),
        ('abort_inflight','lease->phase!=DMA_PREPARED','lease->phase!=DMA_IN_FLIGHT'),
        ('complete_without_submit','lease->phase!=DMA_IN_FLIGHT','lease->phase!=DMA_PREPARED'),
        ('completion_identity','observed_tag!=tag','observed_tag!=(tag^1u)'),
        ('completion_phase','observed_phase!=expected_phase','observed_phase!=(expected_phase^1u)'),
        ('uncertain_reuse','dma_lease_quarantine(lease);return valid?DMA_LEASE_OK','lease->phase=DMA_AVAILABLE;lease->tag=0;return valid?DMA_LEASE_OK'),
        ('quarantine_reprepare','lease->phase!=DMA_AVAILABLE','false'),
        ('reset_requires_active','if(!dma_device_epoch_active(device) || device->generation==UINT64_MAX)','if(false || device->generation==UINT64_MAX)'),
        ('reset_generation_exhaustion','device->generation==UINT64_MAX','false'),
        ('quiescent_requires_cc_disabled','cc_enabled || csts_ready','(cc_enabled && csts_ready)'),
        ('retire_requires_quarantine','lease->phase!=DMA_QUARANTINED','false'),
        ('retire_device_identity','lease->device_id!=device->device_id || lease->generation!=device->generation)\n        return DMA_LEASE_INVALID_TRANSITION;\n    lease->generation=device->generation+1;lease->phase',
         'false || lease->generation!=device->generation)\n        return DMA_LEASE_INVALID_TRANSITION;\n    lease->generation=device->generation+1;lease->phase'),
        ('retire_generation','lease->generation!=device->generation)\n        return DMA_LEASE_INVALID_TRANSITION;\n    lease->generation=device->generation+1;lease->phase',
         'false)\n        return DMA_LEASE_INVALID_TRANSITION;\n    lease->generation=device->generation+1;lease->phase'),
        ('resume_requires_queues','!queues_ready','(queues_ready && false)'),
    ];caught=[]
    if args.mutations:
        original=source.read_text()
        for name,before,after in changes:
            assert original.count(before)>=1,(name,'anchor')
            try:
                source.write_text(original.replace(before,after,1));binary=build('gcc',['-O2'],name)
                result=subprocess.run([binary],cwd=HERE,capture_output=True)
                assert result.returncode!=0,('survived',name);caught.append(name)
            finally:source.write_text(original)
    admission=None
    if args.lean_root:
        assert args.cesta_source;sys.path.insert(0,str(args.cesta_source/'tests'));import ownership_admission
        record,=ownership_admission.parse(plan.read_bytes())
        admission=ownership_admission.admit_record(record,work/'DmaLeaseOwnership.lean',args.lean_root)
    names=['src/dma_lease.cesta','src/dma_lease_interface.cesta','tests/dma_lease.py','tests/dma_lease/host.c']
    (work/'summary.json').write_text(json.dumps({'builds':builds,'mutations':caught,'admission':admission,
        'source_sha256':{name:hashlib.sha256((HERE/name).read_bytes()).hexdigest() for name in names},
        'scope':'pure BOOT-region suballocation and DMA lease lifecycle; device submission/completion adapters and controller reset remain separate'},indent=2)+'\n')
    print('DMA lease qualification passed:',len(builds),'builds,',len(caught),'mutants')

if __name__=='__main__':main()
