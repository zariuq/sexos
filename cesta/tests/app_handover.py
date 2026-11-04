#!/usr/bin/env python3
"""Independent whole-manifest handover model and bounded notification controls."""
import argparse,hashlib,json,random,subprocess,sys,tempfile
from pathlib import Path
HERE=Path(__file__).resolve().parent.parent
U64=(1<<64)-1;PHYS=0x000ffffffffff000
def run(argv,data=None):
 p=subprocess.run(list(map(str,argv)),cwd=HERE,input=data,capture_output=True)
 if p.returncode:raise RuntimeError((list(map(str,argv)),p.returncode,p.stdout[-1000:],p.stderr[-5000:]))
 return p.stdout
def model(v):
 if v[0]:
  acked,hinted,last,now,role=v[1]&255,v[2]&255,v[3],v[4],v[5]&255
  due=role<6 and not(role<5 and acked&(1<<role)) and(not(hinted&(1<<role))or((now-last)&U64)>=16)
  return str(int(due))
 n=v[1];p=[[(i+1)*4096,i+1,(i+1)*4096|5|(9<<59)] for i in range(512)];q=[[(1024+i+1)*4096,513+i,(i+1)*4096|7|(14<<59)|(1<<63)] for i in range(512)]
 state=[0]*1026
 for i in range(min(n,512)):state[i+1]=p[i][2]|v[5]
 count=n;at,field,value=v[2:5]
 if at<512:
  if 1<=field<=3:p[at][field-1]=value
  elif 4<=field<=6:q[at][field-4]=value
  elif field==7:state[at+1]=value
  elif field==8:state[513+at]=value
  elif field==9:count=value
 valid=0<n<=512 and n==count;error=1 if not valid else 3
 if valid:
  src,dst=p[:n],q[:n]
  valid=all(a[0]>0 and a[0]%4096==0 and a[1]>0 and a[2]&5==5 for a in src)
  valid&=all(a[0]>0 and a[0]%4096==0 and a[1]>0 and a[2]&7==7 and a[2]>>63 for a in dst)
  valid&=all((a[2]&PHYS)==(b[2]&PHYS) for a,b in zip(src,dst))
  valid&=all((state[a[1]] if a[1]<1026 else U64)&~0x60==a[2]&~0x60 for a in src)
  valid&=all((state[a[1]] if a[1]<1026 else U64)==0 for a in dst)
  valid&=all(len({a[f] for a in group})==n for group in (src,dst) for f in (0,1))
  valid&=len({a[2]&PHYS for a in src})==n
  valid&=not({a[0] for a in src}&{a[0] for a in dst})and not({a[1] for a in src}&{a[1] for a in dst})
 effects=[]
 if valid:
  for group,clear in ((p[:n],True),(q[:n],False)):
   for va,leaf,expected in group:
    value=0 if clear else expected;state[leaf]=value;effects.extend([f'W:{leaf}:{value}',f'F:{va}:0'])
 out=f'{0 if valid else error} {n if valid else 0}'
 if effects:out+='|'+'|'.join(effects)
 return out+'|L'+''.join(':'+str(x)for x in state[1:])
