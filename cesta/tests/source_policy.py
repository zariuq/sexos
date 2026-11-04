#!/usr/bin/env python3
"""Compare bounded provider calls and complete register replies with actual Rust."""
import argparse,datetime,hashlib,json,random,subprocess,sys,tempfile
from pathlib import Path
HERE=Path(__file__).resolve().parent.parent
U64=(1<<64)-1

def run(argv,data=None):
 p=subprocess.run(list(map(str,argv)),cwd=HERE,input=data,capture_output=True)
 if p.returncode:raise RuntimeError((list(map(str,argv)),p.returncode,p.stdout[-1000:],p.stderr[-5000:]))
 return p.stdout

def vectors(seed,cases):
 rows=[];controls=[]
 def base(op):
  v=[0]*32;v[:10]=[op,1,13,13,15,15,1,13,13,2];v[13:17]=[1,1,0,85];v[18:28]=[0,0,0,1,1,0x20,0x20,0x26,2,0x80]
  if op==2:v[9]=1
  return v
 def add(v,expect=None):
  if expect is not None:controls.append((len(rows),expect))
  rows.append(tuple(v))
 for op in range(3):
  good=base(op);add(good)
  for field in range(1,13):
   for n in (0,1,2,3,13,15,(1<<32)-1,1<<32,(1<<32)|1,1<<63,U64):
    v=good[:];v[field]=n;add(v)
 for available in (0,1):
  for success in range(10):
   for word in (0,1,U64):v=base(0);v[13:16]=[available,success,word];add(v)
 for mode in (1,2,3):
  v=base(0);v[9]=mode;v[1]=0;add(v,[U64,0,0,0,0,0,0,0])
 # Full raw-byte perturbation, including all mode/status and calendar bytes.
 for field in range(18,28):
  for byte in range(256):
   v=base(1);v[field]=byte;add(v);v=v[:];v[0]=3;add(v)
 for pattern in range(256):
  v=base(1);v[16]=pattern;v[28]=pattern%10;v[29]=1;add(v)
 for field in range(10):
  v=base(1);v[16]=170;v[28]=field;v[29]=1;add(v,[U64-2,0,0,0,0,0,0,4])
 # Independent Unix calendar witnesses (UTC, not the Rust conversion).
 rng=random.Random(seed)
 def bcd(n):return (n//10)*16+n%10
 # Boundary dates and every AM/PM hour need positive witnesses: changing one
 # raw byte at midnight does not exercise valid twelve-hour representations.
 for year,month,day in [(2020,2,29),(2024,2,29),(2096,2,29),(2099,12,31),
                        (2019,12,31),(2100,1,1),(2021,2,29),(2020,2,30),(2024,4,31)]:
  for binary in (False,True):
   v=base(1);fields=[57,58,23,day,month,year%100,year//100]
   v[18:25]=fields if binary else list(map(bcd,fields));v[26]=6 if binary else 2
   try:
    date=datetime.datetime(year,month,day,23,58,57,tzinfo=datetime.timezone.utc)
    valid=2020<=year<=2099
   except ValueError:valid=False
   add(v,[0,int(date.timestamp()),1,1,0,0,0,1] if valid else [U64-3,0,0,0,0,0,0,1])
 for binary in (False,True):
  for hour in range(24):
   v=base(1);fields=[57,58,hour%12 or 12,29,2,24,20]
   v[18:25]=fields if binary else list(map(bcd,fields))
   if hour>=12:v[20]|=0x80
   v[26]=4 if binary else 0
   epoch=int(datetime.datetime(2024,2,29,hour,58,57,tzinfo=datetime.timezone.utc).timestamp())
   add(v,[0,epoch,1,1,0,0,0,1])
 for _ in range(cases):
  year=rng.randrange(2020,2100);month=rng.randrange(1,13);day=rng.randrange(1,29);hour=rng.randrange(24);minute=rng.randrange(60);second=rng.randrange(60)
  for binary in (False,True):
   v=base(1);v[18:25]=[second,minute,hour,day,month,year%100,20] if binary else list(map(bcd,[second,minute,hour,day,month,year%100,20]));v[26]=6 if binary else 2
   epoch=int(datetime.datetime(year,month,day,hour,minute,second,tzinfo=datetime.timezone.utc).timestamp());add(v,[0,epoch,1,1,0,0,0,1])
  v=base(rng.randrange(3));v[rng.randrange(1,32)]=rng.getrandbits(64);add(v)
 return rows,controls

def main():
 ap=argparse.ArgumentParser();ap.add_argument('--cesta',type=Path,required=True);ap.add_argument('--work',type=Path);ap.add_argument('--seed',type=int,default=20261031);ap.add_argument('--cases',type=int,default=2000);ap.add_argument('--mutations',action='store_true');ap.add_argument('--lean-root',type=Path);ap.add_argument('--cesta-source',type=Path);args=ap.parse_args()
 w=args.work or Path(tempfile.mkdtemp(prefix='source-policy-',dir=HERE/'build'));w.mkdir(parents=True,exist_ok=True)
 inventory=json.loads((HERE/'interfaces/source_policy.source.json').read_text())
 for entry in inventory['sources']:
  assert hashlib.sha256((HERE.parent/entry['path']).read_bytes()).hexdigest()==entry['sha256'],entry['path']
 actual={'entropy':'crates/sex-pdx/src/entropy_source.rs','clock':'crates/sex-pdx/src/clock_source.rs','trust':'crates/sex-pdx/src/trust_source.rs','trust_config':'crates/sex-trust-config/src/lib.rs'}
 for name,path in actual.items():
  raw=(HERE.parent/path).read_bytes()
  if name=='clock':raw+=b'\npub fn probe_seconds(raw:Raw)->Option<u64>{seconds(raw)}\n'
  (w/('actual_'+name+'.rs')).write_bytes(raw)
 (w/'oracle.rs').write_bytes((HERE/'tests/source_policy/oracle.rs').read_bytes());run(['rustc','--edition=2021','--cap-lints','allow','-O',w/'oracle.rs','-o',w/'rust'])
 rows,controls=vectors(args.seed,args.cases);data=''.join(' '.join(map(str,v))+'\n' for v in rows).encode();(w/'commands.txt').write_bytes(data)
 expected=run([w/'rust'],data);(w/'rust.out').write_bytes(expected);lines=expected.splitlines()
 for i,words in controls:assert list(map(int,lines[i].split()))==words,(i,rows[i],lines[i],words)
 source=w/'source_policy.c';plan=w/'sources.plan';run([args.cesta,'export-c','src/source_policy.cesta','--std=c11','--plan',plan,'-o',source,'--','-Igenerated/include'])
 flags=['-std=c11','-g0','-Wall','-Wextra','-Wpedantic','-Werror','-Igenerated/include'];builds=[]
 def check(binary):
  out=run([binary],data)
  if out!=expected:
   (w/(binary.name+'.out')).write_bytes(out);a=out.splitlines();i=next((i for i,pair in enumerate(zip(a,lines)) if pair[0]!=pair[1]),min(len(a),len(lines)));raise AssertionError((binary.name,i,rows[i],a[i:i+1],lines[i:i+1]))
 for cc in ('gcc','clang'):
  for mode in ('O0','O2','sanitize'):
   extra=['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all'] if mode=='sanitize' else ['-'+mode]
   binary=w/(cc+'-'+mode);run([cc,*flags,*extra,'tests/source_policy/host.c',source,'-o',binary]);check(binary)
   if mode=='O2':
    authored=w/(cc+'-authored');run([args.cesta,'cc','--cc',cc,*flags,*extra,'tests/source_policy/host.c','src/source_policy.cesta','-o',authored]);assert authored.read_bytes()==binary.read_bytes();check(authored)
   builds.append({'compiler':cc,'mode':mode,'commands':len(rows)})
  print(cc,len(rows),'register/provider traces match real Rust',flush=True)
 changes=[('wrong_task','a.task==b.task','true'),('reserved_alias','!(a|b|c)','!((a|b|c)&UINT64_C(0))'),('sample_zero_sentinel','if(word.present)','if(word.present && word.value)'),('unbounded_entropy','attempt<=SOURCE_ENTROPY_ATTEMPTS','attempt<SOURCE_ENTROPY_ATTEMPTS'),('clock_no_pair_check','!source_clock_raw_equal(first,pair.second)','false'),('clock_no_uip','(first.status_a&0x80)','false'),('wrong_busy_precedence','busy?SOURCE_BUSY:SOURCE_UNAVAILABLE','busy?SOURCE_UNAVAILABLE:SOURCE_BUSY'),('bad_dst','if(first.status_b&1)','if(false)'),('wrong_calendar','day.value>source_month_days(year,month.value)','day.value>31'),('reject_leap_day','!day.value || day.value>','!day.value || (month.value==2 && day.value==29) || day.value>'),('no_twelve_hour_conversion','hour.value=hour.value%12+((raw.hour&0x80)?12:0);','hour.value=hour.value;'),('wrong_trust_pack','g.consumer.pd|((uint64_t)g.consumer.task<<32)','((uint64_t)g.consumer.pd<<32)|g.consumer.task')];caught=[]
 if args.mutations:
  original=source.read_text()
  for name,before,after in changes:
   assert before in original,(name,'anchor')
   try:
    source.write_text(original.replace(before,after));binary=w/name;run(['gcc',*flags,'-O2','tests/source_policy/host.c',source,'-o',binary]);assert run([binary],data)!=expected,('mutation survived',name);caught.append(name)
   finally:source.write_text(original)
 admission=None
 if args.lean_root:
  assert args.cesta_source;sys.path.insert(0,str(args.cesta_source/'tests'));import ownership_admission
  record,=ownership_admission.parse(plan.read_bytes());admission=ownership_admission.admit_record(record,w/'SourceOwnership.lean',args.lean_root)
 names=['src/source_policy.cesta','src/source_policy_interface.cesta','interfaces/source_policy.source.json','tests/source_policy.py','tests/source_policy/host.c','tests/source_policy/oracle.rs']
 (w/'summary.json').write_text(json.dumps({'pin':inventory['pin'],'builds':builds,'independent_controls':len(controls),'mutations':caught,'admission':admission,'sources':{n:hashlib.sha256((HERE/n).read_bytes()).hexdigest() for n in names},'rust_sources':{n:hashlib.sha256((HERE.parent/n).read_bytes()).hexdigest() for n in actual.values()},'scope':'pure actual Rust authorization/calendar/provider ordering; independent UTC epoch controls. Hardware/provider integrity remains a native boundary.'},indent=2)+'\n')
if __name__=='__main__':main()
