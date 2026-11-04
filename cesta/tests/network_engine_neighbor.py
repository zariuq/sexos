"""Qualify the explicit bounded neighbor and route state for the current engine."""
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
BASIS=0xCBF29CE484222325;MASK=(1<<64)-1


def run(command,data=None,*,cwd=HERE,env=None):
    result=subprocess.run(list(map(str,command)),cwd=cwd,input=data,capture_output=True,env=env)
    assert result.returncode==0,(command,result.returncode,result.stdout[-2000:],result.stderr[-8000:]);return result.stdout


HOST=r'''
#include "network_engine_neighbor.h"
#include <inttypes.h>
#include <stdio.h>
static uint64_t hash(const uint8_t *bytes,size_t n){uint64_t h=UINT64_C(0xcbf29ce484222325);for(size_t i=0;i<n;i++)h=(h^bytes[i])*UINT64_C(0x100000001b3);return h;}
static NetConfig config(unsigned mode){NetConfig c=net_config_controlled();if(mode==1){c.dns=0xc6336435;c.http=0xcb007109;}else if(mode==2){c.local=0xc0a82c11;c.prefix=23;c.gateway=0xc0a82c01;c.dns=0xc0a82d01;c.http=0xc0a82d02;c.listen_port=46086;c.http_port=46088;c.dns_port=46089;}return c;}
static void show(const NetworkNeighborTable *t){printf(" T %u %u %"PRIu64" %zu",(unsigned)t->probe_present,t->probe_address,t->next_probe,network_neighbor_count(t));for(size_t i=0;i<NETWORK_NEIGHBOR_CAPACITY;i++){NetworkNeighborEntry e=t->entries[i];printf(" %u %u %"PRIu64" %"PRIu64" %"PRIu64,(unsigned)e.present,e.address,hash(e.mac.bytes,6),e.expires,e.last_used);}putchar('\n');}
int main(void){NetworkNeighborTable table;network_neighbor_init(&table);unsigned mode=0;char kind;while(scanf(" %c",&kind)==1){
 if(kind=='I'){network_neighbor_init(&table);printf("I");show(&table);}
 else if(kind=='C'){if(scanf(" %u",&mode)!=1)return 2;printf("C %u",mode);show(&table);}
 else if(kind=='D'){uint32_t destination;if(scanf(" %u",&destination)!=1)return 2;NetworkRouteDecision r=network_route_decide(config(mode),destination);printf("D %u %u %u\n",(unsigned)r.valid,(unsigned)r.on_link,r.next_hop);}
 else if(kind=='O'){uint64_t now;unsigned valid,op;uint32_t sip,tip;if(scanf(" %"SCNu64" %u %u %u %u",&now,&valid,&op,&sip,&tip)!=5)return 2;NetworkArpPacket p={0};p.valid=valid!=0;p.operation=(uint16_t)op;p.sender_ip=sip;p.target_ip=tip;for(size_t i=0;i<6;i++){unsigned b;if(scanf(" %u",&b)!=1)return 2;p.sender_mac.bytes[i]=(uint8_t)b;}bool ok=network_neighbor_observe(&table,config(mode),p,now);printf("O %u",(unsigned)ok);show(&table);}
 else if(kind=='R'){uint64_t now;uint32_t destination;if(scanf(" %"SCNu64" %u",&now,&destination)!=2)return 2;NetworkNeighborResolution r=network_neighbor_resolve(&table,config(mode),destination,now);printf("R %u %u %"PRIu64,(unsigned)r.kind,r.next_hop,hash(r.mac.bytes,6));show(&table);}
 else if(kind=='E'){uint64_t now;if(scanf(" %"SCNu64,&now)!=1)return 2;network_neighbor_expire(&table,now);printf("E");show(&table);}
 else if(kind=='S'){printf("S");show(&table);}else return 2;}return 0;}
'''


def fnv(data):
    h=BASIS
    for b in data:h=((h^b)*0x100000001B3)&MASK
    return h


