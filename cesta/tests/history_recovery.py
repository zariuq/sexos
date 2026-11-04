#!/usr/bin/env python3
"""Actual generic Rust roster/decoder versus Cesta,including all query effects."""
import argparse,hashlib,json,random,subprocess,sys,tempfile
from pathlib import Path
HERE=Path(__file__).resolve().parent.parent
U64=(1<<64)-1

def run(argv,data=None):
 p=subprocess.run(list(map(str,argv)),cwd=HERE,input=data,capture_output=True)
 if p.returncode:raise RuntimeError((list(map(str,argv)),p.returncode,p.stdout[-1000:],p.stderr[-6000:]))
 return p.stdout

def vectors(n,seed,cases):
 base=[0,11,12,1808,18,11,0,0,12,1012,11,12,0,0,0,0,0,0,12,1012];rows=[]
 for op in (0,1):
  for field in range(1,20):
   for value in (0,1,2,3,8,11,12,17,18,64,65,(1<<32)-1,1<<32,U64-18,U64-17,U64):
    v=base[:];v[0]=op;v[field]=value;rows.append(v)
  v=base[:];v[0]=op;rows.append(v)
 for i in range(n):
  for target,task in ((0,1012),(12+i,0),(12,1012),(12+i,1012),(12,1012+13*i),(12+i,1012+13*i)):
   v=base[:];v[0]=1;v[7:10]=[i+1,target,task];rows.append(v)
 for kind in range(n+3):
  for field in range(5):
   for value in (0,1,2,3,11,12,18,n,1<<32,U64):
    for canonical in ((0,1) if field==0 else (0,)):
     v=base[:];v[0]=1;v[14:18]=[kind+1,field,value,canonical];rows.append(v)
 rng=random.Random(seed+n)
 for _ in range(cases):
  v=base[:];v[0]=rng.randrange(2);v[rng.randrange(1,20)]=rng.getrandbits(64);rows.append(v)
 return rows

