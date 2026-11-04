#!/usr/bin/env python3
"""Qualify the static Phase-4 AArch64 mechanism boundary."""
import argparse,hashlib,json,struct,subprocess,tempfile
from pathlib import Path
HERE=Path(__file__).resolve().parent.parent
def run(argv):
    result=subprocess.run(list(map(str,argv)),cwd=HERE,capture_output=True)
    if result.returncode:raise RuntimeError((argv,result.returncode,result.stdout[-1000:],result.stderr[-5000:]))
    return result.stdout
def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def verify(image):
    data=image.read_bytes();assert data[:6]==b'\x7fELF\x02\x01'
    e_type,e_machine=struct.unpack_from('<HH',data,16);entry=struct.unpack_from('<Q',data,24)[0]
    assert(e_type,e_machine,entry)==(2,183,0x40080000)
    phoff=struct.unpack_from('<Q',data,32)[0];phentsize,phnum=struct.unpack_from('<HH',data,54);loads=[]
    for i in range(phnum):
        at=phoff+i*phentsize;p_type,p_flags=struct.unpack_from('<II',data,at);values=struct.unpack_from('<QQQQQQ',data,at+8)
        if p_type==1:loads.append((p_flags,*values))
    assert len(loads)==3 and all(v==p and v>=0x40080000 and not(f&2 and f&1)for f,_,v,p,_,_,_ in loads)
    rows=run(['readelf','-sW',image]).decode().splitlines();symbols={}
    for row in rows:
        fields=row.split()
        if len(fields)>=8 and fields[0].endswith(':'):
            try:symbols[fields[7]]=int(fields[1],16)
            except ValueError:pass
    need=('_start','sex_arm_vectors','sex_arm_el0_enter','sex_arm_restore_eret','sex_arm_first',
      'sex_arm_tick','sex_arm_kill_warden','sex_arm_ap_entry','sex_arm_ap_main','arm64_phase4_entry',
      'arm64_ap_stack','__kernel_start','__kernel_end','__bss_start','__bss_end')
    for name in need:assert name in symbols,name
    assert symbols['sex_arm_vectors']%2048==0 and symbols['__bss_start']%4096==0
    assert symbols['__bss_end']%4096==0 and symbols['__kernel_end']%4096==0
    assert not [row for row in rows if ' UND ' in row and row.split()[0]!='0:']
    strings=run(['strings',image]).decode();markers=('[sex.arm.p4] entry=1','[sex.arm.gic] gicd=ok',
      '[sex.arm.gic] gicr=ok','[sex.arm.gic] cpuif=ok','[sex.arm.gic] ppi30=on',
      '[sex.arm.smp] ap_boot=1','SEX_ARM_AP_OK','[sex.arm.smp] ap_ack=1','[sex.arm.timer] freq=',
      '[sex.arm.sched] start=1','[sex.arm.sched] a_run=','[sex.arm.sched] b_run=',
      '[sex.arm.neg] kill_task=contained','[sex.arm.sched] a_after_dead=10','SEX_ARM_PHASE4_OK','SEX_ARM_PHASE4_FAIL')
    for marker in markers:assert marker in strings,marker
    return {'sha256':digest(image),'bytes':len(data),'entry':entry,'loads':loads,'symbols':{n:symbols[n]for n in need}}
def main():
    p=argparse.ArgumentParser();p.add_argument('--image',type=Path,required=True);p.add_argument('--plain-image',type=Path);p.add_argument('--work',type=Path);a=p.parse_args()
    w=a.work or Path(tempfile.mkdtemp(prefix='arm64-phase4-image-',dir=HERE/'build'));w.mkdir(parents=True,exist_ok=True)
    image=verify(a.image.resolve());plain=None
    if a.plain_image:plain=verify(a.plain_image.resolve());assert a.image.read_bytes()==a.plain_image.read_bytes()
    vectors=(HERE/'runtime/arm64/phase4_vectors.S').read_text();mechanism=(HERE/'src/arm64_phase4.cesta').read_text()
    for anchor in ('sub sp, sp, #272','str x9, [sp, #248]','str x9, [sp, #256]',
      'mrs x19, ICC_IAR1_EL1','msr ICC_EOIR1_EL1, x19','bl sex_arm_kill_warden',
      'msr HCR_EL2, x9','bl sex_arm_ap_main','b sex_arm_restore_eret'):
        assert anchor in vectors,anchor
    for anchor in ('arm64_phase4_gic_valid','arm64_phase4_ap_valid','arm64_phase4_state_init',
      'arm64_phase4_tick','arm64_phase4_kill','atomic_store_explicit','atomic_load_explicit'):
        assert anchor in mechanism,anchor
    combined=(vectors+mechanism).lower()
    assert not any(term in combined for term in ('por_el0','por_el1','tcr2_el1','pthread','unistd.h','stdio.h'))
    sources=['src/arm64_phase4.cesta','src/arm64_phase4_policy.cesta','runtime/arm64/phase4_vectors.S','runtime/arm64/kernel.ld','tests/arm64_phase4_image.py']
    (w/'summary.json').write_text(json.dumps({'image':image,'plain_image':plain,
      'source_sha256':{n:digest(HERE/n)for n in sources},'runtime':'qualified separately by tests/arm64_runtime.py',
      'scope':'static Phase-4 GICv3/timer/AP/two-task scheduler and containment mechanism; no device execution claim'},indent=2)+'\n')
    print('ARM64 Phase-4 static image passed: W^X ELF, required mechanism symbols and readable-C identity admitted')
if __name__=='__main__':main()
