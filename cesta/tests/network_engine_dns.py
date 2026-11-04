"""Compare the bounded Cesta DNS engine turn with current sex-net behavior."""
import argparse
import hashlib
import importlib
import json
import os
from pathlib import Path
import subprocess
import sys

HERE=Path(__file__).resolve().parent.parent
BASIS=0xCBF29CE484222325;MASK=(1<<64)-1


def run(command,data=None,*,cwd=HERE,env=None):
    result=subprocess.run(list(map(str,command)),cwd=cwd,input=data,capture_output=True,env=env)
    assert result.returncode==0,(command,result.returncode,result.stdout[-3000:],result.stderr[-10000:]);return result.stdout


RUST=r'''
use std::io::BufRead;
use sex_net::{Engine,Frame,Storage,DnsMetadata,DNS_BYTES,FLOW_BYTES};
use sex_net_config::Config;
fn h(bytes:&[u8])->u64{let mut v=0xcbf29ce484222325u64;for b in bytes{v=(v^*b as u64).wrapping_mul(0x100000001b3);}v}
fn error(e:sex_net::Error)->u8{use sex_net::Error::*;match e{Config=>1,Mac=>2,Clock=>3,Busy=>4,Length=>5,Stopped=>6,Protocol=>7}}
fn config(mode:u8)->Config{let mut c=Config::CONTROLLED;if mode==1{c.dns=[198,51,100,53];c.http=[203,0,113,9];}else if mode==2{c.local=[192,168,44,17];c.prefix=23;c.gateway=[192,168,44,1];c.dns=[192,168,45,1];c.http=[192,168,45,2];c.listen_port=46086;c.http_port=46088;c.dns_port=46089;}c}
fn make(c:Config)->Result<Engine<'static>,sex_net::Error>{Engine::new(c,[0x52,0x54,0,0x12,0x34,0x56],0,0x7365786f735f3736,Box::leak(Box::new([0;FLOW_BYTES])),Box::leak(Box::new([0;FLOW_BYTES])),Box::leak(Box::new([0;FLOW_BYTES])),Box::leak(Box::new([0;FLOW_BYTES])),Box::leak(Box::new([DnsMetadata::EMPTY])),Box::leak(Box::new([0;DNS_BYTES])),Box::leak(Box::new([DnsMetadata::EMPTY])),Box::leak(Box::new([0;DNS_BYTES])),Box::leak(Box::new([Storage::EMPTY,Storage::EMPTY,Storage::EMPTY])))}
fn show(e:&Engine<'_>){print!(" O {} {} {}",e.can_receive()as u8,e.failure().map(error).unwrap_or(0),e.statistics().packets);}
fn be16(b:&[u8],n:usize)->u16{u16::from_be_bytes([b[n],b[n+1]])}fn be32(b:&[u8],n:usize)->u32{u32::from_be_bytes([b[n],b[n+1],b[n+2],b[n+3]])}
fn frame(e:&mut Engine<'_>,c:&Config){let Some(f)=e.take_frame()else{print!("T 0");show(e);println!();return;};let b=&f.bytes[..f.length];if b.len()>=42&&be16(b,12)==0x0806{print!("T A {} {} {} {} {}",be16(b,20),be32(b,28),be32(b,38),h(&b[22..28]),h(&b[32..38]));}
 else if b.len()>=42&&be16(b,12)==0x0800&&b[23]==17{let ih=(b[14]&15)as usize*4;let o=14+ih;let n=be16(b,o+4)as usize;let payload=&b[o+8..o+n];print!("T U {} {} {} {} {} {} {}",be32(b,26),be32(b,30),be16(b,o),be16(b,o+2),payload.len(),h(payload),f.dns_ticket(c).unwrap_or(0));}
 else{print!("T X {} {}",b.len(),h(b));}show(e);println!();}
fn main(){let mut engine:Option<Engine<'static>>=None;let mut cfg=Config::CONTROLLED;for line in std::io::stdin().lock().lines(){let line=line.unwrap();let mut p=line.split_whitespace();match p.next().unwrap(){
 "N"=>{let mode:u8=p.next().unwrap().parse().unwrap();cfg=config(mode);match make(cfg){Ok(e)=>{engine=Some(e);print!("N 0");show(engine.as_ref().unwrap());println!();},Err(e)=>println!("N {}",error(e))}},
 "B"=>{let t:u32=p.next().unwrap().parse().unwrap();let e=engine.as_mut().unwrap();let r=e.begin_dns(t).err().map(error).unwrap_or(0);print!("B {}",r);show(e);println!();},
 "S"=>{let n:usize=p.next().unwrap().parse().unwrap();let mut b=Vec::with_capacity(n);for _ in 0..n{b.push(p.next().unwrap().parse().unwrap());}let e=engine.as_mut().unwrap();let r=e.submit_frame(&b).err().map(error).unwrap_or(0);print!("S {}",r);show(e);println!();},
 "A"=>{let now:u64=p.next().unwrap().parse().unwrap();let e=engine.as_mut().unwrap();e.advance(now);print!("A");show(e);println!();},
 "T"=>frame(engine.as_mut().unwrap(),&cfg),
 "Q"=>{let e=engine.as_mut().unwrap();let _=e.take_frame();print!("Q");show(e);println!();},
 "D"=>{let e=engine.as_mut().unwrap();let mut out=[0u8;DNS_BYTES];match e.dns_receive(&mut out){Ok(Some(n))=>print!("D 0 1 {} {}",n,h(&out[..n])),Ok(None)=>print!("D 0 0 0 {}",h(&[])),Err(x)=>print!("D {} 0 0 {}",error(x),h(&[]))}show(e);println!();},
 "X"=>{let e=engine.as_mut().unwrap();e.dns_abort();print!("X");show(e);println!();},
 "P"=>{let e=engine.as_mut().unwrap();e.stop(sex_net::Error::Stopped);print!("P");show(e);println!();},_=>panic!()}}}
'''


