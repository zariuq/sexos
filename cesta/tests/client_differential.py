#!/usr/bin/env python3
"""Compare the original Rust browser's client interactions with its Cesta port.

The Rust application's business logic is compiled from the pinned source.
Only its allocator/entry scaffolding and trap provider are replaced. The
observation is ordered logical log lines, non-print PDX calls, listens and
yields. Raw serial fragmentation, pointer addresses and binary code layout
are outside this observation.
"""

import argparse
import difflib
import hashlib
import json
from pathlib import Path
import random
import shutil
import subprocess
import tempfile
from layout_text import LayoutText

HERE = Path(__file__).resolve().parent.parent
REPO = HERE.parent
REVISION = "4a040a86"

C_PROVIDER = r'''
#include "sexos_client.h"
#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include "cases.h"
static const Case *scenario;
static uint64_t operation, chunk, empty;
static unsigned int runs;

SexosReply sexos_pdx_call(uint64_t slot, uint64_t opcode, uint64_t a0, uint64_t a1, uint64_t a2)
{
    if (slot == 0 && opcode == 69) {
        fputs("L ", stdout);
        fwrite((const void *)(uintptr_t)a0, 1, (size_t)a1, stdout);
        return (SexosReply){0, 0};
    }
    SexosReply result = {0, 0};
    if (slot == 18 && opcode == 512) {
        result.status = scenario->route_status;
        if (a0 >= 519 && a0 <= 522) { operation = a0; chunk = a1; empty = scenario->empty; }
    } else if (slot == 18 && opcode == 523) result.value = scenario->dns;
    printf("C %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64
           " -> %" PRIu64 " %" PRIu64 "\n", slot, opcode, a0, a1, a2, result.status, result.value);
    return result;
}

SexosMessage sexos_listen_once(uint64_t slot)
{
    SexosMessage result = {0, 0, 0, 0, 0, 0};
    if (slot == 1) {
        result.type_id = runs++ < scenario->runs ? 1 : 65535;
        result.caller_pd = 43;
    } else if (empty) --empty;
    else {
        result.caller_pd = 42;
        result.arg1 = 48879;
        if (operation == 519) { result.type_id = scenario->live_type; result.arg0 = scenario->live_length; }
        else if (operation == 521) { result.type_id = scenario->body_type; result.arg0 = scenario->body_length; }
        else {
            assert(operation == 520 || operation == 522);
            const uint8_t *payload = operation == 520 ? scenario->live : scenario->body;
            uint64_t failure = operation == 520 ? scenario->live_fail : scenario->body_fail;
            result.type_id = chunk == failure || chunk >= 8 ? 2 : 1;
            if (chunk < 8)
                for (unsigned int i = 0; i < 8; ++i)
                    result.arg0 |= (uint64_t)payload[chunk * 8 + i] << (i * 8);
        }
    }
    printf("T %" PRIu64 " %" PRIu64 " %u %" PRIu64 " %" PRIu64 " %" PRIu64 "\n",
           slot, result.type_id, result.caller_pd, result.arg0, result.arg1, result.arg2);
    return result;
}
void sexos_yield(void) { puts("Y"); }
int main(int argc, char **argv)
{
    assert(argc == 2);
    size_t index = (size_t)strtoull(argv[1], NULL, 10);
    assert(index < sizeof cases / sizeof *cases);
    scenario = &cases[index];
    kaleidoscope_main();
    return 0;
}
'''

