#!/usr/bin/env python3
"""Quil's current line commands against upstream's Rust.

Indent and outdent (Ctrl+] / Ctrl+[), Ctrl+K, Ctrl+J, Ctrl+Shift+D, Ctrl+Up /
Ctrl+Down and line-local Home / End are taken verbatim from the pinned
main.rs and view.rs, over upstream's own selection and UTF-8 modules and
quil_replace, and run on generated documents and selections. The Cesta Quil
(SEXOS_QUIL_CURRENT_KEYS) runs the same steps through its key dispatchers.
Each step prints the document, caret, selection and the command's record.
"""
import argparse
import importlib.util
from pathlib import Path
import random
import subprocess
import sys
import tempfile

HERE = Path(__file__).resolve().parent.parent
REPO = HERE.parent
PIN = "b6be1b451a6c21985d00d192674feedf66bb11dd"
RECORDS = (b"[quil.indent]", b"[quil.line.")
sys.path.insert(0, str(HERE / "tests"))
from layout_text import LayoutText  # noqa: E402


def run(argv, *, data=None, cwd=HERE):
    result = subprocess.run(list(map(str, argv)), cwd=cwd, input=data, capture_output=True)
    if result.returncode:
        raise RuntimeError((argv, result.returncode, result.stdout[-2000:], result.stderr[-6000:]))
    return result.stdout


def item(source, start):
    at = source.index(start)
    while True:
        previous = source.rfind("\n", 0, at - 1) + 1
        if not source[previous:at].lstrip().startswith(("#[", "///")):
            break
        at = previous
    depth, i = 0, source.index("{", source.index(start))
    while True:
        depth += {"{": 1, "}": -1}.get(source[i], 0)
        i += 1
        if depth == 0:
            return source[at:i]


def rust_reference(main_rs, view_rs, selection_rs, encoding_rs):
    commands = [item(main_rs, s) for s in ("unsafe fn quil_replace(", "unsafe fn quil_indent(", "unsafe fn quil_kill_or_join(",
                                            "unsafe fn quil_duplicate_line(", "unsafe fn quil_move_line(")]
    view = [item(view_rs, s) for s in ("pub fn line_start(", "pub fn line_end(", "pub fn replacement_len(", "pub fn replace(")]
    return ("#![allow(dead_code, unused, static_mut_refs)]\n"
            "mod sex_pdx {\n  macro_rules! serial_record { ($($t:tt)*) => { println!($($t)*) } }\n  pub(crate) use serial_record;\n"
            "  pub mod text_encoding {\n" + encoding_rs + "\n}\n  pub mod text_selection {\n" + selection_rs + "\n}\n}\n"
            "mod view {\n  use crate::sex_pdx::text_encoding as utf8;\n" + "\n".join(view) + "\n}\n"
            + RUST_STATE + "\n".join(commands) + "\n")


# The document state quil_replace edits, and the two hooks it calls: the undo
# ring (outside these commands' results) and the revision change, which
# rebinds the selection at the caret as upstream's quil_revision_changed does.
RUST_STATE = r'''
static mut QUIL_BUFFER: [u8; 12288] = [0; 12288];
static mut QUIL_BUFFER_LEN: usize = 0;
static mut QUIL_CURSOR_POS: usize = 0;
static mut QUIL_GOAL_COL: usize = usize::MAX;
static mut QUIL_DIRTY: bool = false;
static mut QUIL_GENERATION: u64 = 1;
static mut QUIL_REVISION: u64 = 0;
static mut QUIL_SELECTION: sex_pdx::text_selection::Selection = sex_pdx::text_selection::Selection::new();
static mut QUIL_REPLACE_SCRATCH: [u8; 12288] = [0; 12288];
unsafe fn quil_source() -> sex_pdx::text_selection::Source { sex_pdx::text_selection::Source { generation: QUIL_GENERATION, revision: QUIL_REVISION } }
unsafe fn quil_selection_bind() {
    QUIL_SELECTION.clear();
    if QUIL_SELECTION.bind(quil_source(), QUIL_BUFFER_LEN).is_ok() { let _ = QUIL_SELECTION.move_to(quil_source(), QUIL_CURSOR_POS.min(QUIL_BUFFER_LEN), false); }
}
unsafe fn quil_selection_record() {}
unsafe fn quil_revision_changed() -> bool { QUIL_REVISION += 1; quil_selection_bind(); true }
unsafe fn text_buffer_undo_push() {}
unsafe fn draw_text_lines(_: &[u8]) {}
'''

