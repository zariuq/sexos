"""Independent model for checked serial Linen publication."""
import argparse,hashlib,importlib,json,os,random,subprocess,sys
from pathlib import Path

HERE=Path(__file__).resolve().parent.parent
MAX=(1<<64)-1
SEEDS=[dict(p=1,id=100,k=1,s=3,label=2,n=b'',disk=0),dict(p=1,id=101,k=0,s=3,label=3,n=b'',disk=0)]+[dict(p=0,id=0,k=0,s=0,label=0,n=b'',disk=0)for _ in range(4)]
EMPTY=dict(p=0,id=0,k=0,s=0,label=0,n=b'',disk=0)

def line(state,ok,revision):
 out=f'{int(ok)} {revision} {state["selected"]} {int(state["fetched"])} {int(state["real"])}'
 for row in state['rows']:
  name=row['n'].ljust(24,b'\0')[:24].hex()
  out+=f'|{row["p"]},{row["id"]},{row["k"]},{row["s"]},{row["label"]},{len(row["n"])},{row["disk"]},{name}'
 return out

def vectors(cases):
 rng=random.Random(0x51A17);commands=[];expected=[]
 state=dict(selected=0,fetched=False,real=False,rows=[dict(EMPTY)for _ in range(16)])
 def row(ident,kind=0,disk=0,name=b'x'):return dict(id=ident,kind=kind,disk=disk,name=name)
 directed=[
  (False,0,0,[row(1,1,7,bytes(range(24))),None,row((1<<63)+9,0,15,b'\x00\xffz')]),
  (False,8,(1<<63)+9,[row(21,0,3,b'a'),row((1<<63)+9,1,4,b'b')]),
  (False,9,999,[]),(True,10,0,[]),
  (False,MAX,100,[row(30)]),(False,12,100,[row(0)]),(False,12,100,[row(MAX)]),
  (False,12,100,[row(31),row(31)]),(False,12,100,[row(32,name=b'a'*25)])]
 scenarios=list(directed)
 for n in range(cases):
  slots=[]
  for i in range(16):
   if rng.randrange(3):
    ident=(1<<63)+n*32+i+1;name=bytes(rng.randrange(256)for _ in range(rng.randrange(25)))
    slots.append(row(ident,rng.randrange(3),rng.randrange(16),name))
   else:slots.append(None)
  choices=[0,100,101,*[r['id']for r in slots if r]]
  scenarios.append((bool(rng.randrange(2)),rng.randrange(0,MAX),rng.choice(choices),slots))
 for legacy,revision,selected,source in scenarios:
  slots=(list(source)+[None]*16)[:16];declared=sum(r is not None for r in slots)
  malformed=revision==MAX;ids=[]
  for r in slots:
   if not r:continue
   malformed|=r['id']in(0,MAX)or len(r['name'])>24 or r['id']in ids;ids.append(r['id'])
  commands.append(f'{int(legacy)} {revision} {selected} {declared}')
  for r in slots:
   if r is None:r=dict(id=0,kind=0,disk=0,name=b'')
   physical=r['name'][:24].ljust(24,b'\0')
   commands.append(' '.join(map(str,[int(r.get('id',0)!=0),r['id'],r['kind'],r['disk'],len(r['name']),*physical])))
  state['selected']=selected
  if malformed:
   expected.append(line(state,False,revision));continue
  rows=[]
  for r in slots:
   if r is None:rows.append(dict(EMPTY));continue
   rows.append(dict(p=1,id=r['id'],k=0 if r['kind']==1 else 1,s=3,label=1,n=r['name'],disk=r['disk']))
  real=declared!=0
  if not real and legacy:rows=[dict(r)for r in SEEDS]+[dict(EMPTY)for _ in range(10)]
  ids=[r['id']for r in rows if r['p']]
  newselected=selected if selected in ids and selected!=0 else(ids[0]if ids else 0)
  state=dict(selected=newselected,fetched=True,real=real,rows=rows)
  expected.append(line(state,True,revision+1))
 return ('\n'.join(commands)+'\n').encode(),('\n'.join(expected)+'\n').encode(),len(scenarios)

