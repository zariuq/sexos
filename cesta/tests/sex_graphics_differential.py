#!/usr/bin/env python3
"""Differential and mutation harness for the Cesta port of crates/sex-graphics."""

import argparse
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
from layout_text import LayoutText

HERE = Path(__file__).resolve().parent.parent
SEXOS = HERE.parent
SEEDS = (1, 2, 3)

RUST_SEX_PDX = r"""
#![no_std]
#[repr(C)]
#[derive(Clone, Copy)]
pub struct Rect {
    pub x: u32,
    pub y: u32,
    pub width: u32,
    pub height: u32,
}
"""

RUST_MAIN = r"""
extern crate sex_graphics;
extern crate sex_pdx;

use sex_graphics::{WindowBuffer, font};
use sex_pdx::Rect;

struct Rng(u64);

impl Rng {
    fn next(&mut self) -> u64 {
        let mut x = self.0;
        x ^= x << 13;
        x ^= x >> 7;
        x ^= x << 17;
        self.0 = x;
        x
    }
    fn u32(&mut self) -> u32 { self.next() as u32 }
    fn u8(&mut self) -> u8 { self.next() as u8 }
}

fn hash_buf(buf: &[u32]) -> u64 {
    let mut h: u64 = 0xcbf29ce484222325;
    for &w in buf {
        h ^= w as u64;
        h = h.wrapping_mul(0x100000001b3);
    }
    h
}

unsafe fn run_seed(seed: u64) {
    let mut rng = Rng(seed);
    println!("seed={}", seed);
    println!("consts char_w={} char_h={}", font::CHAR_W, font::CHAR_H);
    for (gi, g) in font::FONT.iter().enumerate() {
        println!(
            "font idx={} rows={:02x},{:02x},{:02x},{:02x},{:02x},{:02x},{:02x},{:02x}",
            gi, g[0], g[1], g[2], g[3], g[4], g[6 - 1], g[6], g[7]
        );
    }

    let mut zero_wb = unsafe { WindowBuffer::new(0, 0, 0, 0) };
    println!(
        "zero draw0=0x{:08x} draw_oob=0x{:08x} read0=0x{:08x} read_oob=0x{:08x}",
        zero_wb.draw_pixel_safe(0, 0, 0x11223344),
        zero_wb.draw_pixel_safe(16, 16, 0x11223344),
        zero_wb.read_pixel_safe(0, 0),
        zero_wb.read_pixel_safe(16, 16),
    );

    const W: u32 = 53;
    const H: u32 = 29;
    const STRIDE: u32 = 64;
    const TOTAL: usize = (STRIDE * H) as usize;
    let mut backing = [0xDEADBEEFu32; TOTAL];
    let mut wb = unsafe { WindowBuffer::new(backing.as_mut_ptr() as u64, W, H, STRIDE) };

    wb.clear(0x001E1E2E);
    println!("clear hash=0x{:016x} p0=0x{:08x} pad=0x{:08x}", hash_buf(&backing), backing[0], backing[W as usize]);

    let fixed_coords: [(u32, u32); 10] = [
        (0, 0), (W - 1, 0), (0, H - 1), (W - 1, H - 1),
        (W, 0), (0, H), (W, H), (u32::MAX, 0), (0, u32::MAX), (0x80000000, 0x80000000),
    ];
    for (i, &(x, y)) in fixed_coords.iter().enumerate() {
        let c = 0xA0000000u32.wrapping_add(i as u32);
        wb.draw_pixel(x, y, c);
        let ds = wb.draw_pixel_safe(x, y, c ^ 0x00550055);
        let rs = wb.read_pixel_safe(x, y);
        println!("px_fixed i={} x={} y={} ds=0x{:08x} rs=0x{:08x} hash=0x{:016x}", i, x, y, ds, rs, hash_buf(&backing));
    }

    for i in 0..400u32 {
        let x = if i % 4 == 0 { rng.u32() } else { rng.u32() % (W + 8) };
        let y = if i % 5 == 0 { rng.u32() } else { rng.u32() % (H + 8) };
        let c = rng.u32();
        if i % 2 == 0 {
            wb.draw_pixel(x, y, c);
        } else {
            let ok = wb.draw_pixel_safe(x, y, c);
            let rd = wb.read_pixel_safe(x, y);
            println!("px_rand i={} ok=0x{:08x} rd=0x{:08x}", i, ok, rd);
        }
    }
    println!("px_rand_done hash=0x{:016x}", hash_buf(&backing));

    let fixed_rects: [Rect; 12] = [
        Rect { x: 0, y: 0, width: 10, height: 10 },
        Rect { x: 45, y: 22, width: 20, height: 20 },
        Rect { x: W, y: 0, width: 5, height: 5 },
        Rect { x: 0, y: H, width: 5, height: 5 },
        Rect { x: 5, y: 5, width: 0, height: 10 },
        Rect { x: 5, y: 5, width: 10, height: 0 },
        Rect { x: 10, y: 10, width: 0x80000000, height: 5 },
        Rect { x: 10, y: 10, width: 5, height: 0x80000000 },
        Rect { x: 0x7FFFFFF0, y: 2, width: 32, height: 4 },
        Rect { x: 2, y: 0x7FFFFFF0, width: 4, height: 32 },
        Rect { x: 0xFFFFFFF0, y: 2, width: 32, height: 4 },
        Rect { x: u32::MAX, y: u32::MAX, width: u32::MAX, height: u32::MAX },
    ];
    for (i, &r) in fixed_rects.iter().enumerate() {
        wb.draw_rect(r, 0x00330000u32.wrapping_add(i as u32 * 0x111));
        println!("rect_fixed i={} hash=0x{:016x}", i, hash_buf(&backing));
    }

    for i in 0..300u32 {
        let r = Rect {
            x: if i % 5 == 0 { rng.u32() } else { rng.u32() % (W + 16) },
            y: if i % 7 == 0 { rng.u32() } else { rng.u32() % (H + 16) },
            width: if i % 6 == 0 { rng.u32() } else { rng.u32() % (W + 16) },
            height: if i % 8 == 0 { rng.u32() } else { rng.u32() % (H + 16) },
        };
        wb.draw_rect(r, rng.u32());
        if i % 25 == 0 {
            println!("rect_rand i={} hash=0x{:016x}", i, hash_buf(&backing));
        }
    }
    println!("rect_rand_done hash=0x{:016x}", hash_buf(&backing));

    for ch in 0..=255u8 {
        let x = ((ch as u32) * 5) % (W + 6);
        let y = ((ch as u32) * 3) % (H + 6);
        let fg = rng.u32();
        let bg = if ch % 2 == 0 { Some(rng.u32()) } else { None };
        font::draw_char(&mut wb, x, y, ch, fg, bg);
        if ch % 16 == 15 {
            println!("char_sweep ch={} hash=0x{:016x}", ch, hash_buf(&backing));
        }
    }
    font::draw_char(&mut wb, 0xFFFFFFFC, 2, b'A', 0x00FF00FF, Some(0x00112233));
    font::draw_char(&mut wb, 2, 0xFFFFFFFC, b'Z', 0x00FFFF00, None);
    println!("char_wrap hash=0x{:016x}", hash_buf(&backing));

    let fixed_strs: [&[u8]; 8] = [
        b"",
        b"SexOS",
        b"spindle> help\nignored",
        b"\nleading newline",
        b"clip_right_edge_1234567890",
        b"!\x7f~@#",
        b"line1\nline2",
        b"a",
    ];
    for (i, &bytes) in fixed_strs.iter().enumerate() {
        let x = (i as u32 * 7) % (W + 4);
        let y = (i as u32 * 5) % (H + 4);
        let fg = rng.u32();
        let bg = if i % 2 == 0 { Some(rng.u32()) } else { None };
        let drawn_w = font::draw_str(&mut wb, x, y, bytes, fg, bg);
        let calc_w = font::str_width(bytes.len());
        println!("str_fixed i={} drawn_w={} calc_w={} hash=0x{:016x}", i, drawn_w, calc_w, hash_buf(&backing));
    }

    let wrap_s = b"ABCD";
    let wrap_drawn = font::draw_str(&mut wb, 0xFFFFFFF0, 4, wrap_s, 0x00AABBCC, Some(0x00010203));
    println!("str_wrap drawn_w={} calc_w={} hash=0x{:016x}", wrap_drawn, font::str_width(wrap_s.len()), hash_buf(&backing));

    for i in 0..200u32 {
        let len = (rng.u8() as usize) % 20;
        let mut buf = [0u8; 20];
        for j in 0..len {
            let b = rng.u8();
            buf[j] = if b % 19 == 0 { b'\n' } else { b };
        }
        let s = &buf[..len];
        let rx = rng.u32();
        let x = if i % 11 == 0 { 0xFFFFFFECu32.wrapping_add(i) } else { rx % (W + 10) };
        let y = rng.u32() % (H + 10);
        let fg = rng.u32();
        let bg = if rng.u8() & 1 != 0 { Some(rng.u32()) } else { None };
        let dw = font::draw_str(&mut wb, x, y, s, fg, bg);
        let cw = font::str_width(s.len());
        println!("str_rand i={} dw={} cw={} hash=0x{:016x}", i, dw, cw, hash_buf(&backing));
    }
}

fn main() {
    for seed in [1u64, 2, 3] {
        unsafe { run_seed(seed); }
    }
}
"""

