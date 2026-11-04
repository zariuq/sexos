#!/usr/bin/env python3
"""Compare bounded Kaleidoscope paths/navigation with pinned Rust."""
import argparse
import hashlib
import json
import os
import subprocess
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent.parent
REPO = HERE.parent
PIN = "44053ea9b017fe89008413f7cc8a6b1a4d28d015"
CODEC_SHA256 = "5844ad35713fecaf175f824facbe1c13c7ee96ca8b0dd417aba326cf90331e79"
NAV_SHA256 = "5b7b7c36c76b79fe27960c8c2f9df7c3900158b53cfd0502ab9cc1e37d646c38"


def run(argv, *, env=None):
    result = subprocess.run(list(map(str, argv)), cwd=HERE, capture_output=True, env=env)
    if result.returncode:
        raise RuntimeError((argv, result.returncode, result.stdout[-1000:], result.stderr[-5000:]))
    return result.stdout


RUST_HARNESS = r'''
extern crate self as sex_net_config;
pub const HOST_NAME:&[u8]=b"probe.sexos.test";
pub const TLS_PORT:u16=443;
#[path="codec.rs"]mod http_wire;
#[path="http_navigation.rs"]mod http_navigation;
use http_wire::Path;use http_navigation::Navigation;
fn show(tag:&str,n:&Navigation){let p=n.current();println!("{} {} {} {} {}",tag,n.active()as u8,n.hops(),p.as_bytes().len(),String::from_utf8_lossy(p.as_bytes()));}
fn path(x:&[u8])->Path{Path::from_location(x).unwrap()}
fn main(){
 for raw in [&b"/"[..],b"/welcome",b"/Guide",b"/guide/text",b"/a-b_c.~"]{match Path::from_location(raw){Ok(p)=>println!("P 1 {} {}",p.as_bytes().len(),String::from_utf8_lossy(p.as_bytes())),Err(_)=>println!("P 0 0 -")}}
 for raw in [&b""[..],b"guide",b"//x",b"/../x",b"/./x",b"/a?b",b"/a#b",b"/a b",b"/a\\b",b"https://x/y"]{println!("N {}",Path::from_location(raw).is_err()as u8);}
 let mut n=Navigation::new();show("I",&n);println!("F {}",n.follow(path(b"/guide"))as u8);show("J",&n);
 n.begin();show("B",&n);println!("F {}",n.follow(path(b"/guide"))as u8);println!("F {}",n.follow(path(b"/guide/text"))as u8);println!("F {}",n.follow(path(b"/third"))as u8);show("D",&n);
 n.begin();println!("F {}",n.follow(path(b"/guide"))as u8);println!("F {}",n.follow(Path::initial())as u8);show("L",&n);
 n.begin();println!("F {}",n.follow(path(b"/Guide"))as u8);println!("F {}",n.follow(path(b"/guide"))as u8);n.complete();show("C",&n);println!("F {}",n.follow(path(b"/next"))as u8);n.cancel();show("X",&n);
}
'''

