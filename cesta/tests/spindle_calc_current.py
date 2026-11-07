#!/usr/bin/env python3
"""Current Spindle calculator, including environment and radix output.

The reference is apps/spindle/src/calc_command.rs in this tree: upstream's
calculator with exact complex values, named by its SHA-256 in
interfaces/spindle_calc_current.source.json. It gives upstream's answer (the
parity revision there) on every generated input without the new syntax, and
the checked Cesta unit gives its answer on the fixed rows, generated complex
sessions and command lines around the 200-byte expression bound.
"""
import argparse,hashlib,json,os,random,subprocess,sys,tempfile
from pathlib import Path
from layout_text import LayoutText
HERE=Path(__file__).resolve().parent.parent
PIN='b6be1b451a6c21985d00d192674feedf66bb11dd'
RUST=r'''
#[path="SOURCE"]mod calc;use std::io::{self,BufRead};
fn hex(x:&[u8])->String{x.iter().map(|b|format!("{b:02x}")).collect()}
fn main(){let mut env=calc::Env::new();for row in io::stdin().lock().lines(){let row=row.unwrap();if row=="R"{env=calc::Env::new();println!("R");continue}let(expr,radix)=calc::split_radix(row.as_bytes());let mut out=[0u8;calc::LINE_MAX];match calc::eval_env(expr,&mut env).and_then(|(v,_)|calc::render_radix(v,radix,&mut out)){Ok(n)=>println!("O {}",hex(&out[..n])),Err(e)=>println!("E {:?}",e)}}}
'''
C=r'''
#include <stdio.h>
#include <string.h>
#include "spindle_calc.h"
int main(void){char row[512];SpindleCalcEnv env;spindle_calc_env_init(&env);while(fgets(row,sizeof row,stdin)){size_t n=strcspn(row,"\r\n");if(n==1&&row[0]=='R'){spindle_calc_env_init(&env);puts("R");continue;}SpindleCalcInput in={0};in.len=n>SPINDLE_CALC_COMMAND_MAX?SPINDLE_CALC_COMMAND_MAX:n;in.too_long=n>SPINDLE_CALC_COMMAND_MAX;for(size_t i=0;i<in.len;i++)in.bytes[i]=(uint8_t)row[i];SpindleCalcRenderResult r=spindle_calc_render_session(spindle_calc_eval_session(&in,&env));if(r.ok){fputs("O ",stdout);for(size_t i=0;i<r.line.len;i++)printf("%02x",r.line.bytes[i]);putchar('\n');}else printf("E %s\n",spindle_calc_error_name(r.error));}return 0;}
'''
def run(argv,*,data=None,cwd=HERE,env=None):
 r=subprocess.run(list(map(str,argv)),cwd=cwd,input=data,capture_output=True,env=env)
 if r.returncode:raise RuntimeError((argv,r.returncode,r.stdout[-1000:],r.stderr[-5000:]))
 return r.stdout
def corpus():
 rows=['R','2**10','-2**2','2**3**2','1.5**2','2**-2','3**-1','2*3**2','sqrt(16)','sqrt(2)','sqrt(2.25)+1','sqrt(-1)','sqrt 4','abs(-2.5)+max(1, 7, 3)-min(4, -1)','round(2.5) + round(-2.5) + floor(-1.2) + ceil(1.2)','round(1, 2)','maxx(1)','2*pi','e','pie','0xff + 0b101','0x','x + 1','2**0.5','2**65','10**40','0**-1','answer','255 in hex','0xff+1 in bin ','-8 in oct','0 in hex','6/4','6/4 in hex','1/3*3 in hex','round(1/3) in hex','2**64 in bin','x in hexagon','R','r = 2','area = pi * r**2','r = r + 1','r * 2 + ans','pi = 3','x == 1','missing']
 rows+=['R','a=1','b=2','c=3','d=4','f=5','g=6','h=7','i=8','j=9','a+b+c+d+f+g+h+i']
 rows+=['R','2*(3+4)/5','ans*2 - ans','(ans)/3','ans+0.000000001']
 rows+=['R','1i*1i','(3+4i)*(3-4i)','(3+4i)/5','norm((3+4i)/5)','abs(3+4i)','abs(1+1i)','conj(2-3i)','re(2-3i) + im(2-3i)','1/1i','(1+1i)**2','(1+1i)**-2','1i/3','(1+2i)/(3-4i)','-(0.5i)','0xai']
 rows+=['norm(1 + 1)','interfere(1, 1)','norm(1 + -1)','interfere(1, -1)','interfere((3+4i)/5, (-3-4i)/5)','interfere(1i, 1)','interfere(0.000000001, 0.000000001i)','interfere(1)','interfere(1, 2, 3)']
 rows+=['min(1i, 2)','round(0.5i)','sqrt(1i)','5%1i','2**1i','i','2in','4ii','4i5','4 i','99999999999999999999i*99999999999999999999i','1i in hex','(255+16i) in hex','-1i in bin','(8-8i) in oct','(0.5+1i) in hex','1i/3*3 in hex','(2**40)*(1+1i) in bin','1i/0','123456789012345678901.123456789+123456789012345678901.123456789i']
 rows+=['R','exp(1)','ln(10)','log(1000)','sin(0.523598776)','cos(pi)','tan(1)','2**0.5','(-8)**0.5','exp(100)','ln(0)','tan(pi/2)','exp(-31)']
 rows+=['R','exp(1i*pi)','e**(1i*pi)','exp(1i*pi/2)','2**1i','interfere(1, exp(1i*pi/3))','norm(exp(0.7i))','exp(1+1i)','exp(0i)','(-2)**1i','1i**1i','1i**0.5','sin(1i)','exp(100+1i)','exp(10000000i)','2**(0.5+0.5i)']
 rows+=['R','i = 3','z = 3+4i','z*conj(z) + i','ans*1i','re = 2','re(z) + re','norm = 1','norm(z) + norm']
 return rows
