#!/usr/bin/env python3
"""Qualify the checked Phase-3 mapping/warden policy and static AArch64 image."""
import argparse,hashlib,json,struct,subprocess,sys,tempfile
from pathlib import Path
HERE=Path(__file__).resolve().parent.parent
def run(argv,cwd=HERE):
    result=subprocess.run(list(map(str,argv)),cwd=cwd,capture_output=True)
    if result.returncode:raise RuntimeError((argv,result.returncode,result.stdout[-1000:],result.stderr[-5000:]))
    return result.stdout
def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def symbols(image):
    rows=run(['readelf','-sW',image]).decode().splitlines();out={}
    for row in rows:
        fields=row.split()
        if len(fields)>=8 and fields[0].endswith(':'):
            try:out[fields[7]]=int(fields[1],16)
            except ValueError:pass
    return out,rows
def verify_image(image):
    data=image.read_bytes();assert data[:6]==b'\x7fELF\x02\x01'
    e_type,e_machine=struct.unpack_from('<HH',data,16);entry=struct.unpack_from('<Q',data,24)[0]
    assert(e_type,e_machine,entry)==(2,183,0x40080000)
    phoff=struct.unpack_from('<Q',data,32)[0];phentsize,phnum=struct.unpack_from('<HH',data,54);loads=[]
    for i in range(phnum):
        at=phoff+i*phentsize;p_type,p_flags=struct.unpack_from('<II',data,at);values=struct.unpack_from('<QQQQQQ',data,at+8)
        if p_type==1:loads.append((p_flags,*values))
    assert len(loads)==3 and all(v==p and v>=0x40080000 and not(f&2 and f&1)for f,_,v,p,_,_,_ in loads)
    table,rows=symbols(image)
    need=('_start','arm64_phase3_main','arm64_phase3_warden_live','sex_arm_vectors','sex_arm_el0_enter',
          '__kernel_start','__kernel_end','__bss_start','__bss_end')
    for name in need:assert name in table,name
    assert table['sex_arm_vectors']%2048==0 and table['__bss_start']%4096==0 and table['__kernel_end']%4096==0
    assert not [row for row in rows if ' UND ' in row and row.split()[0]!='0:']
    text=run(['strings',image]).decode()
    markers=('[sex.arm.mmu] tables=ok','[sex.arm.mmu] on=1','[sex.arm.mmu] wx=1','[sex.arm.mmu] hhdm=ok',
             'SEX_ARM_MMU_OK','[sex.arm.exc] vbar=ok','[sex.arm.el0] enter=1',
             '[sex.arm.neg] el0_read_kernel=aborted','[sex.arm.neg] el0_write_ro=aborted',
             '[sex.arm.neg] el0_exec_data=aborted','[sex.arm.el0] done=1','SEX_ARM_PHASE3_OK','SEX_ARM_PHASE3_FAIL')
    for marker in markers:assert marker in text,marker
    return {'sha256':digest(image),'bytes':len(data),'entry':entry,'loads':loads,'symbols':{n:table[n]for n in need}}
def payload_check(work):
    assembly=work/'payload.s';obj=work/'payload.o';binary=work/'payload.bin'
    assembly.write_text('''.text
.global payload
payload:
 movz x9,#0x41,lsl #16
 movz x0,#1
 str x0,[x9]
 movz x10,#0x4008,lsl #16
 ldr x1,[x10]
 movz x0,#2
 str x0,[x9]
 str x0,[x10]
 movz x0,#3
 str x0,[x9]
 movz x11,#0x41,lsl #16
 br x11
 movz x0,#4
 str x0,[x9]
 movz x12,#0x5000,lsl #16
 ldr x1,[x12]
 b .
''')
    run(['clang','--target=aarch64-none-elf','-c',assembly,'-o',obj])
    section=run(['readelf','-x','.text',obj]).decode();words=[]
    for line in section.splitlines():
        fields=line.split()
        if fields and fields[0].startswith('0x'):
            for field in fields[1:5]:
                if len(field)==8:
                    try:words.extend(bytes.fromhex(field))
                    except ValueError:pass
    binary.write_bytes(bytes(words));expected=struct.pack('<17I',
        0xd2a00829,0xd2800020,0xf9000120,0xd2a8010a,0xf9400141,0xd2800040,0xf9000120,
        0xf9000140,0xd2800060,0xf9000120,0xd2a0082b,0xd61f0160,0xd2800080,0xf9000120,
        0xd2aa000c,0xf9400181,0x14000000)
    assert binary.read_bytes()==expected
