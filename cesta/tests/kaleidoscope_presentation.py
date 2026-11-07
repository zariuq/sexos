#!/usr/bin/env python3
"""Compare checked current-presentation correlation with pinned Rust."""

import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
from layout_text import LayoutText

HERE = Path(__file__).resolve().parent.parent
REPO = HERE.parent
PIN = "b6be1b451a6c21985d00d192674feedf66bb11dd"


def run(argv, *, env=None):
    result = subprocess.run(list(map(str, argv)), cwd=HERE, capture_output=True, env=env)
    if result.returncode:
        raise RuntimeError((argv, result.returncode, result.stdout[-2000:], result.stderr[-5000:]))
    return result.stdout


def scenarios():
    g = (8, 80, 2, 1, 1)
    receipt = lambda seq, present, geom, digest, viewport, source, current_viewport, current_ticket, current_hash: (
        3, seq, present, *geom, digest, viewport, source, current_viewport, current_ticket, current_hash)
    clear_receipt = lambda seq, present, digest, viewport, source, current: (
        5, seq, present, digest, viewport, source, current)
    return [
        [(0,), (1, 1, 1, 1), (1, 1, 1, 1), (2, *g, 1, 7, 1, 99),
         receipt(1, 1, g, 99, 1, 7, 1, 7, 99), receipt(1, 1, g, 99, 1, 7, 1, 7, 99)],
        [(0,), (1, 1, 1, 2), (1, 2, 1, 1), (1, 1, 0, 0), (1, 1, 1, 1),
         (2, *g, 2, 7, 1, 9), (2, *g, 1, 0, 1, 9), (2, *g, 1, 7, 1, 9)],
        [(0,), (1, 1, 1, 1), (2, *g, 1, 7, 1, 55),
         receipt(2, 1, g, 55, 1, 7, 1, 7, 55),
         receipt(1, 1, (9, 80, 2, 1, 1), 55, 1, 7, 1, 7, 55),
         receipt(1, 1, g, 55, 1, 8, 1, 7, 55),
         receipt(1, 1, g, 55, 1, 7, 1, 7, 55)],
        [(0,), (1, 1, 1, 1), (4, 1, 1), (4, 1, 1),
         clear_receipt(1, 1, 0, 1, 0, 1), clear_receipt(1, 0, 1, 1, 0, 1),
         clear_receipt(1, 0, 0, 1, 0, 1), clear_receipt(1, 0, 0, 1, 0, 1)],
        [(0,), (1, 1, 1, 1), (2, *g, 1, 7, 2**32 - 1, 0), (6,),
         (1, 2, 1, 1), (2, *g, 1, 7, 2**32 - 1, 0), (4, 1, 2**32 - 1)],
        [(0,), (7, 0), (7, 1), (7, 1), (7, 0)],
        [(0,), (8, 2**64 - 1), (1, 2**64 - 1, 1, 1), (2, *g, 1, 7, 1, 0),
         (7, 1), (6,), (1, 2**64 - 1, 1, 1)],
    ]


