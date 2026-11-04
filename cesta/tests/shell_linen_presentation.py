"""Three-slot Linen refresh/presentation lifecycle controls."""
import argparse,hashlib,importlib,json,os,subprocess,sys
from pathlib import Path

HERE=Path(__file__).resolve().parent.parent

def main():
 ap=argparse.ArgumentParser();ap.add_argument('--cesta',type=Path,required=True);ap.add_argument('--cesta-source',type=Path);ap.add_argument('--lean-root',type=Path);ap.add_argument('--output',type=Path,default=HERE/'build/linen-presentation');args=ap.parse_args()
 w=args.output.resolve();w.mkdir(parents=True,exist_ok=True);include='-I'+str(HERE/'generated/include');plans={};sources=[]
 for unit in('shell_text_delivery','shell_linen_presentation'):
  c=w/(unit+'.c');plan=c.with_suffix('.plan');subprocess.run([args.cesta,'export-c',HERE/('src/'+unit+'.cesta'),'--cc','gcc','--std=c11','-o',c,'--plan',plan,'--',include],check=True);sources.append(c);plans[hashlib.sha256(plan.read_bytes()).hexdigest()]=plan
 records=[]
 for cc in('gcc','clang'):
  for mode,flags in [('O0',['-O0']),('O2',['-O2']),('SAN',['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-pie','-no-pie'])]:
   binary=w/(cc+'-'+mode);subprocess.run([cc,'-std=c11','-Wall','-Wextra','-Wpedantic','-Werror',*flags,include,*sources,HERE/'tests/shell_linen_presentation/host.c','-o',binary],check=True)
   result=subprocess.run([binary],capture_output=True,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=1:abort_on_error=1'});assert result.returncode==0,result.stderr[-3000:]
   records.append(dict(compiler=cc,mode=mode,output=result.stdout.decode().strip()))
 original=(HERE/'src/shell_linen_presentation.cesta').read_text();mutations=(
  ('wrong_task','caller!=1 || task!=1','caller!=1'),
  ('job_collision','UINT64_C(0x8000000000000000)|runtime->next_presentation_job','runtime->next_presentation_job'),
  ('forget_superseded','if(runtime->presentation_desired.present)presentation_terminal(runtime,runtime->presentation_desired,SHELL_PRESENTATION_SUPERSEDED);',''),
  ('false_undo_claim','runtime->presentation_active.submitted?SHELL_PRESENTATION_INDETERMINATE:SHELL_PRESENTATION_FAILED','SHELL_PRESENTATION_FAILED'),
  ('ignore_exhaustion','runtime->next_presentation_job>=UINT64_C(0x7ffffffffffffffe)','false'),
 )
 for name,old,new in mutations:
  assert original.count(old)>=1,(name,original.count(old));source=w/(name+'.cesta');source.write_text(original.replace(old,new));c=w/(name+'.c')
  subprocess.run([args.cesta,'export-c',source,'--std=c11','-o',c,'--',include],check=True);binary=w/name
  subprocess.run(['gcc','-std=c11','-O2',include,w/'shell_text_delivery.c',c,HERE/'tests/shell_linen_presentation/host.c','-o',binary],check=True)
  result=subprocess.run([binary],capture_output=True);assert result.returncode!=0,('surviving mutation',name)
 admission=[]
 if args.lean_root:
  assert args.cesta_source;sys.path.insert(0,str(args.cesta_source.resolve()/'tests'));native=importlib.import_module('ownership_admission')
  for i,plan in enumerate(plans.values()):record,=native.parse(plan.read_bytes());admission.append(native.admit_record(record,w/f'Presentation{i}.lean',args.lean_root.resolve()))
 (w/'summary.json').write_text(json.dumps(dict(runtime=records,mutants_caught=len(mutations),admission=admission),indent=2)+'\n')
 print('Linen presentation:',len(records),'configurations;',len(mutations),'mutations caught;',sum(x['functions']for x in admission),'plans admitted')

if __name__=='__main__':main()
