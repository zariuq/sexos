#!/usr/bin/env python3
"""Compare servers/sexinput with its Cesta translation.

The Rust server is compiled from the pinned revision; only its no_std
scaffolding is removed. crates/sex-pdx is replaced by a scripted provider that
keeps the pinned pdx_call_checked and pdx_try_listen_raw with the listen
syscall routed to it, and heap_init comes from the pinned sex-rt with its two
syscalls routed to the provider. The Cesta server runs on the client runtime with the same provider
written in C. Both print one line per log line, PDX call, listen result, state
change, memory syscall and yield, until a yield limit ends the run. The seeded
provider makes shell calls succeed, fail with ERR_CAP_INVALID first or fail at
random; offers USB mouse reports (normalized tuples, relative and absolute
packed reports, out-of-range coordinates and button bits, zero reports),
keyboard reports (mapped, unmapped, cursor and released keys), PS/2 scancodes,
foreign message types and empty listens; and lets the heap syscalls fail. The
build variants cover the five build-time flags.
"""

import argparse
from itertools import combinations
import os
from pathlib import Path
import random
import shutil
import tempfile

from differential_support import LISTEN, first_difference, pinned_function, run

HERE = Path(__file__).resolve().parent.parent
REPO = HERE.parent
REVISION = "4a040a86"
FLAGS = ["SEXOS_PROOFS_DISABLED", "SEXOS_SILKBAR_CLICK_PROOF", "SEXOS_KEYBOARD_CURSOR",
         "SEXOS_KEYBOARD_PROOF", "SEXOS_CURSOR_AUTODEMO"]
DEFAULT_VARIANTS = [(), *((flag,) for flag in FLAGS), tuple(FLAGS[1:]), tuple(FLAGS)]

