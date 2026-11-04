"""Current upstream Sexnet job wire algebra against Cesta."""
import argparse,hashlib,importlib,itertools,json,os,random,subprocess,sys
from pathlib import Path
HERE=Path(__file__).resolve().parent.parent
def run(cmd,data=None,env=None,cwd=HERE):
 r=subprocess.run(list(map(str,cmd)),cwd=cwd,input=data,capture_output=True,env=env)
 assert r.returncode==0,(cmd,r.returncode,r.stdout[-1000:],r.stderr[-5000:]);return r.stdout
RUST=r'''
use std::io::BufRead;
fn main(){for line in std::io::stdin().lock().lines(){let v:Vec<u64>=line.unwrap().split_whitespace().map(|x|x.parse().unwrap()).collect();let r=Request{seq:v[0]as u32,kind:v[1]as u8,offset:v[2]as u16,length:v[3]as u16,ticket:v[4]as u32,a:v[5]};let w=r.encode();print!("E {} {} {} V {}",w[0],w[1],w[2],r.valid()as u8);match Request::decode(w){Ok(x)=>print!(" D 1 {} {} {} {} {} {}",x.seq,x.kind,x.offset,x.length,x.ticket,x.a),Err(e)=>print!(" D 0 {}",e)}let state=State::decode(r.kind);match state{Some(s)=>{let x=status(s,r.offset as usize);let(y,n)=decode_status(x).unwrap();print!(" S 1 {} {} {}",x,y as u8,n)},None=>print!(" S 0")};let rw=reply(r.seq,r.ticket);match decode_reply(rw,r.seq){Ok(x)=>print!(" R 1 {}",x),Err(e)=>print!(" R 0 {}",e)}let ew=error(r.seq,r.ticket);match decode_reply(ew,r.seq){Ok(x)=>print!(" X 1 {}",x),Err(e)=>print!(" X 0 {}",e)}println!();}}
'''
C=r'''
#include "network_job_abi.h"
#include <inttypes.h>
#include <stdio.h>
int main(void){uint64_t v[6];while(scanf("%"SCNu64" %"SCNu64" %"SCNu64" %"SCNu64" %"SCNu64" %"SCNu64,&v[0],&v[1],&v[2],&v[3],&v[4],&v[5])==6){NetworkJobRequest r={(uint32_t)v[0],(uint8_t)v[1],(uint16_t)v[2],(uint16_t)v[3],(uint32_t)v[4],v[5]};NetworkJobWords w=network_job_encode(r);printf("E %"PRIu64" %"PRIu64" %"PRIu64" V %u",w.words[0],w.words[1],w.words[2],(unsigned)network_job_request_valid(r));NetworkJobRequestDecode d=network_job_decode(w);if(d.ok)printf(" D 1 %u %u %u %u %u %"PRIu64,d.request.seq,(unsigned)d.request.kind,(unsigned)d.request.offset,(unsigned)d.request.length,d.request.ticket,d.request.a);else printf(" D 0 %u",d.error);NetworkJobStateDecode sd=network_job_state_decode(r.kind);if(sd.ok){uint32_t x=network_job_status(sd.state,r.offset);NetworkJobStatusDecode y=network_job_decode_status(x);printf(" S 1 %u %u %zu",x,(unsigned)y.state,y.response_length);}else printf(" S 0");uint64_t rw=network_job_reply(r.seq,r.ticket);NetworkJobReplyDecode rd=network_job_decode_reply(rw,r.seq);if(rd.valid)printf(" R %u %u",(unsigned)rd.success,rd.value);else printf(" R 0 %u",NETWORK_JOB_INVALID);uint64_t ew=network_job_error(r.seq,r.ticket);rd=network_job_decode_reply(ew,r.seq);if(rd.valid)printf(" X %u %u",(unsigned)rd.success,rd.value);else printf(" X 0 %u",NETWORK_JOB_INVALID);putchar('\n');}return 0;}
'''
def corpus():
 rows=[];seqs=[0,1,2,0xffffff,0x1000000,0xffffffff];kinds=list(range(14));offs=[0,1,4,4095,65535];lens=[0,1,4,5,8,9,4096,65535];tickets=[0,1,17535,17536,0xffffffff];values=[0,1,0xff,0x100,0xffffffff,0xffffffffffffffff]
 for r in itertools.product(seqs,kinds,offs,lens,tickets,values):
  if len(rows)%17==0 or r[1] in(1,2,5,8,9,10,11):rows.append(r)
 rng=random.Random(0x20C);rows += [tuple(rng.getrandbits(n)for n in(32,8,16,16,32,64))for _ in range(5000)]
 return ('\n'.join(' '.join(map(str,r))for r in rows)+'\n').encode(),len(rows)
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--cesta',type=Path,required=True);ap.add_argument('--cesta-source',type=Path);ap.add_argument('--lean-root',type=Path);ap.add_argument('--output',type=Path,default=HERE/'build/network-job-abi');a=ap.parse_args();w=a.output.resolve();w.mkdir(parents=True,exist_ok=True)
 inv=json.loads((HERE/'interfaces/network_job_abi.source.json').read_text());src=run(['git','show',inv['pin']+':'+inv['path']],cwd=HERE.parent);assert hashlib.sha256(src).hexdigest()==inv['sha256'];src=src.replace(b'//!',b'//')
 rust=w/'reference.rs';rust.write_bytes(src+b'\n'+RUST.encode());ref=w/'reference';run(['rustc','--edition=2021','-O',rust,'-o',ref]);data,commands=corpus();expected=run([ref],data);(w/'host.c').write_text(C)
 rows=[];plans={};exports={}
 for cc in('gcc','clang'):
  c=w/f'abi-{cc}.c';p=w/f'abi-{cc}.plan';run([a.cesta,'export-c','src/network_job_abi.cesta','--cc',cc,'--std=c11','-o',c,'--plan',p,'--','-Igenerated/include']);exports[cc]=c;plans[hashlib.sha256(p.read_bytes()).hexdigest()]=p
  for mode,opt in(('O0',['-O0']),('O2',['-O2']),('SAN',['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-pie','-no-pie'])):
   b=w/f'{cc}-{mode}';run([cc,'-std=c11','-Wall','-Wextra','-Wpedantic','-Werror',*opt,'-Igenerated/include',c,w/'host.c','-o',b]);actual=run([b],data,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=1:abort_on_error=1'});assert actual==expected,(cc,mode);rows.append(dict(compiler=cc,mode=mode,sha256=hashlib.sha256(actual).hexdigest()))
 base=exports['gcc'].read_text();changes=[('seq-mask','words.words[0]&NETWORK_JOB_MAX_SEQ','words.words[0]&(NETWORK_JOB_MAX_SEQ-1)'),('write-max','r.length<=8','r.length<=7'),('read-max','r.length<=4','r.length<=3'),('reply-tag','UINT64_C(0x78)<<56','UINT64_C(0x77)<<56'),('error-tag','UINT64_C(0xE8)<<56','UINT64_C(0xE7)<<56'),('state-bound','value>NETWORK_JOB_CANCELLED','value>=NETWORK_JOB_CANCELLED')];caught=[]
 for name,old,new in changes:
  assert base.count(old)==1,(name,base.count(old));m=w/f'mutant-{name}.c';m.write_text(base.replace(old,new));b=w/f'mutant-{name}';run(['gcc','-std=c11','-O2','-Igenerated/include',m,w/'host.c','-o',b]);assert run([b],data)!=expected,('survived',name);caught.append(name)
 admission=[]
 if a.lean_root:
  assert a.cesta_source;sys.path.insert(0,str(a.cesta_source.resolve()/'tests'));h=importlib.import_module('ownership_admission')
  for i,p in enumerate(plans.values()):record,=h.parse(p.read_bytes());admission.append(h.admit_record(record,w/f'Abi{i}.lean',a.lean_root.resolve()))
 (w/'summary.json').write_text(json.dumps(dict(pin=inv['pin'],commands=commands,configurations=rows,mutants_caught=caught,admission=admission),indent=2)+'\n');print('network job ABI:',commands,'commands;',len(rows),'configurations;',len(caught),'mutants;',sum(x['functions']for x in admission),'plans admitted')
if __name__=='__main__':main()