C_MAIN = r"""
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "sex_graphics.h"

typedef struct { uint64_t state; } Rng;
static uint64_t rng_next(Rng *r) {
    uint64_t x = r->state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    r->state = x;
    return x;
}
static uint32_t rng_u32(Rng *r) { return (uint32_t)rng_next(r); }
static uint8_t rng_u8(Rng *r) { return (uint8_t)rng_next(r); }

static uint64_t hash_buf(const uint32_t *buf, size_t n) {
    uint64_t h = UINT64_C(0xcbf29ce484222325);
    for (size_t i = 0; i < n; ++i) {
        h ^= (uint64_t)buf[i];
        h *= UINT64_C(0x100000001b3);
    }
    return h;
}

static void run_seed(uint64_t seed) {
    Rng rng = { seed };
    printf("seed=%" PRIu64 "\n", seed);
    printf("consts char_w=%" PRIu32 " char_h=%" PRIu32 "\n", SEX_GRAPHICS_CHAR_W, SEX_GRAPHICS_CHAR_H);
    for (size_t gi = 0; gi < SEX_GRAPHICS_FONT_GLYPHS; ++gi) {
        const uint8_t *g = SEX_GRAPHICS_FONT[gi];
        printf(
            "font idx=%zu rows=%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x\n",
            gi, g[0], g[1], g[2], g[3], g[4], g[5], g[6], g[7]
        );
    }

    WindowBuffer null_wb = window_buffer_new(NULL, 16, 16, 16);
    if (window_buffer_draw_pixel_safe(&null_wb, 0, 0, 0x11223344u) != 0u ||
        window_buffer_read_pixel_safe(&null_wb, 0, 0) != 0u) {
        printf("null_guard_broken\n");
    }

    WindowBuffer zero_wb = window_buffer_new(NULL, 0, 0, 0);
    printf("zero draw0=0x%08" PRIx32 " draw_oob=0x%08" PRIx32 " read0=0x%08" PRIx32 " read_oob=0x%08" PRIx32 "\n",
           window_buffer_draw_pixel_safe(&zero_wb, 0, 0, 0x11223344u),
           window_buffer_draw_pixel_safe(&zero_wb, 16, 16, 0x11223344u),
           window_buffer_read_pixel_safe(&zero_wb, 0, 0),
           window_buffer_read_pixel_safe(&zero_wb, 16, 16));

    enum { W = 53, H = 29, STRIDE = 64, TOTAL = STRIDE * H };
    uint32_t backing[TOTAL];
    for (size_t i = 0; i < TOTAL; ++i) backing[i] = 0xDEADBEEFu;
    WindowBuffer wb = window_buffer_new(backing, W, H, STRIDE);

    window_buffer_clear(&wb, 0x001E1E2Eu);
    printf("clear hash=0x%016" PRIx64 " p0=0x%08" PRIx32 " pad=0x%08" PRIx32 "\n",
           hash_buf(backing, TOTAL), backing[0], backing[W]);

    const uint32_t fixed_coords[10][2] = {
        {0, 0}, {W - 1, 0}, {0, H - 1}, {W - 1, H - 1},
        {W, 0}, {0, H}, {W, H}, {UINT32_MAX, 0}, {0, UINT32_MAX}, {0x80000000u, 0x80000000u}
    };
    for (size_t i = 0; i < 10; ++i) {
        uint32_t x = fixed_coords[i][0];
        uint32_t y = fixed_coords[i][1];
        uint32_t c = 0xA0000000u + (uint32_t)i;
        window_buffer_draw_pixel(&wb, x, y, c);
        uint32_t ds = window_buffer_draw_pixel_safe(&wb, x, y, c ^ 0x00550055u);
        uint32_t rs = window_buffer_read_pixel_safe(&wb, x, y);
        printf("px_fixed i=%zu x=%" PRIu32 " y=%" PRIu32 " ds=0x%08" PRIx32 " rs=0x%08" PRIx32 " hash=0x%016" PRIx64 "\n",
               i, x, y, ds, rs, hash_buf(backing, TOTAL));
    }

    for (uint32_t i = 0; i < 400; ++i) {
        uint32_t rx = rng_u32(&rng);
        uint32_t ry = rng_u32(&rng);
        uint32_t x = (i % 4 == 0) ? rx : (rx % (W + 8));
        uint32_t y = (i % 5 == 0) ? ry : (ry % (H + 8));
        uint32_t c = rng_u32(&rng);
        if (i % 2 == 0) {
            window_buffer_draw_pixel(&wb, x, y, c);
        } else {
            uint32_t ok = window_buffer_draw_pixel_safe(&wb, x, y, c);
            uint32_t rd = window_buffer_read_pixel_safe(&wb, x, y);
            printf("px_rand i=%" PRIu32 " ok=0x%08" PRIx32 " rd=0x%08" PRIx32 "\n", i, ok, rd);
        }
    }
    printf("px_rand_done hash=0x%016" PRIx64 "\n", hash_buf(backing, TOTAL));

    const SexGraphicsRect fixed_rects[12] = {
        { 0, 0, 10, 10 },
        { 45, 22, 20, 20 },
        { W, 0, 5, 5 },
        { 0, H, 5, 5 },
        { 5, 5, 0, 10 },
        { 5, 5, 10, 0 },
        { 10, 10, 0x80000000u, 5 },
        { 10, 10, 5, 0x80000000u },
        { 0x7FFFFFF0u, 2, 32, 4 },
        { 2, 0x7FFFFFF0u, 4, 32 },
        { 0xFFFFFFF0u, 2, 32, 4 },
        { UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX },
    };
    for (size_t i = 0; i < 12; ++i) {
        window_buffer_draw_rect(&wb, fixed_rects[i], 0x00330000u + (uint32_t)i * 0x111u);
        printf("rect_fixed i=%zu hash=0x%016" PRIx64 "\n", i, hash_buf(backing, TOTAL));
    }

    for (uint32_t i = 0; i < 300; ++i) {
        uint32_t rx = rng_u32(&rng);
        uint32_t ry = rng_u32(&rng);
        uint32_t rw = rng_u32(&rng);
        uint32_t rh = rng_u32(&rng);
        SexGraphicsRect r = {
            (i % 5 == 0) ? rx : (rx % (W + 16)),
            (i % 7 == 0) ? ry : (ry % (H + 16)),
            (i % 6 == 0) ? rw : (rw % (W + 16)),
            (i % 8 == 0) ? rh : (rh % (H + 16)),
        };
        window_buffer_draw_rect(&wb, r, rng_u32(&rng));
        if (i % 25 == 0) {
            printf("rect_rand i=%" PRIu32 " hash=0x%016" PRIx64 "\n", i, hash_buf(backing, TOTAL));
        }
    }
    printf("rect_rand_done hash=0x%016" PRIx64 "\n", hash_buf(backing, TOTAL));

    for (unsigned ch = 0; ch <= 255; ++ch) {
        uint32_t x = (ch * 5u) % (W + 6);
        uint32_t y = (ch * 3u) % (H + 6);
        uint32_t fg = rng_u32(&rng);
        bool has_bg = (ch % 2 == 0);
        uint32_t bg = has_bg ? rng_u32(&rng) : 0u;
        sex_graphics_draw_char(&wb, x, y, (uint8_t)ch, fg, has_bg, bg);
        if (ch % 16 == 15) {
            printf("char_sweep ch=%u hash=0x%016" PRIx64 "\n", ch, hash_buf(backing, TOTAL));
        }
    }
    sex_graphics_draw_char(&wb, 0xFFFFFFFCu, 2, (uint8_t)'A', 0x00FF00FFu, true, 0x00112233u);
    sex_graphics_draw_char(&wb, 2, 0xFFFFFFFCu, (uint8_t)'Z', 0x00FFFF00u, false, 0u);
    printf("char_wrap hash=0x%016" PRIx64 "\n", hash_buf(backing, TOTAL));

    static const char *const fixed_strs[8] = {
        "",
        "SexOS",
        "spindle> help\nignored",
        "\nleading newline",
        "clip_right_edge_1234567890",
        "!\x7f~@#",
        "line1\nline2",
        "a",
    };
    static const size_t fixed_lens[8] = { 0, 5, 21, 16, 26, 5, 11, 1 };
    for (size_t i = 0; i < 8; ++i) {
        uint32_t x = ((uint32_t)i * 7u) % (W + 4);
        uint32_t y = ((uint32_t)i * 5u) % (H + 4);
        uint32_t fg = rng_u32(&rng);
        bool has_bg = (i % 2 == 0);
        uint32_t bg = has_bg ? rng_u32(&rng) : 0u;
        uint32_t drawn_w = sex_graphics_draw_str(&wb, x, y, (const uint8_t *)fixed_strs[i], fixed_lens[i], fg, has_bg, bg);
        uint32_t calc_w = sex_graphics_str_width(fixed_lens[i]);
        printf("str_fixed i=%zu drawn_w=%" PRIu32 " calc_w=%" PRIu32 " hash=0x%016" PRIx64 "\n",
               i, drawn_w, calc_w, hash_buf(backing, TOTAL));
    }

    const uint8_t wrap_s[4] = { 'A', 'B', 'C', 'D' };
    uint32_t wrap_drawn = sex_graphics_draw_str(&wb, 0xFFFFFFF0u, 4, wrap_s, 4, 0x00AABBCCu, true, 0x00010203u);
    printf("str_wrap drawn_w=%" PRIu32 " calc_w=%" PRIu32 " hash=0x%016" PRIx64 "\n",
           wrap_drawn, sex_graphics_str_width(4), hash_buf(backing, TOTAL));

    for (uint32_t i = 0; i < 200; ++i) {
        size_t len = (size_t)(rng_u8(&rng) % 20);
        uint8_t buf[20] = { 0 };
        for (size_t j = 0; j < len; ++j) {
            uint8_t b = rng_u8(&rng);
            buf[j] = (b % 19 == 0) ? (uint8_t)'\n' : b;
        }
        uint32_t rx = rng_u32(&rng);
        uint32_t x = (i % 11 == 0) ? (0xFFFFFFECu + i) : (rx % (W + 10));
        uint32_t y = rng_u32(&rng) % (H + 10);
        uint32_t fg = rng_u32(&rng);
        bool has_bg = (rng_u8(&rng) & 1u) != 0;
        uint32_t bg = has_bg ? rng_u32(&rng) : 0u;
        uint32_t dw = sex_graphics_draw_str(&wb, x, y, buf, len, fg, has_bg, bg);
        uint32_t cw = sex_graphics_str_width(len);
        printf("str_rand i=%" PRIu32 " dw=%" PRIu32 " cw=%" PRIu32 " hash=0x%016" PRIx64 "\n",
               i, dw, cw, hash_buf(backing, TOTAL));
    }
}

int main(void) {
    uint64_t seeds[3] = { 1, 2, 3 };
    for (size_t i = 0; i < 3; ++i) {
        run_seed(seeds[i]);
    }
    return 0;
}
"""

