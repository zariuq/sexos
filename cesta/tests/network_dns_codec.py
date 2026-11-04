"""Differentially qualify the current bounded sex-net DNS codec."""
import argparse
import hashlib
import importlib
import json
import os
from pathlib import Path
import random
import subprocess
import sys

HERE = Path(__file__).resolve().parent.parent
ID = 0x7942
NAME = b"\x05probe\x05sexos\x04test\x00"
QUESTION = NAME + b"\x00\x01\x00\x01"
ADDRESS = bytes([10, 0, 2, 2])


def run(command, data=None, *, cwd=HERE, env=None):
    result = subprocess.run(list(map(str, command)), cwd=cwd, input=data, capture_output=True, env=env)
    assert result.returncode == 0, (command, result.returncode, result.stdout[-2000:], result.stderr[-8000:])
    return result.stdout


RUST_DRIVER = r'''
use std::io::BufRead;
fn hash(bytes:&[u8])->u64{let mut h=0xcbf29ce484222325u64;for b in bytes{h=(h^*b as u64).wrapping_mul(0x100000001b3);}h}
fn error(e:dns_wire::Error)->(u8,u8){use dns_wire::Error::*;match e{InvalidId=>(1,0),Oversized=>(2,0),Truncated=>(3,0),Malformed=>(4,0),Unsupported=>(5,0),Rcode(n)=>(6,n),NoAnswer=>(7,0)}}
fn main(){for line in std::io::stdin().lock().lines(){let line=line.unwrap();let mut p=line.split_whitespace();match p.next().unwrap(){
 "Q"=>{let id:u16=p.next().unwrap().parse().unwrap();let fill:u8=p.next().unwrap().parse().unwrap();let mut out=[fill;512];match dns_wire::query(id,&mut out){Ok(n)=>println!("Q 1 0 {} {}",n,hash(&out)),Err(e)=>{let(e,_)=error(e);println!("Q 0 {} 0 {}",e,hash(&out));}}},
 "P"=>{let id:u16=p.next().unwrap().parse().unwrap();let n:usize=p.next().unwrap().parse().unwrap();let mut bytes=Vec::with_capacity(n);for _ in 0..n{bytes.push(p.next().unwrap().parse().unwrap());}match dns_wire::parse(id,&bytes){Ok(None)=>println!("N"),Ok(Some(a))=>println!("A {} {} {} {} {}",a.address[0],a.address[1],a.address[2],a.address[3],a.ttl),Err(e)=>{let(e,r)=error(e);println!("E {} {}",e,r);}}},_=>panic!()}}
}
'''


HOST = r'''
#include "network_dns_codec.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
static uint64_t hash(const uint8_t *bytes,size_t n){uint64_t h=UINT64_C(0xcbf29ce484222325);for(size_t i=0;i<n;i++)h=(h^bytes[i])*UINT64_C(0x100000001b3);return h;}
int main(void){char kind;while(scanf(" %c",&kind)==1){
 if(kind=='Q'){unsigned id,fill;if(scanf(" %u %u",&id,&fill)!=2)return 2;NetworkDnsFrame frame;memset(frame.bytes,(int)(uint8_t)fill,sizeof(frame.bytes));frame.len=999;
  NetworkDnsQueryResult r=network_dns_query((uint16_t)id,&frame);printf("Q %u %u %zu %"PRIu64"\n",(unsigned)r.ok,(unsigned)r.error,r.ok?r.len:0,hash(frame.bytes,sizeof(frame.bytes)));}
 else if(kind=='P'){unsigned id;size_t n;if(scanf(" %u %zu",&id,&n)!=2)return 2;NetworkDnsFrame frame;memset(&frame,0,sizeof(frame));frame.len=n;
  for(size_t i=0;i<n;i++){unsigned byte;if(scanf(" %u",&byte)!=1)return 2;if(i<sizeof(frame.bytes))frame.bytes[i]=(uint8_t)byte;}
  NetworkDnsParseResult r=network_dns_parse((uint16_t)id,&frame);if(r.kind==NETWORK_DNS_RESULT_UNRELATED)puts("N");else if(r.kind==NETWORK_DNS_RESULT_ANSWER)printf("A %u %u %u %u %u\n",r.answer.address[0],r.answer.address[1],r.answer.address[2],r.answer.address[3],r.answer.ttl);else printf("E %u %u\n",(unsigned)r.error,(unsigned)r.rcode);}
 else return 2;}return 0;}
'''


