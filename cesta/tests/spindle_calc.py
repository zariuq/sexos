"""Current upstream Spindle exact-decimal calculator against checked Cesta."""
import argparse
import hashlib
import importlib
import json
import os
from pathlib import Path
import random
import subprocess
import sys

HERE=Path(__file__).resolve().parent.parent

def run(command,data=None,env=None,accepted=True,cwd=HERE):
    result=subprocess.run(list(map(str,command)),cwd=cwd,input=data,capture_output=True,env=env)
    assert (result.returncode==0)==accepted,(command,result.returncode,result.stdout[-2000:],result.stderr[-6000:])
    return result.stdout

RUST_DRIVER=r'''
use std::io::BufRead;
fn main(){
 let mut answer:Option<i128>=None;
 for line in std::io::stdin().lock().lines(){
  let line=line.unwrap();
  if line=="R"{answer=None;println!("R");continue;}
  let bytes=line.as_bytes();assert!(bytes.len()%2==0);
  let mut input=Vec::new();
  for at in (0..bytes.len()).step_by(2){input.push(u8::from_str_radix(std::str::from_utf8(&bytes[at..at+2]).unwrap(),16).unwrap());}
  match eval_with(&input,answer){
   Ok(value)=>{answer=Some(value.scaled);let mut out=[0u8;LINE_MAX];let n=render(value,&mut out);print!("O ");for b in &out[..n]{print!("{:02x}",b);}println!();},
   Err(error)=>println!("E {}",error as u8),
  }
 }
}
'''

C_DRIVER=r'''
#include "spindle_calc.h"
#include <stdio.h>
#include <string.h>
static int nibble(int c){if(c>='0'&&c<='9')return c-'0';if(c>='a'&&c<='f')return c-'a'+10;return -1;}
int main(void){
 char line[1024];bool has_answer=false;SpindleCalcInt answer=0;
 while(fgets(line,sizeof line,stdin)){
  size_t n=strcspn(line,"\r\n");line[n]=0;
  if(n==1&&line[0]=='R'){has_answer=false;puts("R");continue;}
  if(n%2){return 3;}
  SpindleCalcInput input={0};size_t bytes=n/2;input.len=bytes>SPINDLE_CALC_INPUT_MAX?SPINDLE_CALC_INPUT_MAX:bytes;input.too_long=bytes>SPINDLE_CALC_INPUT_MAX;
  for(size_t i=0;i<input.len;++i){int a=nibble(line[2*i]),b=nibble(line[2*i+1]);if(a<0||b<0)return 3;input.bytes[i]=(uint8_t)((a<<4)|b);}
  SpindleCalcResult result=spindle_calc_eval(&input,has_answer,answer);
  if(!result.ok){printf("E %u\n",(unsigned)result.error);continue;}
  has_answer=true;answer=result.value.scaled;SpindleCalcLine out=spindle_calc_render(result.value);fputs("O ",stdout);for(size_t i=0;i<out.len;++i)printf("%02x",out.bytes[i]);putchar('\n');
 }
 return 0;
}
'''