def main():
 ap=argparse.ArgumentParser();ap.add_argument('--cesta',type=Path,required=True);ap.add_argument('--cesta-source',type=Path);ap.add_argument('--lean-root',type=Path);ap.add_argument('--cases',type=int,default=256);ap.add_argument('--output',type=Path,default=HERE/'build/models-publish');args=ap.parse_args()
 w=args.output.resolve();w.mkdir(parents=True,exist_ok=True);include='-I'+str(HERE/'generated/include');data,expected,count=vectors(args.cases);records=[];plans={}
 for cc in ('gcc','clang'):
  sources=[]
  for unit in ('silk_shell_models_projection','silk_shell_models_publish'):
   c=w/(unit+'-'+cc+'.c');plan=c.with_suffix('.plan')
   subprocess.run([args.cesta,'export-c',HERE/('src/'+unit+'.cesta'),'--cc',cc,'--std=c11','-o',c,'--plan',plan,'--',include],check=True)
   sources.append(c);plans[hashlib.sha256(plan.read_bytes()).hexdigest()]=plan
  for mode,flags in [('O0',['-O0']),('O2',['-O2']),('SAN',['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-pie','-no-pie'])]:
   binary=w/(cc+'-'+mode)
   subprocess.run([cc,'-std=c11','-Wall','-Wextra','-Wpedantic','-Werror',*flags,include,*sources,HERE/'tests/silk_shell_models_publish/host.c','-o',binary],check=True)
   result=subprocess.run([binary],input=data,capture_output=True,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=1:abort_on_error=1'})
   assert result.returncode==0,result.stderr[-4000:]
   if result.stdout!=expected:
    actual=result.stdout.splitlines();wanted=expected.splitlines()
    mismatch=next((i for i,(a,b)in enumerate(zip(actual,wanted))if a!=b),min(len(actual),len(wanted)))
    raise AssertionError((cc,mode,mismatch,actual[mismatch:mismatch+1],wanted[mismatch:mismatch+1]))
   records.append(dict(compiler=cc,mode=mode,scenarios=count,trace_sha256=hashlib.sha256(result.stdout).hexdigest()))
 original=(HERE/'src/silk_shell_models_publish.cesta').read_text();mutations=(
  ('skip_revision_guard','if(linen->publication_revision==UINT64_MAX)return false;',''),
  ('allow_reserved','row->id==0 || row->id==UINT64_MAX','false'),
  ('allow_duplicate','for(size_t j=0;j<i;++j)if(snapshot->present[j] && snapshot->objects[j].id==row->id)return false;',''),
  ('allow_bad_count','if(count!=snapshot->count)return false;',''),
  ('seed_strong_empty','if(!real && legacy_seed_empty)','if(!real)'),
  ('drop_selection_retention','if(retained)selected=previous;',''),
 )
 for name,old,new in mutations:
  assert original.count(old)==1,(name,original.count(old));source=w/(name+'.cesta');source.write_text(original.replace(old,new));c=w/(name+'.c')
  subprocess.run([args.cesta,'export-c',source,'--cc','gcc','--std=c11','-o',c,'--',include],check=True);binary=w/name
  subprocess.run(['gcc','-std=c11','-O2',include,w/'silk_shell_models_projection-gcc.c',c,HERE/'tests/silk_shell_models_publish/host.c','-o',binary],check=True)
  result=subprocess.run([binary],input=data,capture_output=True)
  assert result.returncode!=0 or result.stdout!=expected,('surviving mutation',name)
 admission=[]
 if args.lean_root:
  assert args.cesta_source;sys.path.insert(0,str(args.cesta_source.resolve()/'tests'));native=importlib.import_module('ownership_admission')
  for i,plan in enumerate(plans.values()):
   record,=native.parse(plan.read_bytes());admission.append(native.admit_record(record,w/f'Publish{i}.lean',args.lean_root.resolve()))
 summary=dict(runtime=records,runtime_mutants_caught=len(mutations),admission=admission,native_array_boundary=False)
 (w/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
 print('Linen publication:',count,'scenarios/configuration;',len(records),'configurations;',len(mutations),'mutations caught;',sum(x['functions']for x in admission),'plans admitted')

if __name__=='__main__':main()