C_HARNESS = r'''
#include <stdio.h>
#include <string.h>
#include "kaleidoscope_navigation.h"
static KaleidoscopeHttpPath raw(const char *s){KaleidoscopeHttpPath p={0};p.len=strlen(s);memcpy(p.bytes,s,p.len);return p;}
static KaleidoscopeHttpPath parsed(const char *s){KaleidoscopeHttpPath i=raw(s),p={0};if(!kaleidoscope_http_path_parse(&i,&p))__builtin_trap();return p;}
static void show(const char *tag,const KaleidoscopeNavigation *n){KaleidoscopeHttpPath p={0};kaleidoscope_navigation_current(n,&p);printf("%s %u %zu %zu %.*s\n",tag,(unsigned)kaleidoscope_navigation_active(n),kaleidoscope_navigation_hops(n),p.len,(int)p.len,p.bytes);}
int main(void){
 const char *valid[]={"/","/welcome","/Guide","/guide/text","/a-b_c.~"};for(size_t k=0;k<5;k++){KaleidoscopeHttpPath i=raw(valid[k]),p={0};printf("P %u ",(unsigned)kaleidoscope_http_path_parse(&i,&p));printf("%zu %.*s\n",p.len,(int)p.len,p.bytes);}
 const char *bad[]={"","guide","//x","/../x","/./x","/a?b","/a#b","/a b","/a\\b","https://x/y"};for(size_t k=0;k<10;k++){KaleidoscopeHttpPath i=raw(bad[k]),p={0};printf("N %u\n",(unsigned)!kaleidoscope_http_path_parse(&i,&p));}
 KaleidoscopeNavigation n;kaleidoscope_navigation_init(&n);show("I",&n);KaleidoscopeHttpPath p=parsed("/guide");printf("F %u\n",(unsigned)kaleidoscope_navigation_follow(&n,&p));show("J",&n);
 kaleidoscope_navigation_begin(&n);show("B",&n);p=parsed("/guide");printf("F %u\n",(unsigned)kaleidoscope_navigation_follow(&n,&p));p=parsed("/guide/text");printf("F %u\n",(unsigned)kaleidoscope_navigation_follow(&n,&p));p=parsed("/third");printf("F %u\n",(unsigned)kaleidoscope_navigation_follow(&n,&p));show("D",&n);
 kaleidoscope_navigation_begin(&n);p=parsed("/guide");printf("F %u\n",(unsigned)kaleidoscope_navigation_follow(&n,&p));kaleidoscope_http_path_initial(&p);printf("F %u\n",(unsigned)kaleidoscope_navigation_follow(&n,&p));show("L",&n);
 kaleidoscope_navigation_begin(&n);p=parsed("/Guide");printf("F %u\n",(unsigned)kaleidoscope_navigation_follow(&n,&p));p=parsed("/guide");printf("F %u\n",(unsigned)kaleidoscope_navigation_follow(&n,&p));kaleidoscope_navigation_complete(&n);show("C",&n);p=parsed("/next");printf("F %u\n",(unsigned)kaleidoscope_navigation_follow(&n,&p));kaleidoscope_navigation_cancel(&n);show("X",&n);return 0;
}
'''


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cesta", type=Path, required=True)
    parser.add_argument("--work", type=Path)
    parser.add_argument("--mutations", action="store_true")
    parser.add_argument("--cesta-source", type=Path)
    parser.add_argument("--lean-root", type=Path)
    args = parser.parse_args()
    work = (args.work or Path(tempfile.mkdtemp(
        prefix="kaleidoscope-navigation-", dir=HERE / "build"
    ))).resolve()
    work.mkdir(parents=True, exist_ok=True)
    codec = subprocess.check_output(["git", "show", PIN + ":crates/sex-http/src/codec.rs"], cwd=REPO)
    nav = subprocess.check_output(["git", "show", PIN + ":apps/kaleidoscope/src/http_navigation.rs"], cwd=REPO)
    assert hashlib.sha256(codec).hexdigest() == CODEC_SHA256
    assert hashlib.sha256(nav).hexdigest() == NAV_SHA256
    (work / "codec.rs").write_bytes(codec); (work / "http_navigation.rs").write_bytes(nav)
    (work / "reference.rs").write_text(RUST_HARNESS); (work / "host.c").write_text(C_HARNESS)
    reference = work / "reference"
    run(["rustc", "--edition=2021", "-C", "opt-level=2", work / "reference.rs", "-o", reference])
    expected = run([reference])
    common = ["-std=c11", "-g0", "-Wall", "-Wextra", "-Wpedantic", "-Werror", "-Igenerated/include"]
    sources = {}; plans = {}; builds = []
    for compiler in ("gcc", "clang"):
        source = work / f"kaleidoscope-navigation-{compiler}.c"
        plan = work / f"kaleidoscope-navigation-{compiler}.plan"
        run([args.cesta, "export-c", "src/kaleidoscope_navigation.cesta", "--cc", compiler,
             "--std=c11", "--plan", plan, "-o", source, "--", "-Igenerated/include"])
        sources[compiler] = source; plans[hashlib.sha256(plan.read_bytes()).hexdigest()] = plan
        for mode, extra in (("O0", ["-O0"]), ("O2", ["-O2"]),
            ("SAN", ["-O1", "-fsanitize=address,undefined", "-fno-sanitize-recover=all", "-fno-pie", "-no-pie"])):
            binary = work / f"{compiler}-{mode}"
            run([compiler, *common, *extra, work / "host.c", source, "-o", binary])
            actual = run([binary], env={**os.environ, "ASAN_OPTIONS": "detect_leaks=1:abort_on_error=1"})
            assert actual == expected, (compiler, mode, actual, expected)
            builds.append((compiler, mode))
    mutations = []
    if args.mutations:
        base = sources["gcc"].read_text()
        changes = [
            ("absolute", "input->bytes[0]!='/'", "false"),
            ("double-slash", "input->bytes[i+1]=='/'", "false"),
            ("dot-segment", "len==1&&input->bytes[segment]=='.'", "false"),
            ("inactive", "!state->active||count==3", "count==3"),
            ("hop-bound", "count==3", "count==4"),
            ("duplicate", "kaleidoscope_http_path_equal(&state->visited[i],path)", "false"),
            ("complete", "void kaleidoscope_navigation_complete(KaleidoscopeNavigation *state){state->active=false;}",
             "void kaleidoscope_navigation_complete(KaleidoscopeNavigation *state){state->active=true;}"),
        ]
        for name, old, new in changes:
            assert base.count(old) >= 1, (name, base.count(old))
            mutant = work / f"{name}.c"; mutant.write_text(base.replace(old, new, 1)); binary = work / name
            built = subprocess.run(["gcc", *common, "-O2", work / "host.c", mutant, "-o", binary], cwd=HERE, capture_output=True)
            if built.returncode == 0:
                result = subprocess.run([binary], cwd=HERE, capture_output=True)
                assert result.returncode != 0 or result.stdout != expected, ("survived", name)
            mutations.append(name)
    admission = []
    if args.lean_root:
        assert args.cesta_source
        sys.path.insert(0, str(args.cesta_source / "tests")); import ownership_admission
        for index, plan in enumerate(plans.values()):
            record, = ownership_admission.parse(plan.read_bytes())
            admission.append(ownership_admission.admit_record(record, work / f"KaleidoscopeNavigationOwnership{index}.lean", args.lean_root))
    (work / "summary.json").write_text(json.dumps({"pin": PIN, "codec_sha256": CODEC_SHA256,
        "navigation_sha256": NAV_SHA256, "builds": builds, "mutations": mutations,
        "admission": admission}, indent=2) + "\n")
    print("kaleidoscope navigation:", len(builds), "builds,", len(mutations), "mutants,",
          sum(item["functions"] for item in admission), "admitted")


if __name__ == "__main__": main()
