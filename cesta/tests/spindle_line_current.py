#!/usr/bin/env python3
"""Spindle's current line chords and history search against upstream's Rust.

Ctrl+U/W/L/R (line_control) and `history <text>` are taken verbatim from the
pinned main.rs, together with the line, history and scrollback types and
tokenize they use, and driven by the same generated script as the Cesta app's
own functions (SEXOS_SPINDLE_CURRENT). Every step prints the line, cursor,
new scrollback lines and the serial records of those two paths.
"""
import argparse
import importlib.util
import os
from pathlib import Path
import random
import re
import subprocess
import tempfile
from layout_text import LayoutText

HERE = Path(__file__).resolve().parent.parent
REPO = HERE.parent
PIN = "b6be1b451a6c21985d00d192674feedf66bb11dd"
RECORDS = ("[spindle.line.ctrl]", "[spindle.history.search]")


def run(argv, *, data=None, cwd=HERE):
    result = subprocess.run(list(map(str, argv)), cwd=cwd, input=data, capture_output=True)
    if result.returncode:
        raise RuntimeError((argv, result.returncode, result.stdout[-2000:], result.stderr[-6000:]))
    return result.stdout


def item(source, start):
    """The text of the item beginning at `start`, through its matching brace."""
    at = source.index(start)
    while True:
        previous = source.rfind("\n", 0, at - 1) + 1
        if not source[previous:at].lstrip().startswith("#["):
            break
        at = previous
    depth, i = 0, source.index("{", source.index(start))
    while True:
        depth += {"{": 1, "}": -1}.get(source[i], 0)
        i += 1
        if depth == 0:
            return source[at:i]


def rust_reference(main_rs, scroll_view):
    items = [item(main_rs, s) for s in ("enum ViMode {", "struct CmdLine {", "impl CmdLine {", "struct History {",
                                         "impl History {", "struct Scrollback {", "impl Scrollback {",
                                         "fn tokenize(", "fn line_control(")]
    arm = item(main_rs, 'b"history" => {')
    body = arm[arm.index("{") + 1:arm.rindex("}")]
    consts = [line for line in main_rs.splitlines()
              if re.match(r"const (CMD_MAX|MAX_HISTORY|HIST_LINE_BYTES|MAX_SCROLLBACK|MAX_LINE_BYTES):", line)]
    text = ("#![allow(dead_code, unused)]\nmacro_rules! serial_record { ($($t:tt)*) => { println!($($t)*) } }\n"
            "macro_rules! serial_println { ($($t:tt)*) => { println!($($t)*) } }\n"
            "mod scroll_view {\n" + scroll_view + "\n}\n" + "\n".join(consts) + "\n" + "\n".join(items)
            + "\nfn history_command(args: &[u8], sb: &mut Scrollback, hist: &mut History) -> bool {" + body + "}\n")
    return text.replace("sex_pdx::serial_record!", "serial_record!").replace("sex_pdx::serial_println!", "serial_println!")


RUST_MAIN = r'''
fn hex(b: &[u8]) -> String { b.iter().map(|c| format!("{c:02x}")).collect() }
fn main() {
    use std::io::BufRead;
    let (mut line, mut hist, mut sb) = (CmdLine::new(), History::new(), Scrollback::new());
    for op in std::io::stdin().lock().lines() {
        let op = op.unwrap();
        let (kind, arg) = op.split_at(1);
        let before = sb.total_lines;
        match kind {
            "R" => { line = CmdLine::new(); hist = History::new(); sb = Scrollback::new(); }
            "P" => { hist.push(arg.as_bytes()); }
            "S" => { line.clear(); for &b in arg.as_bytes() { line.insert_at(line.len, b); } }
            "M" => { line.cur = arg.parse::<usize>().unwrap().min(line.len); }
            "K" => { line_control(arg.parse::<u8>().unwrap(), &mut line, &hist, &mut sb); }
            _ => { let (cmd, args) = tokenize(arg.as_bytes()); assert_eq!(cmd, b"history"); history_command(args, &mut sb, &mut hist); }
        }
        let shown = (sb.total_lines.saturating_sub(before) as usize).min(8);
        let fresh: Vec<String> = (0..shown).map(|k| hex(sb.get_retained(scroll_view::retained(sb.total_lines) - shown + k))).collect();
        println!("= line={} cur={} total={} new={}", hex(&line.buf[..line.len]), line.cur, sb.total_lines, fresh.join(","));
    }
}
'''

