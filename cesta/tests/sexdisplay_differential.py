#!/usr/bin/env python3
"""Compare servers/sexdisplay with its Cesta translation, pixels included.

The Rust compositor is compiled from the pinned revision against the pinned
silkbar-model crate. Only its no_std scaffolding is removed, and the two
platform addresses (the fallback framebuffer and the high-half base) move to
fixed host mappings. crates/sex-pdx is replaced by a scripted provider that
keeps the pinned pdx_try_listen_raw with its listen syscall routed to it. The
Cesta compositor runs on the client runtime with the same provider in C and
the same two addresses given at build time.

Both print one line per log line, listen result, tick read and yield, plus a
hash of each framebuffer whenever it changed since the last yield. The seeded
provider hands over a live framebuffer of the scenario's size (and refuses
first with low or oversized ones), then sends SilkBar updates (clocks forward,
backward and out of range), surface creation, moves (cursor moves tagged with
sequence and shell tick), focus, destruction, window-manager registration,
fill rectangles, text (newlines, lowercase, high offsets), appearance tokens,
tab info with chrome and hover flags, the legacy opcodes and unknown ones,
from owners, the window manager and other domains. Ticks are absent, real,
stalled, or real then stalled. The build variants cover the five flags.
The glass blur runs over a vertical gradient whose rows differ by at most one
per channel, so the extent of its radius-1 window cannot show in any frame;
its arithmetic can, and is controlled.
"""

import argparse
from itertools import combinations
import os
from pathlib import Path
import random
import shutil

from differential_support import LISTEN, first_difference, pinned_function, run, run_commands

HERE = Path(__file__).resolve().parent.parent
REPO = HERE.parent
REVISION = "4a040a86"
FLAGS = ["SEXOS_SILKBAR_PHASE3_RECEIVE_PROOF", "SEXOS_SILKBAR_PHASE5_PIXEL_PROOF", "SEXOS_SILK_RENDER_PROOF_PROFILE",
         "SEXOS_ATLAS_PHASE_C_RENDER_STUB_PROOF", "SEXOS_ATLAS_PHASE_D_FRAME_PREVIEW_STUB_PROOF"]
DEFAULT_VARIANTS = [(), *((flag,) for flag in FLAGS), tuple(FLAGS)]
FALLBACK = 0x600000000000
LIVE = 0x610000000000
HIGH_HALF = 0x500000000000

