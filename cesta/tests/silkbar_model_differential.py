#!/usr/bin/env python3
"""Compare crates/silkbar-model with its Cesta translation.

The Rust crate is compiled from the pinned revision; the Cesta model is
compiled by cesta cc. Both run the same seeded scenarios and print one line per
result: colour derivation, bevel palettes, hit tests on the default and on
random layouts, random update sequences (including unknown kinds, out-of-range
indices and payloads), queue operations, equality of the data-carrying cases,
the validation gates and the contract fingerprint. Every line must match.
"""

import argparse
from pathlib import Path
import shutil
import subprocess
from layout_text import LayoutText

HERE = Path(__file__).resolve().parent.parent
REPO = HERE.parent
REVISION = "4a040a86"

RUST = r'''
use silkbar_model::*;

struct Rng(u64);
impl Rng {
    fn next(&mut self) -> u64 {
        self.0 = self.0.wrapping_add(0x9E3779B97F4A7C15);
        let mut z = self.0;
        z = (z ^ (z >> 30)).wrapping_mul(0xBF58476D1CE4E5B9);
        z = (z ^ (z >> 27)).wrapping_mul(0x94D049BB133111EB);
        z ^ (z >> 31)
    }
    fn below(&mut self, n: u64) -> u64 { self.next() % n }
    fn pick(&mut self) -> u32 {
        const EDGES: [u32; 14] = [0, 1, 2, 3, 7, 8, 9, 23, 24, 59, 60, 255, 256, u32::MAX];
        if self.below(3) == 0 { self.next() as u32 } else { EDGES[self.below(14) as usize] }
    }
}

fn module_code(m: Module) -> (u64, u64) {
    match m {
        Module::Launcher => (0, 0), Module::Workspaces(i) => (1, i as u64),
        Module::StatusChip(i) => (2, i as u64), Module::Clock => (3, 0), Module::Bell => (4, 0),
    }
}
fn module_of(tag: u64, index: u64) -> Module {
    match tag % 5 {
        0 => Module::Launcher, 1 => Module::Workspaces(index as usize),
        2 => Module::StatusChip(index as usize), 3 => Module::Clock, _ => Module::Bell,
    }
}
fn action_code(a: Action) -> String {
    match a {
        Action::None => "0 0 0 0".into(),
        Action::OpenLauncher => "1 0 0 0".into(),
        Action::SwitchWorkspace(w) => format!("2 {} 0 0", w),
        Action::ToggleModule(m) => { let (t, i) = module_code(m); format!("3 0 {} {}", t, i) }
        Action::OpenClock => "4 0 0 0".into(),
        Action::OpenBell => "5 0 0 0".into(),
    }
}
fn action_of(tag: u64, w: u64, mt: u64, mi: u64) -> Action {
    match tag % 6 {
        0 => Action::None, 1 => Action::OpenLauncher, 2 => Action::SwitchWorkspace(w as u8),
        3 => Action::ToggleModule(module_of(mt, mi)), 4 => Action::OpenClock, _ => Action::OpenBell,
    }
}
fn chip_code(k: ChipKind) -> u32 { k as u32 }
fn bar_line(b: &SilkBar) -> String {
    let mut s = String::new();
    for l in b.layout.iter() {
        let (t, i) = module_code(l.module);
        s += &format!("{} {} {} {} {} {} {};", l.x, l.y, l.w, l.h, t, i, action_code(l.action));
    }
    for w in b.workspaces.iter() { s += &format!("{} {} {};", w.index, w.active as u8, w.urgent as u8); }
    for c in b.chips.iter() { s += &format!("{} {};", chip_code(c.kind), c.visible as u8); }
    s += &format!("{} {} {} {} ", b.clock_hh, b.clock_mm, b.clock_ss, b.selected_options_mask);
    s += &format!("{} {} {} ", b.bell_state.total_visible, b.bell_state.redacted_count, b.bell_state.flags);
    s += &format!("{} {} {} {} {}", b.phase1.active_app_sid, b.phase1.accent_tint_idx,
                  b.phase1.palette_open as u8, b.phase1.palette_selected, b.phase1.palette_available);
    s
}
fn random_update(r: &mut Rng) -> SilkBarUpdate {
    let kind = if r.below(16) == 0 { r.pick() } else { r.below(12) as u32 };
    let index = if r.below(8) == 0 { r.next() as u8 } else { r.below(8) as u8 };
    SilkBarUpdate::new(kind, index, r.pick(), r.pick())
}

fn main() {
    let mut r = Rng(SEED);
    for _ in 0..4096 { let c = r.pick(); println!("L {} {} {}", c, lighten(c), darken(c)); }
    for _ in 0..256 {
        let t = Theme { bg_top: r.pick(), bg_bottom: r.pick(), panel_fill: r.pick(), panel_glow: r.pick(),
            text: r.pick(), muted: r.pick(), active: r.pick(), urgent: r.pick(), chip_fill: r.pick(), chip_border: r.pick() };
        let p = t.bevels();
        println!("B {} {} {} {} {} {} {} {} {} {} {} {} {} {}", p.ws_active_highlight, p.ws_active_shadow,
            p.ws_inactive_highlight, p.ws_inactive_shadow, p.chip_highlight, p.glow_top_inner, p.glow_bottom_inner,
            p.glow_bottom_outer, p.panel_border_top, p.panel_border_bottom, p.launcher_highlight, p.launcher_shadow,
            p.launcher_fill, p.panel_body);
    }
    let bar = DEFAULT_SILK_BAR;
    println!("D {}", bar_line(&bar));
    let mut y = 0usize;
    while y < 64 { let mut x = 0usize; while x < 1300 { println!("H {} {} {}", x, y, action_code(hit_test_action(&bar, x, y))); x += 7; } y += 1; }
    for _ in 0..64 {
        let mut b = DEFAULT_SILK_BAR;
        for l in b.layout.iter_mut() {
            l.x = (60 + r.below(1200)) as usize; l.y = r.below(60) as usize;
            l.w = r.below(80) as usize; l.h = r.below(30) as usize;
            l.module = module_of(r.next(), r.below(6));
            l.action = action_of(r.next(), r.next(), r.next(), r.below(6));
        }
        for _ in 0..256 {
            let x = r.below(1300) as usize; let y = r.below(64) as usize;
            println!("R {} {} {}", x, y, action_code(hit_test_action(&b, x, y)));
        }
    }
    for _ in 0..2000 {
        let mut b = DEFAULT_SILK_BAR;
        let n = 1 + r.below(20);
        for _ in 0..n {
            let u = random_update(&mut r);
            let ok = if r.below(2) == 0 { apply_update(&mut b, u) } else { apply_raw_update(&mut b, u.kind, u.index, u.a, u.b) };
            println!("U {} {} {} {} {} {}", u.kind, u.index, u.a, u.b, ok as u8, bar_line(&b));
        }
    }
    let mut q = SilkBarUpdateQueue::empty();
    let mut b = DEFAULT_SILK_BAR;
    for _ in 0..20000 {
        match r.below(7) {
            0..=2 => { let u = random_update(&mut r); println!("Q push {}", q.push(u) as u8); }
            3..=5 => match q.pop() { Some(u) => println!("Q pop {} {} {} {}", u.kind, u.index, u.a, u.b), None => println!("Q pop none") },
            _ => { let n = q.drain_into(&mut b); println!("Q drain {} {}", n, bar_line(&b)); }
        }
    }
    for _ in 0..4096 {
        let m1 = module_of(r.next(), r.below(3)); let m2 = module_of(r.next(), r.below(3));
        let a1 = action_of(r.next(), r.below(3), r.next(), r.below(3));
        let a2 = action_of(r.next(), r.below(3), r.next(), r.below(3));
        println!("E {} {}", (m1 == m2) as u8, (a1 == a2) as u8);
    }
    println!("V {} {} {} {} {}", validate_invariants() as u8, validate_silkbar_contract(), validate_contract() as u8,
             validate_deterministic_vectors() as u8, contract_fingerprint());
}
'''