HOST=r'''
#include "network_engine_dns.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
static uint64_t h(const uint8_t *b,size_t n){uint64_t v=UINT64_C(0xcbf29ce484222325);for(size_t i=0;i<n;i++)v=(v^b[i])*UINT64_C(0x100000001b3);return v;}
static NetConfig config(unsigned mode){NetConfig c=net_config_controlled();if(mode==1){c.dns=0xc6336435;c.http=0xcb007109;}else if(mode==2){c.local=0xc0a82c11;c.prefix=23;c.gateway=0xc0a82c01;c.dns=0xc0a82d01;c.http=0xc0a82d02;c.listen_port=46086;c.http_port=46088;c.dns_port=46089;}return c;}
static void show(NetworkEngineDns *e){NetworkEngineDnsSnapshot s=network_engine_dns_snapshot(e);printf(" O %u %u %"PRIu64,(unsigned)network_engine_dns_can_receive(e),(unsigned)s.failure,s.packets);}
static void take(NetworkEngineDns *e,NetConfig c){NetworkEngineFrameResult r=network_engine_dns_take_frame(e);if(!r.present){printf("T 0");show(e);putchar('\n');return;}NetworkArpPacket a=network_engine_arp_decode(&r.frame);NetworkIpv4Packet ip=network_engine_ipv4_decode(&r.frame);NetworkUdpPacket u=network_engine_udp_decode(&r.frame);if(a.valid)printf("T A %u %u %u %"PRIu64" %"PRIu64,(unsigned)a.operation,a.sender_ip,a.target_ip,h(a.sender_mac.bytes,6),h(a.target_mac.bytes,6));else if(ip.valid&&u.valid){NetworkEngineTicket t=network_engine_dns_ticket(&r.frame,c);printf("T U %u %u %u %u %zu %"PRIu64" %u",ip.source,ip.destination,(unsigned)u.source_port,(unsigned)u.destination_port,u.payload_length,h(&r.frame.bytes[u.payload_offset],u.payload_length),t.present?t.value:0);}else printf("T X %zu %"PRIu64,r.frame.length,h(r.frame.bytes,r.frame.length));show(e);putchar('\n');}
int main(void){NetworkEngineDns engine={0};NetConfig c=net_config_controlled();NetworkEngineMac mac={{0x52,0x54,0,0x12,0x34,0x56}};char k;while(scanf(" %c",&k)==1){if(k=='N'){unsigned mode;if(scanf(" %u",&mode)!=1)return 2;c=config(mode);NetworkEngineError r=network_engine_dns_init(&engine,c,mac,0);printf("N %u",(unsigned)r);if(!r)show(&engine);putchar('\n');}
 else if(k=='B'){uint32_t t;if(scanf(" %u",&t)!=1)return 2;NetworkEngineError r=network_engine_dns_begin(&engine,t);printf("B %u",(unsigned)r);show(&engine);putchar('\n');}
 else if(k=='S'){size_t n;if(scanf(" %zu",&n)!=1)return 2;NetworkEngineFrame f={0};f.length=n;for(size_t i=0;i<n;i++){unsigned b;if(scanf(" %u",&b)!=1)return 2;if(i<sizeof(f.bytes))f.bytes[i]=(uint8_t)b;}NetworkEngineError r=network_engine_dns_submit(&engine,&f);printf("S %u",(unsigned)r);show(&engine);putchar('\n');}
 else if(k=='A'){uint64_t now;if(scanf(" %"SCNu64,&now)!=1)return 2;network_engine_dns_advance(&engine,now);printf("A");show(&engine);putchar('\n');}
 else if(k=='T')take(&engine,c);else if(k=='Q'){(void)network_engine_dns_take_frame(&engine);printf("Q");show(&engine);putchar('\n');}else if(k=='D'){NetworkEngineDnsResult r=network_engine_dns_receive(&engine);printf("D %u %u %zu %"PRIu64,(unsigned)(r.ok?0:r.error),(unsigned)r.present,r.present?r.frame.len:0,h(r.present?r.frame.bytes:(const uint8_t *)"",r.present?r.frame.len:0));show(&engine);putchar('\n');}
 else if(k=='X'){network_engine_dns_abort(&engine);printf("X");show(&engine);putchar('\n');}else if(k=='P'){network_engine_dns_stop(&engine,NETWORK_ENGINE_STOPPED);printf("P");show(&engine);putchar('\n');}else return 2;}return 0;}
'''


