#!/usr/bin/env python3
"""Qualify checked Phase-2 policy and the freestanding AArch64 image boundary."""
import argparse,hashlib,json,struct,subprocess,sys,tempfile
from pathlib import Path
HERE=Path(__file__).resolve().parent.parent
def run(argv,cwd=HERE):
    result=subprocess.run(list(map(str,argv)),cwd=cwd,capture_output=True)
    if result.returncode:raise RuntimeError((argv,result.returncode,result.stdout[-1000:],result.stderr[-5000:]))
    return result.stdout
def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def verify_image(image):
    data=image.read_bytes();assert data[:6]==b'\x7fELF\x02\x01'
    e_type,e_machine=struct.unpack_from('<HH',data,16);entry=struct.unpack_from('<Q',data,24)[0]
    assert (e_type,e_machine,entry)==(2,183,0x40080000)
    phoff=struct.unpack_from('<Q',data,32)[0];phentsize,phnum=struct.unpack_from('<HH',data,54)
    loads=[]
    for i in range(phnum):
        at=phoff+i*phentsize;p_type,p_flags=struct.unpack_from('<II',data,at);offset,vaddr,paddr,filesz,memsz,align=struct.unpack_from('<QQQQQQ',data,at+8)
        if p_type==1:loads.append((p_flags,offset,vaddr,paddr,filesz,memsz,align))
    assert len(loads)==3 and all(v==p and v>=0x40080000 and not(f&2 and f&1) for f,_,v,p,_,_,_ in loads)
    assert any(f==5 for f,*_ in loads) and any(f==6 and mem>size for f,_,_,_,size,mem,_ in loads)
    symbols=run(['readelf','-sW',image]).decode()
    for name in ('_start','arm64_phase2_main','__boot_stack_bottom','__boot_stack_top','__bss_start','__bss_end','__kernel_start','__kernel_end'):
        assert name in symbols,name
    assert not [line for line in symbols.splitlines() if ' UND ' in line and line.split()[0]!='0:']
    strings=run(['strings',image]).decode()
    for marker in ('[sex.arm.entry] current_el=','[sex.arm.el1] ok=','[sex.arm.uart] pl011=ok','SEX_ARM_ALIVE','SEX_ARM_PHASE2_OK','[sex.arm.phase2.fail] code='):
        assert marker in strings,marker
    return {'sha256':digest(image),'bytes':len(data),'entry':entry,'loads':loads}
def main():
    p=argparse.ArgumentParser();p.add_argument('--cesta',type=Path,required=True);p.add_argument('--image',type=Path,required=True);p.add_argument('--plain-image',type=Path)
    p.add_argument('--work',type=Path);p.add_argument('--mutations',action='store_true');p.add_argument('--cesta-source',type=Path);p.add_argument('--lean-root',type=Path);a=p.parse_args()
    w=a.work or Path(tempfile.mkdtemp(prefix='arm64-phase2-',dir=HERE/'build'));w.mkdir(parents=True,exist_ok=True)
    source=w/'arm64_boot_policy.c';plan=w/'arm64_boot_policy.plan'
    run([a.cesta,'export-c','src/arm64_boot_policy.cesta','--std=c11','--plan',plan,'-o',source,'--','-Igenerated/arm64/include'])
    flags=['-std=c11','-g0','-Wall','-Wextra','-Wpedantic','-Werror','-Igenerated/arm64/include']
    def build(cc,extra,name):out=w/name;run([cc,*flags,*extra,'tests/arm64_boot_policy/host.c',source,'-o',out]);return out
    builds=[]
    for cc in ('gcc','clang'):
        for mode in ('O0','O2','sanitize'):
            extra=['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all']if mode=='sanitize'else['-'+mode]
            run([build(cc,extra,cc+'-'+mode)]);builds.append({'compiler':cc,'mode':mode})
    changes=[
      ('entered_el','observation.entered_el!=1u && observation.entered_el!=2u','false'),
      ('current_el','observation.current_el!=1u','false'),
      ('dtb_alignment','(observation.dtb&7u)!=0','false'),
      ('dtb_magic','!observation.dtb_magic_ok','false'),
    ];caught=[]
    if a.mutations:
        original=source.read_text()
        for name,before,after in changes:
            assert original.count(before)==1,(name,'anchor')
            try:
                source.write_text(original.replace(before,after));binary=build('gcc',['-O2'],name)
                result=subprocess.run([binary],cwd=HERE,capture_output=True);assert result.returncode!=0,('survived',name);caught.append(name)
            finally:source.write_text(original)
    admission=None
    if a.lean_root:
        assert a.cesta_source;sys.path.insert(0,str(a.cesta_source/'tests'));import ownership_admission
        record,=ownership_admission.parse(plan.read_bytes());admission=ownership_admission.admit_record(record,w/'Arm64BootOwnership.lean',a.lean_root)
    image=verify_image(a.image.resolve());plain=None
    if a.plain_image:
        plain=verify_image(a.plain_image.resolve());assert a.image.read_bytes()==a.plain_image.read_bytes()
    assembly=(HERE/'runtime/arm64/entry.S').read_text();phase=(HERE/'src/arm64_phase2.cesta').read_text()
    for anchor in ('mrs x21, CurrentEL','msr hcr_el2, x9','msr sp_el1, x9','isb','bl arm64_phase2_main'):
        assert anchor in assembly
    assert not any(word in (assembly+phase).lower() for word in ('por_el0','por_el1','tcr2_el1','pthread','unistd.h','stdio.h'))
    names=['cesta.arm64.project','src/arm64_boot_interface.cesta','src/arm64_boot_policy.cesta','src/arm64_phase2.cesta','runtime/arm64/entry.S','runtime/arm64/kernel.ld','tests/arm64_phase2.py','tests/arm64_boot_policy/host.c']
    (w/'summary.json').write_text(json.dumps({'builds':builds,'mutations':caught,'admission':admission,'image':image,'plain_image':plain,
      'source_sha256':{n:digest(HERE/n)for n in names},'scope':'Cesta AArch64 Phase 2: EL2/EL1 entry, stack/BSS, fail-closed optional DTB policy and PL011 image structure. No runtime observation on this host; MMU, exceptions, GIC, scheduler, AP, SVC, PDX and S1POE remain later phases.'},indent=2)+'\n')
    print('ARM64 Phase-2 qualification passed:',len(builds),'policy builds,',len(caught),'mutants; ELF/readable-C identity admitted')
if __name__=='__main__':main()
