#!/usr/bin/env python3
"""Bounded authenticated display readiness preserves ordinary startup messages."""
import argparse,hashlib,importlib,json,os,subprocess,sys
from pathlib import Path
from silk_shell_models_differential import mutate_function
HERE=Path(__file__).resolve().parent.parent
def run(command,env=None,accepted=True):
 result=subprocess.run([str(x) for x in command],cwd=HERE,capture_output=True,env=env)
 if accepted and result.returncode:raise AssertionError((command,result.stdout.decode(errors='replace'),result.stderr.decode(errors='replace')))
 if not accepted and not result.returncode:raise AssertionError(('unexpected acceptance',command))
 return result.stdout
HARNESS=r'''#include "silk_shell_core.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static SexosMessage message(uint64_t id){return(SexosMessage){id,id+1,id+2,id+3,(uint32_t)(id+4),(uint32_t)(id+5)};}
int main(void){SilkShellCoreState state={0};SexosMessage out={0};
assert(shell_startup_observe(&state,(SexosMessage){OP_DISPLAY_READY,DISPLAY_READY_VERSION,0,0,2,0})==SHELL_STARTUP_REJECTED_CONTROL);assert(!state.startup_ready&&!state.startup_len);
assert(shell_startup_observe(&state,(SexosMessage){OP_DISPLAY_READY,DISPLAY_READY_VERSION,0,0,1,0})==SHELL_STARTUP_READY);assert(state.startup_ready&&!state.startup_len);
for(size_t i=0;i<SHELL_STARTUP_INBOX_CAPACITY;++i){assert(shell_startup_observe(&state,message(1000+i))==SHELL_STARTUP_DEFERRED);}
assert(shell_startup_pending(&state));assert(shell_startup_observe(&state,message(9999))==SHELL_STARTUP_FULL);assert(state.startup_len==SHELL_STARTUP_INBOX_CAPACITY);
for(size_t i=0;i<SHELL_STARTUP_INBOX_CAPACITY;++i){assert(shell_startup_pop(&state,&out));SexosMessage expected=message(1000+i);assert(!memcmp(&out,&expected,sizeof out));}assert(!shell_startup_pending(&state)&&!shell_startup_pop(&state,&out));
for(size_t i=0;i<10;++i){assert(shell_startup_observe(&state,message(2000+i))==SHELL_STARTUP_DEFERRED);}for(size_t i=0;i<5;++i){assert(shell_startup_pop(&state,&out));assert(out.type_id==2000+i);}for(size_t i=0;i<5;++i){assert(shell_startup_observe(&state,message(3000+i))==SHELL_STARTUP_DEFERRED);}for(size_t i=5;i<10;++i){assert(shell_startup_pop(&state,&out));assert(out.type_id==2000+i);}for(size_t i=0;i<5;++i){assert(shell_startup_pop(&state,i==4?NULL:&out));if(i!=4)assert(out.type_id==3000+i);}assert(!shell_startup_pending(&state));puts("PASS startup ready=1 fifo=256 wrap=1 full=1 malformed=1");return 0;}
'''
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--cesta',type=Path,required=True);ap.add_argument('--cesta-source',type=Path);ap.add_argument('--lean-root',type=Path);ap.add_argument('--output',type=Path,default=HERE/'build/silk-shell-startup');args=ap.parse_args();w=args.output.resolve();w.mkdir(parents=True,exist_ok=True);manifest=json.loads((HERE/'interfaces/silk_shell_startup.source.json').read_text());source=(HERE.parent/manifest['source']).read_bytes();assert hashlib.sha256(source).hexdigest()==manifest['sha256'];assert manifest['capacity']==256;(w/'host.c').write_text(HARNESS);plans={};exports={}
 for cc in ('gcc','clang'):
  target=w/f'startup-{cc}.c';plan=w/f'startup-{cc}.plan';run([args.cesta,'export-c','src/silk_shell_startup.cesta','--cc',cc,'--std=c11','-o',target,'--plan',plan,'--','-Igenerated/include']);exports[cc]=target;plans[hashlib.sha256(plan.read_bytes()).hexdigest()]=plan
 records=[];reference=None
 for cc in ('gcc','clang'):
  for mode,opts in (('O0',['-O0']),('O2',['-O2']),('SAN',['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-pie','-no-pie'])):
   binary=w/f'{cc}-{mode}';run([cc,'-std=c11','-Wall','-Wextra','-Wpedantic','-Werror',*opts,'-Igenerated/include',exports[cc],w/'host.c','-o',binary]);out=run([binary],env={**os.environ,'ASAN_OPTIONS':'detect_leaks=1:abort_on_error=1'});reference=reference or out;assert out==reference;records.append(dict(compiler=cc,mode=mode,sha256=hashlib.sha256(out).hexdigest()))
 base=exports['gcc'].read_text();changes=[('forged-ready','shell_startup_observe','message.caller_pd==1','message.caller_pd==2'),('short-capacity','shell_startup_observe','==SHELL_STARTUP_INBOX_CAPACITY','+1==SHELL_STARTUP_INBOX_CAPACITY'),('lifo','shell_startup_pop','size_t index=state->startup_head','size_t index=(state->startup_head+state->startup_len-1)%SHELL_STARTUP_INBOX_CAPACITY'),('retain-length','shell_startup_pop','--state->startup_len','(void)state->startup_len')];mutants=[]
 for label,function,old,new in changes:
  path=w/f'mutant-{label}.c';path.write_text(mutate_function(base,function,old,new));binary=w/f'mutant-{label}';run(['gcc','-std=c11','-O2','-Igenerated/include',path,w/'host.c','-o',binary]);result=subprocess.run([binary],cwd=HERE,capture_output=True);assert result.returncode or result.stdout!=reference,('survived',label);mutants.append(label)
 admission=[]
 if args.lean_root:
  assert args.cesta_source;sys.path.insert(0,str(args.cesta_source.resolve()/'tests'));native=importlib.import_module('ownership_admission')
  for i,plan in enumerate(plans.values()):record,=native.parse(plan.read_bytes());admission.append(native.admit_record(record,w/f'Startup{i}.lean',args.lean_root.resolve()))
 (w/'summary.json').write_text(json.dumps(dict(pin=manifest['pin'],configurations=records,mutants_caught=mutants,admission=admission),indent=2)+'\n');print('Shell startup inbox: 256 messages; 6 configurations;',len(mutants),'mutants caught;',sum(x['functions'] for x in admission),'plans admitted')
if __name__=='__main__':main()