RUST_MAIN = r'''
fn hex(b: &[u8]) -> String { b.iter().map(|c| format!("{c:02x}")).collect() }
fn unhex(s: &str) -> Vec<u8> { (0..s.len()).step_by(2).map(|i| u8::from_str_radix(&s[i..i + 2], 16).unwrap()).collect() }
fn main() {
    use std::io::BufRead;
    unsafe {
        for row in std::io::stdin().lock().lines() {
            let row = row.unwrap();
            let f: Vec<&str> = row.split(' ').collect();
            let text = unhex(f[0]);
            let (anchor, caret): (usize, usize) = (f[1].parse().unwrap(), f[2].parse().unwrap());
            QUIL_BUFFER = [0; 12288];
            QUIL_BUFFER[..text.len()].copy_from_slice(&text);
            QUIL_BUFFER_LEN = text.len();
            QUIL_CURSOR_POS = caret;
            QUIL_REVISION += 1;
            QUIL_SELECTION.clear();
            QUIL_SELECTION.bind(quil_source(), QUIL_BUFFER_LEN).unwrap();
            QUIL_SELECTION.move_to(quil_source(), anchor, false).unwrap();
            QUIL_SELECTION.move_to(quil_source(), caret, true).unwrap();
            let s = QUIL_SELECTION.snapshot().unwrap();
            match f[3] {
                "indent" | "outdent" => { quil_indent(s.range(), f[3] == "outdent"); }
                "kill" | "join" => quil_kill_or_join(f[3] == "join"),
                "duplicate" => quil_duplicate_line(),
                "up" | "down" => quil_move_line(f[3] == "up"),
                "home" => { QUIL_CURSOR_POS = view::line_start(&QUIL_BUFFER[..QUIL_BUFFER_LEN], s.caret()); }
                _ => { QUIL_CURSOR_POS = view::line_end(&QUIL_BUFFER[..QUIL_BUFFER_LEN], s.caret()); }
            }
            let range = match f[3] { "home" | "end" => (0, 0), _ => QUIL_SELECTION.snapshot().unwrap().range() };
            println!("= text={} caret={} selection={}-{}", hex(&QUIL_BUFFER[..QUIL_BUFFER_LEN]), QUIL_CURSOR_POS, range.0, range.1);
        }
    }
}
'''

C_MAIN = r'''
#include "quil.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static void hex(const uint8_t *b, size_t n) { for (size_t i = 0; i < n; i++) printf("%02x", b[i]); }
int main(void) {
    static char row[40000];
    Quil *q = &quil_state;
    while (fgets(row, sizeof row, stdin)) {
        /* Fields are separated by single spaces; the document's hex may be empty. */
        char *text = row, *anchor_s = strchr(text, ' ');
        *anchor_s++ = 0;
        char *caret_s = strchr(anchor_s, ' ');
        *caret_s++ = 0;
        char *op = strchr(caret_s, ' ');
        *op++ = 0;
        op[strcspn(op, "\n")] = 0;
        size_t anchor = strtoul(anchor_s, 0, 10), caret = strtoul(caret_s, 0, 10), n = strlen(text) / 2;
        memset(q, 0, sizeof *q);
        for (size_t i = 0; i < n; i++) { unsigned v; sscanf(text + 2 * i, "%2x", &v); q->buffer[i] = (uint8_t)v; }
        q->buffer_len = n;
        q->cursor_pos = caret;
        q->sel_start = anchor < caret ? anchor : caret;
        q->sel_end = anchor < caret ? caret : anchor;
        q->goal_col = SIZE_MAX;
        q->control_held = true;
        bool home_end = false;
        if (!strcmp(op, "indent")) control_text_key(q, 0x1B);
        else if (!strcmp(op, "outdent")) control_text_key(q, 0x1A);
        else if (!strcmp(op, "kill")) control_text_key(q, 0x25);
        else if (!strcmp(op, "join")) control_text_key(q, 0x24);
        else if (!strcmp(op, "duplicate")) { q->shift_held = true; control_text_key(q, 0x20); }
        else if (!strcmp(op, "up")) control_text_key(q, 0x48);
        else if (!strcmp(op, "down")) control_text_key(q, 0x50);
        else { q->control_held = false; text_key(q, !strcmp(op, "home") ? 0x47 : 0x4F, false); home_end = true; }
        size_t s0 = q->sel_start < q->sel_end ? q->sel_start : q->cursor_pos, s1 = q->sel_start < q->sel_end ? q->sel_end : q->cursor_pos;
        if (home_end) s0 = s1 = 0;
        printf("= text="); hex(q->buffer, q->buffer_len);
        printf(" caret=%zu selection=%zu-%zu\n", q->cursor_pos, s0, s1);
        fflush(stdout);
    }
    return 0;
}
'''


def cases(seed, count):
    rng = random.Random(seed)
    pieces = ["a", "b", "word", " ", "  ", "    ", "\t", "\n", "\n", "\n\n", "\t\tx", "    y", "é", "中", "z z"]
    ops = ["indent", "outdent", "kill", "join", "duplicate", "up", "down", "home", "end"]
    rows = []
    for _ in range(count):
        text = "".join(rng.choice(pieces) for _ in range(rng.randrange(0, 14))).encode()
        bounds = [i for i in range(len(text) + 1) if i == len(text) or (text[i] & 0xC0) != 0x80]
        caret = rng.choice(bounds)
        anchor = caret if rng.random() < 0.5 else rng.choice(bounds)
        rows.append(f"{text.hex()} {anchor} {caret} {rng.choice(ops)}")
    return ("\n".join(rows) + "\n").encode()


