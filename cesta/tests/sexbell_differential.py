#!/usr/bin/env python3
"""Compare servers/sexbell with its Cesta translation.

The Rust server is compiled from the pinned revision with only its no_std
scaffolding removed; crates/sex-pdx is replaced by a scripted provider that
keeps the pinned pdx_listen_raw with its listen syscall routed to it. Both
servers print one line per log line, listen result, reply, tick read and
yield. The seeded provider sends notifications from several senders (valid
and malformed fields, SelfCapDenied, urgency hints), enough to fill the queue
and force evictions among tied lanes; lists with valid and invalid filters
and counts, from readers and others; closes and actions on recent, dismissed
and unknown event ids; clears of all lanes, one lane and invalid lanes; mutes
and unmutes; policies from the author and others (privacy and lane
overrides, privacy reductions, forced mutes, removals, a full table);
subscriptions; and unknown opcodes. Ticks are absent, slow or fast, so spam
windows stay shut or reopen. Subscription replies expose every generation
change, so which entry an eviction took stays observable after the log
budgets run out. The build variants cover the three flags. Removing a policy
clears the vacated slot's target, a store no lookup reads (they stop at the
count), so no trace can show it.
"""

import argparse
from itertools import combinations
import os
from pathlib import Path
import random
import shutil

from differential_support import LISTEN, first_difference, pinned_function, run

HERE = Path(__file__).resolve().parent.parent
REPO = HERE.parent
REVISION = "4a040a86"
FLAGS = ["SEXOS_BELL_DELIVERY_PROOF", "SEXOS_BELL_BRIDGE_STUB_PROOF", "SEXOS_BELL_LANE_RENAME_PROOF"]
VARIANTS = [c for n in range(len(FLAGS) + 1) for c in combinations(FLAGS, n)]

