#!/usr/bin/env python3
"""Compare servers/sexstore with its Cesta translation, in two parts.

The Rust server is compiled from the pinned revision with only its no_std
scaffolding removed: the attributes, the panic handler, the exported entry,
and the global allocator it installs but never uses. crates/sex-pdx is
replaced by a scripted provider whose pdx_listen_raw is the pinned function
with its syscall routed to the provider.

The durable part runs the pinned durable_* functions and the Cesta durable
layer (over the checked page codec) on the same seeded regions: empty, one
valid page, two pages with chosen sequence numbers in one non-wrapping window,
random bit flips, and damage to some or all records under a recomputed page
checksum. Each
case loads into a seeded table, writes snapshots back, and prints the logs,
the loaded table and the whole region.

The pinned Rust has a recovery defect at sequence wraparound. Full-width
cyclic histories are therefore checked against an independent logical-clock
oracle in sexstore_versions.py, including an explicit legacy regression.

The server part runs both servers against seeded callers: PUT, GET and DEL
from silk-shell and from other domains, keys across the invalid, shell and
reserved ranges (and above 32 bits), envelopes for key 1 that pass or fail,
values with bit 63, unknown opcodes and empty listens. It compares every log
line, listen result, reply and yield, with and without the KV contract proof.
A generation wraps from 255 to 1 only after 254 writes to one slot, when every
log budget is long spent and replies carry no generations, so no trace can
show the wrap; that line is checked by reading it. Two branches of the Rust
are unreachable here as well: no caller can fill the table (the shell owns
15 keys, the table has 16 slots; only the KV proof fills it, by raw puts), so
the full and reclaim paths never run, and a RAM-backed write always verifies.
"""

import argparse
import os
from pathlib import Path
import random
import shutil

from differential_support import LISTEN, first_difference, pinned_function, run
from layout_text import LayoutText

HERE = Path(__file__).resolve().parent.parent
REPO = HERE.parent
REVISION = "4a040a86"
VARIANTS = [(), ("SEXOS_SEXSTORE_KV_PROOF",)]