# A scenario is seed, limit (yields), listen (percent of non-empty listens),
# ticks (0 absent, 1 real, 2 stalled, 3 real then stalled, 4 absent then real,
# 5 jumps of a full minute), fb_at (the yield that brings the live framebuffer;
# 0 never), fb_w, fb_h, script (1 opens with SCRIPT before the seeded messages).
RUST_PROVIDER = r'''
use std::cell::RefCell;
#[derive(Clone, Copy)]
struct Scenario { seed: u64, limit: u64, listen: u64, ticks: u64, fb_at: u64, fb_w: u64, fb_h: u64, script: u64 }
struct State { rng: u64, yields: u64, ticks: u64, fb_sent: bool, seq: u64, live_hash: u64, scripted: usize, scenario: Scenario }
thread_local! { static STATE: RefCell<State> = const { RefCell::new(State { rng: 0, yields: 0, ticks: 0, fb_sent: false, seq: 0,
    live_hash: 0, scripted: 0, scenario: Scenario { seed: 0, limit: 0, listen: 0, ticks: 0, fb_at: 0, fb_w: 0, fb_h: 0, script: 0 } }) }; }
const FB_FALLBACK: u64 = FALLBACK_VALUE;
const FB_LIVE: u64 = LIVE_VALUE;
extern "C" { fn mmap(addr: *mut u8, len: usize, prot: i32, flags: i32, fd: i32, offset: i64) -> *mut u8; }
fn map_fixed(addr: u64, len: usize) {
    let p = unsafe { mmap(addr as *mut u8, len, 3, 0x02 | 0x20 | 0x100000, -1, 0) };
    assert_eq!(p as u64, addr);
}
fn next(s: &mut State) -> u64 {
    s.rng = s.rng.wrapping_add(0x9E3779B97F4A7C15);
    let mut z = s.rng;
    z = (z ^ (z >> 30)).wrapping_mul(0xBF58476D1CE4E5B9);
    z = (z ^ (z >> 27)).wrapping_mul(0x94D049BB133111EB);
    z ^ (z >> 31)
}
fn hash(addr: u64, pixels: u64) -> u64 {
    let mut h: u64 = 0xcbf29ce484222325;
    for i in 0..pixels { h ^= unsafe { *((addr + 4 * i) as *const u32) } as u64; h = h.wrapping_mul(0x100000001b3); }
    h
}
fn pick(s: &mut State, values: &[u64]) -> u64 { let i = (next(s) % values.len() as u64) as usize; values[i] }
// The opening script: a framed, focused, titled surface, a tagged cursor, a
// window manager adjusting the frame, and text.
const SCRIPT: [(u64, u64, u64, u64, u64); 8] = [
    (0xEC, 3, 200, (60 << 32) | 20, (200 << 32) | 300),
    (0xFD, 3, 200, 2, 1 | (0x21 << 8)),
    (0xED, 3, 200, 0, 0),
    (0xEC, 3, 0x90, (100 << 32) | 100, (16 << 32) | 8),
    (0xEB, 3, 0x90, (1 << 32) | 120, (7 << 32) | 90),
    (0xF5, 4, 0, 0, 0),
    (0xFD, 4, 200, 3, 2 | (0x0F << 8)),
    (0xFB, 3, 200, 0x7571_0A4E_454E_494C, (8 << 8) | (0x00C0FFEE << 32)),
];
fn listen_once(slot: u64) -> (u64, u64, u64, u64, u64) {
    STATE.with(|cell| {
        let s = &mut *cell.borrow_mut();
        let (mut t, mut c, mut a0, mut a1, mut a2) = (0u64, 0u64, 0u64, 0u64, 0u64);
        if !s.fb_sent && s.scenario.fb_at != 0 && s.yields >= s.scenario.fb_at {
            s.fb_sent = true;
            t = 0x11; a0 = FB_LIVE; a1 = (s.scenario.fb_h << 32) | s.scenario.fb_w;
        } else if s.fb_sent && s.scenario.script != 0 && s.scripted < SCRIPT.len() {
            (t, c, a0, a1, a2) = SCRIPT[s.scripted];
            s.scripted += 1;
        } else if next(s) % 100 < s.scenario.listen {
            let kind = next(s) % 32;
            const SIDS: [u64; 9] = [0x90, 0x92, 200, 201, 0x99, 100, 5, 6, 0];
            const CALLERS: [u64; 5] = [3, 3, 3, 4, 7];
            const XS: [u64; 6] = [0, 10, 100, 0xFFFF_FFEC, 300, 1100];
            const YS: [u64; 6] = [0, 20, 51, 60, 200, 0xFFFF_FFFB];
            c = pick(s, &CALLERS);
            match kind {
                0..=5 => {
                    t = 0xF2;
                    a0 = next(s) % 12;
                    if a0 == 4 {
                        a1 = pick(s, &[10, 9, 23, 30, 0]);
                        a2 = (pick(s, &[42, 41, 0, 59, 70]) << 8) | pick(s, &[0, 1, 2, 5, 30, 59, 61, 3]);
                    } else {
                        a1 = ((next(s) % 8) << 32) | (next(s) % 300);
                        a2 = next(s) % 0x1_0000;
                    }
                }
                6..=9 => {
                    t = 0xEC; a0 = pick(s, &SIDS); a1 = (pick(s, &YS) << 32) | pick(s, &XS);
                    a2 = (pick(s, &[0, 8, 40, 100, 300, 5000]) << 32) | pick(s, &[0, 8, 64, 200, 5000, 10000]);
                }
                10..=14 => {
                    t = 0xEB; a0 = pick(s, &SIDS);
                    let tagged = next(s) % 3;
                    if tagged > 0 { s.seq += 1; }
                    let seq = if tagged > 0 { s.seq } else { 0 };
                    let tick = if tagged == 2 { s.seq * 7 } else { 0 };
                    a1 = (seq << 32) | pick(s, &XS); a2 = (tick << 32) | pick(s, &YS);
                }
                15 => { t = 0xED; a0 = pick(s, &SIDS); }
                16 => { t = 0xEE; a0 = pick(s, &SIDS); }
                17 => { t = 0xF5; c = pick(s, &[3, 4, 0]); }
                18 | 19 => {
                    t = 0xEF; a0 = pick(s, &SIDS);
                    a1 = (pick(s, &[0xFFFF_FFF6, 0, 5, 100, 70000]) << 32) | pick(s, &[0xFFFF_FFF6, 0, 5, 100, 70000]);
                    a2 = (pick(s, &[0, 1, 7, 8, 15]) << 56) | ((next(s) & 0xFF_FFFF) << 32) | (pick(s, &[0, 4, 30, 9000]) << 16) | pick(s, &[0, 4, 30, 9000]);
                }
                20 => { t = 0xFA; a0 = pick(s, &SIDS); }
                21..=23 => {
                    t = 0xFB; a0 = pick(s, &SIDS);
                    const ALPHABET: [u64; 12] = [0x41, 0x5A, 0x61, 0x7A, 0x30, 0x0A, 0x20, 0x21, 0x7F, 0x4C, 0x6E, 0x3A];
                    for i in 0..8 { a1 |= pick(s, &ALPHABET) << (8 * i); }
                    let offset = pick(s, &[0, 8, 20, 255, 300, 508, 600]);
                    a2 = (offset & 0xFF) | (next(s) % 16) << 8 | ((offset >> 8) & 0xF) << 12 | ((next(s) & 0xFF_FFFF) << 32);
                }
                24 => { t = 0xFC; a0 = next(s); a1 = next(s); a2 = next(s); }
                25..=27 => {
                    t = 0xFD; a0 = pick(s, &SIDS); a1 = pick(s, &[0, 1, 2, 3, 9]);
                    a2 = pick(s, &[0, 1, 2, 5]) | (pick(s, &[0, 1, 3, 5, 0x0D, 0x21, 0x3F, 0xFF]) << 8);
                }
                28 => { t = 0xE4; a0 = pick(s, &XS); a1 = pick(s, &YS); a2 = (pick(s, &[0, 30, 200]) << 32) | pick(s, &[0, 30, 200]); }
                29 => { t = 0xDE; a0 = next(s); }
                30 => {
                    t = 0x11;
                    match next(s) % 3 { 0 => { a0 = 0x1000; a1 = (100 << 32) | 100; }
                                        1 => { a0 = FB_LIVE; a1 = 100; }
                                        _ => { a0 = FB_LIVE; a1 = (100 << 32) | 9000; } }
                }
                _ => { t = 0x123; }
            }
        }
        println!("T {slot} {t} {c} {a0} {a1} {a2}");
        (t, c, a0, a1, a2)
    })
}
mod sex_pdx {
    macro_rules! serial_println { ($($argument:tt)*) => { println!("L {}", format!($($argument)*)) }; }
    pub(crate) use serial_println;
    pub struct PdxMessage { pub type_id: u64, pub arg0: u64, pub arg1: u64, pub arg2: u64, pub caller_pd: u32, _pad: u32 }
    pub fn get_ticks() -> u64 {
        crate::STATE.with(|cell| {
            let s = &mut *cell.borrow_mut();
            let step = crate::next(s) % 5;
            s.ticks = match s.scenario.ticks {
                0 => 0, 1 => s.ticks + step, 2 => 7, 3 => if s.yields < 300 { s.ticks + step } else { s.ticks },
                4 => if s.yields < 150 { 0 } else { s.ticks + step },
                _ => s.ticks + if step < 2 { 3720 } else { 62 },
            };
            println!("K {}", s.ticks);
            s.ticks
        })
    }
    pub fn sys_yield() {
        crate::STATE.with(|cell| {
            let s = &mut *cell.borrow_mut();
            println!("Y");
            s.yields += 1;
            let live = crate::hash(crate::FB_LIVE, s.scenario.fb_w * s.scenario.fb_h);
            if live != s.live_hash { s.live_hash = live; println!("F live {live:016x}"); }
            if s.yields == 1 { println!("F fallback {:016x}", crate::hash(crate::FB_FALLBACK, 1280 * 800)); }
            if s.yields >= s.scenario.limit {
                println!("F fallback {:016x}", crate::hash(crate::FB_FALLBACK, 1280 * 800));
                println!("END");
                std::process::exit(0);
            }
        })
    }
'''

