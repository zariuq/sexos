#!/usr/bin/env python3
"""Compare servers/silkbar with its Cesta translation.

The Rust server is compiled from the pinned revision against the pinned
silkbar-model crate; only its no_std scaffolding is removed. crates/sex-pdx is
replaced by a scripted provider that keeps the pinned pdx_call_checked and
pdx_try_listen_raw with the listen syscall routed to it. The Cesta server runs on the client runtime
with the same provider written in C. Both print one line per log line, PDX
call, listen result, tick read and yield, until a yield limit ends the run. The
seeded provider makes display and Bell calls succeed, fail with ERR_CAP_INVALID
or fail otherwise; offers empty listens, workspace and focus messages
(including out-of-range values), Bell replies (counts, generations, denials),
foreign callers and unknown types; and serves ticks that are absent, real,
stalled, or stalled then resumed. Four build variants cover both proof flags.
"""

import argparse
import os
from pathlib import Path
import random
import shutil

from differential_support import LISTEN, first_difference, pinned_function, run

HERE = Path(__file__).resolve().parent.parent
REPO = HERE.parent
REVISION = "4a040a86"
VARIANTS = [(), ("SEXOS_BELL_DELIVERY_PROOF",), ("SEXOS_SILKBAR_CLOCK_FORCE_STALL_PROOF",),
            ("SEXOS_BELL_DELIVERY_PROOF", "SEXOS_SILKBAR_CLOCK_FORCE_STALL_PROOF")]

# Shared provider logic, written once per language. P is a scenario:
# limit, display, bell, ticks, listen (percent of non-empty listens).
RUST_PROVIDER = r'''
use std::cell::RefCell;
struct State { rng: u64, yields: u64, ticks: u64, calls: u64 }
thread_local! { static STATE: RefCell<State> = const { RefCell::new(State { rng: 0, yields: 0, ticks: 0, calls: 0 }) }; }
struct Scenario { seed: u64, limit: u64, display: u64, bell: u64, ticks: u64, listen: u64 }
fn next(s: &mut State) -> u64 {
    s.rng = s.rng.wrapping_add(0x9E3779B97F4A7C15);
    let mut z = s.rng;
    z = (z ^ (z >> 30)).wrapping_mul(0xBF58476D1CE4E5B9);
    z = (z ^ (z >> 27)).wrapping_mul(0x94D049BB133111EB);
    z ^ (z >> 31)
}
fn status_for(s: &mut State, mode: u64) -> u64 {
    s.calls += 1;
    match mode {
        0 => 0,
        1 => if s.calls <= 6 { 0xFFFF_FFFF_FFFF_FFFC } else { 0 },
        _ => match next(s) % 4 { 0 => 0xFFFF_FFFF_FFFF_FFFC, 1 => 0xFFFF_FFFF_FFFF_FFFE, _ => 0 },
    }
}
fn listen_once(slot: u64) -> (u64, u64, u64, u64, u64) {
    STATE.with(|cell| {
        let s = &mut *cell.borrow_mut();
        let (mut t, mut c, mut a0, mut a1, a2) = (0u64, 0u64, 0u64, 0u64, 0u64);
        if next(s) % 100 < SCENARIO.listen {
            let r = next(s);
            let pick = next(s) % 8;
            match pick {
                0 | 1 => { t = 0xF3; a0 = r % 8; }
                2 | 3 => { t = 0xF4; a0 = r % 6; a1 = [0, 1, 6, 8, 15][(r >> 8) as usize % 5]; }
                4 | 5 => { t = 1; c = 1;
                           a0 = [0, 1, 2, 3, u64::MAX, (5u64 << 56) | 3, r][(r >> 8) as usize % 7]; }
                6 => { t = 1; c = 2; a0 = r; }
                _ => { t = 0x100 + r % 4; c = (r >> 16) as u32 as u64 % 16; }
            }
        }
        println!("T {slot} {t} {c} {a0} {a1} {a2}");
        (t, c, a0, a1, a2)
    })
}
#[macro_export]
macro_rules! serial_println { ($($argument:tt)*) => { println!("L {}", format!($($argument)*)) }; }
mod sex_pdx {
    pub use crate::serial_println;
    pub const SLOT_DISPLAY: u64 = 5;
    pub const SLOT_BELL: u64 = 12;
    pub const OP_BELL_LIST: u64 = 0xC3;
    pub const OP_BELL_SUBSCRIBE: u64 = 0xC5;
    pub const OP_SILKBAR_WORKSPACE_ACTIVE: u64 = 0xF3;
    pub const OP_SILKBAR_FOCUS_STATE: u64 = 0xF4;
    pub const ERR_CAP_INVALID: u64 = 0xFFFF_FFFF_FFFF_FFFC;
    pub struct PdxMessage { pub type_id: u64, pub arg0: u64, pub arg1: u64, pub arg2: u64, pub caller_pd: u32, _pad: u32 }
    pub fn pdx_call(slot: u64, opcode: u64, a0: u64, a1: u64, a2: u64) -> (u64, u64) {
        crate::STATE.with(|cell| {
            let s = &mut *cell.borrow_mut();
            let mode = if slot == SLOT_BELL { crate::SCENARIO.bell } else { crate::SCENARIO.display };
            let status = crate::status_for(s, mode);
            let value = crate::next(s) % 1000;
            println!("C {slot} {opcode} {a0} {a1} {a2} -> {status} {value}");
            (status, value)
        })
    }
    pub fn sys_yield() {
        crate::STATE.with(|cell| {
            let s = &mut *cell.borrow_mut();
            println!("Y");
            s.yields += 1;
            if s.yields >= crate::SCENARIO.limit { println!("END"); std::process::exit(0); }
        })
    }
    pub fn get_ticks() -> u64 {
        crate::STATE.with(|cell| {
            let s = &mut *cell.borrow_mut();
            let step = crate::next(s) % 3;
            s.ticks = match crate::SCENARIO.ticks {
                0 => 0,
                1 => s.ticks + step,
                2 => 7,
                _ => if s.yields % 400 < 200 { s.ticks + step } else { s.ticks },
            };
            println!("K {}", s.ticks);
            s.ticks
        })
    }
'''