# Shared provider logic, written once per language. A scenario is
# seed, limit, shell (call status mode), usb and ps2 (percent of non-empty
# listens on slots 0 and 3), heap (0 works, 1 allocation fails, 2 mapping fails).
RUST_PROVIDER = r'''
use std::cell::RefCell;
#[derive(Clone, Copy)]
struct Scenario { seed: u64, limit: u64, shell: u64, usb: u64, ps2: u64, heap: u64, zero_poll: u64 }
struct State { rng: u64, yields: u64, calls: u64, polls: u64, scenario: Scenario }
thread_local! { static STATE: RefCell<State> = const { RefCell::new(State { rng: 0, yields: 0, calls: 0,
    polls: 0, scenario: Scenario { seed: 0, limit: 0, shell: 0, usb: 0, ps2: 0, heap: 0, zero_poll: 0 } }) }; }
fn next(s: &mut State) -> u64 {
    s.rng = s.rng.wrapping_add(0x9E3779B97F4A7C15);
    let mut z = s.rng;
    z = (z ^ (z >> 30)).wrapping_mul(0xBF58476D1CE4E5B9);
    z = (z ^ (z >> 27)).wrapping_mul(0x94D049BB133111EB);
    z ^ (z >> 31)
}
fn status_for(s: &mut State) -> u64 {
    s.calls += 1;
    match s.scenario.shell {
        0 => 0,
        1 => if s.calls <= 5 { 0xFFFF_FFFF_FFFF_FFFC } else { 0 },
        _ => match next(s) % 4 { 0 => 0xFFFF_FFFF_FFFF_FFFC, 1 => 0xFFFF_FFFF_FFFF_FFFE, _ => 0 },
    }
}
fn sexos_allocate_memory(size: u64) -> u64 {
    STATE.with(|cell| {
        let pfn = if cell.borrow().scenario.heap == 1 { u64::MAX } else { 0x1234 };
        println!("A {size} -> {pfn}");
        pfn
    })
}
fn sexos_map_memory(pfn: u64, size: u64) -> u64 {
    STATE.with(|cell| {
        let vaddr = if cell.borrow().scenario.heap == 2 { u64::MAX } else { 0x4000_0000_0000 };
        println!("M {pfn} {size} -> {vaddr}");
        vaddr
    })
}
fn listen_once(slot: u64) -> (u64, u64, u64, u64, u64) {
    STATE.with(|cell| {
        let s = &mut *cell.borrow_mut();
        let (mut t, mut c, mut a0, mut a1, mut a2) = (0u64, 0u64, 0u64, 0u64, 0u64);
        if s.scenario.zero_poll != 0 {
            if slot == 0 {
                s.polls += 1;
                if s.polls == s.scenario.zero_poll { t = 0x260; c = 5; }
            }
            println!("T {slot} {t} {c} {a0} {a1} {a2}");
            return (t, c, a0, a1, a2);
        }
        let percent = if slot == sex_pdx::SLOT_INPUT { s.scenario.ps2 } else { s.scenario.usb };
        if crate::next(s) % 100 < percent {
            let r = crate::next(s);
            let q = crate::next(s);
            let pick = crate::next(s) % 10;
            if slot == sex_pdx::SLOT_INPUT {
                const SCANCODES: [u64; 11] = [0x1C, 0x9C, 0x11, 0x91, 0x48, 0xC8, 0x1E, 0x4D, 0x20, 0x3F, 0x7F];
                if pick < 8 { t = 0x201; a0 = if r % 12 < 11 { SCANCODES[(r % 12) as usize] } else { q & 0xFFF }; }
                else { t = 0x202; a0 = q; }
            } else {
                c = (r >> 40) % 8;
                let small = [0, 1, 7, 5u64.wrapping_neg(), 0x1_0000_0003, q];
                let buttons = [0u64, 1, 2, 3, 4, 7, 8, 0x81];
                let coords = [0u64, 1, 100, 16000, 32766, 32767, 32768, q & 0xFFFF];
                let keys = [0u64, 0, 0x04, 0x1A, 0x28, 0x47, 0x52, 0x4F, 0x51, 0x3A, 0x1D, q & 0xFF];
                match pick {
                    0 | 1 => { t = 0x260; a0 = 2 + r % 3;
                               a1 = small[((r >> 8) % 6) as usize]; a2 = small[((r >> 16) % 6) as usize]; }
                    2 | 3 | 4 => { t = 0x260; a0 = [0, 5, q][((r >> 8) % 3) as usize];
                                   a1 = buttons[((r >> 12) % 8) as usize]; a2 = q & 0xFF_FFFF;
                                   if (r >> 20) % 5 == 0 { a1 = 0; a2 = 0; } }
                    5 | 6 => { t = 0x260; a1 = [0, 1, 1, 9][((r >> 12) % 4) as usize];
                               a2 = (1 << 32) | (coords[((r >> 24) % 8) as usize] << 16) | coords[((r >> 16) % 8) as usize]; }
                    7 | 8 => { t = 0x261; a1 = q >> 56; a2 = keys[((r >> 8) % 12) as usize]; }
                    _ => { t = [0x262, 0x201, 0x100][(r % 3) as usize]; a0 = q; }
                }
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
    pub const EV_KEY: u64 = 1;
    pub const EV_REL: u64 = 2;
    pub const EV_ABS: u64 = 3;
    pub const EV_BTN: u64 = 4;
    pub const SLOT_INPUT: u64 = 3;
    pub const SLOT_SHELL: u64 = 6;
    pub const SVC_STATE_LISTENING: u64 = 1;
    pub const ERR_CAP_INVALID: u64 = 0xFFFF_FFFF_FFFF_FFFC;
    pub struct PdxMessage { pub type_id: u64, pub arg0: u64, pub arg1: u64, pub arg2: u64, pub caller_pd: u32, _pad: u32 }
    pub fn pdx_call(slot: u64, opcode: u64, a0: u64, a1: u64, a2: u64) -> (u64, u64) {
        crate::STATE.with(|cell| {
            let s = &mut *cell.borrow_mut();
            let status = crate::status_for(s);
            let value = crate::next(s) % 1000;
            println!("C {slot} {opcode} {a0} {a1} {a2} -> {status} {value}");
            (status, value)
        })
    }
    pub unsafe fn sys_set_state(state: u64) -> u64 { println!("S {state}"); 0 }
    pub fn sys_yield() {
        crate::STATE.with(|cell| {
            let s = &mut *cell.borrow_mut();
            println!("Y");
            s.yields += 1;
            if s.yields >= s.scenario.limit { println!("END"); std::process::exit(0); }
        })
    }
'''

