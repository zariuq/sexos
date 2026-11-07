#!/usr/bin/env python3
"""Compare checked bitmap custody with the pinned Rust outbox."""

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
    def init(floor=0): return (0, 15, 77, 300, 0, floor)
    def fill(n, seed=1): return (1, n, seed)
    def queue(x, y, w, h, scale, viewport, source): return (2, x, y, w, h, scale, viewport, source)
    def clear(viewport, source=0): return (3, viewport, source)
    def poll(kind=0, error=0): return (4, kind, error)
    def receive(caller, word): return (5, caller, word)
    def applied(surface, sequence): return 0x57 | surface << 8 | sequence << 24
    return [
        [init(), fill(2, 3), queue(8, 80, 2, 1, 1, 10, 7), poll(), poll(), poll(),
         poll(), receive(2, applied(300, 1)), receive(1, applied(301, 1)),
         receive(1, applied(300, 1)), (8,)],
        [init(), fill(2, 10), queue(1, 2, 2, 1, 1, 1, 1), poll(1), poll(),
         poll(2, 99), (8,)],
        [init(), fill(1, 1), queue(1, 1, 1, 1, 1, 1, 1),
         queue(2, 2, 1, 1, 1, 2, 2), queue(3, 3, 1, 1, 1, 3, 3),
         poll(), poll(), poll(), receive(1, applied(300, 1)), poll(), poll(), poll(),
         receive(1, applied(300, 3)), (8,)],
        [init(), fill(2, 4), queue(1, 2, 2, 1, 1, 1, 1), clear(9), poll(), poll(),
         receive(1, applied(300, 2)), (8,)],
        [init(), fill(1, 7), queue(1, 2, 1, 1, 1, 3, 4), poll(), poll(), poll(),
         clear(8), clear(8), receive(1, applied(300, 1)), poll(), receive(1, applied(300, 2)), (8,)],
        [init(2**32 - 1), fill(1), queue(1, 1, 1, 1, 1, 1, 1), clear(1), (8,)],
        [init(), fill(1), queue(1, 1, 0, 1, 1, 1, 1), queue(1, 1, 2, 1, 1, 1, 1),
         queue(1, 1, 1, 1, 1, 1, 1), queue(1, 1, 1, 1, 1, 1, 1), (8,)],
        [init(), fill(2, 11), queue(1, 1, 2, 1, 1, 1, 1), poll(), (6, 0, 1),
         poll(), (8,)],
        [init(), fill(1), queue(1, 1, 1, 1, 1, 1, 1), (7,), poll(), clear(1), (8,)],
    ]


