#!/usr/bin/env python3
"""Qualify the permanent-AP-park state used by the current replay profile."""
import argparse,hashlib,json,subprocess,sys,tempfile
from pathlib import Path
HERE=Path(__file__).resolve().parent.parent
def run(argv):
    result=subprocess.run(list(map(str,argv)),cwd=HERE,capture_output=True)
    if result.returncode:raise RuntimeError((argv,result.returncode,result.stdout[-1000:],result.stderr[-5000:]))
    return result.stdout
def main():
    p=argparse.ArgumentParser();p.add_argument('--cesta',type=Path,required=True);p.add_argument('--work',type=Path)
    p.add_argument('--mutations',action='store_true');p.add_argument('--cesta-source',type=Path);p.add_argument('--lean-root',type=Path);a=p.parse_args()
    w=a.work or Path(tempfile.mkdtemp(prefix='smp-park-',dir=HERE/'build'));w.mkdir(parents=True,exist_ok=True)
    src=w/'smp_park.c';plan=w/'smp_park.plan'
    run([a.cesta,'export-c','src/smp_park.cesta','--std=c11','--plan',plan,'-o',src,'--','-Igenerated/include'])
    flags=['-std=c11','-g0','-Wall','-Wextra','-Wpedantic','-Werror','-Igenerated/include']
    def build(cc,extra,name):out=w/name;run([cc,*flags,*extra,'tests/smp_park/host.c',src,'-o',out]);return out
    builds=[]
    for cc in ('gcc','clang'):
        for mode in ('O0','O2','sanitize'):
            extra=['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all']if mode=='sanitize'else['-'+mode]
            run([build(cc,extra,cc+'-'+mode)]);builds.append({'compiler':cc,'mode':mode})
    changes=[
      ('unique_lapic','state->cores[i].lapic_id==lapic_id','state->cores[i].lapic_id==(lapic_id^1u)'),
      ('bsp_identity','bsp!=(lapic_id==state->bsp_lapic_id)','false'),
      ('arrival_identity','core->lapic_id!=lapic_id','core->lapic_id!=(lapic_id^1u)'),
      ('duplicate_arrival','core->arrived || core->parked','false'),
      ('seal_parked','!core.parked','false'),
      ('quiescent_parked','!state->cores[i].parked','false'),
      ('commit_generation','state->generation==expected','true'),
      ('advance_once','next!=expected+1u','false'),
    ];caught=[]
    if a.mutations:
        original=src.read_text()
        for name,before,after in changes:
            assert original.count(before)>=1,(name,'anchor')
            try:
                src.write_text(original.replace(before,after,1));binary=build('gcc',['-O2'],name)
                result=subprocess.run([binary],cwd=HERE,capture_output=True);assert result.returncode!=0,('survived',name);caught.append(name)
            finally:src.write_text(original)
    admission=None
    if a.lean_root:
        assert a.cesta_source;sys.path.insert(0,str(a.cesta_source/'tests'));import ownership_admission
        record,=ownership_admission.parse(plan.read_bytes());admission=ownership_admission.admit_record(record,w/'SmpParkOwnership.lean',a.lean_root)
    names=['src/smp_park.cesta','src/smp_park_interface.cesta','tests/smp_park.py','tests/smp_park/host.c']
    (w/'summary.json').write_text(json.dumps({'builds':builds,'mutations':caught,'admission':admission,
      'source_sha256':{n:hashlib.sha256((HERE/n).read_bytes()).hexdigest()for n in names},
      'scope':'bounded permanent AP enrollment/park and BSP epoch permission; no AP scheduling or shootdown'},indent=2)+'\n')
    print('SMP park qualification passed:',len(builds),'builds,',len(caught),'mutants')
if __name__=='__main__':main()