def configs(mode):
    if mode==2:return dict(local=0xC0A82C11,prefix=23,gateway=0xC0A82C01,dns=0xC0A82D01,http=0xC0A82D02)
    return dict(local=0x0A00020F,prefix=24,gateway=0x0A000202,dns=0xC6336435 if mode==1 else 0x0A000202,http=0xCB007109 if mode==1 else 0x0A000202)


def contains(c,address):
    mask=((1<<32)-1)<<(32-c['prefix'])&0xffffffff;host=address&(~mask&0xffffffff);local_host=c['local']&(~mask&0xffffffff)
    first=address>>24;local_first=c['local']>>24
    return first not in (0,127) and first<224 and local_first not in (0,127) and local_first<224 and host not in (0,(~mask)&0xffffffff) and local_host not in (0,(~mask)&0xffffffff) and address&mask==c['local']&mask


class Model:
    def __init__(self):self.entries=[None]*8;self.probe=None;self.next_probe=0;self.mode=0
    def route(self,destination):
        c=configs(self.mode);first=destination>>24
        if first in (0,127) or first>=224 or destination==c['local']:return (0,0,0)
        on=contains(c,destination);return (1,int(on),destination if on else c['gateway'])
    def expire(self,now):
        for i,e in enumerate(self.entries):
            if e is not None and now>=e['expires']:self.entries[i]=None
    def observe(self,now,valid,op,sip,tip,mac):
        c=configs(self.mode);first=mac[0] if mac else 0
        if not valid or op not in (1,2) or tip!=c['local'] or not contains(c,sip) or not any(mac) or first&1 or now>MASK-60000:return False
        self.expire(now);slot=next((i for i,e in enumerate(self.entries) if e and e['address']==sip),None)
        if slot is None:slot=next((i for i,e in enumerate(self.entries) if e is None),None)
        if slot is None:slot=min(range(8),key=lambda i:(self.entries[i]['last'],i))
        self.entries[slot]=dict(address=sip,mac=bytes(mac),expires=now+60000,last=now)
        if self.probe==sip:self.probe=None;self.next_probe=0
        return True
    def resolve(self,now,destination):
        valid,on,next_hop=self.route(destination)
        if not valid:return (0,0,bytes(6))
        self.expire(now)
        for e in self.entries:
            if e and e['address']==next_hop:e['last']=now;return (1,next_hop,e['mac'])
        if self.probe!=next_hop:self.probe=next_hop;self.next_probe=now
        if now>=self.next_probe:self.next_probe=min(MASK,now+1000);return (2,next_hop,bytes(6))
        return (3,next_hop,bytes(6))
    def show(self,prefix):
        out=[prefix,"T",str(int(self.probe is not None)),str(self.probe or 0),str(self.next_probe),str(sum(e is not None for e in self.entries))]
        for e in self.entries:
            if e:out += ["1",str(e['address']),str(fnv(e['mac'])),str(e['expires']),str(e['last'])]
            else:out += ["0","0",str(fnv(bytes(6))),"0","0"]
        return " ".join(out)+"\n"