def filtered(output):
    lines = (l[2:] if l.startswith(b"L ") else l for l in output.splitlines())
    return b"\n".join(l for l in lines if l.startswith(b"= ") or l.startswith(RECORDS))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cesta", type=Path, required=True)
    parser.add_argument("--work", type=Path)
    parser.add_argument("--cases", type=int, default=6000)
    parser.add_argument("--mutations", action="store_true")
    args = parser.parse_args()
    work = (args.work or Path(tempfile.mkdtemp(prefix="quil-lines-", dir=HERE / "build"))).resolve()
    work.mkdir(parents=True, exist_ok=True)

    show = lambda path: run(["git", "show", f"{PIN}:{path}"], cwd=REPO).decode()
    (work / "reference.rs").write_text(rust_reference(show("servers/quil/src/main.rs"), show("servers/quil/src/view.rs"),
                                                      show("crates/sex-pdx/src/text_selection.rs"),
                                                      show("crates/sex-pdx/src/text_encoding.rs")) + RUST_MAIN)
    run(["rustc", "--edition=2021", "-O", work / "reference.rs", "-o", work / "reference"])

    spec = importlib.util.spec_from_file_location("quil_differential", HERE / "tests/quil_differential.py")
    differential = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(differential)
    (work / "provider.c").write_text(differential.PROVIDER)
    run([args.cesta, "export-c", "src/quil.cesta", "-o", work / "quil.c", "--", "-Igenerated/include"])
    (work / "harness.c").write_text(C_MAIN)
    runtime = [HERE / "runtime/client_runtime.c", HERE / "runtime/server_runtime.c",
               *sorted((HERE / "generated/runtime").glob("*.c"))]
    flags = ["-DSEXOS_HOST", "-DSEXOS_QUIL_CURRENT_KEYS=1", "-DSEXOS_QUIL_CURRENT_DOCUMENT=0", "-Dquil_main=quil_main_unused", "-Wno-unused-function",
             "-I", HERE / "generated/include", "-I", work]
    data = cases(20261105, args.cases)
    expected = filtered(run([work / "reference"], data=data))
    builds = 0
    for cc in ("gcc", "clang"):
        for mode, extra in (("O2", ["-O2"]), ("SAN", ["-O1", "-fsanitize=address,undefined", "-fno-sanitize-recover=all"])):
            binary = work / f"harness-{cc}-{mode}"
            run([cc, "-std=c11", *extra, *flags, work / "harness.c", work / "provider.c", *runtime, "-o", binary])
            actual = filtered(run([binary], data=data))
            if actual != expected:
                rows = data.decode().splitlines()
                for k, (a, b) in enumerate(zip(expected.splitlines(), actual.splitlines())):
                    if a != b:
                        step = expected.splitlines()[:k].count(b"") + sum(1 for l in expected.splitlines()[:k] if l.startswith(b"= "))
                        raise SystemExit(f"{cc} {mode} differs at output line {k} (case {step}: {rows[step][:160]}):\n"
                                         f"  rust  {a.decode()[:300]}\n  cesta {b.decode()[:300]}")
                raise SystemExit(f"{cc} {mode} output lengths differ")
            builds += 1
    mutants = []
    if args.mutations:
        base = LayoutText((work / "quil.c").read_text())
        for name, old, new in (("outdent-spaces", "while (k < 4 && (seg + k < stop", "while (k < 3 && (seg + k < stop"),
                               ("join-space", "text[end - 1] != 32u", "text[end - 1] != 33u"),
                               ("move-column", "size_t moved = up ? r0 + col : r0 + a + 1 + col;",
                                "size_t moved = up ? r0 + col : r0 + a + col;"),
                               ("duplicate-break", "quil_replace_scratch[0] = 0x0A;", "quil_replace_scratch[0] = 0x20;"),
                               ("line-local-home", "move_cursor_logged(q, line_start_at(q, q->cursor_pos), \"home\");",
                                "move_cursor_logged(q, 0, \"home\");")):
            assert base.count(old) == 1, (name, base.count(old))
            (work / "mutant.c").write_text(base.replace(old, new))
            (work / "harness-mutant.c").write_text(C_MAIN.replace('#include "quil.c"', '#include "mutant.c"'))
            binary = work / f"mutant-{name}"
            run(["gcc", "-std=c11", "-O2", *flags, work / "harness-mutant.c", work / "provider.c", *runtime, "-o", binary])
            assert filtered(run([binary], data=data)) != expected, ("mutant survived", name)
            mutants.append(name)
    print(f"quil line commands: {args.cases} cases, {expected.count(b'[quil.')} records, {builds} builds equal to upstream "
          f"{PIN[:8]}; {len(mutants)} mutants caught")


if __name__ == "__main__":
    main()