RUST_PROVIDER = r'''
use std::cell::RefCell;
#[derive(Clone)]
struct State { case: usize, operation: u64, chunk: u64, empty: u64, runs: u64, budget: u64 }
thread_local! { static STATE: RefCell<State> = const { RefCell::new(State {
    case: 0, operation: 0, chunk: 0, empty: 0, runs: 0, budget: 8 }) }; }
#[macro_export]
macro_rules! serial_println { ($($argument:tt)*) => { println!("L {}", format!($($argument)*)) }; }
mod silkclient {
    pub trait SexApp { fn new(pdx: u32) -> Self; fn run(&mut self, pdx: u32) -> bool; }
}
mod sex_pdx {
    pub use crate::serial_println;
    pub const SLOT_NET: u64 = 18;
    pub const SLOT_DISPLAY: u64 = 5;
    pub struct PdxMessage { pub type_id: u64, pub arg0: u64, pub arg1: u64, pub arg2: u64, pub caller_pd: u32 }
    pub fn pdx_call(slot: u64, opcode: u64, a0: u64, a1: u64, a2: u64) -> (u64, u64) {
        crate::STATE.with(|cell| {
            let mut state = cell.borrow_mut();
            let case = &crate::CASES[state.case];
            let (mut status, mut value) = (0, 0);
            if slot == 18 && opcode == 512 {
                status = case.route_status;
                if (519..=522).contains(&a0) { state.operation = a0; state.chunk = a1; state.empty = case.empty; }
            } else if slot == 18 && opcode == 523 { value = case.dns; }
            println!("C {slot} {opcode} {a0} {a1} {a2} -> {status} {value}");
            (status, value)
        })
    }
    fn listen_once(slot: u64) -> PdxMessage {
        crate::STATE.with(|cell| {
            let mut state = cell.borrow_mut();
            let case = &crate::CASES[state.case];
            let mut result = PdxMessage { type_id: 0, arg0: 0, arg1: 0, arg2: 0, caller_pd: 0 };
            if slot == 1 {
                result.type_id = if state.runs < case.runs { 1 } else { 65535 };
                state.runs += 1; result.caller_pd = 43;
            } else if state.empty != 0 { state.empty -= 1; }
            else {
                result.caller_pd = 42; result.arg1 = 48879;
                if state.operation == 519 { result.type_id = case.live_type; result.arg0 = case.live_length; }
                else if state.operation == 521 { result.type_id = case.body_type; result.arg0 = case.body_length; }
                else {
                    assert!(state.operation == 520 || state.operation == 522);
                    let payload = if state.operation == 520 { &case.live } else { &case.body };
                    let failure = if state.operation == 520 { case.live_fail } else { case.body_fail };
                    result.type_id = if state.chunk == failure || state.chunk >= 8 { 2 } else { 1 };
                    if state.chunk < 8 {
                        for i in 0..8 { result.arg0 |= (payload[(state.chunk * 8 + i) as usize] as u64) << (i * 8); }
                    }
                }
            }
            println!("T {slot} {} {} {} {} {}", result.type_id, result.caller_pd, result.arg0, result.arg1, result.arg2);
            result
        })
    }
    pub fn pdx_listen_raw(slot: u64) -> PdxMessage {
        loop {
            let result = listen_once(slot);
            if result.type_id == 0 { println!("Y"); continue; }
            let print = crate::STATE.with(|cell| { let mut s = cell.borrow_mut();
                if s.budget > 0 { s.budget -= 1; true } else { false } });
            if print {
                serial_println!("[pdx.listen.raw.wrapper] type={:#x} caller={} a0={:#x} a1={:#x}",
                    result.type_id, result.caller_pd, result.arg0, result.arg1);
            }
            return result;
        }
    }
}
'''

FIELDS = ("route_status", "dns", "live_type", "live_length", "body_type", "body_length",
          "live_fail", "body_fail", "empty", "runs")


def scenarios(count):
    randomizer = random.Random(0xCE57A)
    result = []
    lengths = [0, 1, 7, 8, 9, 15, 63, 64, 65, 255, (1 << 64) - 1]
    for index in range(max(count, len(lengths))):
        case = dict(zip(FIELDS, [randomizer.randrange(4), randomizer.choice([0, 0x0A000202, (1 << 64) - 1]),
                    1 if index < len(lengths) else randomizer.choice([1, 1, 2]), lengths[index % len(lengths)],
                    randomizer.choice([1, 1, 2]), lengths[(index * 3) % len(lengths)],
                    randomizer.randrange(10), randomizer.randrange(10), randomizer.randrange(3), randomizer.randrange(4)]))
        case["live"] = [randomizer.randrange(256) for _ in range(64)]
        case["body"] = [randomizer.randrange(256) for _ in range(64)]
        result.append(case)
    return result


