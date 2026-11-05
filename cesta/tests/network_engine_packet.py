"""Qualify the shared ARP/IPv4/UDP packet boundary for the current engine."""
import argparse
import hashlib
import importlib
import json
import os
from pathlib import Path
import random
import subprocess
import sys
from layout_text import LayoutText

HERE = Path(__file__).resolve().parent.parent
BASIS = 0xCBF29CE484222325
MASK = (1 << 64) - 1


def run(command, data=None, *, cwd=HERE, env=None):
    result = subprocess.run(list(map(str, command)), cwd=cwd, input=data, capture_output=True, env=env)
    assert result.returncode == 0, (command, result.returncode, result.stdout[-2000:], result.stderr[-8000:])
    return result.stdout


HOST = r'''
#include "network_engine_packet.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
static uint64_t hash(const uint8_t *bytes,size_t n){uint64_t h=UINT64_C(0xcbf29ce484222325);for(size_t i=0;i<n;i++)h=(h^bytes[i])*UINT64_C(0x100000001b3);return h;}
static uint64_t mac_hash(const uint8_t bytes[6]){return hash(bytes,6);}
static NetworkEngineMac read_mac(void){NetworkEngineMac m={{0}};for(size_t i=0;i<6;i++){unsigned n;if(scanf(" %u",&n)!=1)return(NetworkEngineMac){{0}};m.bytes[i]=(uint8_t)n;}return m;}
static void show(const NetworkEngineFrame *f){NetworkArpPacket a=network_engine_arp_decode(f);NetworkIpv4Packet i=network_engine_ipv4_decode(f);NetworkUdpPacket u=network_engine_udp_decode(f);
 printf("A %u %u %u %u %"PRIu64" %"PRIu64" I %u %u %u %u %zu %zu %zu U %u %u %u %zu %zu\n",(unsigned)a.valid,(unsigned)a.operation,a.sender_ip,a.target_ip,mac_hash(a.sender_mac.bytes),mac_hash(a.target_mac.bytes),(unsigned)i.valid,(unsigned)i.protocol,i.source,i.destination,i.header_length,i.transport_offset,i.transport_length,(unsigned)u.valid,(unsigned)u.source_port,(unsigned)u.destination_port,u.payload_offset,u.payload_length);}
int main(void){char kind;while(scanf(" %c",&kind)==1){
 if(kind=='D'){size_t n;if(scanf(" %zu",&n)!=1)return 2;NetworkEngineFrame f;memset(&f,0,sizeof(f));f.length=n;for(size_t i=0;i<n;i++){unsigned b;if(scanf(" %u",&b)!=1)return 2;if(i<sizeof(f.bytes))f.bytes[i]=(uint8_t)b;}show(&f);}
 else if(kind=='U'){uint32_t sip,dip;unsigned sp,dp;size_t n;if(scanf(" %u %u %u %u %zu",&sip,&dip,&sp,&dp,&n)!=5)return 2;NetworkEngineMac dm=read_mac(),sm=read_mac();NetworkEnginePayload p;memset(&p,0,sizeof(p));p.len=n;for(size_t i=0;i<n;i++){unsigned b;if(scanf(" %u",&b)!=1)return 2;if(i<sizeof(p.bytes))p.bytes[i]=(uint8_t)b;}NetworkEngineFrame f;memset(&f,0xA5,sizeof(f));bool ok=network_engine_udp_build(&f,dm,sm,sip,dip,(uint16_t)sp,(uint16_t)dp,&p);printf("B %u %zu %"PRIu64" ",(unsigned)ok,ok?f.length:0,hash(f.bytes,ok?f.length:sizeof(f.bytes)));show(&f);}
 else if(kind=='R'||kind=='P'){uint32_t ip;if(scanf(" %u",&ip)!=1)return 2;NetworkEngineMac local=read_mac(),peer={{0}};if(kind=='P')peer=read_mac();NetworkEngineFrame f;memset(&f,0xA5,sizeof(f));NetConfig c=net_config_controlled();bool ok=kind=='R'?network_engine_arp_request(&f,c,local,ip):network_engine_arp_reply(&f,c,local,peer,ip);printf("B %u %zu %"PRIu64" ",(unsigned)ok,ok?f.length:0,hash(f.bytes,ok?f.length:sizeof(f.bytes)));show(&f);}
 else return 2;}return 0;}
'''