def main():
 ap=argparse.ArgumentParser();ap.add_argument('--cesta',type=Path,required=True);ap.add_argument('--work',type=Path)
 ap.add_argument('--counts',default='0,1,2,8,64,65');ap.add_argument('--seed',type=int,default=20261101);ap.add_argument('--cases',type=int,default=500)
 ap.add_argument('--mutations',action='store_true');ap.add_argument('--lean-root',type=Path);ap.add_argument('--cesta-source',type=Path);args=ap.parse_args()
 w=args.work or Path(tempfile.mkdtemp(prefix='history-roster-',dir=HERE/'build'));w.mkdir(parents=True,exist_ok=True)
 inv=json.loads((HERE/'interfaces/history_recovery.source.json').read_text())
 for r in inv['sources']:assert hashlib.sha256((HERE.parent/r['path']).read_bytes()).hexdigest()==r['sha256'],r['path']
 counts=[int(n) for n in args.counts.split(',')];builds=[];caught=[];admission=[]
 for n in counts:
  d=w/str(n);d.mkdir(exist_ok=True);(d/'actual_history.rs').write_bytes((HERE.parent/'crates/sex-pdx/src/history_recovery.rs').read_bytes());(d/'oracle.rs').write_bytes((HERE/'tests/history_recovery/oracle.rs').read_bytes())
  (d/'count.rs').write_text(f'pub const SPINDLE_COUNT:usize={n};pub const SPINDLE_TARGETS:[u32;{n}]=['+','.join(str(12+i) for i in range(n))+'];\n')
  run(['rustc','--edition=2021','--cap-lints','allow','-O',d/'oracle.rs','-o',d/'oracle'])
  rows=vectors(n,args.seed,args.cases);data=''.join(' '.join(map(str,v))+'\n' for v in rows).encode();(d/'commands.txt').write_bytes(data)
  expected=run([d/'oracle'],data);(d/'rust.out').write_bytes(expected)
  flags=['-std=c11','-g0','-Wall','-Wextra','-Wpedantic','-Werror','-Igenerated/include','-DHISTORY_ROSTER_COUNT='+str(n)]
  for unit in ('history_recovery','history_decode'):
   run([args.cesta,'export-c','src/'+unit+'.cesta','--std=c11','--plan',d/(unit+'.plan'),'-o',d/(unit+'.c'),'--',*flags])
  def build(cc,extra,name):
   binary=d/name;run([cc,*flags,*extra,'tests/history_recovery/host.c',d/'history_recovery.c',d/'history_decode.c','-o',binary]);return binary
  def check(binary):
   out=run([binary],data)
   if out!=expected:
    (d/(binary.name+'.out')).write_bytes(out);got=out.splitlines();want=expected.splitlines();i=next((i for i,p in enumerate(zip(got,want)) if p[0]!=p[1]),min(len(got),len(want)));raise AssertionError((n,binary.name,i,rows[i],got[i:i+1],want[i:i+1]))
  for cc in ('gcc','clang'):
   for mode in ('O0','O2','sanitize'):
    extra=['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all'] if mode=='sanitize' else ['-'+mode]
    binary=build(cc,extra,cc+'-'+mode);check(binary);builds.append({'count':n,'compiler':cc,'mode':mode,'requests':len(rows)})
    if mode=='O2':
     authored=d/(cc+'-authored');run([args.cesta,'cc','--cc',cc,*flags,*extra,'tests/history_recovery/host.c','src/history_recovery.cesta','src/history_decode.cesta','-o',authored]);assert authored.read_bytes()==binary.read_bytes()
  print('roster count',n,':all6 builds match complete Rust state/query traces',flush=True)
  if args.mutations and n==8:
   changes=[('duplicate_task','history_recovery','g.participants[j].task==p.task','false'),('wrong_holder','history_recovery','g.holder!=holder','((void)holder,false)'),('extent_wrap','history_recovery','g.base>UINT64_MAX-g.sectors','false'),('wrong_participant_task','history_recovery','found.value.task==task','((void)task,true)'),('narrow_kind','history_recovery','if(!kind)return','kind=(uint32_t)kind;\n    if(!kind)return'),('header_version','history_decode','header.value3!=HISTORY_VERSION','false'),('partial_extent','history_decode','extent.value2 || extent.value3','false'),('roster_order','history_decode','raw.value0!=targets.values[i]','((void)targets.values[i],false)'),('task_narrow','history_decode','raw.value1>UINT32_MAX','false'),('wrong_index','history_decode','raw.value2!=i','false'),('dirty_reply','history_decode','r.status && (r.value0|r.value1|r.value2|r.value3)?1:r.status','r.status'),('skip_final_validation','history_decode','if(!history_valid(g))','if(false)')]
   for name,unit,before,after in changes:
    path=d/(unit+'.c');original=path.read_text();assert before in original,(name,'anchor')
    try:
     path.write_text(original.replace(before,after));binary=build('gcc',['-O2'],name);assert run([binary],data)!=expected,('survived',name);caught.append(name)
    finally:path.write_text(original)
  if args.lean_root and n==8:
   assert args.cesta_source;sys.path.insert(0,str(args.cesta_source/'tests'));import ownership_admission
   for unit in ('history_recovery','history_decode'):
    record,=ownership_admission.parse((d/(unit+'.plan')).read_bytes());admission.append(ownership_admission.admit_record(record,d/(unit+'Ownership.lean'),args.lean_root))
 names=['src/history_recovery.cesta','src/history_decode.cesta','src/history_recovery_interface.cesta','tests/history_recovery.py','tests/history_recovery/host.c','tests/history_recovery/oracle.rs','interfaces/history_recovery.source.json']
 (w/'summary.json').write_text(json.dumps({'pin':inv['pin'],'builds':builds,'mutations':caught,'admission':admission,'source_sha256':{n:hashlib.sha256((HERE/n).read_bytes()).hexdigest() for n in names},'scope':'generic roster/query and atomic decoder,including error redaction/full returned widths and every provider reply. Held lookup/native ABI/BOOT/disk are separate gates.'},indent=2)+'\n')
if __name__=='__main__':main()