def checksum(data,initial=0):
    s=initial
    for i in range(0,len(data),2):s+=data[i]<<8|(data[i+1] if i+1<len(data) else 0)
    while s>>16:s=(s&0xffff)+(s>>16)
    return (~s)&0xffff
def p16(b,n,v):b[n:n+2]=v.to_bytes(2,"big")
def p32(b,n,v):b[n:n+4]=v.to_bytes(4,"big")
OUR=bytes([0x52,0x54,0,0x12,0x34,0x56]);PEER=bytes([2,0,0,0,0,1]);GW=bytes([2,0,0,0,0,2])
def arp(peer,local,mac=PEER,reply=False):
    b=bytearray(60);b[:6]=OUR if reply else bytes([255])*6;b[6:12]=mac;p16(b,12,0x0806);p16(b,14,1);p16(b,16,0x0800);b[18:20]=bytes([6,4]);p16(b,20,2 if reply else 1);b[22:28]=mac;p32(b,28,peer);b[32:38]=OUR if reply else bytes(6);p32(b,38,local);return bytes(b)
def udp(sip,dip,sp,dp,payload,smac=PEER):
    b=bytearray(42+len(payload));b[:6]=OUR;b[6:12]=smac;p16(b,12,0x0800);b[14]=0x45;p16(b,16,28+len(payload));p16(b,20,0x4000);b[22]=64;b[23]=17;p32(b,26,sip);p32(b,30,dip);p16(b,24,checksum(b[14:34]));p16(b,34,sp);p16(b,36,dp);p16(b,38,8+len(payload));b[42:]=payload;pseudo=(sip>>16)+(sip&0xffff)+(dip>>16)+(dip&0xffff)+17+8+len(payload);c=checksum(b[34:],pseudo);p16(b,40,c or 0xffff);return bytes(b)
def line_frame(frame):return "S "+str(len(frame))+" "+" ".join(map(str,frame))
def corpus():
    q=[]
    def add(x):q.append(x)
    local=0x0A00020F;peer=0x0A000202
    add("N 0");add("B 1");add("A 1");add("T");add(line_frame(arp(peer,local,PEER,True)));add("A 2");add("T");add(line_frame(udp(peer,local,46079,48001,b"answer")));add("A 3");add("D");add("D")
    add("B 2");add("A 4");add("T")
    for frame,now in ((udp(0x0A000203,local,46079,48002,b"wrong source"),5),(udp(peer,0x0A000210,46079,48002,b"wrong destination"),6),(udp(peer,local,46080,48002,b"wrong port"),7),(udp(peer,local,46079,48003,b"wrong ticket"),8)):
        add(line_frame(frame));add(f"A {now}");add("Q");add("D")
    add(line_frame(udp(peer,local,46079,48002,b"first")));add("A 9");add(line_frame(udp(peer,local,46079,48002,b"second")));add("A 10");add("D")
    add("B 3");add("A 11");add("X");add("T");add("B 4");add("B 0");add("A 12");add("T")
    add("N 0");add(line_frame(arp(peer,local)));add("A 1");add("X");add(line_frame(arp(0x0A000220,local,bytes([2,0,0,0,0,32]))));add("A 2");add("T");add("A 3");add("T")
    add("N 1");add("B 9");add("A 1");add("T");add(line_frame(arp(0xCB007109,local,PEER,False)));add("A 2");add("T");add(line_frame(arp(peer,local,GW,True)));add("A 3");add("T")
    add("X");add("A 60002");add("B 2");add("A 60003");add("T")
    add("N 1");add("B 1");add("A 10");add("T");add("A 9");add("B 2");add("P");add("B 3");add("D")
    return ("\n".join(q)+"\n").encode(),len(q)