C_PROVIDER = r'''
#include "sexos_server.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
static struct { uint64_t seed, limit, shell, usb, ps2, heap, zero_poll; } scenario;
static uint64_t rng, yields, calls, polls;
static uint64_t next(void) {
    rng += 0x9E3779B97F4A7C15ull;
    uint64_t z = rng;
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
    return z ^ (z >> 31);
}
static uint64_t status_for(void) {
    ++calls;
    if (scenario.shell == 0) return 0;
    if (scenario.shell == 1) return calls <= 5 ? 0xFFFFFFFFFFFFFFFCull : 0;
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
    uint64_t status = status_for();
    uint64_t value = next() % 1000;
    printf("C %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " -> %" PRIu64 " %" PRIu64 "\n",
           slot, opcode, a0, a1, a2, status, value);
    return (SexosReply){status, value};
}
uint64_t sexos_set_state(uint64_t state) { printf("S %" PRIu64 "\n", state); return 0; }
uint64_t sexos_allocate_memory(uint64_t size) {
    uint64_t pfn = scenario.heap == 1 ? UINT64_MAX : 0x1234;
    printf("A %" PRIu64 " -> %" PRIu64 "\n", size, pfn);
    return pfn;
}
uint64_t sexos_map_memory(uint64_t pfn, uint64_t size) {
    uint64_t vaddr = scenario.heap == 2 ? UINT64_MAX : 0x400000000000ull;
    printf("M %" PRIu64 " %" PRIu64 " -> %" PRIu64 "\n", pfn, size, vaddr);
    return vaddr;
}
void sexos_yield(void) {
    printf("Y\n");
    if (++yields >= scenario.limit) { printf("END\n"); exit(0); }
}
SexosMessage sexos_listen_once(uint64_t slot) {
    SexosMessage m = {0, 0, 0, 0, 0, 0};
    if (scenario.zero_poll != 0) {
        if (slot == 0 && ++polls == scenario.zero_poll) { m.type_id = 0x260; m.caller_pd = 5; }
        printf("T %" PRIu64 " %" PRIu64 " %" PRIu32 " %" PRIu64 " %" PRIu64 " %" PRIu64 "\n",
               slot, m.type_id, m.caller_pd, m.arg0, m.arg1, m.arg2);
        return m;
    }
    uint64_t percent = slot == 3 ? scenario.ps2 : scenario.usb;
    if (next() % 100 < percent) {
        uint64_t r = next(), q = next(), pick = next() % 10;
        if (slot == 3) {
            static const uint64_t scancodes[11] = {0x1C, 0x9C, 0x11, 0x91, 0x48, 0xC8, 0x1E, 0x4D, 0x20, 0x3F, 0x7F};
            if (pick < 8) { m.type_id = 0x201; m.arg0 = r % 12 < 11 ? scancodes[r % 12] : (q & 0xFFF); }
            else { m.type_id = 0x202; m.arg0 = q; }
        } else {
            m.caller_pd = (uint32_t)((r >> 40) % 8);
            const uint64_t small[6] = {0, 1, 7, (uint64_t)0 - 5, 0x100000003ull, q};
            static const uint64_t buttons[8] = {0, 1, 2, 3, 4, 7, 8, 0x81};
            const uint64_t coords[8] = {0, 1, 100, 16000, 32766, 32767, 32768, q & 0xFFFF};
            const uint64_t keys[12] = {0, 0, 0x04, 0x1A, 0x28, 0x47, 0x52, 0x4F, 0x51, 0x3A, 0x1D, q & 0xFF};
            if (pick <= 1) {
                m.type_id = 0x260; m.arg0 = 2 + r % 3;
                m.arg1 = small[(r >> 8) % 6]; m.arg2 = small[(r >> 16) % 6];
            } else if (pick <= 4) {
                const uint64_t first[3] = {0, 5, q};
                m.type_id = 0x260; m.arg0 = first[(r >> 8) % 3];
                m.arg1 = buttons[(r >> 12) % 8]; m.arg2 = q & 0xFFFFFF;
                if ((r >> 20) % 5 == 0) { m.arg1 = 0; m.arg2 = 0; }
            } else if (pick <= 6) {
                static const uint64_t abs_buttons[4] = {0, 1, 1, 9};
                m.type_id = 0x260; m.arg1 = abs_buttons[(r >> 12) % 4];
                m.arg2 = (1ull << 32) | (coords[(r >> 24) % 8] << 16) | coords[(r >> 16) % 8];
            } else if (pick <= 8) {
                m.type_id = 0x261; m.arg1 = q >> 56; m.arg2 = keys[(r >> 8) % 12];
            } else {
                static const uint64_t types[3] = {0x262, 0x201, 0x100};
                m.type_id = types[r % 3]; m.arg0 = q;
            }
        }
    }
    printf("T %" PRIu64 " %" PRIu64 " %" PRIu32 " %" PRIu64 " %" PRIu64 " %" PRIu64 "\n",
           slot, m.type_id, m.caller_pd, m.arg0, m.arg1, m.arg2);
    return m;
}
int main(int argc, char **argv) {
    if (argc != 8) return 2;
    uint64_t *fields[7] = {&scenario.seed, &scenario.limit, &scenario.shell, &scenario.usb, &scenario.ps2, &scenario.heap, &scenario.zero_poll};
    for (int i = 0; i < 7; ++i) *fields[i] = strtoull(argv[i + 1], NULL, 10);
    rng = scenario.seed;
    sexinput_main();
    return 0;
}
'''

