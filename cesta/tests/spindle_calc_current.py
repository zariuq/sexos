#!/usr/bin/env python3
"""Current pinned Spindle calculator, including environment and radix output."""
import argparse,hashlib,json,os,subprocess,sys,tempfile
from pathlib import Path
HERE=Path(__file__).resolve().parent.parent
PIN='44053ea9b017fe89008413f7cc8a6b1a4d28d015'
RUST=r'''
#[path="SOURCE"]mod calc;use std::io::{self,BufRead};
fn hex(x:&[u8])->String{x.iter().map(|b|format!("{b:02x}")).collect()}
fn main(){let mut env=calc::Env::new();for row in io::stdin().lock().lines(){let row=row.unwrap();if row=="R"{env=calc::Env::new();println!("R");continue}let(expr,radix)=calc::split_radix(row.as_bytes());let mut out=[0u8;calc::LINE_MAX];match calc::eval_env(expr,&mut env).and_then(|(v,_)|calc::render_radix(v,radix,&mut out)){Ok(n)=>println!("O {}",hex(&out[..n])),Err(e)=>println!("E {:?}",e)}}}
'''
C=r'''
#include <stdio.h>
#include <string.h>
#include "spindle_calc.h"
int main(void){char row[512];SpindleCalcEnv env;spindle_calc_env_init(&env);while(fgets(row,sizeof row,stdin)){size_t n=strcspn(row,"\r\n");if(n==1&&row[0]=='R'){spindle_calc_env_init(&env);puts("R");continue;}SpindleCalcInput in={0};in.len=n>SPINDLE_CALC_INPUT_MAX?SPINDLE_CALC_INPUT_MAX:n;in.too_long=n>SPINDLE_CALC_INPUT_MAX;for(size_t i=0;i<in.len;i++)in.bytes[i]=(uint8_t)row[i];SpindleCalcRenderResult r=spindle_calc_render_session(spindle_calc_eval_session(&in,&env));if(r.ok){fputs("O ",stdout);for(size_t i=0;i<r.line.len;i++)printf("%02x",r.line.bytes[i]);putchar('\n');}else printf("E %s\n",spindle_calc_error_name(r.error));}return 0;}
'''
def run(argv,*,data=None,cwd=HERE,env=None):
 r=subprocess.run(list(map(str,argv)),cwd=cwd,input=data,capture_output=True,env=env)
 if r.returncode:raise RuntimeError((argv,r.returncode,r.stdout[-1000:],r.stderr[-5000:]))
 return r.stdout
def corpus():
 rows=['R','2**10','-2**2','2**3**2','1.5**2','2**-2','3**-1','2*3**2','sqrt(16)','sqrt(2)','sqrt(2.25)+1','sqrt(-1)','sqrt 4','abs(-2.5)+max(1, 7, 3)-min(4, -1)','round(2.5) + round(-2.5) + floor(-1.2) + ceil(1.2)','round(1, 2)','maxx(1)','2*pi','e','pie','0xff + 0b101','0x','x + 1','2**0.5','2**65','10**40','0**-1','answer','255 in hex','0xff+1 in bin ','-8 in oct','0 in hex','6/4','6/4 in hex','1/3*3 in hex','round(1/3) in hex','2**64 in bin','x in hexagon','R','r = 2','area = pi * r**2','r = r + 1','r * 2 + ans','pi = 3','x == 1','missing']
 rows+=['R','a=1','b=2','c=3','d=4','f=5','g=6','h=7','i=8','j=9','a+b+c+d+f+g+h+i']
 rows+=['R','2*(3+4)/5','ans*2 - ans','(ans)/3','ans+0.000000001']
 return ('\n'.join(rows)+'\n').encode(),len(rows)
def main():
 p=argparse.ArgumentParser();p.add_argument('--cesta',type=Path,required=True);p.add_argument('--work',type=Path);p.add_argument('--mutations',action='store_true');p.add_argument('--cesta-source',type=Path);p.add_argument('--lean-root',type=Path);a=p.parse_args();w=a.work or Path(tempfile.mkdtemp(prefix='spindle-calc-current-',dir=HERE/'build'));w.mkdir(parents=True,exist_ok=True)
 source=run(['git','show',PIN+':apps/spindle/src/calc_command.rs'],cwd=HERE.parent);rs=w/'calc.rs';rs.write_bytes(source);driver=w/'driver.rs';driver.write_text(RUST.replace('SOURCE',str(rs)));reference=w/'reference';run(['rustc','--edition=2021','-O',driver,'-o',reference]);data,count=corpus();expected=run([reference],data=data)
 host=w/'host.c';host.write_text(C);exports={};plans={};builds=[]
 for cc in('gcc','clang'):
  c=w/f'calc-{cc}.c';plan=w/f'calc-{cc}.plan';run([a.cesta,'export-c','src/spindle_calc.cesta','--cc',cc,'--std=c11','--plan',plan,'-o',c,'--','-Igenerated/include']);exports[cc]=c;plans[hashlib.sha256(plan.read_bytes()).hexdigest()]=plan
  for mode,flags in(('O0',['-O0']),('O2',['-O2']),('SAN',['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-pie','-no-pie'])):
   binary=w/f'{cc}-{mode}';run([cc,'-std=c11','-g0','-Wall','-Wextra','-Wpedantic','-Werror',*flags,'-Igenerated/include',c,host,'-o',binary]);actual=run([binary],data=data,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=1:abort_on_error=1'});assert actual==expected,(cc,mode);builds.append((cc,mode))
 mutations=[]
 if a.mutations:
  base=exports['gcc'].read_text();changes=[('power-bound','calc_magnitude(quotient)>64u','false'),('sqrt-domain','if(value<0)','if(false)'),('hex-prefix',"session.radix=16","session.radix=8"),('unknown-name','*error=SPINDLE_CALC_UNKNOWN','*error=SPINDLE_CALC_SYNTAX'),('variable-capacity','env->count==SPINDLE_CALC_VARIABLES','false'),('not-whole','value.inexact||','false||')]
  for name,old,new in changes:
   assert base.count(old)>=1,(name,base.count(old));m=w/f'{name}.c';m.write_text(base.replace(old,new,1));b=w/name;run(['gcc','-std=c11','-O2','-Igenerated/include',m,host,'-o',b]);r=subprocess.run([b],cwd=HERE,input=data,capture_output=True);assert r.returncode!=0 or r.stdout!=expected,('survived',name);mutations.append(name)
 admission=[]
 if a.lean_root:
  assert a.cesta_source;sys.path.insert(0,str(a.cesta_source/'tests'));import ownership_admission
  for i,plan in enumerate(plans.values()):record,=ownership_admission.parse(plan.read_bytes());admission.append(ownership_admission.admit_record(record,w/f'SpindleCalcCurrent{i}.lean',a.lean_root))
 (w/'summary.json').write_text(json.dumps({'pin':PIN,'rust_sha256':hashlib.sha256(source).hexdigest(),'commands':count,'builds':builds,'mutations':mutations,'admission':admission},indent=2)+'\n');print('spindle current calc:',count,'commands,',len(builds),'builds,',len(mutations),'mutants,',sum(x['functions']for x in admission),'admitted')
if __name__=='__main__':main()
