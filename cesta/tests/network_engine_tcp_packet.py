"""Qualify the shared current-engine TCP segment boundary."""
import argparse,hashlib,importlib,json,os,random,subprocess,sys
from pathlib import Path
from layout_text import LayoutText
HERE=Path(__file__).resolve().parent.parent;BASIS=0xCBF29CE484222325;MASK=(1<<64)-1
def run(cmd,data=None,*,cwd=HERE,env=None):
 r=subprocess.run(list(map(str,cmd)),cwd=cwd,input=data,capture_output=True,env=env);assert r.returncode==0,(cmd,r.returncode,r.stdout[-2000:],r.stderr[-8000:]);return r.stdout
HOST=r'''
#include "network_engine_tcp_packet.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
static uint64_t h(const uint8_t *b,size_t n){uint64_t v=UINT64_C(0xcbf29ce484222325);for(size_t i=0;i<n;i++)v=(v^b[i])*UINT64_C(0x100000001b3);return v;}
static NetworkEngineMac mac(void){NetworkEngineMac m={{0}};for(size_t i=0;i<6;i++){unsigned b;if(scanf(" %u",&b)!=1)return(NetworkEngineMac){{0}};m.bytes[i]=(uint8_t)b;}return m;}
static void show(const NetworkEngineFrame *f){NetworkIpv4Packet ip=network_engine_ipv4_decode(f);NetworkTcpPacket t=network_engine_tcp_decode(f);size_t safe=t.valid&&t.payload_offset<=f->length&&t.payload_length<=f->length-t.payload_offset?t.payload_length:0;printf("I %u %u %u %u %zu %zu T %u %u %u %u %u %u %u %zu %zu %"PRIu64"\n",(unsigned)ip.valid,(unsigned)ip.protocol,ip.source,ip.destination,ip.header_length,ip.transport_length,(unsigned)t.valid,(unsigned)t.source_port,(unsigned)t.destination_port,t.sequence,t.acknowledgment,(unsigned)t.flags,(unsigned)t.window,t.header_length,t.payload_length,safe?h(&f->bytes[t.payload_offset],safe):h((const uint8_t *)"",0));}
int main(void){char k;while(scanf(" %c",&k)==1){if(k=='D'){size_t n;if(scanf(" %zu",&n)!=1)return 2;NetworkEngineFrame f={0};f.length=n;for(size_t i=0;i<n;i++){unsigned b;if(scanf(" %u",&b)!=1)return 2;if(i<sizeof(f.bytes))f.bytes[i]=(uint8_t)b;}show(&f);}else if(k=='B'){uint32_t sip,dip,seq,ack;unsigned sp,dp,flags,window;size_t n;if(scanf(" %u %u %u %u %u %u %u %u %zu",&sip,&dip,&sp,&dp,&seq,&ack,&flags,&window,&n)!=9)return 2;NetworkEngineMac dm=mac(),sm=mac();NetworkEnginePayload p={0};p.len=n;for(size_t i=0;i<n;i++){unsigned b;if(scanf(" %u",&b)!=1)return 2;if(i<sizeof(p.bytes))p.bytes[i]=(uint8_t)b;}NetworkEngineFrame f;memset(&f,0xA5,sizeof(f));bool ok=network_engine_tcp_build(&f,dm,sm,sip,dip,(uint16_t)sp,(uint16_t)dp,seq,ack,(uint8_t)flags,(uint16_t)window,&p);printf("B %u %zu %"PRIu64" ",(unsigned)ok,ok?f.length:0,h(f.bytes,ok?f.length:sizeof(f.bytes)));show(&f);}else return 2;}return 0;}
'''
def fnv(b):
 v=BASIS
 for x in b:v=((v^x)*0x100000001B3)&MASK
 return v
def checksum(b,s=0):
 for i in range(0,len(b),2):s+=b[i]<<8|(b[i+1] if i+1<len(b) else 0)
 while s>>16:s=(s&0xffff)+(s>>16)
 return (~s)&0xffff
