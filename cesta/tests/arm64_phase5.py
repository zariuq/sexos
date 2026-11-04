#!/usr/bin/env python3
"""Qualify the checked Phase-5 SVC and completion policy."""
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
    w=a.work or Path(tempfile.mkdtemp(prefix='arm64-phase5-',dir=HERE/'build'));w.mkdir(parents=True,exist_ok=True)
    source=w/'arm64_phase5_policy.c';plan=w/'arm64_phase5_policy.plan'
    run([a.cesta,'export-c','src/arm64_phase5_policy.cesta','--std=c11','--plan',plan,'-o',source,'--','-Igenerated/arm64/include'])
    flags=['-std=c11','-g0','-Wall','-Wextra','-Wpedantic','-Werror','-Igenerated/arm64/include']
    def build(cc,extra,name):out=w/name;run([cc,*flags,*extra,'tests/arm64_phase5_policy/host.c',source,'-o',out]);return out
    builds=[]
    for cc in ('gcc','clang'):
        for mode in ('O0','O2','sanitize'):
            extra=['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all']if mode=='sanitize'else['-'+mode]
            run([build(cc,extra,cc+'-'+mode)]);builds.append({'compiler':cc,'mode':mode})
    changes=[
      ('echo_selector','selector==1u','selector==3u'),
      ('echo_increment','argument+1u','argument'),
      ('cap_selector','selector==2u','selector==3u'),
      ('cap_identity','argument==42u','argument==41u'),
      ('completion_tag','c.tag==5u','true'),
      ('completion_ec','c.ec==UINT8_C(0x24)','true'),
      ('completion_far','c.far==UINT64_C(0x60000000)','true'),
      ('completion_echo','c.echo==42u','true'),
      ('completion_bad','c.cap_bad==ARM64_ERR_CAP_INVALID','true'),
      ('completion_good','c.cap_good==0','true'),
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
        record,=ownership_admission.parse(plan.read_bytes());admission=ownership_admission.admit_record(record,w/'Arm64Phase5Ownership.lean',a.lean_root)
    names=['src/arm64_boot_interface.cesta','src/arm64_phase5_policy.cesta','tests/arm64_phase5.py','tests/arm64_phase5_policy/host.c']
    (w/'summary.json').write_text(json.dumps({'builds':builds,'mutations':caught,'admission':admission,
      'source_sha256':{n:hashlib.sha256((HERE/n).read_bytes()).hexdigest()for n in names},
      'scope':'checked Phase-5 SVC mechanism and completion record; not AArch64 PDX/userland parity'},indent=2)+'\n')
    print('ARM64 Phase-5 policy qualification passed:',len(builds),'builds,',len(caught),'mutants')
if __name__=='__main__':main()