C_MAIN = r'''
#include "spindle.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static void hex(const uint8_t *b, size_t n) { for (size_t i = 0; i < n; i++) printf("%02x", b[i]); }
int main(void) {
    static char op[1024];
    Spindle *sp = &spindle_state;
    memset(sp, 0, sizeof *sp);
    while (fgets(op, sizeof op, stdin)) {
        size_t n = strcspn(op, "\n");
        op[n] = 0;
        const char *arg = op + 1;
        uint32_t before = sp->sb.total_lines;
        switch (op[0]) {
        case 'R': {
            /* Upstream keeps the Ctrl+R state in a static, which outlives a reset. */
            static uint8_t text[CMD_MAX], shown[CMD_MAX];
            size_t text_len = sp->search_len, shown_len = sp->search_shown_len, next = sp->search_next;
            memcpy(text, sp->search_text, sizeof text);
            memcpy(shown, sp->search_shown, sizeof shown);
            memset(sp, 0, sizeof *sp);
            memcpy(sp->search_text, text, sizeof text);
            memcpy(sp->search_shown, shown, sizeof shown);
            sp->search_len = text_len, sp->search_shown_len = shown_len, sp->search_next = next;
            break;
        }
        case 'P': history_push(&sp->hist, (ByteSlice){(const uint8_t *)arg, strlen(arg)}); break;
        case 'S': cmdline_clear(&sp->line); for (size_t i = 0; arg[i]; i++) cmdline_insert_at(&sp->line, sp->line.len, (uint8_t)arg[i]); break;
        case 'M': { size_t m = (size_t)strtoul(arg, 0, 10); sp->line.cur = m < sp->line.len ? m : sp->line.len; break; }
        case 'K': line_control(sp, (uint8_t)strtoul(arg, 0, 10)); break;
        default: (void)dispatch(sp, (ByteSlice){(const uint8_t *)arg, strlen(arg)}); break;
        }
        uint32_t fresh = sp->sb.total_lines > before ? sp->sb.total_lines - before : 0;
        size_t shown = fresh < 8 ? fresh : 8, retained = sp->sb.total_lines < MAX_SCROLLBACK ? sp->sb.total_lines : MAX_SCROLLBACK;
        printf("= line="); hex(sp->line.buf, sp->line.len);
        printf(" cur=%zu total=%u new=", sp->line.cur, (unsigned)sp->sb.total_lines);
        for (size_t k = 0; k < shown; k++) {
            size_t logical = retained - shown + k;
            size_t ring = (sp->sb.write_pos + MAX_SCROLLBACK - retained + logical) % MAX_SCROLLBACK;
            ByteSlice l = scrollback_get(&sp->sb, ring);
            if (k) putchar(',');
            hex(l.ptr, l.len);
        }
        putchar('\n');
        fflush(stdout);
    }
    return 0;
}
'''


def script(seed, count):
    rng = random.Random(seed)
    words = ["calc 2+2", "help", "history", "echo hi", "echo  two  words", "ls", "calc ln(2)",
             "notes", "history clear", "history calc", "x", "", "calc 1/3", "a b c", "  lead"]
    ops = ["R"]
    for n in range(count):
        if n % 40 == 0:
            ops.append("R")
        k = rng.random()
        if k < 0.25:
            ops.append("P" + rng.choice(words))
        elif k < 0.45:
            ops.append("S" + rng.choice(words + ["cal", "ec", "his", "o", "zzz", " "]))
        elif k < 0.55:
            ops.append("M" + str(rng.randrange(0, 20)))
        elif k < 0.85:
            ops.append("K" + str(rng.choice([0x16, 0x11, 0x26, 0x13, 0x13, 0x13, 0x10])))
        else:
            ops.append("D" + rng.choice(["history " + w for w in ["calc", "echo", "e", "zzz", "2", "history", " x"]]
                                         + ["history"]))
    return ("\n".join(ops) + "\n").encode()