def p16(b,n,v):b[n:n+2]=int(v).to_bytes(2,'big')
def p32(b,n,v):b[n:n+4]=int(v).to_bytes(4,'big')
def g16(b,n):return int.from_bytes(b[n:n+2],'big')
def g32(b,n):return int.from_bytes(b[n:n+4],'big')
def build(sip,dip,sp,dp,seq,ack,flags,window,payload,dm,sm):
 if len(payload)>1460 or not all((sip,dip,sp,dp)):return None
 b=bytearray(54+len(payload));b[:6]=dm;b[6:12]=sm;p16(b,12,0x0800);b[14]=0x45;p16(b,16,40+len(payload));p16(b,20,0x4000);b[22]=64;b[23]=6;p32(b,26,sip);p32(b,30,dip);p16(b,24,checksum(b[14:34]));p16(b,34,sp);p16(b,36,dp);p32(b,38,seq);p32(b,42,ack);b[46]=0x50;b[47]=flags;p16(b,48,window);b[54:]=payload;pseudo=(sip>>16)+(sip&0xffff)+(dip>>16)+(dip&0xffff)+6+20+len(payload);p16(b,50,checksum(b[34:],pseudo));return b
def decode(b,n):
 ip=(0,0,0,0,0,0);tcp=(0,0,0,0,0,0,0,0,0,fnv(b''))
 if n<=1514 and n>=34 and g16(b,12)==0x0800 and b[14]>>4==4:
  ih=(b[14]&15)*4
  if ih>=20 and n>=14+ih:
   total=g16(b,16)
   if total>=ih and total<=n-14 and g16(b,20)&0x3fff==0 and checksum(b[14:14+ih])==0:ip=(1,b[23],g32(b,26),g32(b,30),ih,total-ih)
 if ip[0] and ip[1]==6 and ip[5]>=20:
  o=14+ip[4];th=(b[o+12]>>4)*4;pseudo=(ip[2]>>16)+(ip[2]&0xffff)+(ip[3]>>16)+(ip[3]&0xffff)+6+ip[5]
  if th>=20 and th<=ip[5] and checksum(b[o:o+ip[5]],pseudo)==0:
   payload=b[o+th:o+ip[5]];tcp=(1,g16(b,o),g16(b,o+2),g32(b,o+4),g32(b,o+8),b[o+13],g16(b,o+14),th,len(payload),fnv(payload))
 return ip,tcp
def show(b,n):
 i,t=decode(b,n);return f"I {' '.join(map(str,i))} T {' '.join(map(str,t))}\n"
def corpus():
 dm=bytes([2,0,0,0,0,1]);sm=bytes([0x52,0x54,0,0x12,0x34,0x56]);rows=[];out=[]
 def dec(b,n=None):
  n=len(b) if n is None else n;rows.append('D '+str(n)+' '+' '.join(map(str,b[:n]))+'\n');out.append(show(b,n))
 for size in (0,1,17,256,512,1460):
  payload=bytes((i*19+7)&255 for i in range(size));b=build(0x0A00020F,0x0A000202,48001,46078,0xFFFFFFF0,0x10203040,0x18,8192,payload,dm,sm);dec(b)
  for cut in range(min(len(b),80)):dec(b[:cut],cut)
  rows.append('B 167772687 167772674 48001 46078 4294967280 270544960 24 8192 '+str(size)+' '+' '.join(map(str,dm+sm+payload))+'\n');out.append(f"B 1 {len(b)} {fnv(b)} "+show(b,len(b)))
 base=build(0x0A00020F,0x0A000202,48001,46078,1,2,0x10,4096,b'packet',dm,sm)
 for at in (12,14,16,20,23,24,34,36,46,47,50):c=bytearray(base);c[at]^=1;dec(c)
 def ipfix(c,length=20):c[24:26]=b'\0\0';p16(c,24,checksum(c[14:14+length]))
 c=bytearray(base);c[23]=17;ipfix(c);dec(c)
 c=bytearray(base);c[46]=0x40;c[50:52]=b'\0\0';pseudo=(g32(c,26)>>16)+(g32(c,26)&0xffff)+(g32(c,30)>>16)+(g32(c,30)&0xffff)+6+(g16(c,16)-20);p16(c,50,checksum(c[34:14+g16(c,16)],pseudo));dec(c)
 c=bytearray(base);c[46]=0x70;c[50:52]=b'\0\0';pseudo=(g32(c,26)>>16)+(g32(c,26)&0xffff)+(g32(c,30)>>16)+(g32(c,30)&0xffff)+6+(g16(c,16)-20);p16(c,50,checksum(c[34:14+g16(c,16)],pseudo));dec(c)
 rng=random.Random(0x5443505041434B45)
 for _ in range(5000):n=rng.randrange(1516);dec(bytes(rng.randrange(256) for _ in range(n)),n)
 payload=bytes(1461);rows.append('B 167772687 167772674 48001 46078 1 2 16 4096 1461 '+' '.join(map(str,dm+sm+payload))+'\n');empty=bytes([0xA5])*1514;out.append(f"B 0 0 {fnv(empty)} "+show(empty,0))
 return ''.join(rows).encode(),''.join(out).encode(),len(rows)