def vectors(seed,cases):
 rows=[]
 def add(v):rows.append(v+[0]*(12-len(v)))
 for n in (0,1,3,8,512,513):
  add([0,n,0,0,0,0])
  for ad in (0x20,0x40,0x60):add([0,n,0,0,0,ad])
 for n in (1,3,8):
  for i in range(n):
   for field in range(1,10):
    for value in (0,1,4096,8192,513,65536,U64):add([0,n,i,field,value,0])
 for role in range(256):
  for ack in (0,1,31,255):
   for hint in (0,1,63,255):
    for last,now in ((0,0),(100,115),(100,116),(U64-8,7)):add([1,ack,hint,last,now,role])
 rng=random.Random(seed)
 for _ in range(cases):add([0,rng.choice((1,3,8)),rng.randrange(8),rng.randrange(10),rng.getrandbits(64),rng.choice((0,0x20,0x40,0x60))])
 return rows
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--cesta',type=Path,required=True);ap.add_argument('--work',type=Path);ap.add_argument('--cases',type=int,default=200);ap.add_argument('--mutations',action='store_true');ap.add_argument('--lean-root',type=Path);ap.add_argument('--cesta-source',type=Path);args=ap.parse_args()
 w=args.work or Path(tempfile.mkdtemp(prefix='handover-',dir=HERE/'build'));w.mkdir(parents=True,exist_ok=True)
 rows=vectors(20261101,args.cases);data=''.join(' '.join(map(str,v))+'\n'for v in rows).encode();expected=''.join(model(v)+'\n'for v in rows).encode();(w/'commands.txt').write_bytes(data);(w/'model.out').write_bytes(expected)
 source=w/'app_private.c';plan=w/'private.plan';run([args.cesta,'export-c','src/app_private.cesta','--std=c11','-o',source,'--plan',plan,'--','-Igenerated/include'])
 flags=['-std=c11','-g0','-Wall','-Wextra','-Wpedantic','-Werror','-Igenerated/include'];builds=[]
 def build(cc,extra,name):
  binary=w/name;run([cc,*flags,*extra,'tests/app_handover/host.c',source,'-o',binary]);return binary
 def check(binary):
  out=run([binary],data)
  if out!=expected:
   (w/(binary.name+'.out')).write_bytes(out);a=out.splitlines();b=expected.splitlines();i=next(i for i,p in enumerate(zip(a,b))if p[0]!=p[1]);raise AssertionError((binary.name,i,rows[i],a[i][:300],b[i][:300]))
 for cc in ('gcc','clang'):
  for mode in ('O0','O2','sanitize'):
   extra=['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all'] if mode=='sanitize' else ['-'+mode]
   check(build(cc,extra,cc+'-'+mode));builds.append({'compiler':cc,'mode':mode,'requests':len(rows)})
  print(cc,'complete handover effects and hint controls match independent model',flush=True)
 caught=[]
 if args.mutations:
  original=source.read_text();changes=[('early_publish','for(size_t i=0;i<p->count;++i){\n        app_private_leaf_write(p->pages[i].leaf,0);','for(size_t i=0;i<p->count;++i){\n        app_private_leaf_write(workspace->pages[i].leaf,workspace->pages[i].expected);\n        app_private_leaf_write(p->pages[i].leaf,0);'),('ignore_alias_present','app_private_leaf_read(next.leaf)','false'),('wrong_physical','(old.expected&APP_PRIVATE_PHYSICAL)!=(next.expected&APP_PRIVATE_PHYSICAL)','false'),('ignore_last_old_leaf','(app_private_leaf_read(old.leaf)&~UINT64_C(0x60))!=(old.expected&~UINT64_C(0x60))','false'),('bad_backoff','now-last>=16','now-last>16'),('lost_ack','recipient<5 && (acked&(1u<<recipient))','recipient<5 && (acked&0)')]
  for name,before,after in changes:
   assert before in original,(name,'anchor')
   try:
    source.write_text(original.replace(before,after));binary=build('gcc',['-O2'],name);result=subprocess.run([binary],cwd=HERE,input=data,capture_output=True);assert result.returncode or result.stdout!=expected,('survived',name);caught.append(name)
   finally:source.write_text(original)
 admission=None
 if args.lean_root:
  assert args.cesta_source;sys.path.insert(0,str(args.cesta_source/'tests'));import ownership_admission
  record,=ownership_admission.parse(plan.read_bytes());admission=ownership_admission.admit_record(record,w/'HandoverOwnership.lean',args.lean_root)
 names=['src/app_private.cesta','src/app_private_interface.cesta','tests/app_handover.py','tests/app_handover/host.c']
 (w/'summary.json').write_text(json.dumps({'builds':builds,'mutations':caught,'admission':admission,'source_sha256':{n:hashlib.sha256((HERE/n).read_bytes()).hexdigest()for n in names},'scope':'independent complete-manifest/final-leaf/effect model and modulo64 backoff; native ownership/trusted leaf/CPU fences remain separate.'},indent=2)+'\n')
if __name__=='__main__':main()