RUST_MAIN = r'''
fn main() {
    let v: Vec<u64> = std::env::args().skip(1).map(|a| a.parse().unwrap()).collect();
    assert_eq!(v.len(), 7);
    STATE.with(|cell| {
        let s = &mut *cell.borrow_mut();
        s.scenario = Scenario { seed: v[0], limit: v[1], shell: v[2], usb: v[3], ps2: v[4], heap: v[5], zero_poll: v[6] };
        s.rng = v[0];
    });
    sexinput_start()
}
'''


def sex_rt_heap(source):
    # heap_init and expand_heap of the pinned sex-rt, with the inline syscalls
    # 31 (allocate) and 30 (map) routed to the provider.
    head = source[source.index("const HEAP_START_VADDR"):source.index("#[global_allocator]")]
    expand = pinned_function(source, "fn expand_heap(needed_size: usize) -> Result<usize, ()> {",
                             {"31": "pfn = crate::sexos_allocate_memory(size_aligned as u64);",
                              "30": "vaddr = crate::sexos_map_memory(pfn, size_aligned as u64);"})
    init = pinned_function(source, "pub fn heap_init() {", {})
    return "mod sex_rt {\nuse core::sync::atomic::{AtomicUsize, Ordering};\n" + head + expand + init + "}\n"


def sex_pdx_pinned(source):
    # pdx_call_checked and pdx_try_listen_raw of the pinned sex-pdx, the listen
    # syscall routed to the provider; the module they join closes here.
    checked = pinned_function(source, "pub fn pdx_call_checked(slot: u64, opcode: u64, arg0: u64, arg1: u64, arg2: u64)", {})
    listen = pinned_function(source, "pub fn pdx_try_listen_raw(slot: u64) -> Option<PdxMessage> {", LISTEN)
    return checked + listen + "}\n"