C = r'''
#include "silkbar_model.h"
#include <inttypes.h>
#include <stdio.h>

static uint64_t state = SEED;
static uint64_t next(void) {
    state += 0x9E3779B97F4A7C15ull;
    uint64_t z = state;
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
    return z ^ (z >> 31);
}
static uint64_t below(uint64_t n) { return next() % n; }
static uint32_t pick(void) {
    static const uint32_t edges[14] = {0, 1, 2, 3, 7, 8, 9, 23, 24, 59, 60, 255, 256, UINT32_MAX};
    if (below(3) == 0) return (uint32_t)next();
    return edges[below(14)];
}
static void module_code(SilkBarModule m, uint64_t *t, uint64_t *i) {
    *t = (uint64_t)m.tag;
    *i = m.tag == SILKBAR_MODULE_WORKSPACES || m.tag == SILKBAR_MODULE_STATUS_CHIP ? (uint64_t)m.index : 0;
}
static SilkBarModule module_of(uint64_t tag, uint64_t index) {
    SilkBarModule m = {(SilkBarModuleTag)(tag % 5), 0};
    if (m.tag == SILKBAR_MODULE_WORKSPACES || m.tag == SILKBAR_MODULE_STATUS_CHIP) m.index = (size_t)index;
    return m;
}
static void print_action(SilkBarAction a) {
    uint64_t t, i;
    switch (a.tag) {
    case SILKBAR_ACTION_SWITCH_WORKSPACE: printf("2 %u 0 0", (unsigned)a.workspace); break;
    case SILKBAR_ACTION_TOGGLE_MODULE: module_code(a.module, &t, &i); printf("3 0 %" PRIu64 " %" PRIu64, t, i); break;
    default: printf("%u 0 0 0", (unsigned)a.tag); break;
    }
}
static SilkBarAction action_of(uint64_t tag, uint64_t w, uint64_t mt, uint64_t mi) {
    SilkBarAction a = {(SilkBarActionTag)(tag % 6), 0, {SILKBAR_MODULE_LAUNCHER, 0}};
    if (a.tag == SILKBAR_ACTION_SWITCH_WORKSPACE) a.workspace = (uint8_t)w;
    if (a.tag == SILKBAR_ACTION_TOGGLE_MODULE) a.module = module_of(mt, mi);
    return a;
}
static void print_bar(const SilkBar *b) {
    for (size_t k = 0; k < LAYOUT_COUNT; ++k) {
        const SilkBarLayoutBox *l = &b->layout[k];
        uint64_t t, i; module_code(l->module, &t, &i);
        printf("%zu %zu %zu %zu %" PRIu64 " %" PRIu64 " ", l->x, l->y, l->w, l->h, t, i);
        print_action(l->action); putchar(';');
    }
    for (size_t k = 0; k < WORKSPACE_COUNT; ++k)
        printf("%u %u %u;", (unsigned)b->workspaces[k].index, (unsigned)b->workspaces[k].active, (unsigned)b->workspaces[k].urgent);
    for (size_t k = 0; k < MAX_CHIPS; ++k) printf("%u %u;", (unsigned)b->chips[k].kind, (unsigned)b->chips[k].visible);
    printf("%u %u %u %" PRIu32 " ", (unsigned)b->clock_hh, (unsigned)b->clock_mm, (unsigned)b->clock_ss, b->selected_options_mask);
    printf("%u %u %u ", (unsigned)b->bell_state.total_visible, (unsigned)b->bell_state.redacted_count, (unsigned)b->bell_state.flags);
    printf("%" PRIu32 " %u %u %u %u", b->phase1.active_app_sid, (unsigned)b->phase1.accent_tint_idx,
           (unsigned)b->phase1.palette_open, (unsigned)b->phase1.palette_selected, (unsigned)b->phase1.palette_available);
}
static SilkBarUpdate random_update(void) {
    uint32_t kind = below(16) == 0 ? pick() : (uint32_t)below(12);
    uint8_t index = below(8) == 0 ? (uint8_t)next() : (uint8_t)below(8);
    uint32_t a = pick(), b = pick();
    return silkbar_update(kind, index, a, b);
}

int main(void) {
    for (int k = 0; k < 4096; ++k) { uint32_t c = pick(); printf("L %" PRIu32 " %" PRIu32 " %" PRIu32 "\n", c, silkbar_lighten(c), silkbar_darken(c)); }
    for (int k = 0; k < 256; ++k) {
        SilkBarTheme t;
        t.bg_top = pick(); t.bg_bottom = pick(); t.panel_fill = pick(); t.panel_glow = pick(); t.text = pick();
        t.muted = pick(); t.active = pick(); t.urgent = pick(); t.chip_fill = pick(); t.chip_border = pick();
        SilkBarBevelPalette p = silkbar_theme_bevels(&t);
        printf("B %u %u %u %u %u %u %u %u %u %u %u %u %u %u\n", p.ws_active_highlight, p.ws_active_shadow,
            p.ws_inactive_highlight, p.ws_inactive_shadow, p.chip_highlight, p.glow_top_inner, p.glow_bottom_inner,
            p.glow_bottom_outer, p.panel_border_top, p.panel_border_bottom, p.launcher_highlight, p.launcher_shadow,
            p.launcher_fill, p.panel_body);
    }
    SilkBar bar = DEFAULT_SILK_BAR;
    printf("D "); print_bar(&bar); putchar('\n');
    for (size_t y = 0; y < 64; ++y)
        for (size_t x = 0; x < 1300; x += 7) { printf("H %zu %zu ", x, y); print_action(silkbar_hit_test_action(&bar, x, y)); putchar('\n'); }
    for (int k = 0; k < 64; ++k) {
        SilkBar b = DEFAULT_SILK_BAR;
        for (size_t i = 0; i < LAYOUT_COUNT; ++i) {
            SilkBarLayoutBox *l = &b.layout[i];
            l->x = (size_t)(60 + below(1200)); l->y = (size_t)below(60);
            l->w = (size_t)below(80); l->h = (size_t)below(30);
            uint64_t mt = next(), mi = below(6);
            l->module = module_of(mt, mi);
            uint64_t at = next(), aw = next(), amt = next(), ami = below(6);
            l->action = action_of(at, aw, amt, ami);
        }
        for (int s = 0; s < 256; ++s) {
            size_t x = (size_t)below(1300), y = (size_t)below(64);
            printf("R %zu %zu ", x, y); print_action(silkbar_hit_test_action(&b, x, y)); putchar('\n');
        }
    }
    for (int k = 0; k < 2000; ++k) {
        SilkBar b = DEFAULT_SILK_BAR;
        uint64_t n = 1 + below(20);
        for (uint64_t s = 0; s < n; ++s) {
            SilkBarUpdate u = random_update();
            bool ok = below(2) == 0 ? silkbar_apply_update(&b, u) : silkbar_apply_raw_update(&b, u.kind, u.index, u.a, u.b);
            printf("U %" PRIu32 " %u %" PRIu32 " %" PRIu32 " %u ", u.kind, (unsigned)u.index, u.a, u.b, (unsigned)ok);
            print_bar(&b); putchar('\n');
        }
    }
    SilkBarUpdateQueue q = silkbar_queue_empty();
    SilkBar b = DEFAULT_SILK_BAR;
    for (int k = 0; k < 20000; ++k) {
        uint64_t op = below(7);
        if (op <= 2) { SilkBarUpdate u = random_update(); printf("Q push %u\n", (unsigned)silkbar_queue_push(&q, u)); }
        else if (op <= 5) {
            SilkBarUpdate u;
            if (silkbar_queue_pop(&q, &u)) printf("Q pop %" PRIu32 " %u %" PRIu32 " %" PRIu32 "\n", u.kind, (unsigned)u.index, u.a, u.b);
            else printf("Q pop none\n");
        } else { uint32_t n = silkbar_queue_drain_into(&q, &b); printf("Q drain %" PRIu32 " ", n); print_bar(&b); putchar('\n'); }
    }
    for (int k = 0; k < 4096; ++k) {
        uint64_t t1 = next(), i1 = below(3), t2 = next(), i2 = below(3);
        SilkBarModule m1 = module_of(t1, i1), m2 = module_of(t2, i2);
        uint64_t at1 = next(), w1 = below(3), mt1 = next(), mi1 = below(3);
        SilkBarAction a1 = action_of(at1, w1, mt1, mi1);
        uint64_t at2 = next(), w2 = below(3), mt2 = next(), mi2 = below(3);
        SilkBarAction a2 = action_of(at2, w2, mt2, mi2);
        printf("E %u %u\n", (unsigned)silkbar_module_equal(m1, m2), (unsigned)silkbar_action_equal(a1, a2));
    }
    printf("V %u %" PRIu32 " %u %u %" PRIu64 "\n", (unsigned)silkbar_validate_invariants(), validate_silkbar_contract(),
           (unsigned)silkbar_validate_contract(), (unsigned)silkbar_validate_deterministic_vectors(), silkbar_contract_fingerprint());
    return 0;
}
'''