def generated(seed,count,complex_values):
 """Random sessions; without complex_values no row uses the new syntax."""
 rng=random.Random(seed);atoms=['0','1','2','3','7','10','0.5','0.1','2.25','123456789','0xff','0b101','pi','e','ans','x','r','i','z','abc','sqrt(4)','sqrt(2)']
 funcs=['abs','min','max','round','floor','ceil','sqrt','exp','ln','log','sin','cos','tan','maxx']
 if complex_values:atoms+=['1i','2i','0.5i','3+4i','1-1i','0.000000001i','99999999999i','pi*1i','1i*pi/3'];funcs+=['re','im','conj','norm','interfere','rex']
 def expr(d=0):
  k=rng.random()
  if d>4 or k<0.35:return rng.choice(atoms)
  if k<0.55:return expr(d+1)+rng.choice(['+','-','*','/','%','**'])+expr(d+1)
  if k<0.65:return '-'+expr(d+1)
  if k<0.8:return '('+expr(d+1)+')'
  return rng.choice(funcs)+'('+', '.join(expr(d+1) for _ in range(rng.randint(1,3)))+')'
 rows=[]
 for n in range(count):
  if n%50==0:rows.append('R')
  e=expr();r=rng.random()
  if r<0.12:e=rng.choice(['x','r','i','z','pi','im','re','norm'])+' = '+e
  elif r<0.17:e+=rng.choice([' in hex',' in bin',' in oct'])
  rows.append(e)
 return rows
def long_rows(seed,count):
 """Command lines around the 200-byte bound, which applies once a radix suffix is split off."""
 rng=random.Random(seed);rows=[]
 for n in range(count):
  if n%40==0:rows+=['R','x = 2-1i']
  body='+'.join(rng.choice(['1','2.5','3i','x','0xff','norm(1+1i)','1/3']) for _ in range(70))[:rng.randint(185,215)].rstrip('+/')
  rows.append(rng.choice(['','x = ','zz='])+body+rng.choice(['',' in hex',' in bin',' in oct',' in hex   ','   ',' in hexx']))
 return rows