C_PROVIDER = r'''
#include "sexos_server.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
typedef struct { uint64_t seed, limit, display, bell, ticks, listen; } Scenario;
static const Scenario SCENARIO = SCENARIO_VALUE;
static uint64_t rng, yields, ticks, calls;
static uint64_t next(void) {
    rng += 0x9E3779B97F4A7C15ull;
    uint64_t z = rng;
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
    return z ^ (z >> 31);
}
static uint64_t status_for(uint64_t mode) {
    ++calls;
    if (mode == 0) return 0;
    if (mode == 1) return calls <= 6 ? 0xFFFFFFFFFFFFFFFCull : 0;
    switch (next() % 4) { case 0: return 0xFFFFFFFFFFFFFFFCull; case 1: return 0xFFFFFFFFFFFFFFFEull; default: return 0; }
}
SexosReply sexos_pdx_call(uint64_t slot, uint64_t opcode, uint64_t a0, uint64_t a1, uint64_t a2) {
    if (slot == 0 && opcode == 69) {
        const char *text = (const char *)(uintptr_t)a0;
        size_t length = (size_t)a1;
        if (length && text[length - 1] == '\n') --length;
        printf("L %.*s\n", (int)length, text);
        return (SexosReply){0, 0};
    }
    uint64_t status = status_for(slot == 12 ? SCENARIO.bell : SCENARIO.display);
    uint64_t value = next() % 1000;
    printf("C %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " -> %" PRIu64 " %" PRIu64 "\n",
           slot, opcode, a0, a1, a2, status, value);
    return (SexosReply){status, value};
}
void sexos_yield(void) {
    printf("Y\n");
    if (++yields >= SCENARIO.limit) { printf("END\n"); exit(0); }
}
uint64_t sexos_allocate_memory(uint64_t size) { (void)size; printf("UNEXPECTED allocate\n"); exit(3); }
uint64_t sexos_map_memory(uint64_t pfn, uint64_t size) { (void)pfn; (void)size; printf("UNEXPECTED map\n"); exit(3); }
uint64_t sexos_get_ticks(void) {
    uint64_t step = next() % 3;
    switch (SCENARIO.ticks) {
    case 0: ticks = 0; break;
    case 1: ticks += step; break;
    case 2: ticks = 7; break;
    default: if (yields % 400 < 200) ticks += step; break;
    }
    printf("K %" PRIu64 "\n", ticks);
    return ticks;
}
SexosMessage sexos_listen_once(uint64_t slot) {
    SexosMessage m = {0, 0, 0, 0, 0, 0};
    if (next() % 100 < SCENARIO.listen) {
        uint64_t r = next(), pick = next() % 8;
        static const uint64_t masks[5] = {0, 1, 6, 8, 15};
        const uint64_t gens[7] = {0, 1, 2, 3, UINT64_MAX, (5ull << 56) | 3, r};
        if (pick <= 1) { m.type_id = 0xF3; m.arg0 = r % 8; }
        else if (pick <= 3) { m.type_id = 0xF4; m.arg0 = r % 6; m.arg1 = masks[(r >> 8) % 5]; }
        else if (pick <= 5) { m.type_id = 1; m.caller_pd = 1; m.arg0 = gens[(r >> 8) % 7]; }
        else if (pick == 6) { m.type_id = 1; m.caller_pd = 2; m.arg0 = r; }
        else { m.type_id = 0x100 + r % 4; m.caller_pd = (uint32_t)(r >> 16) % 16; }
    }
    printf("T %" PRIu64 " %" PRIu64 " %" PRIu32 " %" PRIu64 " %" PRIu64 " %" PRIu64 "\n",
           slot, m.type_id, m.caller_pd, m.arg0, m.arg1, m.arg2);
    return m;
}
int main(void) { rng = SCENARIO.seed; silkbar_main(); return 0; }
'''