def rust_reference(source):
    beginning = source.index("// --- Bump allocator ---")
    ending = source.index("const KALEIDO_SURFACE_ID")
    # Keep every application method and its data unchanged. Scaffolding alone
    # is replaced by a hosted entry and a declared trap/serial provider.
    source = source[:beginning] + source[ending:]
    source = source.replace("#![no_std]", "").replace("#![no_main]", "")
    source = source.replace("use silkclient::{app_main, SexApp};", "use silkclient::SexApp;")
    assert source.count("app_main!(App);") == 1
    source = source.replace("app_main!(App);", r'''
fn main() {
    let index: usize = std::env::args().nth(1).unwrap().parse().unwrap();
    assert!(index < CASES.len());
    STATE.with(|cell| cell.borrow_mut().case = index);
    let mut application = App::new(0);
    while application.run(0) {}
    let _ = sex_pdx::pdx_call(0, 255, 0, 0, 0);
}
''')
    return source


def execute(command, cwd=HERE):
    result = subprocess.run(list(map(str, command)), cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=60)
    if result.returncode:
        raise RuntimeError((command, result.returncode, result.stderr.decode(errors="replace")[-6000:]))
    return result.stdout


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cesta", type=Path, required=True)
    parser.add_argument("--cases", type=int, default=64)
    parser.add_argument("--sanitizers", action="store_true")
    parser.add_argument("--mutations", action="store_true")
    args = parser.parse_args()
    binary = Path(shutil.which(str(args.cesta)) or args.cesta).resolve()
    inputs = [binary, HERE / "src/kaleidoscope.cesta", HERE / "src/client_interface.cesta",
              HERE / "runtime/client_runtime.c", *sorted((HERE / "generated").rglob("*.c")),
              *sorted((HERE / "generated/include").glob("*.h"))]
    before = {str(path.relative_to(HERE)) if path.is_relative_to(HERE) else "cesta-compiler":
              hashlib.sha256(path.read_bytes()).hexdigest() for path in inputs}
    work = Path(tempfile.mkdtemp(prefix="client-differential-", dir=HERE / "build"))
    cases = scenarios(args.cases)
    (work / "scenarios.json").write_text(json.dumps(cases, indent=2) + "\n")
    c_fields = " ".join("uint64_t " + name + ";" for name in FIELDS)
    rust_fields = " ".join(name + ": u64," for name in FIELDS)
    c_cases, rust_cases = [], []
    for case in cases:
        c_cases.append("{" + ", ".join([*("UINT64_C(" + str(case[name]) + ")" for name in FIELDS),
            *("{" + ",".join(map(str, case[name])) + "}" for name in ("live", "body"))]) + "}")
        rust_cases.append("Case {" + ", ".join([*(name + ": " + str(case[name]) for name in FIELDS),
            *(name + ": [" + ",".join(map(str, case[name])) + "]" for name in ("live", "body"))]) + "}")
    (work / "cases.h").write_text("typedef struct {" + c_fields + " uint8_t live[64], body[64]; } Case;\n"
        + "static const Case cases[] = {" + ",\n".join(c_cases) + "};\n")
    rust_cases_text = ("struct Case {" + rust_fields + " live: [u8;64], body: [u8;64] }\n"
        + "static CASES: &[Case] = &[" + ",\n".join(rust_cases) + "];\n")
    original = execute(["git", "show", REVISION + ":apps/kaleidoscope/src/main.rs"], REPO)
    rust = work / "reference.rs"
    rust.write_text(RUST_PROVIDER + rust_cases_text + rust_reference(original.decode()))
    (work / "host.c").write_text(C_PROVIDER)
    reference = work / "rust-reference"
    execute(["rustc", "--edition=2021", "-C", "opt-level=2", rust, "-o", reference])
    expected = [execute([reference, index]) for index in range(len(cases))]
    for index, trace in enumerate(expected):
        (work / ("reference-" + str(index) + ".trace")).write_bytes(trace)
    exported = work / "kaleidoscope.c"
    exported.write_bytes(execute([binary, "export-c", "src/kaleidoscope.cesta", "--"]))
    libraries = [HERE / "runtime/client_runtime.c", HERE / "generated/runtime/packed_words.c",
                 HERE / "generated/runtime/format_bytes.c", work / "host.c"]
    variants = [(compiler, route, False) for compiler in ("gcc", "clang") for route in ("authored", "exported")]
    if args.sanitizers:
        variants.append(("clang", "exported", True))
    for compiler, route, sanitizer in variants:
        flags = ["-std=c11", "-O1" if sanitizer else "-O2", "-DSEXOS_HOST", "-fno-builtin",
                 "-Wall", "-Wextra", "-Wpedantic", "-Werror", "-I", HERE / "generated/include", "-I", work]
        if sanitizer:
            flags += ["-g", "-fsanitize=address,undefined", "-fno-sanitize-recover=all"]
        executable = work / (compiler + "-" + route + ("-sanitized" if sanitizer else ""))
        command = [binary, "cc", "--cc", compiler] if route == "authored" else [compiler]
        source = HERE / "src/kaleidoscope.cesta" if route == "authored" else exported
        execute([*command, *flags, source, *libraries, "-o", executable])
        for index, wanted in enumerate(expected):
            actual = execute([executable, index])
            if actual != wanted:
                (work / (compiler + "-" + str(index) + ".trace")).write_bytes(actual)
                difference = "\n".join(difflib.unified_diff(wanted.decode().splitlines(), actual.decode().splitlines()))
                raise AssertionError((compiler, route, sanitizer, index, work, difference[:10000]))
        print(compiler, route, "sanitized" if sanitizer else "native", ":", len(cases), "Rust client traces equal", flush=True)
    refused = []
    if args.mutations:
        original_client = LayoutText((HERE / "src/kaleidoscope.cesta").read_text())
        changes = [("color", "0x00CDD6F4", "0x00112233"),
                   ("body-routing", "(if (> body_length 0)\n              (block\n                (call sexos_line \"[browser.body.render.begin]\")",
                    "(if (== body_length 0)\n              (block\n                (call sexos_line \"[browser.body.render.begin]\")"),
                   ("reply-status", "(!= (. reply type_id) 1)", "(== (. reply type_id) 1)")]
        for name, before_text, after_text in changes:
            assert before_text in original_client, name + ": mutation point disappeared"
            source = work / ("mutated-" + name + ".cesta")
            source.write_text(original_client.replace(before_text, after_text, 1))
            executable = work / ("mutated-" + name)
            execute([binary, "cc", "--cc", "gcc", "-std=c11", "-O2", "-DSEXOS_HOST", "-fno-builtin",
                     "-I", HERE / "generated/include", "-I", work, source, *libraries, "-o", executable])
            for index, wanted in enumerate(expected):
                actual = execute([executable, index])
                if actual != wanted:
                    (work / ("mutated-" + name + ".trace")).write_bytes(actual)
                    refused.append({"mutation": name, "scenario": index})
                    break
            else:
                raise AssertionError("the differential gate missed " + name)
        print("Rust client gate refuses all three deliberately incorrect ports", flush=True)
    after = {str(path.relative_to(HERE)) if path.is_relative_to(HERE) else "cesta-compiler":
             hashlib.sha256(path.read_bytes()).hexdigest() for path in inputs}
    assert before == after, "client inputs changed during verification"
    summary = {"cases": len(cases), "variants": len(variants), "revision": REVISION, "inputs": before,
        "refused_mutations": refused,
        "compilers": {name: execute([name, "--version"]).decode().splitlines()[0]
                      for name in ("gcc", "clang", "rustc")},
        "rust_application_sha256": hashlib.sha256(original).hexdigest(), "binary_sha256": hashlib.sha256(binary.read_bytes()).hexdigest(),
        "work": str(work), "observation": "ordered logical lines, non-print PDX calls/replies, listens and yields"}
    (work / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")
    (HERE / "build/client-differential-summary.json").write_text(json.dumps(summary, indent=2) + "\n")


if __name__ == "__main__":
    main()
