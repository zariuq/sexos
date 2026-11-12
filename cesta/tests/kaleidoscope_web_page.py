#!/usr/bin/env python3
"""Compare Kaleidoscope's current web address model with the pinned Rust source."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
from layout_text import LayoutText

HERE = Path(__file__).resolve().parent.parent
REPO = HERE.parent
PIN = "8019f72d617927949545bde1fb5fc6fc56acd939"
SOURCE_SHA256 = "3a5ca19cc2b7db2fa53a8511c8618d7026825d672e1239890771c262ec723d58"

RUST_STUB = r'''
#[path="web_page.rs"] mod implementation;
pub use implementation::{numeric,parse,resolve,typed,URL_MAX};
extern crate self as sex_pdx;
pub mod memory_region { pub const QUERY_VIEW:u64=1; pub const READ:u64=2; pub const WRITE:u64=4; }
pub mod web {
    pub const CLIENT_SLOT:u64=259; pub const REGION_BYTES:usize=65536; pub const OP:u64=0x211;
    pub const UP:u8=1; pub const RESOLVE:u8=2; pub const CONNECT:u8=3; pub const STATE:u8=4;
    pub const SEND:u8=5; pub const RECEIVE:u8=6; pub const CLOSE:u8=7;
    pub const TLS_START:u8=8; pub const TLS_STAGE:u8=9; pub const FINISH_REQUEST:u8=10;
    pub const STATE_OPENING:u32=0; pub const STATE_OPEN:u32=1;
    pub const STATE_PEER_CLOSED:u32=2; pub const STATE_CLOSED:u32=3;
    pub const TLS_HANDSHAKE:u32=0; pub const TLS_READY:u32=1;
    pub const TLS_FAILED_CERTIFICATE:u32=2; pub const TLS_FAILED_SIGNATURE:u32=3;
    pub const OK:u8=0; pub const WAIT:u8=1; pub const DENIED:u8=2;
    pub const OFFLINE:u8=3; pub const NO_NAME:u8=4; pub const BUSY:u8=5;
    pub const fn request(seq:u32,kind:u8)->u64{(seq as u64&0xFF_FFFF)|((kind as u64)<<32)}
    pub const fn address_port(address:[u8;4],port:u16)->u64{(u32::from_be_bytes(address) as u64)<<16|port as u64}
    pub fn decode_reply(_word:u64)->Option<(u32,u8,u32)>{None}
}
pub const SLOT_NET:u64=15;
pub struct View { pub virtual_address:u64 }
pub fn memory_region_view(_slot:u64,_offset:u64,_len:u64,_rights:u64)->Result<View,()>{Err(())}
pub fn pdx_call_checked(_slot:u64,_op:u64,_request:u64,_value:u64,_arg:u64)->Result<u64,u64>{Err(0)}
#[macro_export] macro_rules! serial_record { ($($arg:tt)*) => {{}} }
pub mod document_reader { pub const LONG_BODY:usize=131072; }
pub mod html_text {
    pub const MAX_LINKS:usize=32;
    #[derive(Clone,Copy)] pub struct Converted { pub len:usize, pub targets:[(usize,usize);MAX_LINKS], pub links:usize }
    pub fn looks_like_html(_source:&[u8])->bool{false}
}
fn main() {
    for url in ["https://a.org/x/y.html","http://host:8080","https://a.org?x","a.org",
                "https://:80/x","https://a:0","https://a:65536","https://a:/","https://a:12x/",""] {
        match parse(url.as_bytes()) {
            Some(p)=>println!("P 1 {} {} {} {}",p.port,String::from_utf8_lossy(p.host),String::from_utf8_lossy(p.path),p.tls as u8),
            None=>println!("P 0"),
        }
    }
    for (base,target) in [
        ("https://a.org/x/y.html","z.html"),("https://a.org/x/y.html","../z"),
        ("https://a.org/x/y.html","/z"),("https://a.org","z"),("http://a.org/","//b.org/c"),
        ("http://a.org/","https://b.org/c#d"),("http://a.org/","mailto:x@y"),
        ("https://a.org/x/y.html","./p/./q/../r"),("https://a.org/x/y.html","../../"),
        ("https://a.org/x/y.html",""),("https://a.org/x/y.html","sibling:80"),
        ("a.org/x","z"),("https://a.org/x/y.html","#fragment")] {
        let mut out=[0u8;URL_MAX];
        let n=resolve(base.as_bytes(),target.as_bytes(),&mut out).unwrap_or(0);
        println!("R {} {}",n,String::from_utf8_lossy(&out[..n]));
    }
    for host in ["127.0.0.1","255.255.255.255","01.2.3.4","256.1.1.1","1.2.3","1.2.3.4.5","1..2.3","1.2.3.a"] {
        match numeric(host.as_bytes()) {
            Some(v)=>println!("N 1 {} {} {} {}",v[0],v[1],v[2],v[3]),
            None=>println!("N 0"),
        }
    }
    for (code,shift) in [(0x02,false),(0x02,true),(0x10,false),(0x10,true),(0x1c,false),(0x2c,false),(0x2c,true),(0x39,false),(58,false)] {
        println!("T {} {} {}",code,shift as u8,typed(code,shift).map_or(-1,|v|v as i32));
    }
}
'''

C_HARNESS = r'''
#include <stdio.h>
#include <string.h>
#include "kaleidoscope_web_page.h"
static void parse_case(const char *url) {
    KaleidoscopeWebParts p;
    if (!kaleidoscope_web_parse((const uint8_t *)url, strlen(url), &p)) {
        puts("P 0");
        return;
    }
    printf("P 1 %u %.*s %.*s %u\n", p.port, (int)p.host_len, p.host, (int)p.path_len, p.path, (unsigned)p.tls);
}
static void resolve_case(const char *base, const char *target) {
    KaleidoscopeWebUrl out;
    size_t n = kaleidoscope_web_resolve((const uint8_t *)base, strlen(base),
                                        (const uint8_t *)target, strlen(target), &out);
    printf("R %zu %.*s\n", n, (int)n, out.bytes);
}
static void numeric_case(const char *host) {
    uint8_t out[4];
    if (!kaleidoscope_web_numeric_ipv4((const uint8_t *)host, strlen(host), out)) {
        puts("N 0");
        return;
    }
    printf("N 1 %u %u %u %u\n", out[0], out[1], out[2], out[3]);
}
int main(void) {
    const char *urls[] = {"https://a.org/x/y.html","http://host:8080","https://a.org?x","a.org",
                          "https://:80/x","https://a:0","https://a:65536","https://a:/","https://a:12x/",""};
    for (size_t i = 0; i < sizeof urls / sizeof urls[0]; ++i) parse_case(urls[i]);
    const char *pairs[][2] = {
        {"https://a.org/x/y.html","z.html"},{"https://a.org/x/y.html","../z"},
        {"https://a.org/x/y.html","/z"},{"https://a.org","z"},{"http://a.org/","//b.org/c"},
        {"http://a.org/","https://b.org/c#d"},{"http://a.org/","mailto:x@y"},
        {"https://a.org/x/y.html","./p/./q/../r"},{"https://a.org/x/y.html","../../"},
        {"https://a.org/x/y.html",""},{"https://a.org/x/y.html","sibling:80"},
        {"a.org/x","z"},{"https://a.org/x/y.html","#fragment"}};
    for (size_t i = 0; i < sizeof pairs / sizeof pairs[0]; ++i) resolve_case(pairs[i][0], pairs[i][1]);
    const char *hosts[] = {"127.0.0.1","255.255.255.255","01.2.3.4","256.1.1.1",
                           "1.2.3","1.2.3.4.5","1..2.3","1.2.3.a"};
    for (size_t i = 0; i < sizeof hosts / sizeof hosts[0]; ++i) numeric_case(hosts[i]);
    const uint8_t codes[] = {0x02,0x02,0x10,0x10,0x1c,0x2c,0x2c,0x39,58};
    const bool shifts[] = {false,true,false,true,false,false,true,false,false};
    for (size_t i = 0; i < sizeof codes; ++i)
        printf("T %u %u %d\n", codes[i], shifts[i], kaleidoscope_web_typed(codes[i], shifts[i]));
    return 0;
}
'''


def run(argv, *, env=None):
    result = subprocess.run(list(map(str, argv)), cwd=HERE, capture_output=True, env=env)
    if result.returncode:
        raise RuntimeError((argv, result.returncode, result.stdout[-2000:], result.stderr[-5000:]))
    return result.stdout


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cesta", type=Path, required=True)
    parser.add_argument("--work", type=Path)
    parser.add_argument("--mutations", action="store_true")
    args = parser.parse_args()
    work = (args.work or Path(tempfile.mkdtemp(prefix="web-page-", dir=HERE / "build"))).resolve()
    work.mkdir(parents=True, exist_ok=True)
    source = subprocess.check_output(["git", "show", PIN + ":apps/kaleidoscope/src/web_page.rs"], cwd=REPO)
    assert hashlib.sha256(source).hexdigest() == SOURCE_SHA256
    assert source.count(b"fn numeric(host") == 1 and source.count(b"const URL_MAX") == 1
    source = source.replace(b"fn numeric(host", b"pub fn numeric(host", 1)
    source = source.replace(b"const URL_MAX", b"pub const URL_MAX", 1)
    (work / "web_page.rs").write_bytes(source)
    (work / "reference.rs").write_text(RUST_STUB)
    (work / "host.c").write_text(C_HARNESS)
    reference = work / "reference"
    run(["rustc", "--edition=2021", "-A", "warnings", "-C", "opt-level=2", work / "reference.rs", "-o", reference])
    expected = run([reference])
    run([args.cesta, "export-c", "src/kaleidoscope_web_page_interface.cesta", "-o", work / "kaleidoscope_web_page.h", "--"])
    run([args.cesta, "export-c", "src/kaleidoscope_web_page.cesta", "--std=c11", "-o", work / "web_page.c", "--", "-I" + str(work)])
    common = ["-std=c11", "-g0", "-Wall", "-Wextra", "-Wpedantic", "-Werror", "-I" + str(work)]
    builds = []
    for compiler in ("gcc", "clang"):
        for mode, extra in (("O0", ["-O0"]), ("O2", ["-O2"]),
                            ("SAN", ["-O1", "-fsanitize=address,undefined", "-fno-sanitize-recover=all", "-fno-pie", "-no-pie"])):
            binary = work / f"{compiler}-{mode}"
            run([compiler, *common, *extra, work / "host.c", work / "web_page.c", "-o", binary])
            actual = run([binary], env={**os.environ, "ASAN_OPTIONS": "detect_leaks=1:abort_on_error=1"})
            assert actual == expected, (compiler, mode, actual, expected)
            builds.append((compiler, mode))
    mutations = []
    if args.mutations:
        original = LayoutText((work / "web_page.c").read_text())
        changes = [
            ("port_base", "port = port * 10u + (uint32_t)(url[j] - '0');", "port = port * 11u + (uint32_t)(url[j] - '0');"),
            ("dotdot_pop", "if (path[len] == '/') break;", "if (false) break;"),
            ("ipv4_bound", "if (value > 255u) return false;", "if (value > 256u) return false;"),
            ("shifted_input", "uint8_t value = shift ? upper[code] : plain[code];", "uint8_t value = plain[code];\n    (void)shift;\n    (void)upper;"),
            ("missing_scheme", "if (!scheme_end) return 0;", "if (false) return 0;"),
        ]
        for name, before, after in changes:
            assert original.count(before) == 1, name
            mutant = work / f"{name}.c"
            mutant.write_text(original.replace(before, after, 1))
            binary = work / name
            built = subprocess.run(["gcc", *common, "-O2", work / "host.c", mutant, "-o", binary], cwd=HERE, capture_output=True)
            assert built.returncode == 0, (name, built.stderr[-1000:])
            actual = subprocess.run([binary], cwd=HERE, capture_output=True)
            assert actual.returncode != 0 or actual.stdout != expected, ("mutation survived", name)
            mutations.append(name)
    summary = {"pin": PIN, "source_sha256": SOURCE_SHA256, "builds": builds,
               "mutations": mutations, "scope": "URL parsing, link resolution, numeric IPv4 and keyboard mapping; no network/TLS or full browser claim"}
    (work / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")
    print("kaleidoscope web address model:", len(builds), "builds,", len(mutations), "mutants")


if __name__ == "__main__":
    main()