# A scenario is seed, limit (yields), listen (percent of non-empty listens),
# ticks (0 absent, 1 slow, 2 fast) and mix (0 balanced, 1 notify-heavy).
RUST_PROVIDER = r'''
use std::cell::RefCell;
#[derive(Clone, Copy)]
struct Scenario { seed: u64, limit: u64, listen: u64, ticks: u64, mix: u64 }
struct State { rng: u64, yields: u64, ticks: u64, scenario: Scenario }
thread_local! { static STATE: RefCell<State> = const { RefCell::new(State { rng: 0, yields: 0, ticks: 0,
    scenario: Scenario { seed: 0, limit: 0, listen: 0, ticks: 0, mix: 0 } }) }; }
fn next(s: &mut State) -> u64 {
    s.rng = s.rng.wrapping_add(0x9E3779B97F4A7C15);
    let mut z = s.rng;
    z = (z ^ (z >> 30)).wrapping_mul(0xBF58476D1CE4E5B9);
    z = (z ^ (z >> 27)).wrapping_mul(0x94D049BB133111EB);
    z ^ (z >> 31)
}
fn pick(s: &mut State, values: &[u64]) -> u64 { let i = (next(s) % values.len() as u64) as usize; values[i] }
fn listen_once(slot: u64) -> (u64, u64, u64, u64, u64) {
    STATE.with(|cell| {
        let s = &mut *cell.borrow_mut();
        let (mut t, mut c, mut a0, mut a1, mut a2) = (0u64, 0u64, 0u64, 0u64, 0u64);
        if next(s) % 100 < s.scenario.listen {
            let kind = next(s) % 20;
            const SENDERS: [u64; 8] = [3, 6, 9, 10, 11, 12, 13, 0];
            c = pick(s, &SENDERS);
            let notify_cut = if s.scenario.mix == 1 { 12 } else { 7 };
            if kind < notify_cut {
                t = 0xC0;
                let category = pick(s, &[0, 1, 5, 6, 7]);
                let urgency = pick(s, &[0, 0, 1, 3, 4]);
                let privacy = pick(s, &[0, 0, 1, 3, 4]);
                let redaction = pick(s, &[0, 2, 3, 5]);
                a0 = category | urgency << 8 | privacy << 16 | redaction << 24;
                let actions = pick(s, &[0, 0, 1, 1, 2]);
                a1 = actions | pick(s, &[0, 1, 7]) << 8;
                a2 = pick(s, &[0, 0, 1, 2]) | pick(s, &[0, 0xC3, 0xFF]) << 8;
            } else {
                match kind - notify_cut {
                    0 | 1 => { t = 0xC3; a0 = pick(s, &[0xFF, 0, 1, 3, 5, 6]) | pick(s, &[1, 2, 4, 0, 5]) << 8; c = pick(s, &[3, 6, 6, 9]); }
                    2 | 3 => { t = 0xC1; a0 = next(s) % 40; }
                    4 => { t = 0xC2; a0 = next(s) % 40; a1 = pick(s, &[0, 1, 7]); }
                    5 => { t = 0xC4; a0 = pick(s, &[0xFF, 0, 3, 4, 6]); }
                    6 => { t = 0xC7; let pd = if next(s) % 2 == 0 { pick(s, &[9, 10, 11, 12, 3]) } else { 100 + next(s) % 24 };
                           a0 = pd | pick(s, &[0, 0, 1, 2]) << 32; }
                    7 | 8 => {
                        t = 0xC6; c = pick(s, &[3, 3, 3, 6]); a0 = pick(s, &[9, 10, 11, 12, 13, 14, 15, 16, 17]);
                        a1 = pick(s, &[0, 1, 2, 3, 4, 5, 7]) | pick(s, &[0, 1, 2, 3]) << 8 | pick(s, &[0, 2, 4, 5, 6, 7]) << 16 | pick(s, &[0, 1]) << 24;
                    }
                    9 | 10 | 11 => { t = 0xC5; c = pick(s, &[3, 6, 9]); }
                    _ => { t = pick(s, &[0xC8, 0x11, 0xB0]); }
                }
            }
        }
        println!("T {slot} {t} {c} {a0} {a1} {a2}");
        (t, c, a0, a1, a2)
    })
}
mod sex_pdx {
    macro_rules! serial_println { ($($argument:tt)*) => { println!("L {}", format!($($argument)*)) }; }
    pub(crate) use serial_println;
    pub const OP_BELL_NOTIFY: u64 = 0xC0;
    pub const OP_BELL_CLOSE: u64 = 0xC1;
    pub const OP_BELL_ACTION: u64 = 0xC2;
    pub const OP_BELL_LIST: u64 = 0xC3;
    pub const OP_BELL_CLEAR: u64 = 0xC4;
    pub const OP_BELL_SUBSCRIBE: u64 = 0xC5;
    pub const OP_BELL_SET_POLICY: u64 = 0xC6;
    pub const OP_BELL_MUTE_SENDER: u64 = 0xC7;
    pub struct PdxMessage { pub type_id: u64, pub arg0: u64, pub arg1: u64, pub arg2: u64, pub caller_pd: u32, _pad: u32 }
    pub fn pdx_reply(target_pd: u32, value: u64) -> u64 { println!("R {target_pd} {value}"); 0 }
    pub fn get_ticks() -> u64 {
        crate::STATE.with(|cell| {
            let s = &mut *cell.borrow_mut();
            let step = crate::next(s) % 8;
            s.ticks = match s.scenario.ticks { 0 => 0, 1 => s.ticks + step, _ => s.ticks + 20 * step };
            println!("K {}", s.ticks);
            s.ticks
        })
    }
    pub fn sys_yield() {
        crate::STATE.with(|cell| {
            let s = &mut *cell.borrow_mut();
            println!("Y");
            s.yields += 1;
            if s.yields >= s.scenario.limit { println!("END"); std::process::exit(0); }
        })
    }
'''

RUST_MAIN = r'''
fn main() {
    let v: Vec<u64> = std::env::args().skip(1).map(|a| a.parse().unwrap()).collect();
    STATE.with(|cell| {
        let s = &mut *cell.borrow_mut();
        s.scenario = Scenario { seed: v[0], limit: v[1], listen: v[2], ticks: v[3], mix: v[4] };
        s.rng = v[0];
    });
    sexbell_start()
}
'''