def main():
    p=argparse.ArgumentParser();p.add_argument('--cesta',type=Path,required=True);p.add_argument('--image',type=Path,required=True);p.add_argument('--plain-image',type=Path)
    p.add_argument('--work',type=Path);p.add_argument('--mutations',action='store_true');p.add_argument('--cesta-source',type=Path);p.add_argument('--lean-root',type=Path);a=p.parse_args()
    w=a.work or Path(tempfile.mkdtemp(prefix='arm64-phase3-',dir=HERE/'build'));w.mkdir(parents=True,exist_ok=True)
    source=w/'arm64_phase3_policy.c';plan=w/'arm64_phase3_policy.plan'
    run([a.cesta,'export-c','src/arm64_phase3_policy.cesta','--std=c11','--plan',plan,'-o',source,'--','-Igenerated/arm64/include'])
    flags=['-std=c11','-g0','-Wall','-Wextra','-Wpedantic','-Werror','-Igenerated/arm64/include']
    def build(cc,extra,name):out=w/name;run([cc,*flags,*extra,'tests/arm64_phase3_policy/host.c',source,'-o',out]);return out
    builds=[]
    for cc in ('gcc','clang'):
        for mode in ('O0','O2','sanitize'):
            extra=['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all']if mode=='sanitize'else['-'+mode]
            run([build(cc,extra,cc+'-'+mode)]);builds.append({'compiler':cc,'mode':mode})
    changes=[
      ('kernel_text_write','!c.el0&&!c.write&&c.execute&&!c.device','!c.el0&&c.execute&&!c.device'),
      ('kernel_data_execute','c.write&&!c.execute&&!c.device','c.write&&!c.device'),
      ('uart_device','c.write&&!c.execute&&c.device','c.write&&!c.execute'),
      ('user_code_write','c.el0&&!c.write&&c.execute&&!c.device','c.el0&&c.execute&&!c.device'),
      ('user_code2_identity','c.va==ARM64_USER_CODE2','true'),
      ('user_data_execute','c.el0&&c.write&&!c.execute&&!c.device','c.el0&&c.write&&!c.device'),
      ('stack_identity','c.va==ARM64_USER_STACK','true'),
      ('hhdm_device_pa','c.va==ARM64_HHDM&&c.pa==0','c.va==ARM64_HHDM'),
      ('hhdm_dram_el0','c.pa==ARM64_BLOCK_1G&&c.bytes==ARM64_BLOCK_1G&&!c.el0','c.pa==ARM64_BLOCK_1G&&c.bytes==ARM64_BLOCK_1G'),
      ('fault12_ec','o.ec==UINT8_C(0x24)&&o.far==ARM64_KERNEL_BASE','o.far==ARM64_KERNEL_BASE'),
      ('fault12_far','o.ec==UINT8_C(0x24)&&o.far==ARM64_KERNEL_BASE','o.ec==UINT8_C(0x24)'),
      ('fault3_ec','o.ec==UINT8_C(0x20)&&o.far==ARM64_USER_DATA','o.far==ARM64_USER_DATA'),
      ('fault3_far','o.ec==UINT8_C(0x20)&&o.far==ARM64_USER_DATA','o.ec==UINT8_C(0x20)'),
      ('fault4_far','o.ec==UINT8_C(0x24)&&o.far==ARM64_USER_UNMAPPED','o.ec==UINT8_C(0x24)'),
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
        record,=ownership_admission.parse(plan.read_bytes());admission=ownership_admission.admit_record(record,w/'Arm64Phase3Ownership.lean',a.lean_root)
    image=verify_image(a.image.resolve());plain=None
    if a.plain_image:plain=verify_image(a.plain_image.resolve());assert a.image.read_bytes()==a.plain_image.read_bytes()
    payload_check(w)
    sources=['cesta.arm64.project','src/arm64_boot_interface.cesta','src/arm64_phase3_policy.cesta','src/arm64_platform.cesta',
      'src/arm64_phase3.cesta','runtime/arm64/phase3_entry.S','runtime/arm64/phase3_vectors.S','runtime/arm64/kernel.ld',
      'tests/arm64_phase3.py','tests/arm64_phase3_policy/host.c']
    implementation=sources[:8]
    joined=''.join((HERE/name).read_text()for name in implementation)
    assert not any(term in joined.lower()for term in ('por_el0','por_el1','tcr2_el1','pthread','unistd.h','stdio.h'))
    (w/'summary.json').write_text(json.dumps({'builds':builds,'mutations':caught,'admission':admission,'image':image,'plain_image':plain,
      'source_sha256':{n:digest(HERE/n)for n in sources},'runtime':'qualified separately by tests/arm64_runtime.py',
      'scope':'Cesta AArch64 Phase 3 static image and checked mapping/fault policy; runtime MMU, HHDM, vectors and EL0 abort sequence remain unobserved.'},indent=2)+'\n')
    print('ARM64 Phase-3 static qualification passed:',len(builds),'policy builds,',len(caught),'mutants; payload, ELF and readable-C identity admitted')
if __name__=='__main__':main()