def run(args, **kwargs):
    return subprocess.run([str(a) for a in args], check=True, capture_output=True, **kwargs)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cesta", default="cesta")
    parser.add_argument("--seeds", type=int, default=3)
    args = parser.parse_args()
    cesta = shutil.which(args.cesta) or args.cesta
    work = HERE / "build/silkbar-model-differential"
    if work.exists():
        shutil.rmtree(work)
    work.mkdir(parents=True)
    source = run(["git", "-C", REPO, "show", f"{REVISION}:crates/silkbar-model/src/lib.rs"]).stdout
    (work / "lib.rs").write_bytes(source)
    run(["rustc", "--edition", "2021", "-O", "--crate-type", "rlib", "--crate-name", "silkbar_model",
         work / "lib.rs", "-o", work / "libsilkbar_model.rlib"])
    for compiler in ("gcc", "clang"):
        run([cesta, "cc", "--cc", compiler, "-std=c11", "-O2", "-Wall", "-Wextra", "-Werror",
             "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
             "-I", HERE / "generated/include", "-c", HERE / "src/silkbar_model.cesta", "-o", work / f"model-{compiler}.o"])
    lines = 0
    for seed in range(args.seeds):
        value = f"0x{0x5EED0000 + seed:X}ull"
        (work / "harness.rs").write_text(RUST.replace("SEED", value.replace("ull", "u64")))
        run(["rustc", "--edition", "2021", "-O", "--extern", f"silkbar_model={work / 'libsilkbar_model.rlib'}",
             work / "harness.rs", "-o", work / "rust-harness"])
        expected = run([work / "rust-harness"]).stdout
        (work / "harness.c").write_text(C.replace("SEED", value))
        for compiler in ("gcc", "clang"):
            run([compiler, "-std=c11", "-O2", "-Wall", "-Wextra", "-Werror", "-fsanitize=address,undefined",
                 "-fno-sanitize-recover=all", "-I", HERE / "generated/include",
                 work / "harness.c", work / f"model-{compiler}.o", "-o", work / f"c-harness-{compiler}"])
            actual = run([work / f"c-harness-{compiler}"]).stdout
            if actual != expected:
                a, e = actual.splitlines(), expected.splitlines()
                first = next(i for i in range(min(len(a), len(e)) + 1) if i >= min(len(a), len(e)) or a[i] != e[i])
                raise SystemExit(f"seed {seed} {compiler}: first difference at line {first + 1}\n"
                                 f"  rust: {e[first] if first < len(e) else '<end>'}\n"
                                 f"  c:    {a[first] if first < len(a) else '<end>'}")
        lines += len(expected.splitlines())
    # Controls: each small change to the translation must change some result.
    model = LayoutText((HERE / "src/silkbar_model.cesta").read_text())
    mutations = [
        ("(+ (& c 0xFFu) 0x22u)", "(+ (& c 0xFFu) 0x21u)"),
        ("(call min_u32 (. update a) 23)", "(call min_u32 (. update a) 22)"),
        ("(if (> (. update a) 7) (return false))", "(if (> (. update a) 8) (return false))"),
        ("(if (== next_tail (-> queue head)) (return false))", "(if (== next_tail (+ (-> queue head) 1)) (return false))"),
        ("(< x (+ (-> lb x) (-> lb w)))", "(<= x (+ (-> lb x) (-> lb w)))"),
        ("(return (== (. a workspace) (. b workspace)))", "(return true)"),
        ("(^= fp (<< (cast uint64_t SILK_DE_BAR_THEME_V1) 16))", "(^= fp (<< (cast uint64_t SILK_DE_BAR_THEME_V1) 15))"),
        ("(init 3 true false)", "(init 3 false false)"),
    ]
    (work / "harness.c").write_text(C.replace("SEED", "0x5EED0000ull"))
    (work / "harness.rs").write_text(RUST.replace("SEED", "0x5EED0000u64"))
    run(["rustc", "--edition", "2021", "-O", "--extern", f"silkbar_model={work / 'libsilkbar_model.rlib'}",
         work / "harness.rs", "-o", work / "rust-harness"])
    expected = run([work / "rust-harness"]).stdout
    for number, (old_text, new_text) in enumerate(mutations):
        assert model.count(old_text) == 1, old_text
        mutant = work / f"mutant-{number}.cesta"
        mutant.write_text(model.replace(old_text, new_text))
        run([cesta, "cc", "--cc", "gcc", "-std=c11", "-O2", "-I", HERE / "generated/include",
             "-c", mutant, "-o", work / "mutant.o"])
        run(["gcc", "-std=c11", "-O2", "-I", HERE / "generated/include", work / "harness.c", work / "mutant.o",
             "-o", work / "mutant-harness"])
        if run([work / "mutant-harness"]).stdout == expected:
            raise SystemExit(f"mutation not detected: {old_text} -> {new_text}")
    print(f"silkbar-model: Cesta translation equals the Rust crate on {lines} results "
          f"({args.seeds} seeds, GCC and Clang, address and UB sanitizers); {len(mutations)} mutations detected")


if __name__ == "__main__":
    main()