def rust_reference(source):
    # Only scaffolding changes: no_std attributes, the panic handler, and the
    # exported _start (which would clash with the host C runtime's) become a
    # hosted main calling the unchanged body.
    source = source.replace("#![no_std]", "").replace("#![no_main]", "")
    handler = "#[panic_handler]\nfn panic(_info: &PanicInfo) -> ! {\n    loop { sys_yield(); }\n}\n"
    assert source.count(handler) == 1
    source = source.replace(handler, "")
    entry = '#[no_mangle]\npub extern "C" fn _start() -> ! {'
    assert source.count(entry) == 1
    return source.replace(entry, "fn sexinput_start() -> ! {")


def scenarios(count):
    r = random.Random(0x5E81)
    out = []
    for k in range(count):
        out.append([r.getrandbits(64), [60, 600, 3000, 8000][k % 4], (k // 4) % 3,
                    [0, 0, 5, 30, 80][(k // 2) % 5], [0, 10, 40][(k // 3) % 3], 0 if k % 7 else (k // 7) % 3, 0])
    # Deliver the boot's zero report before, during and after the synthetic
    # drag. A report during the drag cancels its last stage, so normalization
    # emits a release; outside the drag it emits nothing. This schedule is
    # shared with the unchanged Rust body rather than inferred from log order.
    out.extend([0, 60, 0, 0, 0, 0, poll] for poll in (5, 6, 7, 8, 9))
    return out


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cesta", default="cesta")
    parser.add_argument("--cases", type=int, default=30)
    parser.add_argument("--all-variants", action="store_true", help="all 32 combinations of the five flags")
    args = parser.parse_args()
    cesta = shutil.which(args.cesta) or args.cesta
    variants = ([c for n in range(len(FLAGS) + 1) for c in combinations(FLAGS, n)]
                if args.all_variants else DEFAULT_VARIANTS)
    work = Path(tempfile.mkdtemp(prefix="sexinput-differential-", dir=HERE / "build"))
    show = lambda path: run(["git", "-C", REPO, "show", f"{REVISION}:{path}"]).stdout.decode()
    rust = work / "reference.rs"
    rust.write_text(RUST_PROVIDER + sex_pdx_pinned(show("crates/sex-pdx/src/lib.rs")) + sex_rt_heap(show("sex-rt/src/lib.rs"))
                    + rust_reference(show("servers/sexinput/src/main.rs")) + RUST_MAIN)
    (work / "provider.c").write_text(C_PROVIDER)
    include = ["-I", HERE / "generated/include"]
    runtime = [HERE / "runtime/client_runtime.c", HERE / "runtime/server_runtime.c",
               *sorted((HERE / "generated/runtime").glob("*.c"))]
    cases = scenarios(args.cases)
    compared = 0
    references = {}
    for variant in variants:
        tag = "-".join(name.removeprefix("SEXOS_").lower() for name in variant) or "default"
        defines = [f"-D{name}" for name in variant]
        env = dict(os.environ)
        for name in variant:
            env[name] = "1"
        reference = work / f"reference-{tag}"
        run(["rustc", "--edition", "2021", "-O", "-A", "warnings", rust, "-o", reference], env=env)
        harnesses = []
        for compiler in ("gcc", "clang"):
            obj = work / f"sexinput-{compiler}-{tag}.o"
            sanitizers = ["-fsanitize=address,undefined", "-fno-sanitize-recover=all"]
            run([cesta, "cc", "--cc", compiler, "-std=c11", "-O2", "-Wall", "-Wextra", "-Wpedantic", "-Wformat=2",
                 "-Werror", *defines, *sanitizers, *include, "-c", HERE / "src/sexinput.cesta", "-o", obj])
            harness = work / f"harness-{compiler}-{tag}"
            run([compiler, "-std=c11", "-O2", "-DSEXOS_HOST", *sanitizers, *include, work / "provider.c", obj,
                 *runtime, "-o", harness])
            harnesses.append((compiler, harness))
        for number, case in enumerate(cases):
            expected = run([reference, *case]).stdout
            if not variant and case[6]:
                release = b"L [sexinput.pointer.send] class=4 a0=1 a1=0\n" in expected
                assert release == (case[6] in (6, 7)), (case[6], "drag cancellation release")
            references[variant, number] = expected
            for compiler, harness in harnesses:
                actual = run([harness, *case]).stdout
                if actual != expected:
                    raise SystemExit(first_difference(actual, expected, f"variant {tag} scenario {number} {compiler}"))
            compared += len(expected.splitlines())
    # Controls: each small change to the translation must change some trace of
    # the variant whose code it touches.
    original = (HERE / "src/sexinput.cesta").read_text()
    click, cursor, keyboard, autodemo = (("SEXOS_SILKBAR_CLICK_PROOF",), ("SEXOS_KEYBOARD_CURSOR",),
                                         ("SEXOS_KEYBOARD_PROOF",), ("SEXOS_CURSOR_AUTODEMO",))
    mutations = [
        ((), '(define DELTA_CLAMP "((int32_t)64)")', '(define DELTA_CLAMP "((int32_t)63)")'),
        ((), "(- low 0x10000)", "(- low 0xFFFF)"),
        ((), "(init 0x28 0x1C)", "(init 0x28 0x1D)"),
        ((), "(= (. report dx) 6)", "(= (. report dx) 7)"),
        ((), "(var bool buttons_bad (!= (& buttons (~ 0x07)) 0))", "(var bool buttons_bad (!= (& buttons (~ 0x0F)) 0))"),
        ((), "(= (. raw_payload_log) 64)", "(= (. raw_payload_log) 63)"),
        ((), "(if (&& (! (-> s cap_ready_shell)) (== (. result status) SEXOS_ERR_CAP_INVALID))",
             "(if (== (. result status) SEXOS_ERR_CAP_INVALID)"),
        ((), "(var int32_t tdx (- (cast int32_t cx) (cast int32_t (-> c start_x))))",
             "(var int32_t tdx (- (cast int32_t cx) (cast int32_t (-> c x))))"),
        (click, "(init 15 1100 25 SEXOS_EV_abs NULL)", "(init 15 1101 25 SEXOS_EV_abs NULL)"),
        (cursor, "(init 0x1E 0x4B (- 8) 0)", "(init 0x1E 0x4B (- 7) 0)"),
        (keyboard, '(init 150 64 1 "[sexinput.kbd_proof.f6.down]")', '(init 151 64 1 "[sexinput.kbd_proof.f6.down]")'),
        (autodemo, "(<= tick 750)", "(<= tick 720)"),
    ]
    for number, (variant, old_text, new_text) in enumerate(mutations):
        assert original.count(old_text) == 1, old_text
        assert (variant, 0) in references, variant
        mutant = work / f"sexinput-mutant-{number}.cesta"
        mutant.write_text(original.replace(old_text, new_text))
        defines = [f"-D{name}" for name in variant]
        run([cesta, "cc", "--cc", "gcc", "-std=c11", "-O2", *defines, *include, "-c", mutant, "-o", work / "mutant.o"])
        run(["gcc", "-std=c11", "-O2", "-DSEXOS_HOST", *include, work / "provider.c", work / "mutant.o",
             *runtime, "-o", work / "mutant-harness"])
        if not any(run([work / "mutant-harness", *case]).stdout != references[variant, index]
                   for index, case in enumerate(cases)):
            raise SystemExit(f"mutation not detected: {old_text} -> {new_text}")
    print(f"sexinput: Cesta translation equals the Rust server on {compared} trace lines "
          f"({len(cases)} scenarios x {len(variants)} build variants, GCC and Clang, address and UB sanitizers); "
          f"{len(mutations)} mutations detected; five boot-report schedules checked")


if __name__ == "__main__":
    main()
