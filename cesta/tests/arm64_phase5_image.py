#!/usr/bin/env python3
"""Qualify the static Phase-5/6A AArch64 mechanism boundary."""
import argparse,hashlib,json,struct,subprocess,tempfile
from pathlib import Path
import arm64_phase4_image
HERE=Path(__file__).resolve().parent.parent
def run(argv):
    result=subprocess.run(list(map(str,argv)),cwd=HERE,capture_output=True)
    if result.returncode:raise RuntimeError((argv,result.returncode,result.stdout[-1000:],result.stderr[-5000:]))
    return result.stdout
def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def payload_check(work):
    assembly=work/'payload5.s';obj=work/'payload5.o'
    assembly.write_text('''.text
.global payload5
payload5:
 movz x9,#0x41,lsl #16
 movz x0,#1
 movz x1,#41
 svc #0
 str x0,[x9,#16]
 movz x0,#2
 movz x1,#0xdead
 svc #0
 str x0,[x9,#24]
 movz x0,#2
 movz x1,#42
 svc #0
 str x0,[x9,#32]
 movz x0,#5
 str x0,[x9]
 movz x12,#0x6000,lsl #16
 ldr x1,[x12]
 b .
''')
    run(['clang','--target=aarch64-none-elf','-c',assembly,'-o',obj])
    section=run(['readelf','-x','.text',obj]).decode();raw=[]
    for line in section.splitlines():
        fields=line.split()
        if fields and fields[0].startswith('0x'):
            for field in fields[1:5]:
                if len(field)==8:
                    try:raw.extend(bytes.fromhex(field))
                    except ValueError:pass
    expected=struct.pack('<18I',0xd2a00829,0xd2800020,0xd2800521,0xd4000001,0xf9000920,0xd2800040,
      0xd29bd5a1,0xd4000001,0xf9000d20,0xd2800040,0xd2800541,0xd4000001,0xf9001120,
      0xd28000a0,0xf9000120,0xd2ac000c,0xf9400181,0x14000000)
    assert bytes(raw)==expected
def extra_verify(image):
    base=arm64_phase4_image.verify(image);rows=run(['readelf','-sW',image]).decode().splitlines();symbols={}
    for row in rows:
        fields=row.split()
        if len(fields)>=8 and fields[0].endswith(':'):
            try:symbols[fields[7]]=int(fields[1],16)
            except ValueError:pass
    for name in ('sex_arm_svc','arm64_phase5_entry','arm64_phase6_entry','arm64_phase5_warden_live'):
        assert name in symbols,name
    text=run(['strings',image]).decode();markers=('[sex.arm.p5] entry=1','[sex.arm.timer] off=1',
      '[sex.arm.el0] enter2=1','[sex.arm.el0] done2=1','[sex.arm.svc] echo=ok',
      '[sex.arm.svc] cap_bad=ERR_CAP_INVALID','[sex.arm.svc] cap_good=ok','SEX_ARM_PHASE5_OK',
      '[sex.arm.p6] entry=1','[sex.arm.poe] id_aa64mmfr3=','behavior=UNPROVEN',
      'SEX_ARM_POE_TAGGED poindex_default=0 nonzero_tags=0','SEX_ARM_PHASE6_OK','[sex.arm.p7] deferred=1')
    for marker in markers:assert marker in text,marker
    base['phase5_symbols']={name:symbols[name]for name in ('sex_arm_svc','arm64_phase5_entry','arm64_phase6_entry','arm64_phase5_warden_live')}
    return base
def main():
    p=argparse.ArgumentParser();p.add_argument('--image',type=Path,required=True);p.add_argument('--plain-image',type=Path);p.add_argument('--work',type=Path);a=p.parse_args()
    w=a.work or Path(tempfile.mkdtemp(prefix='arm64-phase5-image-',dir=HERE/'build'));w.mkdir(parents=True,exist_ok=True)
    image=extra_verify(a.image.resolve());plain=None
    if a.plain_image:plain=extra_verify(a.plain_image.resolve());assert a.image.read_bytes()==a.plain_image.read_bytes()
    payload_check(w)
    implementation=['src/arm64_phase5.cesta','src/arm64_phase5_policy.cesta','runtime/arm64/phase4_vectors.S']
    joined=''.join((HERE/name).read_text().lower()for name in implementation)
    for write in ('msr tcr2_el1','msr por_el0','msr por_el1'):assert write not in joined
    assert 'id>>16' in joined and 'arm64_fault_phase6' in joined
    sources=[*implementation,'cesta.arm64.project','tests/arm64_phase5_image.py']
    (w/'summary.json').write_text(json.dumps({'image':image,'plain_image':plain,
      'source_sha256':{n:digest(HERE/n)for n in sources},'runtime':'qualified separately by tests/arm64_runtime.py',
      'scope':'static Phase-5 SVC mechanism and Phase-6A feature detection; no PDX parity or POE enforcement claim'},indent=2)+'\n')
    print('ARM64 Phase-5/6A static image passed: payload, detection boundary and readable-C identity admitted')
if __name__=='__main__':main()