RUST_PROVIDER = r'''
use std::cell::RefCell;
#[derive(Clone, Copy)]
struct Scenario { seed: u64, limit: u64, listen: u64 }
struct State { rng: u64, yields: u64, scenario: Scenario }
thread_local! { static STATE: RefCell<State> = const { RefCell::new(State { rng: 0, yields: 0,
    scenario: Scenario { seed: 0, limit: 0, listen: 0 } }) }; }
fn next(s: &mut State) -> u64 {
    s.rng = s.rng.wrapping_add(0x9E3779B97F4A7C15);
    let mut z = s.rng;
    z = (z ^ (z >> 30)).wrapping_mul(0xBF58476D1CE4E5B9);
    z = (z ^ (z >> 27)).wrapping_mul(0x94D049BB133111EB);
    z ^ (z >> 31)
}
fn rng() -> u64 { STATE.with(|cell| next(&mut *cell.borrow_mut())) }
fn envelope(q: u64) -> u64 {
    let mut b = q.to_le_bytes();
    b[0] = 0xAC; b[1] = 0x01;
    b[7] = (b[0] ^ b[1] ^ b[2] ^ b[3] ^ b[4] ^ b[5] ^ b[6]) & 0x7F;
    u64::from_le_bytes(b)
}
fn listen_once(slot: u64) -> (u64, u64, u64, u64, u64) {
    STATE.with(|cell| {
        let s = &mut *cell.borrow_mut();
        let (mut t, mut c, mut a0, mut a1) = (0u64, 0u64, 0u64, 0u64);
        if next(s) % 100 < s.scenario.listen {
            let r = next(s);
            let q = next(s);
            let pick = next(s) % 20;
            t = if pick < 8 { 0xB1 } else if pick < 14 { 0xB0 } else if pick < 18 { 0xB2 } else if pick == 18 { 0xB3 } else { 0x260 };
            c = [3u64, 3, 3, 3, 3, 3, 3, 0, 4, 99][((r >> 8) % 10) as usize];
            a0 = [1u64, 1, 2, 3, 4, 5, 7, 9, 12, 15, 0, 16, 0x80, (1 << 32) | 2, q & 0xFFFF_FFFF][((r >> 16) % 15) as usize];
            a1 = match (r >> 24) % 4 { 0 => envelope(q), 1 => q & 0x7FFF_FFFF_FFFF_FFFF, 2 => q | (1 << 63), _ => r % 100 };
        }
        println!("T {slot} {t} {c} {a0} {a1} 0");
        (t, c, a0, a1, 0)
    })
}
mod sex_pdx {
    macro_rules! serial_println { ($($argument:tt)*) => { println!("L {}", format!($($argument)*)) }; }
    pub(crate) use serial_println;
    pub const SLOT_SEXSTORE: u64 = 10;
    pub struct PdxMessage { pub type_id: u64, pub arg0: u64, pub arg1: u64, pub arg2: u64, pub caller_pd: u32, _pad: u32 }
    pub fn pdx_reply(target_pd: u32, value: u64) -> u64 { println!("R {target_pd} {value}"); 0 }
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
static struct { uint64_t seed, limit, listen; } scenario;
static uint64_t rng, yields;
static uint64_t next(void) {
    rng += 0x9E3779B97F4A7C15ull;
    uint64_t z = rng;
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
    return z ^ (z >> 31);
}
static uint64_t envelope(uint64_t q) {
    uint8_t b[8];
    for (int i = 0; i < 8; ++i) b[i] = (uint8_t)(q >> (8 * i));
    b[0] = 0xAC; b[1] = 0x01;
    b[7] = (uint8_t)((b[0] ^ b[1] ^ b[2] ^ b[3] ^ b[4] ^ b[5] ^ b[6]) & 0x7F);
    uint64_t v = 0;
    for (int i = 0; i < 8; ++i) v |= (uint64_t)b[i] << (8 * i);
    return v;
}
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
uint64_t sexos_reply(uint32_t target_pd, uint64_t value) {
    printf("R %" PRIu32 " %" PRIu64 "\n", target_pd, value);
    return 0;
}
void sexos_yield(void) {
    printf("Y\n");
    if (++yields >= scenario.limit) { printf("END\n"); exit(0); }
}
SexosMessage sexos_listen_once(uint64_t slot) {
    SexosMessage m = {0, 0, 0, 0, 0, 0};
    if (next() % 100 < scenario.listen) {
        uint64_t r = next(), q = next(), pick = next() % 20;
        static const uint64_t callers[10] = {3, 3, 3, 3, 3, 3, 3, 0, 4, 99};
        const uint64_t keys[15] = {1, 1, 2, 3, 4, 5, 7, 9, 12, 15, 0, 16, 0x80, (1ull << 32) | 2, q & 0xFFFFFFFF};
        m.type_id = pick < 8 ? 0xB1 : pick < 14 ? 0xB0 : pick < 18 ? 0xB2 : pick == 18 ? 0xB3 : 0x260;
        m.caller_pd = (uint32_t)callers[(r >> 8) % 10];
        m.arg0 = keys[(r >> 16) % 15];
        switch ((r >> 24) % 4) {
        case 0: m.arg1 = envelope(q); break;
        case 1: m.arg1 = q & 0x7FFFFFFFFFFFFFFFull; break;
        case 2: m.arg1 = q | (1ull << 63); break;
        default: m.arg1 = r % 100; break;
        }
    }
    printf("T %" PRIu64 " %" PRIu64 " %" PRIu32 " %" PRIu64 " %" PRIu64 " %" PRIu64 "\n",
           slot, m.type_id, m.caller_pd, m.arg0, m.arg1, m.arg2);
    return m;
}
'''

C_SERVER_MAIN = r'''
int main(int argc, char **argv) {
    if (argc != 4) return 2;
    scenario.seed = strtoull(argv[1], NULL, 10);
    scenario.limit = strtoull(argv[2], NULL, 10);
    scenario.listen = strtoull(argv[3], NULL, 10);
    rng = scenario.seed;
    sexstore_main();
    return 0;
}
'''

RUST_SERVER_MAIN = r'''
fn main() {
    let v: Vec<u64> = std::env::args().skip(1).map(|a| a.parse().unwrap()).collect();
    STATE.with(|cell| {
        let s = &mut *cell.borrow_mut();
        s.scenario = Scenario { seed: v[0], limit: v[1], listen: v[2] };
        s.rng = v[0];
    });
    sexstore_start()
}
'''