def query_packet(identifier=ID):
    packet = bytearray(12 + len(QUESTION)); packet[0:2] = identifier.to_bytes(2, "big")
    packet[2] = 0x81; packet[3] = 0x80; packet[5] = 1; packet[12:] = QUESTION; return packet


def wire(text):
    result = bytearray()
    for label in text.split("."): result.append(len(label)); result.extend(label.encode())
    result.append(0); return bytes(result)


def pointer(offset): return (0xC000 | offset).to_bytes(2, "big")


def rr(packet, section, owner, kind, data, ttl=60):
    field = 6 + section * 2; count = int.from_bytes(packet[field:field+2], "big") + 1; packet[field:field+2] = count.to_bytes(2, "big")
    start = len(packet); packet.extend(owner); packet.extend(kind.to_bytes(2, "big")); packet.extend(b"\x00\x01")
    packet.extend(ttl.to_bytes(4, "big")); packet.extend(len(data).to_bytes(2, "big")); data_start = len(packet); packet.extend(data); return start, data_start


def valid(compressed=True):
    packet = query_packet(); rr(packet, 0, pointer(12) if compressed else NAME, 1, ADDRESS, 60); return packet


def chain(links):
    packet = query_packet(); owner = NAME
    for n in range(links):
        target = wire(f"alias{n}.sexos.test"); rr(packet, 0, owner, 5, target, 50-n); owner = target
    rr(packet, 0, owner, 1, ADDRESS, 60); return packet