def corpus():
    m=Model();commands=[];expected=[]
    def add(command,output):commands.append(command+"\n");expected.append(output)
    add("I",m.show("I"));m.mode=1;add("C 1",m.show("C 1"))
    for destination in (0,0x7F000001,0xE0000001,0x0A00020F,0x0A000201,0xCB007109):
        r=m.route(destination);add(f"D {destination}",f"D {r[0]} {r[1]} {r[2]}\n")
    for now in (0,1,999,1000,1999,2000,3000):
        r=m.resolve(now,0xCB007109);add(f"R {now} 3405803785",m.show(f"R {r[0]} {r[1]} {fnv(r[2])}"))
    gateway=bytes([2,0,0,0,0,2]);ok=m.observe(3001,True,2,0x0A000202,0x0A00020F,gateway);add("O 3001 1 2 167772674 167772687 2 0 0 0 0 2",m.show(f"O {int(ok)}"))
    r=m.resolve(3002,0xCB007109);add("R 3002 3405803785",m.show(f"R {r[0]} {r[1]} {fnv(r[2])}"))
    add("E 63001",(m.expire(63001) or m.show("E")));r=m.resolve(63001,0xCB007109);add("R 63001 3405803785",m.show(f"R {r[0]} {r[1]} {fnv(r[2])}"))
    m=Model();add("I",m.show("I"))
    invalid_observations=[
        (0,True,2,0x0A000202,0x0A00020F,bytes([1,0,0,0,0,2])),
        (1,True,2,0x0A000202,0x0A00020F,bytes(6)),
        (2,True,2,0x0A000202,0x0A000210,bytes([2,0,0,0,0,2])),
        (3,True,2,0xCB007109,0x0A00020F,bytes([2,0,0,0,0,3])),
        (MASK-100,True,2,0x0A000202,0x0A00020F,bytes([2,0,0,0,0,4])),
    ]
    for now,valid,op,sip,tip,mac in invalid_observations:
        ok=m.observe(now,valid,op,sip,tip,mac);add(f"O {now} {int(valid)} {op} {sip} {tip} "+" ".join(map(str,mac)),m.show(f"O {int(ok)}"))
    m=Model();add("I",m.show("I"))
    for n in range(16):
        ip=0x0A000220+n;mac=bytes([2,0,0,0,0,n+1]);ok=m.observe(n,True,1,ip,0x0A00020F,mac);add(f"O {n} 1 1 {ip} 167772687 "+" ".join(map(str,mac)),m.show(f"O {int(ok)}"))
    rng=random.Random(0x4E45494748424F52);now=100
    for _ in range(6000):
        choice=rng.randrange(5);now+=rng.randrange(4)
        if choice==0:
            destination=rng.choice([0x0A000200|rng.randrange(1,255),0xCB007100|rng.randrange(1,255),0,0xE0000001]);r=m.route(destination);add(f"D {destination}",f"D {r[0]} {r[1]} {r[2]}\n")
        elif choice==1:
            destination=rng.choice([0x0A000200|rng.randrange(1,255),0xCB007100|rng.randrange(1,255)]);r=m.resolve(now,destination);add(f"R {now} {destination}",m.show(f"R {r[0]} {r[1]} {fnv(r[2])}"))
        elif choice==2:
            sip=0x0A000200|rng.randrange(1,255);mac=bytes([2,rng.randrange(256),rng.randrange(256),rng.randrange(256),rng.randrange(256),rng.randrange(256)]);valid=rng.randrange(5)!=0;op=rng.choice([0,1,2,3]);tip=rng.choice([0x0A00020F,0x0A000210]);ok=m.observe(now,valid,op,sip,tip,mac);add(f"O {now} {int(valid)} {op} {sip} {tip} "+" ".join(map(str,mac)),m.show(f"O {int(ok)}"))
        elif choice==3:add(f"E {now}",(m.expire(now) or m.show("E")))
        else:add("S",m.show("S"))
    return "".join(commands).encode(),"".join(expected).encode(),len(commands)