# The durable cases, written once per language over the same generator.
RUST_DURABLE_MAIN = r'''
fn random_table() -> [KvSlot; 16] {
    let mut t = [KvSlot { state: 0, generation: 0, key: 0, val: 0 }; 16];
    for slot in t.iter_mut() {
        slot.state = (rng() % 4) as u8;
        slot.generation = (rng() % 256) as u8;
        slot.key = if rng() % 2 == 0 { (rng() % 16) as u32 } else { rng() as u32 };
        slot.val = rng();
    }
    t
}
fn choose_seq() -> u32 {
    let pick = rng() % 6;
    if pick < 5 { [1u32, 2, 7, 1000, 999][pick as usize] } else { (rng() % 1024) as u32 }
}
fn dump(label: &str, t: &[KvSlot; 16]) {
    let mut line = String::from(label);
    for s in t.iter() { line += &format!(" {}:{}:{}:{}", s.state, s.generation, s.key, s.val); }
    println!("{line}");
}
fn region_hex() -> String { unsafe { (*core::ptr::addr_of!(DURABLE_REGION)).iter().map(|b| format!("{:02x}", b)).collect() } }
fn main() {
    let v: Vec<u64> = std::env::args().skip(1).map(|a| a.parse().unwrap()).collect();
    STATE.with(|cell| cell.borrow_mut().rng = v[0]);
    for case in 0..v[1] {
        unsafe {
            DURABLE_REGION = [0u8; 1024];
            LOG_DURABLE_WRITE = 16;
            LOG_DURABLE_WRITE_FAIL = 8;
            let region = &mut *core::ptr::addr_of_mut!(DURABLE_REGION);
            let mode = rng() % 7;
            if mode >= 1 { let t = random_table(); let seq = choose_seq(); region[..512].copy_from_slice(&durable_build_page(&t, seq)); }
            if mode >= 2 { let t = random_table(); let seq = choose_seq(); region[512..].copy_from_slice(&durable_build_page(&t, seq)); }
            if mode == 3 || mode == 4 {
                for _ in 0..(1 + rng() % 4) { let pos = (rng() % 1024) as usize; region[pos] ^= 1 << (rng() % 8); }
            }
            if mode == 5 {
                for _ in 0..(1 + rng() % 4) { let pos = 16 + (rng() % 496) as usize; region[pos] ^= 1 << (rng() % 8); }
            }
            if mode == 6 {
                for i in 0..16 { let pos = 16 + i * 24 + (rng() % 24) as usize; region[pos] ^= 1 << (rng() % 8); }
            }
            if mode >= 5 {
                region[8..12].copy_from_slice(&[0u8; 4]);
                let crc = crc32c(&region[..512]);
                region[8..12].copy_from_slice(&crc.to_le_bytes());
            }
            println!("CASE {case} mode={mode}");
            println!("G {}", region_hex());
            let mut table = if rng() % 2 == 0 { [KvSlot { state: 0, generation: 0, key: 0, val: 0 }; 16] } else { random_table() };
            if rng() % 4 != 0 { println!("I {}", durable_init(&table) as u8); }
            let (total, valid, corrupt) = durable_load_into_ram(&mut table);
            println!("S {total} {valid} {corrupt}");
            dump("K", &table);
            for _ in 0..(rng() % 5) {
                let j = (rng() % 16) as usize;
                table[j].state = (rng() % 3) as u8;
                table[j].generation = (rng() % 256) as u8;
                table[j].key = (rng() % 16) as u32;
                table[j].val = rng();
                println!("W {}", durable_write_all(&table) as u8);
            }
            println!("E {}", region_hex());
        }
    }
}
'''