def corpus():
    commands = [("Q", 0, 0xA5), ("Q", 1, 0), ("Q", ID, 0xA5), ("Q", 65535, 0x5A)]
    packets = []
    for compressed in (False, True):
        packet = valid(compressed); packets.append((ID, bytes(packet)))
        packets.extend((ID, bytes(packet[:cut])) for cut in range(len(packet)))
        for index in range(len(packet)):
            for xor in (1, 0x80, 0xFF):
                changed = bytearray(packet); changed[index] ^= xor; packets.append((ID, bytes(changed)))
    for rcode in range(1, 16):
        packet = valid(); packet[3] |= rcode; packets.append((ID, bytes(packet)))
    for index, mask, operation in ((2, 2, "or"), (2, 0x80, "and-not"), (2, 8, "or"), (3, 0x40, "or")):
        packet = valid(); packet[index] = packet[index] & ~mask if operation == "and-not" else packet[index] | mask; packets.append((ID, bytes(packet)))
    for qd in (0, 2, 255):
        packet = valid(); packet[4:6] = qd.to_bytes(2, "big"); packets.append((ID, bytes(packet)))
    packet = valid(); packet[6:8] = (9).to_bytes(2, "big"); packets.append((ID, bytes(packet)))
    packet = valid(); packet[10:12] = (9).to_bytes(2, "big"); packets.append((ID, bytes(packet)))
    packet = valid(); packet.append(0); packets.append((ID, bytes(packet)))
    for target in (0, 4, 12, 13, 18, 31, 34, 35, 36, 40, 46, 255):
        packet = valid(); packet[34:36] = pointer(target); packets.append((ID, bytes(packet)))
    for extras in (4, 5):
        packet = query_packet(); target = 12
        for _ in range(8): target = rr(packet, 0, pointer(target), 1, ADDRESS)[0]
        for _ in range(extras):
            start = len(packet); _, data = rr(packet, 1, pointer(target), 2, pointer(start)); target = data
        packets.append((ID, bytes(packet)))
    for last in (61, 62):
        target = bytearray()
        for length, byte in ((63, b'a'), (63, b'b'), (63, b'c'), (last, b'd')):
            target.append(length); target.extend(byte * length)
        target.append(0); packet = query_packet(); _, offset = rr(packet, 0, pointer(12), 5, bytes(target), 20)
        rr(packet, 0, pointer(offset), 1, ADDRESS, 60); packets.append((ID, bytes(packet)))
    for links in range(6):
        packet = chain(links); packets.append((ID, bytes(packet))); packets.extend((ID, bytes(packet[:cut])) for cut in range(len(packet)))
    packet = query_packet(); alias = wire("Alias.SEXos.Test"); rr(packet, 0, wire("PROBE.sexos.TEST"), 5, alias, 40); rr(packet, 0, wire("ALIAS.sexos.test"), 1, ADDRESS, 30); packets.append((ID, bytes(packet)))
    packet = valid(); ns = wire("ns.sexos.test"); rr(packet, 1, pointer(18), 2, ns); soa = ns + wire("hostmaster.sexos.test") + bytes(20); rr(packet, 1, pointer(18), 6, soa); rr(packet, 2, ns, 28, bytes(16)); rr(packet, 2, ns, 1, bytes([192,0,2,1])); packets.append((ID, bytes(packet)))
    packet = query_packet();
    for _ in range(8): rr(packet, 0, pointer(12), 1, ADDRESS)
    for _ in range(8): rr(packet, 2, pointer(12), 1, bytes([192,0,2,1]), 0)
    packets.append((ID, bytes(packet))); rr(packet, 2, pointer(12), 1, ADDRESS); packets.append((ID, bytes(packet)))
    zero_id = valid(); zero_id[0:2] = b"\x00\x00"
    packets.extend([(ID+1, bytes(valid())), (0, bytes(valid())), (ID, bytes(zero_id)), (ID, bytes(query_packet()))])
    rng = random.Random(0x444E5357495245)
    for _ in range(6000):
        length = rng.randrange(514); packets.append((rng.randrange(65536), bytes(rng.randrange(256) for _ in range(length))))
    commands.extend(("P", expected, packet) for expected, packet in packets)
    lines = []
    for command in commands:
        if command[0] == "Q": lines.append(f"Q {command[1]} {command[2]}\n")
        else: lines.append(f"P {command[1]} {len(command[2])} " + " ".join(map(str, command[2])) + "\n")
    return "".join(lines).encode(), len(commands)