def rust_reference(source):
    # Only scaffolding changes: no_std attributes, the panic handler, and the
    # exported _start (which would clash with the host C runtime's) become a
    # hosted main calling the unchanged body.
    source = source.replace("#![no_std]", "").replace("#![no_main]", "")
    marker = "#[panic_handler]"
    assert source.count(marker) == 1
    source = source[:source.index(marker)]
    entry = '#[no_mangle]\npub extern "C" fn _start() -> ! {'
    assert source.count(entry) == 1
    source = source.replace(entry, "fn silkbar_start() -> ! {")
    return source + "\nfn main() { STATE.with(|cell| cell.borrow_mut().rng = SCENARIO.seed); silkbar_start() }\n"


def sex_pdx_pinned(source):
    # pdx_call_checked and pdx_try_listen_raw of the pinned sex-pdx, the listen
    # syscall routed to the provider; the module they join closes here.
    checked = pinned_function(source, "pub fn pdx_call_checked(slot: u64, opcode: u64, arg0: u64, arg1: u64, arg2: u64)", {})
    listen = pinned_function(source, "pub fn pdx_try_listen_raw(slot: u64) -> Option<PdxMessage> {", LISTEN)
    return checked + listen + "}\n"


def scenarios(count):
    r = random.Random(0x51B4)
    out = []
    for k in range(count):
        out.append(dict(seed=r.getrandbits(64), limit=[40, 300, 2500, 6000][k % 4],
                        display=k % 3, bell=(k // 3) % 3, ticks=(k // 2) % 4, listen=[0, 5, 30, 80][(k // 4) % 4]))
    return out


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cesta", default="cesta")
    parser.add_argument("--cases", type=int, default=24)
    args = parser.parse_args()
    cesta = shutil.which(args.cesta) or args.cesta
    work = HERE / "build/silkbar-differential"
    if work.exists():
        shutil.rmtree(work)
    work.mkdir(parents=True)
    show = lambda path: run(["git", "-C", REPO, "show", f"{REVISION}:{path}"]).stdout
    (work / "model.rs").write_bytes(show("crates/silkbar-model/src/lib.rs"))
    run(["rustc", "--edition", "2021", "-O", "--crate-type", "rlib", "--crate-name", "silkbar_model",
         work / "model.rs", "-o", work / "libsilkbar_model.rlib"])
    server = show("servers/silkbar/src/main.rs").decode()
    pinned = sex_pdx_pinned(show("crates/sex-pdx/src/lib.rs").decode())
    include = ["-I", HERE / "generated/include"]
    runtime = [HERE / "runtime/client_runtime.c", HERE / "runtime/server_runtime.c",
               *sorted((HERE / "generated/runtime").glob("*.c"))]
    compared = 0
    reference_traces = {}
    for variant in VARIANTS:
        defines = ["-DSEXOS_SILKBAR_DISPLAY_READY=0", "-DSEXOS_SILKBAR_BELL_REPLY_CALLER=1", *(f"-D{name}" for name in variant)]
        objects = []
        for compiler in ("gcc", "clang"):
            group = []
            for unit in ("silkbar", "silkbar_model"):
                obj = work / f"{unit}-{compiler}-{len(variant)}-{'-'.join(variant)}.o"
                run([cesta, "cc", "--cc", compiler, "-std=c11", "-O2", "-Wall", "-Wextra", "-Werror", *defines,
                     "-fsanitize=address,undefined", "-fno-sanitize-recover=all", *include,
                     "-c", HERE / "src" / f"{unit}.cesta", "-o", obj])
                group.append(obj)
            objects.append((compiler, group))
        for number, case in enumerate(scenarios(args.cases)):
            scenario = ("Scenario { seed: %d, limit: %d, display: %d, bell: %d, ticks: %d, listen: %d }"
                        % (case["seed"], case["limit"], case["display"], case["bell"], case["ticks"], case["listen"]))
            rust = work / "reference.rs"
            rust.write_text(RUST_PROVIDER + pinned + f"static SCENARIO: Scenario = {scenario};\n" + rust_reference(server))
            env = dict(os.environ)
            for name in variant:
                env[name] = "1"
            run(["rustc", "--edition", "2021", "-O", "-A", "warnings", "--extern",
                 f"silkbar_model={work / 'libsilkbar_model.rlib'}", rust, "-o", work / "reference"], env=env)
            expected = run([work / "reference"]).stdout
            if not variant:
                reference_traces[number] = expected
            c_scenario = "{%dull, %d, %d, %d, %d, %d}" % (case["seed"], case["limit"], case["display"],
                                                          case["bell"], case["ticks"], case["listen"])
            (work / "provider.c").write_text(C_PROVIDER.replace("SCENARIO_VALUE", c_scenario))
            for compiler, group in objects:
                harness = work / f"harness-{compiler}"
                run([compiler, "-std=c11", "-O2", "-DSEXOS_HOST", *defines, "-fsanitize=address,undefined",
                     "-fno-sanitize-recover=all", *include, work / "provider.c", *group, *runtime, "-o", harness])
                actual = run([harness]).stdout
                if actual != expected:
                    raise SystemExit(first_difference(actual, expected, f"variant {variant} scenario {number} {compiler}"))
                compared += len(expected.splitlines())
    # Controls: each small change to the translation must change some trace.
    original = (HERE / "src/silkbar.cesta").read_text()
    mutations = [
        ("(= (. cadence_start) 16)", "(= (. cadence_start) 15)"),
        ("(define STALE_REAL_TICK_FALLBACK_LOOPS \"((uint16_t)16)\")", "(define STALE_REAL_TICK_FALLBACK_LOOPS \"((uint16_t)15)\")"),
        ("(<< (cast uint64_t (. update index)) 32)", "(<< (cast uint64_t (. update index)) 31)"),
        ("(var uint8_t redacted (cast uint8_t (& (>> packed 56) 0xFF)))", "(var uint8_t redacted (cast uint8_t (& (>> packed 48) 0xFF)))"),
        ("(if (> raw 3)", "(if (> raw 4)"),
        ("(= chip_phase (cast uint8_t (& (+ chip_phase 1) 3)))", "(= chip_phase (cast uint8_t (& (+ chip_phase 2) 3)))"),
        ("(if (|| (<= (. s clock_send_count) 12)", "(if (|| (<= (. s clock_send_count) 11)"),
        ("(if (== loop_iter 0) (call sexos_line \"[bootgraph.silkbar.loop_ready]\"))", "(if (== loop_iter 1) (call sexos_line \"[bootgraph.silkbar.loop_ready]\"))"),
    ]
    model_object = work / "model-mutation.o"
    run([cesta, "cc", "--cc", "gcc", "-std=c11", "-O2", *include, "-c", HERE / "src/silkbar_model.cesta", "-o", model_object])
    for number, (old_text, new_text) in enumerate(mutations):
        assert original.count(old_text) == 1, old_text
        mutant = work / f"silkbar-mutant-{number}.cesta"
        mutant.write_text(original.replace(old_text, new_text))
        run([cesta, "cc", "--cc", "gcc", "-std=c11", "-O2", "-DSEXOS_SILKBAR_DISPLAY_READY=0", "-DSEXOS_SILKBAR_BELL_REPLY_CALLER=1", *include, "-c", mutant, "-o", work / "mutant.o"])
        detected = False
        for index, case in enumerate(scenarios(args.cases)):
            c_scenario = "{%dull, %d, %d, %d, %d, %d}" % (case["seed"], case["limit"], case["display"],
                                                          case["bell"], case["ticks"], case["listen"])
            (work / "provider.c").write_text(C_PROVIDER.replace("SCENARIO_VALUE", c_scenario))
            run(["gcc", "-std=c11", "-O2", "-DSEXOS_HOST", *include, work / "provider.c", work / "mutant.o",
                 model_object, *runtime, "-o", work / "mutant-harness"])
            if run([work / "mutant-harness"]).stdout != reference_traces[index]:
                detected = True
                break
        if not detected:
            raise SystemExit(f"mutation not detected: {old_text} -> {new_text}")
    print(f"silkbar: Cesta translation equals the Rust server on {compared} trace lines "
          f"({args.cases} scenarios x {len(VARIANTS)} build variants, GCC and Clang, address and UB sanitizers); "
          f"{len(mutations)} mutations detected")


if __name__ == "__main__":
    main()