def padded(row):
    return tuple(row) + (0,) * (8 - len(row))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cesta", type=Path, required=True)
    parser.add_argument("--work", type=Path)
    parser.add_argument("--mutations", action="store_true")
    parser.add_argument("--cesta-source", type=Path)
    parser.add_argument("--lean-root", type=Path)
    args = parser.parse_args()
    work = (args.work or Path(tempfile.mkdtemp(prefix="kaleidoscope-bitmap-", dir=HERE / "build"))).resolve()
    work.mkdir(parents=True, exist_ok=True)
    delivery = subprocess.check_output(["git", "show", f"{PIN}:apps/kaleidoscope/src/bitmap_delivery.rs"], cwd=REPO)
    batch = subprocess.check_output(["git", "show", f"{PIN}:crates/sex-pdx/src/bitmap_batch.rs"], cwd=REPO)
    rows = scenarios()
    commands = []
    spans = []
    for scenario in rows:
        start = len(commands); commands.extend(map(padded, scenario)); spans.append((start, len(scenario)))
    rust_commands = ",\n".join("Command{op:%d,a:%d,b:%d,c:%d,d:%d,e:%d,f:%d,g:%d}" % row
                               for row in commands)
    rust_spans = ",".join(f"({a},{b})" for a, b in spans)
    text = "extern crate self as sex_pdx;\nmod bitmap_batch {\n" + batch.decode().replace("//!", "//") + "\n}\n"
    text += r'''mod incarnation {
#[derive(Clone,Copy)]pub struct Identity{target:u32,task:u32,surface:u64,ordinal:u8}
impl Identity{pub const fn new(target:u32,task:u32,surface:u64,ordinal:u8)->Self{Self{target,task,surface,ordinal}}pub const fn target(self)->u32{self.target}pub const fn task(self)->u32{self.task}pub const fn surface(self)->u64{self.surface}pub const fn ordinal(self)->u8{self.ordinal}}
}
mod delivery {
''' + delivery.decode().replace("//!", "//") + "\n}\n"
    text += f"#[derive(Clone,Copy)]struct Command{{op:u64,a:u64,b:u64,c:u64,d:u64,e:u64,f:u64,g:u64}}\nstatic COMMANDS:&[Command]=&[{rust_commands}];\nstatic SCENARIOS:&[(usize,usize)]=&[{rust_spans}];\n"
    text += r'''
fn show_receipt(r:&delivery::Receipt){let g=r.geometry.unwrap_or(bitmap_batch::Geometry{x:0,y:0,width:0,height:0,scale:0});print!(" {} {} {},{},{},{},{} {:016x} {},{}",r.sequence,r.geometry.is_some()as u8,g.x,g.y,g.width,g.height,g.scale,r.hash,r.metadata.viewport,r.metadata.source)}
fn main(){let n:usize=std::env::args().nth(1).unwrap().parse().unwrap();let(start,count)=SCENARIOS[n];let mut out=delivery::Outbox::new(incarnation::Identity::new(15,77,300,0));let mut pixels=Vec::<u32>::new();for c in &COMMANDS[start..start+count]{match c.op{
0=>{out=delivery::Outbox::with_sequence_floor(incarnation::Identity::new(c.a as u32,c.b as u32,c.c,c.d as u8),c.e as u32);println!("I")},
1=>{pixels=(0..c.a as u32).map(|i|c.b as u32+i.wrapping_mul(0x01020304)).collect();println!("F {}",pixels.len())},
2=>{let r=out.queue(&pixels,bitmap_batch::Geometry{x:c.a as u16,y:c.b as u16,width:c.c as u8,height:c.d as u8,scale:c.e as u8},delivery::Metadata{viewport:c.f,source:c.g});match r{Ok(s)=>println!("Q 0 {s}"),Err(e)=>println!("Q {} 0",match e{delivery::QueueError::Closed=>1,delivery::QueueError::InvalidGeometry=>2,delivery::QueueError::InvalidPixels=>3,delivery::QueueError::ClearPending=>4,delivery::QueueError::SequenceExhausted=>5})}},
3=>{let r=out.clear(delivery::Metadata{viewport:c.a,source:c.b});match r{Ok(s)=>println!("C 0 {s}"),Err(e)=>println!("C {} 0",match e{delivery::QueueError::Closed=>1,delivery::QueueError::InvalidGeometry=>2,delivery::QueueError::InvalidPixels=>3,delivery::QueueError::ClearPending=>4,delivery::QueueError::SequenceExhausted=>5})}},
4=>{let mut op=None;let admission=match c.a{0=>delivery::Admission::Accepted,1=>delivery::Admission::Full,_=>delivery::Admission::Rejected(c.b)};let p=out.poll(&pixels,1,|x|{op=Some(x);admission});let(state,error)=match p{delivery::Poll::Idle=>(0,0),delivery::Poll::Pending=>(1,0),delivery::Poll::Deferred=>(2,0),delivery::Poll::Rejected(e)=>(3,e),delivery::Poll::InvalidPixels=>(4,0),delivery::Poll::Closed=>(5,0)};if let Some(o)=op{println!("P {state} {error} O {},{},{},{}",o.opcode,o.surface,o.packed,o.control)}else{println!("P {state} {error} -")}},
5=>{let r=out.receive(c.a as u32,c.b);print!("R {}",r.is_some()as u8);if let Some(r)=r{show_receipt(&r)}println!()},
6=>{if(c.a as usize)<pixels.len(){pixels[c.a as usize]^=c.b as u32}println!("M")},
7=>{out.close();println!("X")},8=>println!("S {} {}",out.pending()as u8,out.sequence()),_=>unreachable!()}}}
'''
    (work / "reference.rs").write_text(text)
    reference = work / "reference"
    run(["rustc", "--edition=2021", "-C", "opt-level=2", work / "reference.rs", "-o", reference])
    expected = [run([reference, i]) for i in range(len(rows))]
    c_commands = ",\n".join("{" + ",".join(map(str, row)) + "}" for row in commands)
    c_spans = ",".join("{" + f"commands+{a},{b}" + "}" for a, b in spans)
    (work / "cases.h").write_text(
        "typedef struct Command {uint64_t op,a,b,c,d,e,f,g;} Command;\n"
        f"static const Command commands[]={{\n{c_commands}\n}};\n"
        "typedef struct Scenario {const Command *commands;size_t count;} Scenario;\n"
        f"static const Scenario scenarios[]={{ {c_spans} }};\n")
    host = HERE / "tests/kaleidoscope_bitmap/host.c"
    exports, plans, builds = {}, {}, []
    for compiler in ("gcc", "clang"):
        source = work / f"bitmap-{compiler}.c"
        plan = work / f"bitmap-{compiler}.plan"
        run([args.cesta, "export-c", "src/kaleidoscope_bitmap.cesta", "--cc", compiler,
             "--std=c11", "-o", source, "--plan", plan, "--", "-Igenerated/include"])
        exports[compiler] = source; plans[hashlib.sha256(plan.read_bytes()).hexdigest()] = plan
        common = ["-std=c11", "-g0", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
                  "-Igenerated/include", "-I", work]
        for mode, flags in (("O0", ["-O0"]), ("O2", ["-O2"]),
                            ("SAN", ["-O1", "-fsanitize=address,undefined",
                                     "-fno-sanitize-recover=all", "-fno-pie", "-no-pie"])):
            binary = work / f"{compiler}-{mode}"
            run([compiler, *common, *flags, host, source, "-o", binary])
            for i, wanted in enumerate(expected):
                actual = run([binary, i], env={**os.environ, "ASAN_OPTIONS": "detect_leaks=1:abort_on_error=1"})
                assert actual == wanted, (compiler, mode, i, actual, wanted)
            builds.append((compiler, mode))
    mutations = []
    if args.mutations:
        base = LayoutText(exports["gcc"].read_text())
        changes = [
            ("geometry", "geometry.width>=1&&geometry.width<=48", "geometry.width<=48"),
            ("sequence", "outbox->sequence==UINT32_MAX", "false"),
            ("hash-byte", "pixels->values[i]>>(8*k)", "pixels->values[i]>>(7*k)"),
            ("full", "admission.kind==KALEIDOSCOPE_BITMAP_ADMIT_FULL", "false"),
            ("caller", "caller!=1", "false"),
            ("clear-barrier", "known_present&&!known.geometry_present", "false&&!known.geometry_present"),
        ]
        common = ["-std=c11", "-O2", "-g0", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
                  "-Igenerated/include", "-I", work]
        for name, old, new in changes:
            assert base.count(old) >= 1, (name, base.count(old))
            mutant = work / f"mutant-{name}.c"; mutant.write_text(base.replace(old, new, 1))
            binary = work / f"mutant-{name}"
            built = subprocess.run(["gcc", *common, host, mutant, "-o", binary],
                                   cwd=HERE, capture_output=True)
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
                record, work / f"KaleidoscopeBitmapOwnership{index}.lean", args.lean_root))
    summary = {"pin": PIN, "cases": len(rows), "builds": builds, "mutations": mutations,
               "admission": admission}
    (work / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")
    print("kaleidoscope bitmap:", len(rows), "scenarios,", len(builds), "builds,",
          len(mutations), "mutants,", sum(x["functions"] for x in admission), "admitted")


if __name__ == "__main__":
    main()