C_PROVIDER = r'''
#include "sexos_server.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
static struct { uint64_t seed, limit, listen, ticks, mix; } scenario;
static uint64_t rng, yields, ticks;
static uint64_t next(void) {
    rng += 0x9E3779B97F4A7C15ull;
    uint64_t z = rng;
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
    return z ^ (z >> 31);
}
#define PICK(...) pick((const uint64_t[]){__VA_ARGS__}, sizeof((const uint64_t[]){__VA_ARGS__}) / sizeof(uint64_t))
static uint64_t pick(const uint64_t *values, size_t count) { return values[next() % count]; }
SexosReply sexos_pdx_call(uint64_t slot, uint64_t opcode, uint64_t a0, uint64_t a1, uint64_t a2) {
    (void)a2;
    if (slot != 0 || opcode != 69) { printf("UNEXPECTED call\n"); exit(3); }
    const char *text = (const char *)(uintptr_t)a0;
    size_t length = (size_t)a1;
    if (length && text[length - 1] == '\n') --length;
    printf("L %.*s\n", (int)length, text);
    return (SexosReply){0, 0};
}
uint64_t sexos_allocate_memory(uint64_t size) { (void)size; printf("UNEXPECTED allocate\n"); exit(3); }
uint64_t sexos_map_memory(uint64_t pfn, uint64_t size) { (void)pfn; (void)size; printf("UNEXPECTED map\n"); exit(3); }
uint64_t sexos_reply(uint32_t target_pd, uint64_t value) { printf("R %" PRIu32 " %" PRIu64 "\n", target_pd, value); return 0; }
uint64_t sexos_get_ticks(void) {
    uint64_t step = next() % 8;
    ticks = scenario.ticks == 0 ? 0 : scenario.ticks == 1 ? ticks + step : ticks + 20 * step;
    printf("K %" PRIu64 "\n", ticks);
    return ticks;
}
void sexos_yield(void) {
    printf("Y\n");
    if (++yields >= scenario.limit) { printf("END\n"); exit(0); }
}
SexosMessage sexos_listen_once(uint64_t slot) {
    SexosMessage m = {0, 0, 0, 0, 0, 0};
    uint64_t c = 0;
    if (next() % 100 < scenario.listen) {
        uint64_t kind = next() % 20;
        c = PICK(3, 6, 9, 10, 11, 12, 13, 0);
        uint64_t notify_cut = scenario.mix == 1 ? 12 : 7;
        if (kind < notify_cut) {
            m.type_id = 0xC0;
            uint64_t category = PICK(0, 1, 5, 6, 7), urgency = PICK(0, 0, 1, 3, 4);
            uint64_t privacy = PICK(0, 0, 1, 3, 4), redaction = PICK(0, 2, 3, 5);
            m.arg0 = category | urgency << 8 | privacy << 16 | redaction << 24;
            uint64_t actions = PICK(0, 0, 1, 1, 2);
            m.arg1 = actions | PICK(0, 1, 7) << 8;
            uint64_t refs = PICK(0, 0, 1, 2);
            m.arg2 = refs | PICK(0, 0xC3, 0xFF) << 8;
        } else {
            switch (kind - notify_cut) {
            case 0: case 1: { m.type_id = 0xC3; uint64_t lane = PICK(0xFF, 0, 1, 3, 5, 6); m.arg0 = lane | PICK(1, 2, 4, 0, 5) << 8; c = PICK(3, 6, 6, 9); break; }
            case 2: case 3: m.type_id = 0xC1; m.arg0 = next() % 40; break;
            case 4: m.type_id = 0xC2; m.arg0 = next() % 40; m.arg1 = PICK(0, 1, 7); break;
            case 5: m.type_id = 0xC4; m.arg0 = PICK(0xFF, 0, 3, 4, 6); break;
            case 6: { m.type_id = 0xC7; uint64_t pd = next() % 2 == 0 ? PICK(9, 10, 11, 12, 3) : 100 + next() % 24;
                      m.arg0 = pd | PICK(0, 0, 1, 2) << 32; break; }
            case 7: case 8: {
                m.type_id = 0xC6; c = PICK(3, 3, 3, 6); m.arg0 = PICK(9, 10, 11, 12, 13, 14, 15, 16, 17);
                uint64_t flags = PICK(0, 1, 2, 3, 4, 5, 7), privacy = PICK(0, 1, 2, 3), lane = PICK(0, 2, 4, 5, 6, 7);
                m.arg1 = flags | privacy << 8 | lane << 16 | PICK(0, 1) << 24;
                break;
            }
            case 9: case 10: case 11: m.type_id = 0xC5; c = PICK(3, 6, 9); break;
            default: m.type_id = PICK(0xC8, 0x11, 0xB0); break;
            }
        }
    }
    m.caller_pd = (uint32_t)c;
    printf("T %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 "\n", slot, m.type_id, c, m.arg0, m.arg1, m.arg2);
    return m;
}
int main(int argc, char **argv) {
    if (argc != 6) return 2;
    uint64_t *fields[5] = {&scenario.seed, &scenario.limit, &scenario.listen, &scenario.ticks, &scenario.mix};
    for (int i = 0; i < 5; ++i) *fields[i] = strtoull(argv[i + 1], NULL, 10);
    rng = scenario.seed;
    sexbell_main();
    return 0;
}
'''