def filtered(output):
    """Step lines and the two paths' records; the host provider prefixes serial lines with `L `."""
    lines = (l[2:] if l.startswith(b"L ") else l for l in output.splitlines())
    return b"\n".join(l for l in lines if l.startswith(b"= ") or l.startswith(tuple(r.encode() for r in RECORDS)))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cesta", type=Path, required=True)
    parser.add_argument("--work", type=Path)
    parser.add_argument("--steps", type=int, default=6000)
    parser.add_argument("--mutations", action="store_true")
    args = parser.parse_args()
    work = (args.work or Path(tempfile.mkdtemp(prefix="spindle-line-", dir=HERE / "build"))).resolve()
    work.mkdir(parents=True, exist_ok=True)

    show = lambda path: run(["git", "show", f"{PIN}:{path}"], cwd=REPO).decode()
    (work / "reference.rs").write_text(rust_reference(show("apps/spindle/src/main.rs"), show("apps/spindle/src/scroll_view.rs")) + RUST_MAIN)
    run(["rustc", "--edition=2021", "-O", work / "reference.rs", "-o", work / "reference"])

    spec = importlib.util.spec_from_file_location("spindle_differential", HERE / "tests/spindle_differential.py")
    differential = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(differential)
    (work / "provider.c").write_text(differential.PROVIDER)
    run([args.cesta, "export-c", "src/spindle.cesta", "-o", work / "spindle.c", "--", "-Igenerated/include"])
    units = []
    for unit in ("spindle_calc", "spindle_complete", "spindle_sextime"):
        units.append(work / f"{unit}.c")
        run([args.cesta, "export-c", f"src/{unit}.cesta", "--std=c11", "-o", units[-1], "--", "-Igenerated/include"])
    (work / "harness.c").write_text(C_MAIN)
    runtime = [HERE / "runtime/client_runtime.c", HERE / "runtime/server_runtime.c",
               *sorted((HERE / "generated/runtime").glob("*.c"))]
    data = script(20261105, args.steps)
    expected = filtered(run([work / "reference"], data=data))
    builds = 0
    for cc in ("gcc", "clang"):
        for mode, flags in (("O2", ["-O2"]), ("SAN", ["-O1", "-fsanitize=address,undefined", "-fno-sanitize-recover=all"])):
            binary = work / f"harness-{cc}-{mode}"
            run([cc, "-std=c11", *flags, "-DSEXOS_HOST", "-DSEXOS_SPINDLE_CURRENT=1", "-Dspindle_main=spindle_main_unused",
                 "-I", HERE / "generated/include", "-I", work, work / "harness.c", work / "provider.c", *units, *runtime, "-o", binary])
            actual = filtered(run([binary], data=data))
            if actual != expected:
                for k, (a, b) in enumerate(zip(expected.splitlines(), actual.splitlines())):
                    if a != b:
                        raise SystemExit(f"{cc} {mode} differs at output line {k}:\n  rust  {a.decode()[:300]}\n  cesta {b.decode()[:300]}")
                raise SystemExit(f"{cc} {mode} output lengths differ")
            builds += 1
    mutants = []
    if args.mutations:
        base = LayoutText((work / "spindle.c").read_text())
        for name, old, new in (("word-spaces", "while (a > 0 && line->buf[a - 1] == 32u) a -= 1;", ""),
                               ("search-continue", "sp->search_next = i + 1;", "sp->search_next = i;"),
                               ("search-reset", "sp->search_next = 0;", ""),
                               ("search-skips-itself", '!slice_starts_with(entry, "history ")', "true")):
            assert base.count(old) == 1, (name, base.count(old))
            (work / "mutant.c").write_text(base.replace(old, new))
            (work / "harness-mutant.c").write_text(C_MAIN.replace('#include "spindle.c"', '#include "mutant.c"'))
            binary = work / f"mutant-{name}"
            run(["gcc", "-std=c11", "-O2", "-DSEXOS_HOST", "-DSEXOS_SPINDLE_CURRENT=1", "-Dspindle_main=spindle_main_unused",
                 "-I", HERE / "generated/include", "-I", work, work / "harness-mutant.c", work / "provider.c", *units, *runtime, "-o", binary])
            assert filtered(run([binary], data=data)) != expected, ("mutant survived", name)
            mutants.append(name)
    steps = data.count(b"\n")
    print(f"spindle line chords and history search: {steps} steps, {expected.count(b'[spindle.')} records, {builds} builds equal to upstream {PIN[:8]}; {len(mutants)} mutants caught")


if __name__ == "__main__":
    main()
