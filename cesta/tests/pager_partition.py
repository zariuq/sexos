#!/usr/bin/env python3
"""Qualify kernel enforcement of deterministic frame partitions."""
import argparse,hashlib,json,subprocess,sys,tempfile
from pathlib import Path
from layout_text import LayoutText
HERE=Path(__file__).resolve().parent.parent
def run(argv):
    r=subprocess.run(list(map(str,argv)),cwd=HERE,capture_output=True)
    if r.returncode:raise RuntimeError((argv,r.returncode,r.stdout[-1000:],r.stderr[-5000:]))
    return r.stdout
def main():
    p=argparse.ArgumentParser();p.add_argument('--cesta',type=Path,required=True);p.add_argument('--work',type=Path);p.add_argument('--mutations',action='store_true')
    p.add_argument('--cesta-source',type=Path);p.add_argument('--lean-root',type=Path);a=p.parse_args()
    w=a.work or Path(tempfile.mkdtemp(prefix='pager-partition-',dir=HERE/'build'));w.mkdir(parents=True,exist_ok=True)
    src=w/'pager_state.c';plan=w/'pager_state.plan'
    run([a.cesta,'export-c','src/pager_state.cesta','--std=c11','--plan',plan,'-o',src,'--',
         '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1','-Igenerated/include'])
    flags=['-std=c11','-g0','-Wall','-Wextra','-Wpedantic','-Werror','-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1','-Igenerated/include']
    def build(cc,extra,name):out=w/name;run([cc,*flags,*extra,'tests/pager_partition/host.c',src,'-o',out]);return out
    builds=[]
    for cc in ('gcc','clang'):
        for mode in ('O0','O2','sanitize'):
            extra=['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all']if mode=='sanitize'else['-'+mode]
            run([build(cc,extra,cc+'-'+mode)]);builds.append({'compiler':cc,'mode':mode})
    changes=[
      ('mapping_partition','!pager_partition_owns(m,p.length/4096,frame)','(!pager_partition_owns(m,p.length/4096,frame)&&false)'),
      ('ledger_partition','b.frame_index<base || b.frame_index-base>=quota','((b.frame_index<base || b.frame_index-base>=quota)&&false)'),
      ('partition_base','(uint64_t)(m.range_id-1)*m.quota','0'),
      ('partition_capacity','m.quota>frames/PG_CLIENTS','false'),
    ];caught=[]
    if a.mutations:
        original=LayoutText(src.read_text())
        for name,before,after in changes:
            assert original.count(before)>=1,(name,'anchor')
            try:
                src.write_text(original.replace(before,after,1));binary=build('gcc',['-O2'],name)
                result=subprocess.run([binary],cwd=HERE,capture_output=True);assert result.returncode!=0,('survived',name);caught.append(name)
            finally:src.write_text(original)
    admission=None
    if a.lean_root:
        assert a.cesta_source;sys.path.insert(0,str(a.cesta_source/'tests'));import ownership_admission
        record,=ownership_admission.parse(plan.read_bytes());admission=ownership_admission.admit_record(record,w/'PagerPartitionOwnership.lean',a.lean_root)
    names=['src/pager_state.cesta','tests/pager_partition.py','tests/pager_partition/host.c']
    (w/'summary.json').write_text(json.dumps({'builds':builds,'mutations':caught,'admission':admission,
      'source_sha256':{n:hashlib.sha256((HERE/n).read_bytes()).hexdigest()for n in names},
      'scope':'partition-enabled pure pager validation and ledger commutation'},indent=2)+'\n')
    print('pager partition qualification passed:',len(builds),'builds,',len(caught),'mutants')
if __name__=='__main__':main()