RUST_MAIN = r'''
fn main() {
    let v: Vec<u64> = std::env::args().skip(1).map(|a| a.parse().unwrap()).collect();
    STATE.with(|cell| {
        let s = &mut *cell.borrow_mut();
        s.scenario = Scenario { seed: v[0], limit: v[1], listen: v[2], ticks: v[3], fb_at: v[4], fb_w: v[5], fb_h: v[6], script: v[7] };
        s.rng = v[0];
    });
    map_fixed(FB_FALLBACK, 1280 * 800 * 4);
    map_fixed(FB_LIVE, 8 * 1024 * 1024);
    sexdisplay_start()
}
'''

C_PROVIDER = r'''
#include "sexos_server.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
static struct { uint64_t seed, limit, listen, ticks, fb_at, fb_w, fb_h, script; } scenario;
static size_t scripted;
static const uint64_t SCRIPT[8][5] = {
    {0xEC, 3, 200, (60ull << 32) | 20, (200ull << 32) | 300},
    {0xFD, 3, 200, 2, 1 | (0x21 << 8)},
    {0xED, 3, 200, 0, 0},
    {0xEC, 3, 0x90, (100ull << 32) | 100, (16ull << 32) | 8},
    {0xEB, 3, 0x90, (1ull << 32) | 120, (7ull << 32) | 90},
    {0xF5, 4, 0, 0, 0},
    {0xFD, 4, 200, 3, 2 | (0x0F << 8)},
    {0xFB, 3, 200, 0x75710A4E454E494Cull, (8 << 8) | (0x00C0FFEEull << 32)},
};
static uint64_t rng, yields, ticks, seq, live_hash;
static bool fb_sent;
static uint64_t next(void) {
    rng += 0x9E3779B97F4A7C15ull;
    uint64_t z = rng;
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
    return z ^ (z >> 31);
}
static uint64_t hash(uint64_t addr, uint64_t pixels) {
    uint64_t h = 0xcbf29ce484222325ull;
    for (uint64_t i = 0; i < pixels; ++i) { h ^= ((const uint32_t *)(uintptr_t)addr)[i]; h *= 0x100000001b3ull; }
    return h;
}
#define PICK(...) pick((const uint64_t[]){__VA_ARGS__}, sizeof((const uint64_t[]){__VA_ARGS__}) / sizeof(uint64_t))
static uint64_t pick(const uint64_t *values, size_t count) { return values[next() % count]; }
static void map_fixed(uint64_t addr, size_t len) {
    void *p = mmap((void *)(uintptr_t)addr, len, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | 0x100000, -1, 0);
    if ((uint64_t)(uintptr_t)p != addr) { printf("MAP FAILED\n"); exit(4); }
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
_Noreturn void sexos_spin_forever(void) { printf("PANIC\n"); exit(0); }
uint64_t sexos_get_ticks(void) {
    uint64_t step = next() % 5;
    switch (scenario.ticks) {
    case 0: ticks = 0; break;
    case 1: ticks += step; break;
    case 2: ticks = 7; break;
    case 3: if (yields < 300) ticks += step; break;
    case 4: ticks = yields < 150 ? 0 : ticks + step; break;
    default: ticks += step < 2 ? 3720 : 62; break;
    }
    printf("K %" PRIu64 "\n", ticks);
    return ticks;
}
void sexos_yield(void) {
    printf("Y\n");
    ++yields;
    uint64_t live = hash(FB_LIVE_ADDR, scenario.fb_w * scenario.fb_h);
    if (live != live_hash) { live_hash = live; printf("F live %016" PRIx64 "\n", live); }
    if (yields == 1) printf("F fallback %016" PRIx64 "\n", hash(FALLBACK_PTR, 1280 * 800));
    if (yields >= scenario.limit) {
        printf("F fallback %016" PRIx64 "\n", hash(FALLBACK_PTR, 1280 * 800));
        printf("END\n");
        exit(0);
    }
}
SexosMessage sexos_listen_once(uint64_t slot) {
    SexosMessage m = {0, 0, 0, 0, 0, 0};
    static const uint64_t SIDS[9] = {0x90, 0x92, 200, 201, 0x99, 100, 5, 6, 0};
    static const uint64_t CALLERS[5] = {3, 3, 3, 4, 7};
    static const uint64_t XS[6] = {0, 10, 100, 0xFFFFFFEC, 300, 1100};
    static const uint64_t YS[6] = {0, 20, 51, 60, 200, 0xFFFFFFFB};
    uint64_t c = 0;
    if (!fb_sent && scenario.fb_at != 0 && yields >= scenario.fb_at) {
        fb_sent = true;
        m.type_id = 0x11; m.arg0 = FB_LIVE_ADDR; m.arg1 = (scenario.fb_h << 32) | scenario.fb_w;
    } else if (fb_sent && scenario.script && scripted < 8) {
        m.type_id = SCRIPT[scripted][0]; c = SCRIPT[scripted][1];
        m.arg0 = SCRIPT[scripted][2]; m.arg1 = SCRIPT[scripted][3]; m.arg2 = SCRIPT[scripted][4];
        ++scripted;
    } else if (next() % 100 < scenario.listen) {
        uint64_t kind = next() % 32;
        c = pick(CALLERS, 5);
        if (kind <= 5) {
            m.type_id = 0xF2;
            m.arg0 = next() % 12;
            if (m.arg0 == 4) {
                m.arg1 = PICK(10, 9, 23, 30, 0);
                uint64_t mm = PICK(42, 41, 0, 59, 70);
                m.arg2 = (mm << 8) | PICK(0, 1, 2, 5, 30, 59, 61, 3);
            } else {
                uint64_t slot_index = next() % 8;
                m.arg1 = (slot_index << 32) | (next() % 300);
                m.arg2 = next() % 0x10000;
            }
        } else if (kind <= 9) {
            m.type_id = 0xEC; m.arg0 = pick(SIDS, 9);
            uint64_t y = pick(YS, 6); m.arg1 = (y << 32) | pick(XS, 6);
            uint64_t h = PICK(0, 8, 40, 100, 300, 5000); m.arg2 = (h << 32) | PICK(0, 8, 64, 200, 5000, 10000);
        } else if (kind <= 14) {
            m.type_id = 0xEB; m.arg0 = pick(SIDS, 9);
            uint64_t tagged = next() % 3;
            if (tagged > 0) ++seq;
            uint64_t s = tagged > 0 ? seq : 0, tick = tagged == 2 ? seq * 7 : 0;
            m.arg1 = (s << 32) | pick(XS, 6); m.arg2 = (tick << 32) | pick(YS, 6);
        } else if (kind == 15) { m.type_id = 0xED; m.arg0 = pick(SIDS, 9); }
        else if (kind == 16) { m.type_id = 0xEE; m.arg0 = pick(SIDS, 9); }
        else if (kind == 17) { m.type_id = 0xF5; c = PICK(3, 4, 0); }
        else if (kind <= 19) {
            m.type_id = 0xEF; m.arg0 = pick(SIDS, 9);
            uint64_t sy = PICK(0xFFFFFFF6, 0, 5, 100, 70000); m.arg1 = (sy << 32) | PICK(0xFFFFFFF6, 0, 5, 100, 70000);
            uint64_t index = PICK(0, 1, 7, 8, 15), rgb = next() & 0xFFFFFF, sh = PICK(0, 4, 30, 9000);
            m.arg2 = (index << 56) | (rgb << 32) | (sh << 16) | PICK(0, 4, 30, 9000);
        } else if (kind == 20) { m.type_id = 0xFA; m.arg0 = pick(SIDS, 9); }
        else if (kind <= 23) {
            m.type_id = 0xFB; m.arg0 = pick(SIDS, 9);
            static const uint64_t ALPHABET[12] = {0x41, 0x5A, 0x61, 0x7A, 0x30, 0x0A, 0x20, 0x21, 0x7F, 0x4C, 0x6E, 0x3A};
            for (int i = 0; i < 8; ++i) m.arg1 |= pick(ALPHABET, 12) << (8 * i);
            uint64_t offset = PICK(0, 8, 20, 255, 300, 508, 600);
            uint64_t count = next() % 16;
            m.arg2 = (offset & 0xFF) | count << 8 | ((offset >> 8) & 0xF) << 12 | ((next() & 0xFFFFFF) << 32);
        } else if (kind == 24) { m.type_id = 0xFC; m.arg0 = next(); m.arg1 = next(); m.arg2 = next(); }
        else if (kind <= 27) {
            m.type_id = 0xFD; m.arg0 = pick(SIDS, 9); m.arg1 = PICK(0, 1, 2, 3, 9);
            uint64_t active = PICK(0, 1, 2, 5); m.arg2 = active | (PICK(0, 1, 3, 5, 0x0D, 0x21, 0x3F, 0xFF) << 8);
        } else if (kind == 28) {
            m.type_id = 0xE4; m.arg0 = pick(XS, 6); m.arg1 = pick(YS, 6);
            uint64_t h = PICK(0, 30, 200); m.arg2 = (h << 32) | PICK(0, 30, 200);
        } else if (kind == 29) { m.type_id = 0xDE; m.arg0 = next(); }
        else if (kind == 30) {
            m.type_id = 0x11;
            switch (next() % 3) {
            case 0: m.arg0 = 0x1000; m.arg1 = (100ull << 32) | 100; break;
            case 1: m.arg0 = FB_LIVE_ADDR; m.arg1 = 100; break;
            default: m.arg0 = FB_LIVE_ADDR; m.arg1 = (100ull << 32) | 9000; break;
            }
        } else m.type_id = 0x123;
    }
    m.caller_pd = (uint32_t)c;
    printf("T %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 "\n",
           slot, m.type_id, c, m.arg0, m.arg1, m.arg2);
    return m;
}
int main(int argc, char **argv) {
    if (argc != 9) return 2;
    uint64_t *fields[8] = {&scenario.seed, &scenario.limit, &scenario.listen, &scenario.ticks, &scenario.fb_at, &scenario.fb_w, &scenario.fb_h, &scenario.script};
    for (int i = 0; i < 8; ++i) *fields[i] = strtoull(argv[i + 1], NULL, 10);
    rng = scenario.seed;
    map_fixed(FALLBACK_PTR, 1280 * 800 * 4);
    map_fixed(FB_LIVE_ADDR, 8 * 1024 * 1024);
    sexdisplay_main();
    return 0;
}
'''


