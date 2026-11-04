"""Current upstream Spindle SexTime command against checked Cesta."""
import argparse,hashlib,importlib,json,os,random,subprocess,sys
from pathlib import Path
HERE=Path(__file__).resolve().parent.parent
def run(cmd,data=None,env=None,cwd=HERE):
 r=subprocess.run(list(map(str,cmd)),cwd=cwd,input=data,capture_output=True,env=env)
 assert r.returncode==0,(cmd,r.returncode,r.stdout[-2000:],r.stderr[-6000:]);return r.stdout
RUST_DRIVER=r'''
use std::io::BufRead;
fn main(){for line in std::io::stdin().lock().lines(){let line=line.unwrap();let b=line.as_bytes();assert!(b.len()%2==0);let mut input=Vec::new();for i in(0..b.len()).step_by(2){input.push(u8::from_str_radix(std::str::from_utf8(&b[i..i+2]).unwrap(),16).unwrap());}match parse(&input).and_then(render){Some(lines)=>{print!("O");for(row,len)in lines.rows.iter().zip(lines.lens){print!(" ");for x in &row[..len]{print!("{:02x}",x);}}println!();},None=>println!("E")}}}
'''
C_DRIVER=r'''
#include "spindle_sextime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int nibble(int c){if(c>='0'&&c<='9')return c-'0';if(c>='a'&&c<='f')return c-'a'+10;return -1;}
void sexos_fail(void){_Exit(88);}
int main(void){char line[1024];while(fgets(line,sizeof line,stdin)){size_t n=strcspn(line,"\r\n");line[n]=0;if(n%2)return 3;size_t bytes=n/2;SpindleSextimeInput input={0};input.len=bytes>SPINDLE_SEXTIME_INPUT_MAX?SPINDLE_SEXTIME_INPUT_MAX:bytes;input.too_long=bytes>SPINDLE_SEXTIME_INPUT_MAX;for(size_t i=0;i<input.len;++i){int a=nibble(line[2*i]),b=nibble(line[2*i+1]);if(a<0||b<0)return 3;input.bytes[i]=(uint8_t)((a<<4)|b);}int64_t unix=0;if(!spindle_sextime_parse(&input,&unix)){puts("E");continue;}SpindleSextimeLines out=spindle_sextime_render(unix);if(!out.ok){puts("E");continue;}putchar('O');for(size_t row=0;row<3;++row){putchar(' ');for(size_t i=0;i<out.lens[row];++i)printf("%02x",out.rows[row][i]);}putchar('\n');}return 0;}
'''
def corpus():
 values=['','-','+1','0','-1','1356048000','1356047999','0','4102444800','9223372036854775807','-9223372036854775808','9223372036854775808','123x','0000000000000000001','12345678901234567890']
 rng=random.Random(0x5E6751)
 values += [str(rng.randrange(-10**15,10**15))for _ in range(300)]
 return ('\n'.join(v.encode().hex()for v in values)+'\n').encode(),len(values)
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--cesta',type=Path,required=True);ap.add_argument('--cesta-source',type=Path);ap.add_argument('--lean-root',type=Path);ap.add_argument('--output',type=Path,default=HERE/'build/spindle-sextime');a=ap.parse_args()
 w=a.output.resolve();w.mkdir(parents=True,exist_ok=True);inv=json.loads((HERE/'interfaces/spindle_sextime.source.json').read_text())
 command=run(['git','show',inv['pin']+':'+inv['command_path']],cwd=HERE.parent);time=run(['git','show',inv['pin']+':'+inv['time_path']],cwd=HERE.parent)
 assert hashlib.sha256(command).hexdigest()==inv['command_sha256'];assert hashlib.sha256(time).hexdigest()==inv['time_sha256']
 command=command.replace(b'//!',b'//')
 rust=w/'reference.rs';rust.write_bytes(b'mod sex_time {\n'+time+b'\n}\n'+command+b'\n'+RUST_DRIVER.encode());ref=w/'reference';run(['rustc','--edition=2021','-O','--cap-lints','allow',rust,'-o',ref])
 data,commands=corpus();expected=run([ref],data);(w/'host.c').write_text(C_DRIVER);plans={};rows=[];exports={}
 for cc in ('gcc','clang'):
  c=w/f'sextime-{cc}.c';p=w/f'sextime-{cc}.plan';run([a.cesta,'export-c','src/spindle_sextime.cesta','--cc',cc,'--std=c11','-o',c,'--plan',p,'--','-Igenerated/include']);exports[cc]=c;plans[hashlib.sha256(p.read_bytes()).hexdigest()]=p
  for mode,opts in(('O0',['-O0']),('O2',['-O2']),('SAN',['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-pie','-no-pie'])):
   binary=w/f'{cc}-{mode}';run([cc,'-std=c11','-Wall','-Wextra','-Wpedantic','-Werror',*opts,'-Igenerated/include',c,w/'host.c','-o',binary]);actual=run([binary],data,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=1:abort_on_error=1'});assert actual==expected,(cc,mode);rows.append(dict(compiler=cc,mode=mode,sha256=hashlib.sha256(actual).hexdigest()))
 base=exports['gcc'].read_text();changes=[('offset','1356048000','1356048001'),('day','86400','86401'),('gregorian','719468','719467'),('long-count','1872000','1871999'),('negative-floor','if(remainder<0)--quotient','if(false)--quotient'),('digit-limit','input->len>SPINDLE_SEXTIME_INPUT_MAX','input->len>=SPINDLE_SEXTIME_INPUT_MAX')];caught=[]
 for name,old,new in changes:
  assert base.count(old)==1,(name,base.count(old));m=w/f'mutant-{name}.c';m.write_text(base.replace(old,new));b=w/f'mutant-{name}';run(['gcc','-std=c11','-O2','-Igenerated/include',m,w/'host.c','-o',b]);assert run([b],data)!=expected,('survived',name);caught.append(name)
 admission=[]
 if a.lean_root:
  assert a.cesta_source;sys.path.insert(0,str(a.cesta_source.resolve()/'tests'));helper=importlib.import_module('ownership_admission')
  for i,p in enumerate(plans.values()):record,=helper.parse(p.read_bytes());admission.append(helper.admit_record(record,w/f'Sextime{i}.lean',a.lean_root.resolve()))
 (w/'summary.json').write_text(json.dumps(dict(pin=inv['pin'],commands=commands,configurations=rows,mutants_caught=caught,admission=admission),indent=2)+'\n')
 print('spindle sextime:',commands,'commands;',len(rows),'configurations;',len(caught),'mutants;',sum(x['functions']for x in admission),'plans admitted')
if __name__=='__main__':main()