MUTATIONS = (
    (
        "draw_pixel uses width instead of stride",
        "(var size_t offset (cast size_t (+ (* y (-> wb stride)) x)))",
        "(var size_t offset (cast size_t (+ (* y (-> wb width)) x)))",
    ),
    (
        "draw_pixel_safe omits NULL guard",
        "(if (|| (== (-> wb virtual_address) NULL) (|| (>= x (-> wb width)) (>= y (-> wb height))))",
        "(if (|| (>= x (-> wb width)) (>= y (-> wb height)))",
    ),
    (
        "read_pixel_safe off-by-one width check",
        "(fn uint32_t window_buffer_read_pixel_safe (((const WindowBuffer) (* wb)) (uint32_t x) (uint32_t y))\n      (if (|| (== (-> wb virtual_address) NULL) (|| (>= x (-> wb width)) (>= y (-> wb height))))\n        (return 0u))",
        "(fn uint32_t window_buffer_read_pixel_safe (((const WindowBuffer) (* wb)) (uint32_t x) (uint32_t y))\n      (if (|| (== (-> wb virtual_address) NULL) (|| (> x (-> wb width)) (>= y (-> wb height))))\n        (return 0u))",
    ),
    (
        "draw_rect omits signed two's-complement negative sum clamp",
        "(var uint32_t end_x (?: (< sum_x 0x80000000u) sum_x 0u))",
        "(var uint32_t end_x sum_x)",
    ),
    (
        "draw_char maps non-ASCII to glyph 0 instead of ch & 0x7f",
        "(var size_t idx (cast size_t (& ch 0x7Fu)))",
        "(var size_t idx (?: (< (cast size_t ch) SEX_GRAPHICS_FONT_GLYPHS) (cast size_t ch) 0))",
    ),
    (
        "draw_char reads glyph bits LSB-first instead of MSB-first",
        "(if (!= (& (>> bits (- 7u col)) 1u) 0)",
        "(if (!= (& (>> bits col) 1u) 0)",
    ),
    (
        "draw_char ignores background colour when has_bg is true",
        "(if has_bg\n                  (call window_buffer_draw_pixel fb px py bg))",
        "(if false\n                  (call window_buffer_draw_pixel fb px py bg))",
    ),
    (
        "draw_str advances by CHAR_W - 1 instead of CHAR_W",
        "(+= cx SEX_GRAPHICS_CHAR_W)",
        "(+= cx 7u)",
    ),
    (
        "str_width multiplies by 6 instead of CHAR_W (8)",
        "(return (* (cast uint32_t byte_len) SEX_GRAPHICS_CHAR_W))",
        "(return (* (cast uint32_t byte_len) 6u))",
    ),
    (
        "font table glyph '!' row 0 corrupted",
        "(init 0x18 0x3C 0x3C 0x18 0x18 0x00 0x18 0x00)",
        "(init 0x19 0x3C 0x3C 0x18 0x18 0x00 0x18 0x00)",
    ),
)