C_DURABLE_MAIN = r'''
#include "sexstore.h"
static KvTable random_table(void) {
    KvTable t;
    for (size_t i = 0; i < KV_SLOT_COUNT; ++i) {
        KvSlot *slot = &t.slots[i];
        slot->state = (uint8_t)(next() % 4);
        slot->generation = (uint8_t)(next() % 256);
        slot->key = next() % 2 == 0 ? (uint32_t)(next() % 16) : (uint32_t)next();
        slot->val = next();
    }
    return t;
}
static uint32_t choose_seq(void) {
    uint64_t pick = next() % 6;
    const uint32_t seqs[5] = {1, 2, 7, 1000, 999};
    return pick < 5 ? seqs[pick] : (uint32_t)(next() % 1024);
}
static void dump(const char *label, const KvTable *t) {
    printf("%s", label);
    for (size_t i = 0; i < KV_SLOT_COUNT; ++i)
        printf(" %u:%u:%" PRIu32 ":%" PRIu64, t->slots[i].state, t->slots[i].generation, t->slots[i].key, t->slots[i].val);
    printf("\n");
}
static void region_hex(const char *label, const Durable *d) {
    printf("%s ", label);
    for (size_t i = 0; i < sizeof d->region; ++i) printf("%02x", d->region[i]);
    printf("\n");
}
int main(int argc, char **argv) {
    if (argc != 3) return 2;
    rng = strtoull(argv[1], NULL, 10);
    uint64_t cases = strtoull(argv[2], NULL, 10);
    for (uint64_t c = 0; c < cases; ++c) {
        Durable d = sexstore_durable_new();
        uint64_t mode = next() % 7;
        if (mode >= 1) { KvTable t = random_table(); uint32_t seq = choose_seq(); DurablePage p = sexstore_durable_build_page(&t, seq); memcpy(d.region, p.bytes, 512); }
        if (mode >= 2) { KvTable t = random_table(); uint32_t seq = choose_seq(); DurablePage p = sexstore_durable_build_page(&t, seq); memcpy(d.region + 512, p.bytes, 512); }
        if (mode == 3 || mode == 4) {
            uint64_t flips = 1 + next() % 4;
            for (uint64_t k = 0; k < flips; ++k) { size_t pos = (size_t)(next() % 1024); d.region[pos] ^= (uint8_t)(1u << (next() % 8)); }
        }
        if (mode == 5) {
            uint64_t flips = 1 + next() % 4;
            for (uint64_t k = 0; k < flips; ++k) { size_t pos = 16 + (size_t)(next() % 496); d.region[pos] ^= (uint8_t)(1u << (next() % 8)); }
        }
        if (mode == 6) {
            for (size_t i = 0; i < 16; ++i) { size_t pos = 16 + i * 24 + (size_t)(next() % 24); d.region[pos] ^= (uint8_t)(1u << (next() % 8)); }
        }
        if (mode >= 5) {
            memset(d.region + 8, 0, 4);
            uint32_t crc = sexstore_pages_crc32c((cesta_read_bytes){d.region, 512});
            for (int i = 0; i < 4; ++i) d.region[8 + i] = (uint8_t)(crc >> (8 * i));
        }
        printf("CASE %" PRIu64 " mode=%" PRIu64 "\n", c, mode);
        region_hex("G", &d);
        KvTable table;
        if (next() % 2 == 0) memset(&table, 0, sizeof table); else table = random_table();
        if (next() % 4 != 0) printf("I %u\n", (unsigned)sexstore_durable_init(&d, &table));
        DurableLoad load = sexstore_durable_load_into_ram(&d, &table);
        printf("S %" PRIu32 " %" PRIu32 " %" PRIu32 "\n", load.total, load.valid, load.corrupt);
        dump("K", &table);
        uint64_t writes = next() % 5;
        for (uint64_t w = 0; w < writes; ++w) {
            size_t j = (size_t)(next() % 16);
            table.slots[j].state = (uint8_t)(next() % 3);
            table.slots[j].generation = (uint8_t)(next() % 256);
            table.slots[j].key = (uint32_t)(next() % 16);
            table.slots[j].val = next();
            printf("W %u\n", (unsigned)sexstore_durable_write_all(&d, &table));
        }
        region_hex("E", &d);
    }
    return 0;
}
'''