def encode(rows):return ('\n'.join(rows)+'\n').encode(),len(rows)
def main():
 p=argparse.ArgumentParser();p.add_argument('--cesta',type=Path,required=True);p.add_argument('--work',type=Path);p.add_argument('--mutations',action='store_true');p.add_argument('--cesta-source',type=Path);p.add_argument('--lean-root',type=Path);a=p.parse_args();w=a.work or Path(tempfile.mkdtemp(prefix='spindle-calc-current-',dir=HERE/'build'));w.mkdir(parents=True,exist_ok=True);w=w.resolve()
 inventory=json.loads((HERE/'interfaces/spindle_calc_current.source.json').read_text());assert inventory['parity']==PIN,inventory
 source=run(['git','show',PIN+':'+inventory['path']],cwd=HERE.parent);pinned=w/'calc-pinned.rs';pinned.write_bytes(source)
 patched=(HERE.parent/inventory['path']).read_bytes();assert hashlib.sha256(patched).hexdigest()==inventory['sha256'],('the Rust calculator changed; review it and update',inventory)
 branch=w/'calc-branch.rs';branch.write_bytes(patched)
 binaries={}
 for name,rs in(('pinned',pinned),('reference',branch)):driver=w/f'driver-{name}.rs';driver.write_text(RUST.replace('SOURCE',str(rs)));binaries[name]=w/name;run(['rustc','--edition=2021','-O',driver,'-o',binaries[name]])
 parity,parity_count=encode(generated(44053,6000,False));assert run([binaries['pinned']],data=parity)==run([binaries['reference']],data=parity),'the branch calc differs from pinned without the new syntax'
 reference=binaries['reference'];data,count=encode(corpus()+generated(1105,6000,True)+long_rows(5,1500));expected=run([reference],data=data)
 host=w/'host.c';host.write_text(C);exports={};plans={};builds=[]
 for cc in('gcc','clang'):
  c=w/f'calc-{cc}.c';plan=w/f'calc-{cc}.plan';run([a.cesta,'export-c','src/spindle_calc.cesta','--cc',cc,'--std=c11','--plan',plan,'-o',c,'--','-Igenerated/include']);exports[cc]=c;plans[hashlib.sha256(plan.read_bytes()).hexdigest()]=plan
  for mode,flags in(('O0',['-O0']),('O2',['-O2']),('SAN',['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-pie','-no-pie'])):
   binary=w/f'{cc}-{mode}';run([cc,'-std=c11','-g0','-Wall','-Wextra','-Wpedantic','-Werror',*flags,'-Igenerated/include',c,host,'-o',binary]);actual=run([binary],data=data,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=1:abort_on_error=1'});assert actual==expected,(cc,mode);builds.append((cc,mode))
 mutations=[]
 if a.mutations:
  base=LayoutText(exports['gcc'].read_text());changes=[('power-bound','calc_magnitude(quotient)>64u','false'),('sqrt-domain','if(value<0)','if(false)'),('hex-prefix',"session.radix=16","session.radix=8"),('unknown-name','*error=SPINDLE_CALC_UNKNOWN','*error=SPINDLE_CALC_SYNTAX'),('variable-capacity','env->count==SPINDLE_CALC_VARIABLES','false'),('not-whole','value.inexact||','false||'),('complex-product','calc_sub_checked(ac,bd,&re)','calc_add_checked(ac,bd,&re)'),('not-real','if(value.im){*error=SPINDLE_CALC_NOT_REAL;','if(false){*error=SPINDLE_CALC_NOT_REAL;'),('imaginary-suffix','*out=(CalcComplex){0,value};','*out=(CalcComplex){value,0};'),('interference-cross','calc_mul_checked(value,2,&value)','calc_mul_checked(value,1,&value)'),('complex-radix-sign','calc_put_radix(&text,imag_whole,session.radix,value.scaled!=0);','calc_put_radix(&text,imag_whole,session.radix,false);'),('euler-imaginary','calc_times_w(m,s,&im,error)','calc_times_w(m,c,&im,error)'),('ln-series','i+=2)','i+=1)'),('tangent-quotient','calc_to_scale(calc_div(numerator,cosine))','calc_to_scale(calc_div(numerator,sine))'),('expression-bound','(suffix?expression_end:input->len)>SPINDLE_CALC_INPUT_MAX','input->len>SPINDLE_CALC_INPUT_MAX')]
  for name,old,new in changes:
   assert base.count(old)>=1,(name,base.count(old));m=w/f'{name}.c';m.write_text(base.replace(old,new,1));b=w/name;run(['gcc','-std=c11','-O2','-Igenerated/include',m,host,'-o',b]);r=subprocess.run([b],cwd=HERE,input=data,capture_output=True);assert r.returncode!=0 or r.stdout!=expected,('survived',name);mutations.append(name)
 admission=[]
 if a.lean_root:
  assert a.cesta_source;sys.path.insert(0,str(a.cesta_source/'tests'));import ownership_admission
  for i,plan in enumerate(plans.values()):record,=ownership_admission.parse(plan.read_bytes());admission.append(ownership_admission.admit_record(record,w/f'SpindleCalcCurrent{i}.lean',a.lean_root))
 (w/'summary.json').write_text(json.dumps({'pin':PIN,'rust_sha256':hashlib.sha256(source).hexdigest(),'rust_tree_sha256':inventory['sha256'],'parity_commands':parity_count,'commands':count,'builds':builds,'mutations':mutations,'admission':admission},indent=2)+'\n');print('spindle current calc:',parity_count,'pinned-parity commands,',count,'commands,',len(builds),'builds,',len(mutations),'mutants,',sum(x['functions']for x in admission),'admitted')
if __name__=='__main__':main()