def padded(row):
    return tuple(row) + (0,) * (14 - len(row))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cesta", type=Path, required=True)
    parser.add_argument("--work", type=Path)
    parser.add_argument("--mutations", action="store_true")
    parser.add_argument("--cesta-source", type=Path)
    parser.add_argument("--lean-root", type=Path)
    args = parser.parse_args()
    work = (args.work or Path(tempfile.mkdtemp(prefix="kaleidoscope-presentation-", dir=HERE / "build"))).resolve()
    work.mkdir(parents=True, exist_ok=True)
    source = subprocess.check_output(["git", "show", f"{PIN}:apps/kaleidoscope/src/asset_presentation.rs"], cwd=REPO)
    rows = scenarios(); commands = []; spans = []
    for scenario in rows:
        start = len(commands); commands.extend(map(padded, scenario)); spans.append((start, len(scenario)))
    rust_commands = ",\n".join("Command{v:[%s]}" % ",".join(map(str, row)) for row in commands)
    text = r'''extern crate self as sex_pdx;
mod asset_view {#[derive(Clone,Copy,Debug,PartialEq,Eq)]pub enum Mode{Fit,Actual}}
mod bitmap_delivery {
 use crate::bitmap_batch::Geometry;
 #[derive(Clone,Copy,Debug,PartialEq,Eq)]pub struct Metadata{pub viewport:u64,pub source:u64}
 #[derive(Clone,Copy,Debug,PartialEq,Eq)]pub struct Receipt{pub sequence:u32,pub geometry:Option<Geometry>,pub hash:u64,pub metadata:Metadata}
}
pub mod bitmap_batch {#[derive(Clone,Copy,Debug,PartialEq,Eq)]pub struct Geometry{pub x:u16,pub y:u16,pub width:u8,pub height:u8,pub scale:u8}impl Geometry{pub fn valid(self)->bool{(1..=48).contains(&self.width)&&(1..=48).contains(&self.height)&&(1..=4).contains(&self.scale)}}}
mod presentation {
''' + source.decode().replace("//!", "//") + r'''
 pub fn generation_set(p:&mut Presentation,v:u64){p.generation=v}
 pub fn snapshot(p:&Presentation)->String{format!("{} {} {} {} {} {} {} {} {} {}",p.mode as u8,p.generation,p.text.is_some()as u8,p.layout.is_some()as u8,p.applied as u8,p.clear.is_some()as u8,p.clear_applied as u8,p.sequence_floor,p.exhausted as u8,p.can_queue(1)as u8)}
}
'''
    text += f"#[derive(Clone,Copy)]struct Command{{v:[u64;14]}}\nstatic COMMANDS:&[Command]=&[{rust_commands}];\n"
    text += "static SCENARIOS:&[(usize,usize)]=&[" + ",".join(
        f"({a},{b})" for a, b in spans) + "];\n"
    text += r'''
fn geometry(v:&[u64;14],at:usize)->sex_pdx::bitmap_batch::Geometry{sex_pdx::bitmap_batch::Geometry{x:v[at]as u16,y:v[at+1]as u16,width:v[at+2]as u8,height:v[at+3]as u8,scale:v[at+4]as u8}}
fn main(){let n:usize=std::env::args().nth(1).unwrap().parse().unwrap();let(s,c)=SCENARIOS[n];let mut p=presentation::Presentation::new();for command in &COMMANDS[s..s+c]{let v=&command.v;print!("A {}",v[0]);match v[0]{
0=>p=presentation::Presentation::new(),
1=>print!(" {}",p.text_applied(v[1],v[2],v[3])as u8),
2=>print!(" {}",p.expect(geometry(v,1),v[6],v[7]as u32,v[8]as u32,v[9])as u8),
3=>{let r=bitmap_delivery::Receipt{sequence:v[1]as u32,geometry:if v[2]!=0{Some(geometry(v,3))}else{None},hash:v[8],metadata:bitmap_delivery::Metadata{viewport:v[9],source:v[10]}};print!(" {}",p.applied(&r,v[11],v[12]as u32,v[13])as u8)},
4=>print!(" {}",p.expect_clear(v[1],v[2]as u32)as u8),
5=>{let r=bitmap_delivery::Receipt{sequence:v[1]as u32,geometry:if v[2]!=0{Some(geometry(v,3))}else{None},hash:v[3],metadata:bitmap_delivery::Metadata{viewport:v[4],source:v[5]}};print!(" {}",p.cleared(&r,v[6])as u8)},
6=>print!(" {}",p.invalidate().is_ok()as u8),
7=>{let r=p.select(if v[1]==0{asset_view::Mode::Fit}else{asset_view::Mode::Actual});match r{Ok(changed)=>print!(" 1 {}",changed as u8),Err(_)=>print!(" 0 0")}},
8=>presentation::generation_set(&mut p,v[1]),_=>unreachable!()}println!(" | {}",presentation::snapshot(&p))}}
'''
    (work / "reference.rs").write_text(text)
    reference = work / "reference"; run(["rustc", "--edition=2021", "-C", "opt-level=2", work / "reference.rs", "-o", reference])
    expected = [run([reference, i]) for i in range(len(rows))]
    c_commands = ",\n".join("{" + ",".join(f"UINT64_C({value})" for value in row) + "}" for row in commands)
    c_spans = ",".join("{" + f"commands+{a},{b}" + "}" for a, b in spans)
    (work / "cases.h").write_text(
        "typedef struct Command {uint64_t op,a,b,c,d,e,f,g,h,i,j,k,l,m;} Command;\n"
        f"static const Command commands[]={{\n{c_commands}\n}};\n"
        "typedef struct Scenario {const Command *commands;size_t count;} Scenario;\n"
        f"static const Scenario scenarios[]={{ {c_spans} }};\n")
    host = HERE / "tests/kaleidoscope_presentation/host.c"
    exports, plans, builds = {}, {}, []
    for compiler in ("gcc", "clang"):
        c_source = work / f"presentation-{compiler}.c"; plan = work / f"presentation-{compiler}.plan"
        run([args.cesta, "export-c", "src/kaleidoscope_presentation.cesta", "--cc", compiler,
             "--std=c11", "-o", c_source, "--plan", plan, "--", "-Igenerated/include"])
        exports[compiler] = c_source; plans[hashlib.sha256(plan.read_bytes()).hexdigest()] = plan
        common = ["-std=c11", "-g0", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
                  "-Igenerated/include", "-I", work]
        for mode, flags in (("O0", ["-O0"]), ("O2", ["-O2"]),
                            ("SAN", ["-O1", "-fsanitize=address,undefined",
                                     "-fno-sanitize-recover=all", "-fno-pie", "-no-pie"])):
            binary = work / f"{compiler}-{mode}"; run([compiler, *common, *flags, host, c_source, "-o", binary])
            for i, wanted in enumerate(expected):
                actual = run([binary, i], env={**os.environ, "ASAN_OPTIONS": "detect_leaks=1:abort_on_error=1"})
                assert actual == wanted, (compiler, mode, i, actual, wanted)
            builds.append((compiler, mode))
    mutations = []
    if args.mutations:
        base = LayoutText(exports["gcc"].read_text())
        changes = [
            ("generation", "presentation->generation++", "presentation->generation+=2"),
            ("text-viewport", "viewport!=current_viewport", "false"),
            ("sequence-floor", "sequence<=presentation->sequence_floor", "false"),
            ("geometry", "!presentation_geometry_equal", "false&&!presentation_geometry_equal"),
            ("metadata-source", "receipt->metadata.source", "layout.ticket"),
            ("single-apply", "presentation->applied||", "false||"),
        ]
        common = ["-std=c11", "-O2", "-g0", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
                  "-Igenerated/include", "-I", work]
        for name, old, new in changes:
            assert base.count(old) >= 1, (name, base.count(old))
            mutant = work / f"mutant-{name}.c"; mutant.write_text(base.replace(old, new, 1))
            binary = work / f"mutant-{name}"
            built = subprocess.run(["gcc", *common, host, mutant, "-o", binary], cwd=HERE, capture_output=True)
            if built.returncode == 0:
                assert any(subprocess.run([binary, str(i)], cwd=HERE, capture_output=True).stdout != wanted
                           for i, wanted in enumerate(expected)), ("survived", name)
            mutations.append(name)
    admission = []
    if args.lean_root:
        assert args.cesta_source
        sys.path.insert(0, str(args.cesta_source / "tests"))
        import ownership_admission
        for index, plan in enumerate(plans.values()):
            record, = ownership_admission.parse(plan.read_bytes())
            admission.append(ownership_admission.admit_record(
                record, work / f"KaleidoscopePresentationOwnership{index}.lean", args.lean_root))
    summary = {"pin": PIN, "cases": len(rows), "builds": builds, "mutations": mutations,
               "admission": admission}
    (work / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")
    print("kaleidoscope presentation:", len(rows), "scenarios,", len(builds), "builds,",
          len(mutations), "mutants,", sum(x["functions"] for x in admission), "admitted")


if __name__ == "__main__":
    main()