def rust_reference(source):
    # Only scaffolding changes; the allocator is installed but never used, so the
    # hosted build keeps the standard one.
    for line in ("#![no_std]", "#![no_main]", "#![feature(alloc_error_handler)]",
                 "use linked_list_allocator::LockedHeap;",
                 "#[global_allocator]\nstatic ALLOCATOR: LockedHeap = LockedHeap::empty();",
                 "#[alloc_error_handler]\nfn alloc_error_handler(_layout: core::alloc::Layout) -> ! {\n    loop {}\n}",
                 "#[panic_handler]\nfn panic(_info: &core::panic::PanicInfo) -> ! {\n    loop {}\n}"):
        assert source.count(line) == 1, line
        source = source.replace(line, "")
    entry = '#[no_mangle]\npub extern "C" fn _start() -> ! {'
    assert source.count(entry) == 1
    return source.replace(entry, "fn sexstore_start() -> ! {")


def server_scenarios(count):
    r = random.Random(0x5E57)
    return [[r.getrandbits(64), [50, 400, 3000, 12000][k % 4], [10, 40, 90][(k // 4) % 3]] for k in range(count)]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cesta", default="cesta")
    parser.add_argument("--cases", type=int, default=24)
    parser.add_argument("--durable-cases", type=int, default=3000)
    parser.add_argument("--output", type=Path, default=HERE / "build/sexstore-differential")
    args = parser.parse_args()
    cesta = shutil.which(args.cesta) or args.cesta
    work = args.output
    if work.exists():
        shutil.rmtree(work)
    work.mkdir(parents=True)
    show = lambda path: run(["git", "-C", REPO, "show", f"{REVISION}:{path}"]).stdout.decode()
    listen = pinned_function(show("crates/sex-pdx/src/lib.rs"), "pub fn pdx_listen_raw(slot: u64) -> PdxMessage {", LISTEN)
    provider = RUST_PROVIDER + listen + "}\n"  # closes mod sex_pdx
    server = rust_reference(show("servers/sexstore/src/main.rs"))
    include = ["-I", HERE / "generated/include"]
    runtime = [HERE / "runtime/client_runtime.c", HERE / "runtime/server_runtime.c",
               *sorted((HERE / "generated/runtime").glob("*.c"))]
    sanitizers = ["-fsanitize=address,undefined", "-fno-sanitize-recover=all"]
    warnings = ["-Wall", "-Wextra", "-Wpedantic", "-Wformat=2", "-Werror"]

    def objects(compiler, defines, tag, sources=("sexstore", "sexstore_durable", "sexstore_pages"), extra=()):
        out = []
        for unit in sources:
            obj = work / f"{unit}-{compiler}-{tag}.o"
            run([cesta, "cc", "--cc", compiler, "-std=c11", "-O2", *warnings, *defines, *extra, *include,
                 "-c", HERE / "src" / f"{unit}.cesta", "-o", obj])
            out.append(obj)
        return out

    # Durable layer.
    rust = work / "durable.rs"
    rust.write_text(provider + server + RUST_DURABLE_MAIN)
    run(["rustc", "--edition", "2021", "-O", "-A", "warnings", rust, "-o", work / "durable-reference"])
    (work / "durable.c").write_text(C_PROVIDER + "#include <string.h>\n" + C_DURABLE_MAIN)
    seed = 0xD0AB1E
    expected_durable = run([work / "durable-reference", seed, args.durable_cases]).stdout
    durable_lines = 0
    for compiler in ("gcc", "clang"):
        objs = objects(compiler, [], "durable", ("sexstore_durable", "sexstore_pages"), sanitizers)
        harness = work / f"durable-{compiler}"
        run([compiler, "-std=c11", "-O2", "-DSEXOS_HOST", *sanitizers, *include, work / "durable.c", *objs, *runtime,
             "-o", harness])
        actual = run([harness, seed, args.durable_cases]).stdout
        if actual != expected_durable:
            raise SystemExit(first_difference(actual, expected_durable, f"durable {compiler}"))
        durable_lines += len(expected_durable.splitlines())
    loads = [line for line in expected_durable.decode().splitlines() if line.startswith("S ")]
    partial = sum(1 for line in loads if line.split()[2] not in ("0", "16"))
    assert partial > 0 and any(line == "S 0 0 0" for line in loads), "durable cases miss partial or empty loads"
    assert b"[sexstore.durable.all_corrupt]" in expected_durable, "durable cases miss an all-corrupt page"

    # Server.
    (work / "server.c").write_text(C_PROVIDER + C_SERVER_MAIN)
    rust = work / "server.rs"
    rust.write_text(provider + server + RUST_SERVER_MAIN)
    cases = server_scenarios(args.cases)
    compared = 0
    references = {}
    for variant in VARIANTS:
        tag = "proof" if variant else "default"
        env = dict(os.environ)
        for name in variant:
            env[name] = "1"
        reference = work / f"server-reference-{tag}"
        run(["rustc", "--edition", "2021", "-O", "-A", "warnings", rust, "-o", reference], env=env)
        harnesses = []
        for compiler in ("gcc", "clang"):
            objs = objects(compiler, [f"-D{name}" for name in variant], tag, extra=sanitizers)
            harness = work / f"server-{compiler}-{tag}"
            run([compiler, "-std=c11", "-O2", "-DSEXOS_HOST", *sanitizers, *include, work / "server.c", *objs, *runtime,
                 "-o", harness])
            harnesses.append((compiler, harness))
        for number, case in enumerate(cases):
            expected = run([reference, *case]).stdout
            references[variant, number] = expected
            for compiler, harness in harnesses:
                actual = run([harness, *case]).stdout
                if actual != expected:
                    raise SystemExit(first_difference(actual, expected, f"server {tag} scenario {number} {compiler}"))
            compared += len(expected.splitlines())

    # Controls: each change to the translation must change a durable or server trace.
    mutations = [
        ("durable", "sexstore_pages", "(u32 2197175160)", "(u32 2197175161)"),
        ("durable", "sexstore_pages", "(u16 32773)", "(u16 32771)"),
        ("durable", "sexstore_pages", "(slice tail read page (usize 12) (usize 500))", "(slice tail read page (usize 13) (usize 499))"),
        ("durable", "sexstore_pages", "(return (le (at page (add offset (usize 8))) (u8 2)))", "(return (le (at page (add offset (usize 8))) (u8 3)))"),
        ("durable", "sexstore_durable", "(if (|| (== second 0) (== first second)) (return true))",
                                        "(if (== first second) (return false)) (if (== second 0) (return true))"),
        ("server", "sexstore", "(if (<= key 0x0F) (return 1))", "(if (<= key 0x0E) (return 1))"),
        ("server", "sexstore", "(if (!= ([] b 7) (& chk 0x7F)) (return false))", "(if (!= ([] b 7) (& chk 0xFF)) (return false))"),
        ("server", "sexstore", "(= (. log_get_reject) 16)", "(= (. log_get_reject) 15)"),
        ("server", "sexstore", "(= (-> slot state) SLOT_TOMBSTONED)\n          (call bump_generation slot)",
                               "(= (-> slot state) SLOT_TOMBSTONED)"),
    ]
    for number, (part, unit, old_text, new_text) in enumerate(mutations):
        original = LayoutText((HERE / "src" / f"{unit}.cesta").read_text())
        assert original.count(old_text) == 1, old_text
        mutant = work / f"{unit}-mutant-{number}.cesta"
        mutant.write_text(original.replace(old_text, new_text))
        units = ("sexstore_durable", "sexstore_pages") if part == "durable" else ("sexstore", "sexstore_durable", "sexstore_pages")
        objs = []
        for name in units:
            source = mutant if name == unit else HERE / "src" / f"{name}.cesta"
            obj = work / f"mutant-{number}-{name}.o"
            run([cesta, "cc", "--cc", "gcc", "-std=c11", "-O2", *include, "-c", source, "-o", obj])
            objs.append(obj)
        harness = work / f"mutant-{number}"
        run(["gcc", "-std=c11", "-O2", "-DSEXOS_HOST", *include, work / f"{part}.c", *objs, *runtime, "-o", harness])
        if part == "durable":
            detected = run([harness, seed, args.durable_cases]).stdout != expected_durable
        else:
            detected = any(run([harness, *case]).stdout != references[(), index] for index, case in enumerate(cases))
        if not detected:
            raise SystemExit(f"mutation not detected: {unit}: {old_text} -> {new_text}")
    print(f"sexstore: Cesta translation equals the Rust server on {compared} server trace lines "
          f"({args.cases} scenarios x {len(VARIANTS)} build variants) and {durable_lines} durable-layer lines "
          f"({args.durable_cases} seeded regions), GCC and Clang, address and UB sanitizers; "
          f"{len(mutations)} mutations detected")


if __name__ == "__main__":
    main()