def main():
    p=argparse.ArgumentParser();p.add_argument("--cesta",type=Path,required=True);p.add_argument("--cesta-source",type=Path);p.add_argument("--lean-root",type=Path);p.add_argument("--output",type=Path,default=HERE/"build/network-engine-dns-tests");a=p.parse_args();w=a.output.resolve();w.mkdir(parents=True,exist_ok=True)
    inv=json.loads((HERE/"interfaces/network_engine_dns.source.json").read_text());
    for path,key in ((inv["library_path"],"library_sha256"),(inv["tests_path"],"tests_sha256")):actual=run(["git","show",f"{inv['pin']}:{path}"],cwd=HERE.parent);assert hashlib.sha256(actual).hexdigest()==inv[key]
    project=w/"oracle";src=project/"src";src.mkdir(parents=True,exist_ok=True);(project/"Cargo.toml").write_text(f'[package]\nname="dns-oracle"\nversion="0.1.0"\nedition="2021"\n[dependencies]\nsex-net={{path="{(HERE.parent/"crates/sex-net").resolve()}"}}\nsex-net-config={{path="{(HERE.parent/"crates/sex-net-config").resolve()}"}}\n[workspace]\n');(src/"main.rs").write_text(RUST)
    run(["cargo","build","--release","--manifest-path",project/"Cargo.toml"]);oracle=project/"target/release/dns-oracle";commands,count=corpus();expected=run([oracle],commands);host=w/"host.c";host.write_text(HOST);configs=[];exports={};plans={}
    for cc in ("gcc","clang"):
        sources=[]
        for name in ("network_config","network_engine_frame","network_engine_packet","network_engine_neighbor","network_dns_codec","network_engine_dns"):
            out=w/f"{name}-{cc}.c";cmd=[a.cesta,"export-c",f"src/{name}.cesta","--cc",cc,"--std=c11","-o",out]
            if name=="network_engine_dns":plan=w/f"{name}-{cc}.plan";cmd.extend(["--plan",plan]);exports[cc]=out;plans[cc]=plan
            run(cmd+["--","-Igenerated/include"]);sources.append(out)
        for mode,flags in (("O0",["-O0"]),("O2",["-O2"]),("SAN",["-O1","-fsanitize=address,undefined","-fno-sanitize-recover=all","-fno-pie","-no-pie"])):
            binary=w/f"{cc}-{mode}";run([cc,"-std=c11","-Wall","-Wextra","-Wpedantic","-Werror",*flags,"-Igenerated/include",*sources,host,"-o",binary]);actual=run([binary],commands,env={**os.environ,"ASAN_OPTIONS":"detect_leaks=1:abort_on_error=1"});
            if actual!=expected:(w/f"{cc}-{mode}.actual").write_bytes(actual);(w/"expected").write_bytes(expected);raise AssertionError((cc,mode))
            configs.append({"compiler":cc,"mode":mode,"sha256":hashlib.sha256(actual).hexdigest()})
    base=exports["gcc"].read_text();mutations=[("backpressure","if(engine->tx_present)return","if(false)return"),("ticket-before-abort","if(!ticket||ticket>NET_CONFIG_MAX_TICKET)return NETWORK_ENGINE_LENGTH","if(false)return NETWORK_ENGINE_LENGTH"),("source-binding","ip.source!=engine->config.dns","false"),("destination-binding","ip.destination!=engine->config.local","false"),("port-binding","udp.source_port!=engine->config.dns_port","false"),("ticket-port","udp.destination_port!=port","false"),("single-response","engine->response_present)return","false)return"),("abort-own-frame","engine->tx_kind==NETWORK_ENGINE_TX_DNS","true"),("clock-rollback","now<engine->last","false"),("arp-request-reply","arp.operation==1","false")];caught=[];prefix=[w/f"{n}-gcc.c" for n in ("network_config","network_engine_frame","network_engine_packet","network_engine_neighbor","network_dns_codec")]
    for name,old,new in mutations:
        assert base.count(old)>=1,(name,base.count(old));mut=w/f"mutant-{name}.c";mut.write_text(base.replace(old,new));binary=w/f"mutant-{name}";run(["gcc","-std=c11","-O2","-Igenerated/include",*prefix,mut,host,"-o",binary]);assert run([binary],commands)!=expected,("survived",name);caught.append(name)
    admission=[]
    if a.lean_root:
        assert a.cesta_source;sys.path.insert(0,str(a.cesta_source.resolve()/"tests"));h=importlib.import_module("ownership_admission");unique={}
        for plan in plans.values():unique[hashlib.sha256(plan.read_bytes()).hexdigest()]=plan
        for i,plan in enumerate(unique.values()):(record,)=h.parse(plan.read_bytes());admission.append(h.admit_record(record,w/f"DnsEngine{i}.lean",a.lean_root.resolve()))
    (w/"summary.json").write_text(json.dumps({"pin":inv["pin"],"commands":count,"configurations":configs,"mutants_caught":caught,"admission":admission},indent=2)+"\n");print("network DNS engine:",count,"commands;",len(configs),"configurations;",len(caught),"mutants;",sum(x["functions"] for x in admission),"plans admitted")
if __name__=="__main__":main()