def rust_reference(source):
    for old, new in (("#![no_std]", ""), ("#![no_main]", ""),
                     ("#[panic_handler]\nfn panic(_: &core::panic::PanicInfo) -> ! {\n    loop {}\n}\n", ""),
                     ('#[no_mangle]\npub extern "C" fn _start() -> ! {', "fn sexbell_start() -> ! {")):
        assert source.count(old) == 1, old
        source = source.replace(old, new)
    return source


def scenarios(count):
    r = random.Random(0xBE11)
    return [[r.getrandbits(64), [60, 500, 3000][k % 3], [20, 60, 95][(k // 3) % 3], (k // 2) % 3, (k // 4) % 2]
            for k in range(count)]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cesta", default="cesta")
    parser.add_argument("--cases", type=int, default=36)
    parser.add_argument("--output", type=Path, default=HERE / "build/sexbell-differential")
    args = parser.parse_args()
    cesta = shutil.which(args.cesta) or args.cesta
    work = args.output
    if work.exists():
        shutil.rmtree(work)
    work.mkdir(parents=True)
    show = lambda path: run(["git", "-C", REPO, "show", f"{REVISION}:{path}"]).stdout.decode()
    listen = pinned_function(show("crates/sex-pdx/src/lib.rs"), "pub fn pdx_listen_raw(slot: u64) -> PdxMessage {", LISTEN)
    rust = work / "reference.rs"
    rust.write_text(RUST_PROVIDER + listen + "}\n" + rust_reference(show("servers/sexbell/src/main.rs")) + RUST_MAIN)
    (work / "provider.c").write_text(C_PROVIDER)
    include = ["-I", HERE / "generated/include"]
    runtime = [HERE / "runtime/client_runtime.c", HERE / "runtime/server_runtime.c",
               *sorted((HERE / "generated/runtime").glob("*.c"))]
    sanitizers = ["-fsanitize=address,undefined", "-fno-sanitize-recover=all"]
    warnings = ["-Wall", "-Wextra", "-Wpedantic", "-Wformat=2", "-Werror"]
    cases = scenarios(args.cases)
    compared = 0
    references = {}
    for variant in VARIANTS:
        tag = "-".join(name.removeprefix("SEXOS_").lower() for name in variant) or "default"
        env = dict(os.environ)
        for name in variant:
            env[name] = "1"
        reference = work / f"reference-{tag}"
        run(["rustc", "--edition", "2021", "-O", "-A", "warnings", rust, "-o", reference], env=env)
        harnesses = []
        for compiler in ("gcc", "clang"):
            obj = work / f"sexbell-{compiler}-{tag}.o"
            run([cesta, "cc", "--cc", compiler, "-std=c11", "-O2", *warnings, *(f"-D{n}" for n in variant), *sanitizers, *include,
                 "-c", HERE / "src/sexbell.cesta", "-o", obj])
            harness = work / f"harness-{compiler}-{tag}"
            run([compiler, "-std=c11", "-O2", "-DSEXOS_HOST", *sanitizers, *include, work / "provider.c", obj, *runtime, "-o", harness])
            harnesses.append((compiler, harness))
        for number, case in enumerate(cases):
            expected = run([reference, *case]).stdout
            references[variant, number] = expected
            for compiler, harness in harnesses:
                actual = run([harness, *case]).stdout
                if actual != expected:
                    raise SystemExit(first_difference(actual, expected, f"variant {tag} scenario {number} {compiler}"))
            compared += len(expected.splitlines())
    drops = sum(r.count(b"[bell.queue.drop]") for r in references.values())
    assert drops > 0, "no scenario evicted a queued event"
    assert any(b"reason=mute_list_full" in r for r in references.values()), "no scenario filled the mute list"
    # Controls, including the tie rule: keeping the oldest of equal lanes, as the
    # Rust's comment says, must differ from what its code does.
    original = (HERE / "src/sexbell.cesta").read_text()
    mutations = [
        ("(&& (== (-> entry final_lane) lowest_lane) (|| (! found) (> (cast uint16_t i) lowest_dist))))",
         "(&& (== (-> entry final_lane) lowest_lane) (! found)))"),
        ("(= ([] (-> q entries) drop_idx) entry)", "(= ([] (-> q entries) (% (+ drop_idx 1) BELL_QUEUE_CAPACITY)) entry)"),
        ("(define SPAM_MAX_PER_WINDOW \"UINT32_C(8)\")", "(define SPAM_MAX_PER_WINDOW \"UINT32_C(7)\")"),
        ("(return (|| (== caller_pd 3) (== caller_pd 6))))", "(return (== caller_pd 3)))"),
        ("(return (?: (> (-> entry privacy_level) event_privacy) (-> entry privacy_level) event_privacy)))",
         "(return (-> entry privacy_level)))"),
        ("(= final_lane LANE_SYSTEM)", "(= final_lane LANE_NOW)"),
        ("(|= packed (<< (cast uint64_t (& redact_count 0xFF)) 56))", "(|= packed (<< (cast uint64_t (& redact_count 0xFF)) 48))"),
        ("(if (> dismiss_count 0) (call bump_generation (& (-> s generation))))", "(call bump_generation (& (-> s generation)))"),
        ("(if (> (-> entry privacy_level) caller_max_privacy)\n            (block\n              (if (== (-> entry privacy_level) 3) (+= redact_count 1))",
         "(if (> (-> entry privacy_level) caller_max_privacy)\n            (block\n              (if (>= (-> entry privacy_level) 2) (+= redact_count 1))"),
        ("(= ([] (-> s policy_table) j) ([] (-> s policy_table) (+ j 1))))", "(= ([] (-> s policy_table) j) ([] (-> s policy_table) j)))"),
    ]
    for number, (old_text, new_text) in enumerate(mutations):
        assert original.count(old_text) == 1, old_text
        mutant = work / f"sexbell-mutant-{number}.cesta"
        mutant.write_text(original.replace(old_text, new_text))
        run([cesta, "cc", "--cc", "gcc", "-std=c11", "-O2", *include, "-c", mutant, "-o", work / "mutant.o"])
        run(["gcc", "-std=c11", "-O2", "-DSEXOS_HOST", *include, work / "provider.c", work / "mutant.o", *runtime, "-o", work / "mutant-harness"])
        if not any(run([work / "mutant-harness", *case]).stdout != references[(), index] for index, case in enumerate(cases)):
            raise SystemExit(f"mutation not detected: {old_text} -> {new_text}")
    print(f"sexbell: Cesta translation equals the Rust server on {compared} trace lines ({args.cases} scenarios x {len(VARIANTS)} "
          f"build variants, GCC and Clang, address and UB sanitizers; {drops} evictions); {len(mutations)} mutations detected")


if __name__ == "__main__":
    main()