def main():
    p=argparse.ArgumentParser();p.add_argument("--cesta",type=Path,required=True);p.add_argument("--cesta-source",type=Path);p.add_argument("--lean-root",type=Path);p.add_argument("--output",type=Path,default=HERE/"build/network-engine-neighbor-tests");a=p.parse_args();w=a.output.resolve();w.mkdir(parents=True,exist_ok=True)
    inv=json.loads((HERE/"interfaces/network_engine_neighbor.source.json").read_text())
    for path,key in ((inv["library_path"],"library_sha256"),(inv["tests_path"],"tests_sha256")):actual=run(["git","show",f"{inv['pin']}:{path}"],cwd=HERE.parent);assert hashlib.sha256(actual).hexdigest()==inv[key]
    cargo=run(["cargo","test","-p","sex-net","--lib","--","--test-threads=1"],cwd=HERE.parent);assert b"54 passed" in cargo
    commands,expected,count=corpus();host=w/"host.c";host.write_text(HOST);exports={};plans={};configs=[]
    for cc in ("gcc","clang"):
        sources=[]
        for name in ("network_config","network_engine_frame","network_engine_packet","network_engine_neighbor"):
            out=w/f"{name}-{cc}.c";cmd=[a.cesta,"export-c",f"src/{name}.cesta","--cc",cc,"--std=c11","-o",out]
            if name=="network_engine_neighbor":plan=w/f"{name}-{cc}.plan";cmd.extend(["--plan",plan]);plans[cc]=plan;exports[cc]=out
            run(cmd+["--","-Igenerated/include"]);sources.append(out)
        for mode,flags in (("O0",["-O0"]),("O2",["-O2"]),("SAN",["-O1","-fsanitize=address,undefined","-fno-sanitize-recover=all","-fno-pie","-no-pie"])):
            binary=w/f"{cc}-{mode}";run([cc,"-std=c11","-Wall","-Wextra","-Wpedantic","-Werror",*flags,"-Igenerated/include",*sources,host,"-o",binary]);actual=run([binary],commands,env={**os.environ,"ASAN_OPTIONS":"detect_leaks=1:abort_on_error=1"});assert actual==expected,(cc,mode);configs.append({"compiler":cc,"mode":mode,"sha256":hashlib.sha256(actual).hexdigest()})
    base=exports["gcc"].read_text();mutations=[("multicast-mac","(mac.bytes[0]&1)==0","true"),("target-local","packet.target_ip!=config.local","false"),("sender-onlink","!net_config_contains(config,packet.sender_ip)","false"),("expiry-overflow","UINT64_MAX-now<NETWORK_NEIGHBOR_LIFETIME","false"),("expiry-edge","now>=table->entries[i].expires","now>table->entries[i].expires"),("lru-direction","last_used<table->entries[selected].last_used","last_used>table->entries[selected].last_used"),("route-next-hop","on_link?destination:config.gateway","destination"),("probe-interval","now+NETWORK_NEIGHBOR_PROBE_INTERVAL","now+1"),("probe-clear","if(table->probe_present&&table->probe_address==packet.sender_ip)","if(false)"),("touch-hit","table->entries[i].last_used=now","table->entries[i].last_used=0")];caught=[];prefix=[w/"network_config-gcc.c",w/"network_engine_frame-gcc.c",w/"network_engine_packet-gcc.c"]
    for name,old,new in mutations:
        assert base.count(old)>=1,(name,base.count(old));mut=w/f"mutant-{name}.c";mut.write_text(base.replace(old,new));binary=w/f"mutant-{name}";run(["gcc","-std=c11","-O2","-Igenerated/include",*prefix,mut,host,"-o",binary]);assert run([binary],commands)!=expected,("survived",name);caught.append(name)
    admission=[]
    if a.lean_root:
        assert a.cesta_source;sys.path.insert(0,str(a.cesta_source.resolve()/"tests"));h=importlib.import_module("ownership_admission");unique={}
        for plan in plans.values():unique[hashlib.sha256(plan.read_bytes()).hexdigest()]=plan
        for i,plan in enumerate(unique.values()):(record,)=h.parse(plan.read_bytes());admission.append(h.admit_record(record,w/f"Neighbor{i}.lean",a.lean_root.resolve()))
    (w/"summary.json").write_text(json.dumps({"pin":inv["pin"],"commands":count,"upstream_rust_tests":54,"configurations":configs,"mutants_caught":caught,"admission":admission},indent=2)+"\n");print("network neighbor:",count,"commands;",len(configs),"configurations;",len(caught),"mutants;",sum(x["functions"] for x in admission),"plans admitted")
if __name__=="__main__":main()