def corpus():
    rows=[]
    def reset():rows.append('R')
    def expr(text):rows.append(text.encode().hex())
    fixed=['2*(3+4)/5','0.1+0.2',' -3 - -4 ','10 % 4','7.5%2','1/3','1/3*3',
           '12345678901234567890','.5*4','1/0','5%0',
           '99999999999999999999999999999*99999999999','','2+','(1','1 2','abc','1..2',
           '('*16+'1'+')'*16,'('*17+'1'+')'*17,
           '('*40+'1'+')'*40,'-'*40,'ans+1','answer','0.0000000001','0.0000000000']
    reset()
    for text in fixed:expr(text)
    reset();expr('2*(3+4)/5');expr('ans*2 - ans');expr('(ans)/3');expr('ans+0.000000001')
    expr('1'*201)
    rng=random.Random(0xCA1C)
    atoms=['0','1','2','3','7','.5','0.1','2.25','ans']
    ops=['+','-','*','/','%']
    for group in range(80):
        if group%8==0:reset();expr(str(group+1))
        depth=rng.randrange(1,5)
        text=rng.choice(atoms)
        for _ in range(depth):
            right=rng.choice(atoms)
            text='('+text+rng.choice(ops)+right+')'
        if rng.randrange(3)==0:text='-'+text
        expr(text)
    return ('\n'.join(rows)+'\n').encode(),len(rows)

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--cesta',type=Path,required=True)
    parser.add_argument('--cesta-source',type=Path);parser.add_argument('--lean-root',type=Path)
    parser.add_argument('--output',type=Path,default=HERE/'build/spindle-calc');args=parser.parse_args()
    work=args.output.resolve();work.mkdir(parents=True,exist_ok=True)
    inventory=json.loads((HERE/'interfaces/spindle_calc.source.json').read_text())
    source=run(['git','show',inventory['pin']+':'+inventory['path']],cwd=HERE.parent)
    assert hashlib.sha256(source).hexdigest()==inventory['sha256']
    rust=work/'reference.rs';rust.write_bytes(source+b'\n'+RUST_DRIVER.encode());reference=work/'reference'
    run(['rustc','--edition=2021','-O','--cap-lints','allow',rust,'-o',reference])
    data,commands=corpus();expected=run([reference],data)
    (work/'host.c').write_text(C_DRIVER)
    plans={};records=[]
    exports={}
    for compiler in ('gcc','clang'):
        c=work/f'calc-{compiler}.c';plan=work/f'calc-{compiler}.plan'
        run([args.cesta,'export-c','src/spindle_calc.cesta','--cc',compiler,'--std=c11','-o',c,'--plan',plan,'--','-Igenerated/include'])
        exports[compiler]=c;plans[hashlib.sha256(plan.read_bytes()).hexdigest()]=plan
        for mode,options in (('O0',['-O0']),('O2',['-O2']),('SAN',['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-pie','-no-pie'])):
            binary=work/f'{compiler}-{mode}'
            run([compiler,'-std=c11','-Wall','-Wextra','-Wpedantic','-Werror',*options,'-Igenerated/include',c,work/'host.c','-o',binary])
            actual=run([binary],data,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=1:abort_on_error=1'})
            assert actual==expected,(compiler,mode)
            records.append(dict(compiler=compiler,mode=mode,sha256=hashlib.sha256(actual).hexdigest()))
    base=exports['gcc'].read_text();mutations=[
      ('scale','1000000000','100000000'),
      ('depth','((uint8_t)16)','((uint8_t)15)'),
      ('fraction-inexact','else if(digit)parser->inexact=true','else if(digit)parser->inexact=false'),
      ('mul-bound','if(a>bound)','if(a>bound||a==2)'),
      ('division-inexact','if(!calc_divmod_signed(numerator,right,&quotient,&remainder)){*error=SPINDLE_CALC_OVERFLOW;return false;}if(remainder)parser->inexact=true',
       'if(!calc_divmod_signed(numerator,right,&quotient,&remainder)){*error=SPINDLE_CALC_OVERFLOW;return false;}if(false)parser->inexact=true'),
      ('render-marker',"value.inexact?'~':'='","value.inexact?'=':'='"),
    ];caught=[]
    for name,old,new in mutations:
        assert base.count(old)==1,(name,base.count(old));path=work/f'mutant-{name}.c';path.write_text(base.replace(old,new));binary=work/f'mutant-{name}'
        run(['gcc','-std=c11','-O2','-Igenerated/include',path,work/'host.c','-o',binary]);actual=run([binary],data)
        assert actual!=expected,('mutation survived',name);caught.append(name)
    admission=[]
    if args.lean_root:
        assert args.cesta_source;sys.path.insert(0,str(args.cesta_source.resolve()/'tests'))
        helper=importlib.import_module('ownership_admission')
        for i,plan in enumerate(plans.values()):
            record,=helper.parse(plan.read_bytes());admission.append(helper.admit_record(record,work/f'Calc{i}.lean',args.lean_root.resolve()))
    summary=dict(pin=inventory['pin'],commands=commands,configurations=records,mutants_caught=caught,
                 admission=admission,boundary='Exact current Rust arithmetic and rendering; C compiler __int128 representation remains a declared target property.')
    (work/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
    print('spindle calc:',commands,'commands;',len(records),'configurations;',len(caught),'mutants;',sum(x['functions']for x in admission),'plans admitted')

if __name__=='__main__':main()
