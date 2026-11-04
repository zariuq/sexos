#!/usr/bin/env python3
"""Qualify the checked Phase-4 GIC/AP/scheduler policy."""
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
    w=a.work or Path(tempfile.mkdtemp(prefix='arm64-phase4-',dir=HERE/'build'));w.mkdir(parents=True,exist_ok=True)
    source=w/'arm64_phase4_policy.c';plan=w/'arm64_phase4_policy.plan'
    run([a.cesta,'export-c','src/arm64_phase4_policy.cesta','--std=c11','--plan',plan,'-o',source,'--','-Igenerated/arm64/include'])
    flags=['-std=c11','-g0','-Wall','-Wextra','-Wpedantic','-Werror','-Igenerated/arm64/include']
    def build(cc,extra,name):out=w/name;run([cc,*flags,*extra,'tests/arm64_phase4_policy/host.c',source,'-o',out]);return out
    builds=[]
    for cc in ('gcc','clang'):
        for mode in ('O0','O2','sanitize'):
            extra=['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all']if mode=='sanitize'else['-'+mode]
            run([build(cc,extra,cc+'-'+mode)]);builds.append({'compiler':cc,'mode':mode})
    changes=[
      ('distinct_frames','frame_a==frame_b','false'),
      ('timer_intid','intid!=30u','intid==UINT32_MAX'),
      ('save_a','state->frame_a=frame','state->frame_b=frame'),
      ('dead_stays_a','state->b_dead?ARM64_TASK_A','state->b_dead?ARM64_TASK_B'),
      ('boot_fault','if(!state->started)','if(false)'),
      ('survivor_fault','state->current!=ARM64_TASK_B','false'),
      ('kill_marks_dead','state->b_dead=true','state->b_dead=false'),
      ('gic_distributor','(o.distributor_control&(UINT32_C(1)<<1))!=0','true'),
      ('gic_redistributor','(o.redistributor_waker&(UINT32_C(1)<<2))==0','true'),
      ('gic_sre','(o.sre&1u)!=0','true'),
      ('gic_pmr','(o.pmr==UINT64_C(0xf8) || o.pmr==UINT64_C(0xfc) ||\n         o.pmr==UINT64_C(0xfe) || o.pmr==UINT64_C(0xff))','true'),
      ('gic_group_enable','(o.group_enable&1u)!=0','true'),
      ('gic_group','(o.timer_group&(UINT32_C(1)<<30))!=0','true'),
      ('gic_enable','(o.timer_enable&(UINT32_C(1)<<30))!=0','true'),
      ('ap_result','o.psci_result==0','true'),
      ('ap_distinct_tables','o.ttbr0!=o.ttbr1','true'),
      ('ap_alignment','(o.ttbr0&UINT64_C(0xfff))==0','true'),
      ('ap_ack','o.acknowledged','true'),
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
        record,=ownership_admission.parse(plan.read_bytes());admission=ownership_admission.admit_record(record,w/'Arm64Phase4Ownership.lean',a.lean_root)
    names=['src/arm64_boot_interface.cesta','src/arm64_phase4_policy.cesta','tests/arm64_phase4.py','tests/arm64_phase4_policy/host.c']
    (w/'summary.json').write_text(json.dumps({'builds':builds,'mutations':caught,'admission':admission,
      'source_sha256':{n:hashlib.sha256((HERE/n).read_bytes()).hexdigest()for n in names},
      'scope':'checked Phase-4 state transitions for timer switching, fault containment, GIC observation, AP acknowledgment and timer interval; no hardware execution claim'},indent=2)+'\n')
    print('ARM64 Phase-4 policy qualification passed:',len(builds),'builds,',len(caught),'mutants')
if __name__=='__main__':main()