def fnv(data):
    value = BASIS
    for byte in data: value = ((value ^ byte) * 0x100000001B3) & MASK
    return value


def checksum(data, initial=0):
    total = initial
    for at in range(0, len(data), 2): total += data[at] << 8 | (data[at+1] if at+1 < len(data) else 0)
    while total >> 16: total = (total & 0xFFFF) + (total >> 16)
    return (~total) & 0xFFFF


def put16(frame, at, value): frame[at:at+2] = int(value).to_bytes(2, "big")
def put32(frame, at, value): frame[at:at+4] = int(value).to_bytes(4, "big")
def get16(frame, at): return int.from_bytes(frame[at:at+2], "big")
def get32(frame, at): return int.from_bytes(frame[at:at+4], "big")


def decode(frame, length):
    zero = bytes(6); arp = (0, 0, 0, 0, fnv(zero), fnv(zero))
    if length <= 1514 and length >= 42 and get16(frame,12)==0x0806 and get16(frame,14)==1 and get16(frame,16)==0x0800 and frame[18]==6 and frame[19]==4 and get16(frame,20) in (1,2):
        arp = (1,get16(frame,20),get32(frame,28),get32(frame,38),fnv(frame[22:28]),fnv(frame[32:38]))
    ip = (0,0,0,0,0,0,0)
    if length <= 1514 and length >= 34 and get16(frame,12)==0x0800 and frame[14]>>4==4:
        header=(frame[14]&15)*4
        if header>=20 and length>=14+header:
            total=get16(frame,16)
            if total>=header and total<=length-14 and get16(frame,20)&0x3fff==0 and checksum(frame[14:14+header])==0:
                ip=(1,frame[23],get32(frame,26),get32(frame,30),header,14+header,total-header)
    udp=(0,0,0,0,0)
    if ip[0] and ip[1]==17 and ip[6]>=8:
        offset=ip[5]; length_udp=get16(frame,offset+4); check=get16(frame,offset+6)
        pseudo=(ip[2]>>16)+(ip[2]&0xffff)+(ip[3]>>16)+(ip[3]&0xffff)+17+length_udp
        if length_udp==ip[6] and length_udp>=8 and (check==0 or checksum(frame[offset:offset+length_udp],pseudo)==0):
            udp=(1,get16(frame,offset),get16(frame,offset+2),offset+8,length_udp-8)
    return arp,ip,udp


def show(frame,length):
    a,i,u=decode(frame,length)
    return f"A {' '.join(map(str,a))} I {' '.join(map(str,i))} U {' '.join(map(str,u))}\n"


def udp_build(sip,dip,sp,dp,dmac,smac,payload):
    if len(payload)>1472 or not all((sip,dip,sp,dp)): return None
    frame=bytearray(14+20+8+len(payload));frame[:6]=dmac;frame[6:12]=smac;put16(frame,12,0x0800);frame[14]=0x45
    put16(frame,16,20+8+len(payload));put16(frame,20,0x4000);frame[22]=64;frame[23]=17;put32(frame,26,sip);put32(frame,30,dip);put16(frame,24,checksum(frame[14:34]))
    at=34;put16(frame,at,sp);put16(frame,at+2,dp);put16(frame,at+4,8+len(payload));frame[at+8:]=payload
    pseudo=(sip>>16)+(sip&0xffff)+(dip>>16)+(dip&0xffff)+17+8+len(payload);c=checksum(frame[at:],pseudo);put16(frame,at+6,c or 0xffff);return frame