def main():
    parser = argparse.ArgumentParser(); parser.add_argument("--cesta", type=Path, required=True)
    parser.add_argument("--cesta-source", type=Path); parser.add_argument("--lean-root", type=Path)
    parser.add_argument("--output", type=Path, default=HERE / "build/network-dns-codec-tests")
    args = parser.parse_args(); output = args.output.resolve(); output.mkdir(parents=True, exist_ok=True)
    inventory = json.loads((HERE / "interfaces/network_dns_codec.source.json").read_text())
    source = run(["git", "show", f"{inventory['pin']}:{inventory['path']}"], cwd=HERE.parent)
    assert hashlib.sha256(source).hexdigest() == inventory["sha256"]
    module = source.decode().replace("sex_net_config::", "crate::sex_net_config::")
    rust_source = "mod sex_net_config { pub const DNS_NAME:&[u8]=b\\\"\\x05probe\\x05sexos\\x04test\\0\\\"; pub const DNS_QUESTION:&[u8]=b\\\"\\x05probe\\x05sexos\\x04test\\0\\0\\x01\\0\\x01\\\"; }\n"
    rust_source += "mod dns_wire {\n" + module + "\n}\n" + RUST_DRIVER
    rust_source = rust_source.replace('b\\"', 'b"').replace('\\";', '";')
    rust = output / "oracle.rs"; rust.write_text(rust_source); oracle = output / "oracle"
    run(["rustc", "--edition=2021", "-O", rust, "-o", oracle])
    cargo = run(["cargo", "test", "-p", "sex-net", "--lib", "--", "--test-threads=1"], cwd=HERE.parent); assert b"54 passed" in cargo
    commands, count = corpus(); expected = run([oracle], commands); host = output / "host.c"; host.write_text(HOST)
    configurations = []; exports = {}; plans = {}
    for compiler in ("gcc", "clang"):
        exported = output / f"network-dns-{compiler}.c"; plan = output / f"network-dns-{compiler}.plan"
        run([args.cesta, "export-c", "src/network_dns_codec.cesta", "--cc", compiler, "--std=c11", "-o", exported, "--plan", plan, "--", "-Igenerated/include"])
        exports[compiler] = exported; plans[compiler] = plan
        for mode, flags in (("O0", ["-O0"]), ("O2", ["-O2"]), ("SAN", ["-O1", "-fsanitize=address,undefined", "-fno-sanitize-recover=all", "-fno-pie", "-no-pie"])):
            binary = output / f"{compiler}-{mode}"; run([compiler, "-std=c11", "-Wall", "-Wextra", "-Wpedantic", "-Werror", *flags, "-Igenerated/include", exported, host, "-o", binary])
            actual = run([binary], commands, env={**os.environ, "ASAN_OPTIONS": "detect_leaks=1:abort_on_error=1"}); assert actual == expected, (compiler, mode)
            configurations.append({"compiler": compiler, "mode": mode, "sha256": hashlib.sha256(actual).hexdigest()})
    base = exports["gcc"].read_text(); mutations = [
        ("zero-query-id", "if(id==0)return(NetworkDnsQueryResult)", "if(false)return(NetworkDnsQueryResult)"),
        ("zero-response-id", "if(id==0)return dns_error", "if(false)return dns_error"),
        ("oversize", "response->len>NETWORK_DNS_MAX_BYTES", "response->len>NETWORK_DNS_MAX_BYTES+1"),
        ("registered-boundary", "||!boundary_has(known,target)", ""),
        ("hop-bound", "hops>DNS_MAX_HOPS", "false"),
        ("expanded-name", "expanded>DNS_MAX_NAME", "expanded>DNS_MAX_NAME+1"),
        ("question-count", "if(questions!=1)", "if(questions>1)"),
        ("response-flag", "if((flags&0x8000)==0)", "if(false)"),
        ("truncation-flag", "if(flags&0x0200)", "if(false)"),
        ("answer-cap", "answers>DNS_MAX_ANSWERS", "answers>DNS_MAX_ANSWERS+1"),
        ("extra-cap", "authority+additional>DNS_MAX_EXTRA", "authority+additional>DNS_MAX_EXTRA+1"),
        ("whole-packet", "if(cursor!=response->len)", "if(false)"),
        ("chain-bound", "if(depth==DNS_MAX_CHAIN)", "if(false)"),
    ]; caught=[]
    for name, old, new in mutations:
        assert base.count(old) >= 1, (name, base.count(old)); mutant = output / f"mutant-{name}.c"; mutant.write_text(base.replace(old, new)); binary = output / f"mutant-{name}"
        run(["gcc", "-std=c11", "-O2", "-Igenerated/include", mutant, host, "-o", binary]); assert run([binary], commands) != expected, ("survived", name); caught.append(name)
    admission=[]
    if args.lean_root:
        assert args.cesta_source; sys.path.insert(0, str(args.cesta_source.resolve() / "tests")); helper=importlib.import_module("ownership_admission"); unique={}
        for plan in plans.values(): unique[hashlib.sha256(plan.read_bytes()).hexdigest()]=plan
        for index,plan in enumerate(unique.values()):
            (record,)=helper.parse(plan.read_bytes()); admission.append(helper.admit_record(record,output/f"Dns{index}.lean",args.lean_root.resolve()))
    summary={"pin":inventory["pin"],"commands":count,"upstream_rust_tests":54,"configurations":configurations,"mutants_caught":caught,"admission":admission}
    (output/"summary.json").write_text(json.dumps(summary,indent=2)+"\n");print("network DNS codec:",count,"commands;",len(configurations),"configurations;",len(caught),"mutants;",sum(x["functions"] for x in admission),"plans admitted")


if __name__ == "__main__": main()