def main():
 p=argparse.ArgumentParser();p.add_argument('--cesta',type=Path,required=True);p.add_argument('--cesta-source',type=Path);p.add_argument('--lean-root',type=Path);p.add_argument('--output',type=Path,default=HERE/'build/network-engine-tcp-packet-tests');a=p.parse_args();w=a.output.resolve();w.mkdir(parents=True,exist_ok=True)
 inv=json.loads((HERE/'interfaces/network_engine_tcp_packet.source.json').read_text())
 for path,key in ((inv['library_path'],'library_sha256'),(inv['tests_path'],'tests_sha256')):actual=run(['git','show',f"{inv['pin']}:{path}"],cwd=HERE.parent);assert hashlib.sha256(actual).hexdigest()==inv[key]
 cargo=run(['cargo','test','-p','sex-net','--lib','--','--test-threads=1'],cwd=HERE.parent);assert b'54 passed' in cargo
 commands,expected,count=corpus();host=w/'host.c';host.write_text(HOST);configs=[];exports={};plans={}
 for cc in ('gcc','clang'):
  sources=[]
  for name in ('network_config','network_engine_frame','network_engine_packet','network_engine_tcp_packet'):
   target=w/f'{name}-{cc}.c';cmd=[a.cesta,'export-c',f'src/{name}.cesta','--cc',cc,'--std=c11','-o',target]
   if name=='network_engine_tcp_packet':plan=w/f'{name}-{cc}.plan';cmd.extend(['--plan',plan]);exports[cc]=target;plans[cc]=plan
   run(cmd+['--','-Igenerated/include']);sources.append(target)
  for mode,flags in (('O0',['-O0']),('O2',['-O2']),('SAN',['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-pie','-no-pie'])):
   binary=w/f'{cc}-{mode}';run([cc,'-std=c11','-Wall','-Wextra','-Wpedantic','-Werror',*flags,'-Igenerated/include',*sources,host,'-o',binary]);actual=run([binary],commands,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=1:abort_on_error=1'});assert actual==expected,(cc,mode);configs.append({'compiler':cc,'mode':mode,'sha256':hashlib.sha256(actual).hexdigest()})
 base=LayoutText(exports['gcc'].read_text());mutations=[('protocol','ip.protocol!=6','false'),('header-min','header<20','header<16'),('header-extent','header>ip.transport_length','false'),('checksum','tcp_checksum(frame,offset,ip.transport_length,tcp_pseudo(ip.source,ip.destination,(uint16_t)ip.transport_length))!=0','false'),('payload-cap','payload->len>1460','payload->len>1459'),('source-port','!source_port','source_port==48001'),('destination-port','!destination_port','destination_port==46078'),('ip-checksum','tcp_put16(frame,24,tcp_checksum(frame,14,20,0))','tcp_put16(frame,24,0)'),('tcp-checksum','tcp_put16(frame,tcp+16,tcp_checksum(frame,tcp,tcp_length,tcp_pseudo(source_ip,destination_ip,(uint16_t)tcp_length)))','tcp_put16(frame,tcp+16,0)')];caught=[];prefix=[w/f'{n}-gcc.c' for n in ('network_config','network_engine_frame','network_engine_packet')]
 for name,old,new in mutations:
  assert base.count(old)>=1,(name,base.count(old));m=w/f'mutant-{name}.c';m.write_text(base.replace(old,new));b=w/f'mutant-{name}';run(['gcc','-std=c11','-O2','-Igenerated/include',*prefix,m,host,'-o',b]);assert run([b],commands)!=expected,('survived',name);caught.append(name)
 admission=[]
 if a.lean_root:
  assert a.cesta_source;sys.path.insert(0,str(a.cesta_source.resolve()/'tests'));h=importlib.import_module('ownership_admission');unique={}
  for plan in plans.values():unique[hashlib.sha256(plan.read_bytes()).hexdigest()]=plan
  for i,plan in enumerate(unique.values()):(record,)=h.parse(plan.read_bytes());admission.append(h.admit_record(record,w/f'TcpPacket{i}.lean',a.lean_root.resolve()))
 (w/'summary.json').write_text(json.dumps({'pin':inv['pin'],'commands':count,'upstream_rust_tests':54,'configurations':configs,'mutants_caught':caught,'admission':admission},indent=2)+'\n');print('network TCP packet:',count,'commands;',len(configs),'configurations;',len(caught),'mutants;',sum(x['functions'] for x in admission),'plans admitted')
if __name__=='__main__':main()