def arp_build(reply,ip,local,peer=bytes(6)):
    if ip==0:return None
    frame=bytearray(42);frame[:6]=peer if reply else bytes([255])*6;frame[6:12]=local;put16(frame,12,0x0806);put16(frame,14,1);put16(frame,16,0x0800);frame[18:20]=bytes([6,4]);put16(frame,20,2 if reply else 1);frame[22:28]=local;put32(frame,28,0x0A00020F);frame[32:38]=peer if reply else bytes([255])*6;put32(frame,38,ip);return frame


def corpus():
    rng=random.Random(0x5041434B4554);commands=[];expected=[]
    def add_decode(frame,length=None):
        n=len(frame) if length is None else length;commands.append("D "+str(n)+" "+" ".join(map(str,frame[:n]))+"\n");expected.append(show(frame,n))
    local=bytes([0x52,0x54,0,0x12,0x34,0x56]);peer=bytes([2,0,0,0,0,1])
    for reply in (False,True):
        frame=arp_build(reply,0x0A000202,local,peer);add_decode(frame)
        for cut in range(len(frame)): add_decode(frame[:cut],cut)
        for index in range(len(frame)):
            changed=bytearray(frame);changed[index]^=0xff;add_decode(changed)
    for size in (0,1,34,256,512,1472):
        payload=bytes((n*17+3)&255 for n in range(size));frame=udp_build(0x0A00020F,0x0A000202,48001,46079,peer,local,payload);add_decode(frame)
        for cut in range(min(len(frame),80)): add_decode(frame[:cut],cut)
        commands.append("U 167772687 167772674 48001 46079 "+str(size)+" "+" ".join(map(str,peer+local+payload))+"\n");expected.append(f"B 1 {len(frame)} {fnv(frame)} "+show(frame,len(frame)))
        for field in (12,14,16,20,23,24,38,40):
            changed=bytearray(frame);changed[field]^=1;add_decode(changed)
    base=udp_build(0x0A00020F,0x0A000202,48001,46079,peer,local,b"packet")
    def fix_ip(frame):
        frame[24:26]=b"\x00\x00";put16(frame,24,checksum(frame[14:34]))
    changed=bytearray(base);changed[14]=0x65;fix_ip(changed);add_decode(changed)
    changed=bytearray(base);changed[14]=0x44;changed[24:26]=b"\x00\x00";put16(changed,24,checksum(changed[14:30]));add_decode(changed)
    changed=bytearray(base);put16(changed,16,get16(changed,16)+1);fix_ip(changed);add_decode(changed)
    changed=bytearray(base);put16(changed,20,0x4001);fix_ip(changed);add_decode(changed)
    changed=bytearray(base);put16(changed,38,get16(changed,38)-1);put16(changed,40,0);add_decode(changed)
    for reply in (False,True):
        command=("P" if reply else "R")+" 167772674 "+" ".join(map(str,local+(peer if reply else b"")))+"\n";frame=arp_build(reply,0x0A000202,local,peer)
        commands.append(command);expected.append(f"B 1 42 {fnv(frame)} "+show(frame,42))
    payload=bytes((n*7)&255 for n in range(1473));commands.append("U 167772687 167772674 48001 46079 1473 "+" ".join(map(str,peer+local+payload))+"\n")
    empty=bytes([0xA5])*1514;expected.append(f"B 0 0 {fnv(empty)} "+show(empty,0))
    for _ in range(5000):
        n=rng.randrange(1516);frame=bytes(rng.randrange(256) for _ in range(n));add_decode(frame,n)
    return "".join(commands).encode(),"".join(expected).encode(),len(commands)