def rust_reference(source):
    # Scaffolding and the two platform addresses change; nothing else does.
    for old, new in (("#![no_std]", ""), ("#![no_main]", ""), ("#![allow(static_mut_refs)]", ""),
                     ("#[panic_handler]\nfn panic(_: &core::panic::PanicInfo) -> ! {\n    loop { core::hint::spin_loop(); }\n}\n",
                      ""),
                     ("const FALLBACK_PTR: u64 = 0xffff8000fd000000;", f"const FALLBACK_PTR: u64 = {FALLBACK:#x};"),
                     ("const HIGH_HALF_BASE: u64 = 0xffff_8000_0000_0000;", f"const HIGH_HALF_BASE: u64 = {HIGH_HALF:#x};"),
                     ('#[no_mangle]\npub extern "C" fn _start() -> ! {', "fn sexdisplay_start() -> ! {")):
        assert source.count(old) == 1, old
        source = source.replace(old, new)
    return source


def scenarios(count):
    r = random.Random(0x5D15)
    sizes = [(160, 90), (320, 120), (640, 60), (1280, 64), (400, 300), (97, 51)]
    out = []
    for k in range(count):
        w, h = sizes[k % len(sizes)]
        out.append([r.getrandbits(64), [80, 400, 1500][k % 3], [5, 30, 70][(k // 3) % 3], (k // 2) % 6,
                    [3, 1, 40, 0][(k // 5) % 4], w, h, (k // 4) % 2])
    return out


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cesta", default="cesta")
    parser.add_argument("--cases", type=int, default=24)
    parser.add_argument("--all-variants", action="store_true", help="all 32 combinations of the five flags")
    parser.add_argument("--jobs",type=int,default=min(4,os.cpu_count() or 1))
    parser.add_argument("--work",type=Path)
    args = parser.parse_args()
    cesta = shutil.which(args.cesta) or args.cesta
    variants = ([c for n in range(len(FLAGS) + 1) for c in combinations(FLAGS, n)]
                if args.all_variants else DEFAULT_VARIANTS)
    work = args.work or HERE / "build/sexdisplay-differential"
    work=work.resolve()
    if work.exists():raise SystemExit("choose a new --work directory; prior evidence is preserved")
    work.mkdir(parents=True)
    show = lambda path: run(["git", "-C", REPO, "show", f"{REVISION}:{path}"]).stdout.decode()
    (work / "model.rs").write_text(show("crates/silkbar-model/src/lib.rs"))
    run(["rustc", "--edition", "2021", "-O", "--crate-type", "rlib", "--crate-name", "silkbar_model",
         work / "model.rs", "-o", work / "libsilkbar_model.rlib"])
    listen = pinned_function(show("crates/sex-pdx/src/lib.rs"), "pub fn pdx_try_listen_raw(slot: u64) -> Option<PdxMessage> {", LISTEN)
    provider = (RUST_PROVIDER.replace("FALLBACK_VALUE", f"{FALLBACK:#x}").replace("LIVE_VALUE", f"{LIVE:#x}")
                + listen + "}\n")
    rust = work / "reference.rs"
    rust.write_text(provider + rust_reference(show("servers/sexdisplay/src/main.rs")) + RUST_MAIN)
    (work / "provider.c").write_text(C_PROVIDER)
    platform = [f"-DFALLBACK_PTR=UINT64_C({FALLBACK:#x})", f"-DHIGH_HALF_BASE=UINT64_C({HIGH_HALF:#x})"]
    include = ["-I", HERE / "generated/include"]
    runtime = [HERE / "runtime/client_runtime.c", HERE / "runtime/server_runtime.c",
               *sorted((HERE / "generated/runtime").glob("*.c"))]
    sanitizers = ["-fsanitize=address,undefined", "-fno-sanitize-recover=all"]
    warnings = ["-Wall", "-Wextra", "-Wpedantic", "-Wformat=2", "-Werror"]
    cases = scenarios(args.cases)
    compared = 0
    references = {}
    for variant in variants:
        tag = "-".join(name.removeprefix("SEXOS_").lower() for name in variant) or "default"
        env = dict(os.environ)
        for name in variant:
            env[name] = "1"
        reference = work / f"reference-{tag}"
        run(["rustc", "--edition", "2021", "-O", "-A", "warnings", "--extern",
             f"silkbar_model={work / 'libsilkbar_model.rlib'}", rust, "-o", reference], env=env)
        defines = ["-DSEXOS_DISPLAY_READY_PUBLISHER=0", "-DSEXOS_DISPLAY_DRAIN_MAX=16", *(f"-D{name}" for name in variant)]
        harnesses = []
        for compiler in ("gcc", "clang"):
            objs = []
            for unit in ("sexdisplay", "silkbar_model"):
                obj = work / f"{unit}-{compiler}-{tag}.o"
                run([cesta, "cc", "--cc", compiler, "-std=c11", "-O2", *warnings, *defines, *platform, *sanitizers, *include,
                     "-c", HERE / "src" / f"{unit}.cesta", "-o", obj])
                objs.append(obj)
            harness = work / f"harness-{compiler}-{tag}"
            run([compiler, "-std=c11", "-O2", "-DSEXOS_HOST", f"-DFB_LIVE_ADDR=UINT64_C({LIVE:#x})", *platform, *sanitizers,
                 *include, work / "provider.c", *objs, *runtime, "-o", harness])
            harnesses.append((compiler, harness))
        expected_cases=run_commands([([reference,*case],None)for case in cases],args.jobs)
        for number, (case,expected) in enumerate(zip(cases,expected_cases)):
            references[variant, number] = expected
        tasks=[(compiler,harness,number,case)for compiler,harness in harnesses for number,case in enumerate(cases)]
        outputs=run_commands([([harness,*case],None)for _,harness,_,case in tasks],args.jobs)
        for (compiler,_,number,_),actual in zip(tasks,outputs):
            expected=references[variant,number]
            if actual != expected:
                raise SystemExit(first_difference(actual, expected, f"variant {tag} scenario {number} {compiler}"))
        compared += sum(len(expected.splitlines()) for expected in expected_cases)
    # Controls: each change to the translation must change some trace or frame.
    original = (HERE / "src/sexdisplay.cesta").read_text()
    atlas = ("SEXOS_ATLAS_PHASE_C_RENDER_STUB_PROOF",)
    mutations = [
        ((), "(var uint32_t r (+ ar (/ (* (- br ar) ti) 255u)))", "(var uint32_t r (+ ar (/ (* (- br ar) ti) 256u)))"),
        ((), "(var uint32_t r (& (/ acc_r count) 0xFF))", "(var uint32_t r (& (/ acc_r (+ count 1)) 0xFF))"),
        ((), "(var size_t title_x (+ FRAME_TOP_BAR_LIGHT_EXCLUSION_PX 12))", "(var size_t title_x (+ FRAME_TOP_BAR_LIGHT_EXCLUSION_PX 13))"),
        ((), "(var size_t row (/ (- y ty) 9))", "(var size_t row (/ (- y ty) 8))"),
        ((), "(if (== (-> bar clock_ss) 0)\n        (block\n          (= (-> bar clock_ss) 1)",
             "(if (== (-> bar clock_ss) 0)\n        (block\n          (= (-> bar clock_ss) 2)"),
        ((), "(= (-> l handoff_reject_streak) 8)", "(= (-> l handoff_reject_streak) 7)"),
        ((), "(return (== (& n 0x3F) 0)))", "(return (== (& n 0x1F) 0)))"),
        ((), "(var uint8_t l1_base (?: l1_hovered 255 (?: close_allowed 224 FRAME_LIGHT_CLOSE_DISABLED_BASE_ALPHA)))",
             "(var uint8_t l1_base (?: l1_hovered 255 (?: close_allowed 223 FRAME_LIGHT_CLOSE_DISABLED_BASE_ALPHA)))"),
        ((), "(if (>= (-> l fallback_stale_real_ticks) 256) (= use_synthetic true))", "(if (>= (-> l fallback_stale_real_ticks) 255) (= use_synthetic true))"),
        ((), "(if (|| (== (-> s input_trace_recv) 4) (== (% (-> s input_trace_recv) 8) 0)) (call input_trace_display_summary s))",
             "(if (|| (== (-> s input_trace_recv) 5) (== (% (-> s input_trace_recv) 8) 0)) (call input_trace_display_summary s))"),
        ((), "(if (== (& bits (>> 0x80 col)) 0) continue)", "(if (== (& bits (>> 0x40 col)) 0) continue)"),
        (atlas, "(var size_t by (call saturating_sub bottom (+ edge 1)))", "(var size_t by (call saturating_sub bottom (+ edge 2)))"),
    ]
    for number, (variant, old_text, new_text) in enumerate(mutations):
        assert original.count(old_text) == 1, old_text
        assert (variant, 0) in references, variant
        mutant = work / f"sexdisplay-mutant-{number}.cesta"
        mutant.write_text(original.replace(old_text, new_text))
        defines = ["-DSEXOS_DISPLAY_READY_PUBLISHER=0", "-DSEXOS_DISPLAY_DRAIN_MAX=16", *(f"-D{name}" for name in variant)]
        run([cesta, "cc", "--cc", "gcc", "-std=c11", "-O2", *defines, *platform, *include, "-c", mutant, "-o", work / "mutant.o"])
        run([cesta, "cc", "--cc", "gcc", "-std=c11", "-O2", *include, "-c", HERE / "src/silkbar_model.cesta", "-o", work / "model.o"])
        run(["gcc", "-std=c11", "-O2", "-DSEXOS_HOST", f"-DFB_LIVE_ADDR=UINT64_C({LIVE:#x})", *platform, *include, work / "provider.c",
             work / "mutant.o", work / "model.o", *runtime, "-o", work / "mutant-harness"])
        mutant_outputs=run_commands([([work/"mutant-harness",*case],None)for case in cases],args.jobs)
        detected=[actual!=references[variant,index]for index,actual in enumerate(mutant_outputs)]
        if not any(detected):
            raise SystemExit(f"mutation not detected: {old_text} -> {new_text}")
    # Current upstream deliberately raises the per-frame bound from the frozen
    # reference's 16 to 256. Exercise the default independently so the
    # compatibility define cannot accidentally become the production value.
    current_display = work / "current-display.o"
    current_model = work / "current-model.o"
    current_harness = work / "current-harness"
    run([cesta, "cc", "--cc", "gcc", "-std=c11", "-O2", *warnings,
         "-DSEXOS_DISPLAY_READY_PUBLISHER=0", *platform, *include,
         "-c", HERE / "src/sexdisplay.cesta", "-o", current_display])
    run([cesta, "cc", "--cc", "gcc", "-std=c11", "-O2", *warnings, *include,
         "-c", HERE / "src/silkbar_model.cesta", "-o", current_model])
    run(["gcc", "-std=c11", "-O2", "-DSEXOS_HOST", f"-DFB_LIVE_ADDR=UINT64_C({LIVE:#x})",
         *platform, *include, work / "provider.c", current_display, current_model,
         *runtime, "-o", current_harness])
    current = run([current_harness, 1, 1, 100, 0, 0, 160, 90, 0]).stdout.splitlines()
    assert sum(line.startswith(b"T ") for line in current) == 256
    assert b"L [sexdisplay.render.budget] drained=256 surface=0 strip=0 primary=0" in current
    print(f"sexdisplay: Cesta translation equals the Rust compositor on {compared} trace lines, framebuffer hashes included "
          f"({args.cases} scenarios x {len(variants)} build variants, GCC and Clang, address and UB sanitizers); "
          f"{len(mutations)} mutations detected; current upstream 256-message drain exercised")


if __name__ == "__main__":
    main()
