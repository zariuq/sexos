#!/usr/bin/env python3
"""Qualify bounded kernel-owned NVMe MSI-X planning."""
import argparse,hashlib,json,subprocess,sys,tempfile
from pathlib import Path
HERE=Path(__file__).resolve().parent.parent
def run(argv):
    result=subprocess.run(list(map(str,argv)),cwd=HERE,capture_output=True)
    if result.returncode:raise RuntimeError((argv,result.returncode,result.stdout[-1000:],result.stderr[-5000:]))
    return result.stdout
def main():
    p=argparse.ArgumentParser();p.add_argument('--cesta',type=Path,required=True);p.add_argument('--work',type=Path);p.add_argument('--mutations',action='store_true')
    p.add_argument('--cesta-source',type=Path);p.add_argument('--lean-root',type=Path);a=p.parse_args()
    w=a.work or Path(tempfile.mkdtemp(prefix='nvme-msix-',dir=HERE/'build'));w.mkdir(parents=True,exist_ok=True)
    source=w/'nvme_msix.c';plan=w/'nvme_msix.plan'
    run([a.cesta,'export-c','src/nvme_msix.cesta','--std=c11','--plan',plan,'-o',source,'--','-Igenerated/include'])
    flags=['-std=c11','-g0','-Wall','-Wextra','-Wpedantic','-Werror','-Igenerated/include']
    def build(cc,extra,name):out=w/name;run([cc,*flags,*extra,'tests/nvme_msix/host.c',source,'-o',out]);return out
    builds=[]
    for cc in ('gcc','clang'):
        for mode in ('O0','O2','sanitize'):
            extra=['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all']if mode=='sanitize'else['-'+mode]
            run([build(cc,extra,cc+'-'+mode)]);builds.append({'compiler':cc,'mode':mode})
    changes=[
      ('cap_offset','capability_offset<0x40u','false'),
      ('cap_alignment','(capability_offset&3u)','false'),
      ('cap_id','(capability&0xffu)!=0x11u','false'),
      ('vector_low','vector<32u','false'),
      ('table_bir','(table&7u)!=0','false'),
      ('pba_bir','(pba&7u)!=0','false'),
      ('table_bounds','!nvme_msix_range(plan.table_offset,16u,bar_length)','false'),
      ('pba_bounds','!nvme_msix_range(plan.pba_offset,8u,bar_length)','false'),
      ('destination','((uint32_t)destination<<12)','0'),
      ('vector_data','plan.message_data=vector','plan.message_data=vector+1u'),
      ('config_enable','capability|UINT32_C(0x80000000)','capability'),
      ('observe_config','capability==p->config_enabled','(capability==p->config_enabled||capability==p->config_masked)'),
      ('observe_address','address_low==p->address_low','(address_low==p->address_low||address_low==(p->address_low|0x1000u))'),
      ('observe_control','vector_control==p->vector_enabled','(vector_control==p->vector_enabled||vector_control==p->vector_masked)'),
    ];caught=[]
    if a.mutations:
        original=source.read_text()
        for name,before,after in changes:
            assert original.count(before)>=1,(name,'anchor')
            try:
                source.write_text(original.replace(before,after,1));binary=build('gcc',['-O2'],name)
                result=subprocess.run([binary],cwd=HERE,capture_output=True);assert result.returncode!=0,('survived',name);caught.append(name)
            finally:source.write_text(original)
    admission=None
    if a.lean_root:
        assert a.cesta_source;sys.path.insert(0,str(a.cesta_source/'tests'));import ownership_admission
        record,=ownership_admission.parse(plan.read_bytes());admission=ownership_admission.admit_record(record,w/'NvmeMsixOwnership.lean',a.lean_root)
    names=['src/nvme_msix_interface.cesta','src/nvme_msix.cesta','tests/nvme_msix.py','tests/nvme_msix/host.c']
    (w/'summary.json').write_text(json.dumps({'builds':builds,'mutations':caught,'admission':admission,
      'source_sha256':{n:hashlib.sha256((HERE/n).read_bytes()).hexdigest()for n in names},
      'fixture':{'capability_offset':0x40,'capability':0x00408011,'table':0x2000,'pba':0x3000,'bar_length':0x4000,'vector':0x22},
      'scope':'pure MSI-X capability/table plan and exact readback; no PCI/MMIO execution claim'},indent=2)+'\n')
    print('NVMe MSI-X policy passed:',len(builds),'builds,',len(caught),'mutants')
if __name__=='__main__':main()