def main():
    parser=argparse.ArgumentParser();parser.add_argument("--cesta",type=Path,required=True);parser.add_argument("--cesta-source",type=Path);parser.add_argument("--lean-root",type=Path);parser.add_argument("--output",type=Path,default=HERE/"build/network-engine-packet-tests");args=parser.parse_args();output=args.output.resolve();output.mkdir(parents=True,exist_ok=True)
    inventory=json.loads((HERE/"interfaces/network_engine_packet.source.json").read_text())
    for path,key in ((inventory["library_path"],"library_sha256"),(inventory["tests_path"],"tests_sha256")):
        actual=run(["git","show",f"{inventory['pin']}:{path}"],cwd=HERE.parent);assert hashlib.sha256(actual).hexdigest()==inventory[key]
    cargo=run(["cargo","test","-p","sex-net","--lib","--","--test-threads=1"],cwd=HERE.parent);assert b"54 passed" in cargo
    commands,expected,count=corpus();host=output/"host.c";host.write_text(HOST);exports={};plans={};configurations=[]
    for compiler in ("gcc","clang"):
        sources=[]
        for name in ("network_config","network_engine_frame","network_engine_packet"):
            target=output/f"{name}-{compiler}.c";command=[args.cesta,"export-c",f"src/{name}.cesta","--cc",compiler,"--std=c11","-o",target]
            if name=="network_engine_packet":plan=output/f"{name}-{compiler}.plan";command.extend(["--plan",plan]);plans[compiler]=plan;exports[compiler]=target
            run(command+["--","-Igenerated/include"]);sources.append(target)
        for mode,flags in (("O0",["-O0"]),("O2",["-O2"]),("SAN",["-O1","-fsanitize=address,undefined","-fno-sanitize-recover=all","-fno-pie","-no-pie"])):
            binary=output/f"{compiler}-{mode}";run([compiler,"-std=c11","-Wall","-Wextra","-Wpedantic","-Werror",*flags,"-Igenerated/include",*sources,host,"-o",binary]);actual=run([binary],commands,env={**os.environ,"ASAN_OPTIONS":"detect_leaks=1:abort_on_error=1"});assert actual==expected,(compiler,mode);configurations.append({"compiler":compiler,"mode":mode,"sha256":hashlib.sha256(actual).hexdigest()})
    base=LayoutText(exports["gcc"].read_text());mutations=[("arp-min","frame->length<42","frame->length<41"),("arp-ether","packet_be16(frame,12)!=0x0806","false"),("arp-shape","frame->bytes[18]!=6","false"),("arp-op","operation!=1&&operation!=2","false"),("ipv4-version","frame->bytes[14]>>4!=4","false"),("ipv4-header","header<20","header<16"),("ipv4-total","total>frame->length-14","false"),("ipv4-fragment","packet_be16(frame,20)&0x3fff","false"),("ipv4-checksum","packet_checksum(frame,14,header,0)!=0","false"),("udp-length","length!=ip.transport_length","false"),("udp-checksum","checksum&&packet_checksum", "false&&packet_checksum"),("payload-cap","payload->len>NETWORK_ENGINE_MAX_PAYLOAD","payload->len>NETWORK_ENGINE_MAX_PAYLOAD-1")];caught=[];prefix=[output/"network_config-gcc.c",output/"network_engine_frame-gcc.c"]
    for name,old,new in mutations:
        assert base.count(old)>=1,(name,base.count(old));mutant=output/f"mutant-{name}.c";mutant.write_text(base.replace(old,new));binary=output/f"mutant-{name}";run(["gcc","-std=c11","-O2","-Igenerated/include",*prefix,mutant,host,"-o",binary]);assert run([binary],commands)!=expected,("survived",name);caught.append(name)
    admission=[]
    if args.lean_root:
        assert args.cesta_source;sys.path.insert(0,str(args.cesta_source.resolve()/"tests"));helper=importlib.import_module("ownership_admission");unique={}
        for plan in plans.values():unique[hashlib.sha256(plan.read_bytes()).hexdigest()]=plan
        for index,plan in enumerate(unique.values()):(record,)=helper.parse(plan.read_bytes());admission.append(helper.admit_record(record,output/f"Packet{index}.lean",args.lean_root.resolve()))
    (output/"summary.json").write_text(json.dumps({"pin":inventory["pin"],"commands":count,"upstream_rust_tests":54,"configurations":configurations,"mutants_caught":caught,"admission":admission},indent=2)+"\n");print("network engine packet:",count,"commands;",len(configurations),"configurations;",len(caught),"mutants;",sum(x["functions"] for x in admission),"plans admitted")
if __name__=="__main__":main()