def run(cmd, quiet=False, **kwargs):
    proc = subprocess.run(cmd, text=True, capture_output=True, **kwargs)
    if proc.returncode != 0:
        if not quiet:
            sys.stderr.write(f"command failed: {' '.join(cmd)}\n{proc.stderr}\n")
        raise subprocess.CalledProcessError(proc.returncode, cmd, output=proc.stdout, stderr=proc.stderr)
    return proc


def build_and_run_c(cesta_bin, work, compiler, source_cesta, exported=False, quiet=False):
    flags = [
        "-std=c11", "-O2", "-g", "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
        "-Wall", "-Wextra", "-Werror", f"-I{HERE / 'generated/include'}",
    ]
    exe = work / f"sex_graphics_{compiler}_{'export' if exported else 'cesta'}"
    main_c = work / "main.c"
    if exported:
        gen_c = work / f"sex_graphics_{compiler}.c"
        run([str(cesta_bin), "export-c", str(source_cesta), "-o", str(gen_c), "--"], quiet=quiet)
        run([compiler, *flags, str(gen_c), str(main_c), "-o", str(exe)], quiet=quiet)
    else:
        obj = work / f"sex_graphics_{compiler}.o"
        run([str(cesta_bin), "cc", "--cc", compiler, *flags, "-c", str(source_cesta), "-o", str(obj)], quiet=quiet)
        run([compiler, *flags, str(main_c), str(obj), "-o", str(exe)], quiet=quiet)
    return run([str(exe)], quiet=quiet).stdout


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cesta", type=Path, default=Path("cesta"))
    parser.add_argument("--seeds", type=int, default=len(SEEDS),
                        help="run seeds 1..N in both the Rust reference and the Cesta harness")
    args = parser.parse_args()
    assert args.seeds >= 1
    seeds = tuple(range(1, args.seeds + 1))
    cesta_bin = Path(shutil.which(str(args.cesta)) or args.cesta).resolve()

    work = HERE / "build/sex-graphics-differential"
    if work.exists():
        shutil.rmtree(work)
    work.mkdir(parents=True)
    if True:
        pdx_src = work / "sex_pdx.rs"
        pdx_rlib = work / "libsex_pdx.rlib"
        pdx_src.write_text(RUST_SEX_PDX)
        run(["rustc", "--edition=2021", "--crate-type", "rlib", "--crate-name", "sex_pdx", "-O", str(pdx_src), "-o", str(pdx_rlib)])

        gfx_rlib = work / "libsex_graphics.rlib"
        run([
            "rustc", "--edition=2021", "--crate-type", "rlib", "--crate-name", "sex_graphics", "-O",
            str(SEXOS / "crates/sex-graphics/src/lib.rs"),
            "--extern", f"sex_pdx={pdx_rlib}",
            "-o", str(gfx_rlib),
        ])

        rust_src = work / "rust_main.rs"
        rust_exe = work / "rust_main"
        rust_src.write_text(RUST_MAIN.replace("for seed in [1u64, 2, 3] {",
                                              "for seed in [" + ", ".join(f"{n}u64" for n in seeds) + "] {"))
        run([
            "rustc", "--edition=2021", "-O", str(rust_src),
            "-L", f"dependency={work}",
            "--extern", f"sex_pdx={pdx_rlib}",
            "--extern", f"sex_graphics={gfx_rlib}",
            "-o", str(rust_exe),
        ])
        rust_out = run([str(rust_exe)]).stdout

        (work / "main.c").write_text(C_MAIN.replace(
            "uint64_t seeds[3] = { 1, 2, 3 };\n    for (size_t i = 0; i < 3; ++i) {",
            f"uint64_t seeds[{len(seeds)}] = {{ {', '.join(map(str, seeds))} }};\n    for (size_t i = 0; i < {len(seeds)}; ++i) {{"))
        source_cesta = HERE / "src/sex_graphics.cesta"
        for compiler in ("gcc", "clang"):
            for exported in (False, True):
                c_out = build_and_run_c(cesta_bin, work, compiler, source_cesta, exported=exported)
                if c_out != rust_out:
                    cl, rl = c_out.splitlines(), rust_out.splitlines()
                    first = next(i for i in range(min(len(cl), len(rl)) + 1) if i >= min(len(cl), len(rl)) or cl[i] != rl[i])
                    sys.stderr.write(
                        f"sex_graphics mismatch under {compiler} (exported={exported}) at line {first + 1}:\n"
                        f"  rust: {rl[first] if first < len(rl) else '<eof>'}\n"
                        f"  c:    {cl[first] if first < len(cl) else '<eof>'}\n"
                    )
                    return 1

        original = LayoutText(source_cesta.read_text())
        for name, target, replacement in MUTATIONS:
            if target not in original:
                sys.stderr.write(f"mutation target missing: {name}\n")
                return 1
            mutated = work / "mutated_sex_graphics.cesta"
            mutated.write_text(original.replace(target, replacement, 1))
            try:
                mut_out = build_and_run_c(cesta_bin, work, "gcc", mutated, exported=True, quiet=True)
            except subprocess.CalledProcessError:
                continue
            if mut_out == rust_out:
                sys.stderr.write(f"mutation survived: {name}\n")
                return 1

    lines = len(rust_out.splitlines())
    print(
        f"sex_graphics differential passed ({lines} lines across {len(seeds)} seeds, "
        f"gcc/clang authored/exported, {len(MUTATIONS)} mutations killed)"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
