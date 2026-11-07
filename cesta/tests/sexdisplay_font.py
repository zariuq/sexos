#!/usr/bin/env python3
"""Compare the checked Cesta Display font with upstream's scalar_glyph over every scalar.

The Rust reference takes upstream's FONT_ASCII_5X7, FONT_UNICODE_5X7, FONT_UNSUPPORTED_5X7 and
scalar_glyph verbatim from the pinned Display source; both sides print the rows of every scalar
from 0 through 0x10FFFF.
"""
import argparse
import hashlib
import json
import subprocess
import sys
import tempfile
from pathlib import Path
from layout_text import LayoutText

HERE = Path(__file__).resolve().parent.parent
REPO = HERE.parent


def run(argv):
    result = subprocess.run(list(map(str, argv)), cwd=HERE, capture_output=True)
    if result.returncode:
        raise RuntimeError((argv, result.returncode, result.stdout[-1000:], result.stderr[-5000:]))
    return result.stdout


def item(source, head, end):
    start = source.index(head)
    return source[start:source.index(end, start + len(head)) + len(end)]


RUST_MAIN = r'''
fn main() {
    use std::io::Write;
    let mut out = std::io::BufWriter::new(std::io::stdout().lock());
    for scalar in 0..=0x10FFFFu32 {
        let rows = scalar_glyph(scalar);
        for r in rows { write!(out, "{:02x}", r).unwrap(); }
        writeln!(out).unwrap();
    }
}
'''

HOST = r'''
#include <stdio.h>
#include "sexdisplay_font.h"
int main(void) {
    for (uint32_t scalar = 0; scalar <= 0x10FFFFu; scalar++) {
        SexdisplayGlyph glyph = {{0}};
        sexdisplay_scalar_glyph(scalar, &glyph);
        for (int i = 0; i < 7; i++) printf("%02x", glyph.rows[i]);
        putchar('\n');
    }
    return 0;
}
'''


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cesta", type=Path, required=True)
    parser.add_argument("--work", type=Path)
    parser.add_argument("--mutations", action="store_true")
    parser.add_argument("--cesta-source", type=Path)
    parser.add_argument("--lean-root", type=Path)
    args = parser.parse_args()
    work = (args.work or Path(tempfile.mkdtemp(prefix="sexdisplay-font-", dir=HERE / "build"))).resolve()
    work.mkdir(parents=True, exist_ok=True)
    inventory = json.loads((HERE / "interfaces/sexdisplay_font.source.json").read_text())
    raw = subprocess.check_output(["git", "show", inventory["pin"] + ":" + inventory["path"]], cwd=REPO)
    assert hashlib.sha256(raw).hexdigest() == inventory["sha256"], inventory
    upstream = raw.decode()
    reference = "\n".join([
        item(upstream, "const FONT_ASCII_5X7: [[u8; 7]; 95] = [", "\n];"),
        item(upstream, "const FONT_UNICODE_5X7: &[(u32, [u8; 7])] = &[", "\n];"),
        item(upstream, "const FONT_UNSUPPORTED_5X7: [u8; 7] = ", ";"),
        item(upstream, "fn scalar_glyph(scalar:u32)->[u8;7]{", "\n}\n"),
    ])
    (work / "reference.rs").write_text(reference + RUST_MAIN)
    run(["rustc", "--edition=2021", "-C", "opt-level=2", work / "reference.rs", "-o", work / "reference"])
    expected = run([work / "reference"])
    assert expected.count(b"\n") == 0x110000
    (work / "host.c").write_text(HOST)
    common = ["-std=c11", "-g0", "-Wall", "-Wextra", "-Wpedantic", "-Werror", "-Igenerated/include"]
    sources, plans, builds = {}, {}, []
    for compiler in ("gcc", "clang"):
        source = work / f"font-{compiler}.c"
        plan = work / f"font-{compiler}.plan"
        run([args.cesta, "export-c", "src/sexdisplay_font.cesta", "--cc", compiler, "--std=c11",
             "--plan", plan, "-o", source, "--", "-Igenerated/include"])
        sources[compiler] = source
        plans[plan.read_bytes()] = plan
        for mode, extra in (("O0", ["-O0"]), ("O2", ["-O2"]),
                            ("SAN", ["-O1", "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
                                     "-fno-pie", "-no-pie"])):
            binary = work / f"{compiler}-{mode}"
            run([compiler, *common, *extra, work / "host.c", source, "-o", binary])
            actual = run([binary])
            assert actual == expected, (compiler, mode, next(
                (i, a, b) for i, (a, b) in enumerate(zip(expected.split(b"\n"), actual.split(b"\n"))) if a != b))
            builds.append((compiler, mode))
    mutations = []
    if args.mutations:
        base = LayoutText(sources["gcc"].read_text())
        changes = [
            ("tab-blank", "out->rows[i]=scalar==9?0:sexdisplay_font_unsupported[i];", "out->rows[i]=sexdisplay_font_unsupported[i];"),
            ("ascii-top", "scalar>=32&&scalar<=126", "scalar>=32&&scalar<=125"),
            ("unicode-last", "for(size_t k=0;k<94;k++)", "for(size_t k=0;k<93;k++)"),
            ("unsupported-box", "sexdisplay_font_unsupported[7]={31,17,21,21,21,17,31}", "sexdisplay_font_unsupported[7]={31,17,17,17,17,17,31}"),
            ("latin-glyph", "{0x00C0u,{8,4,14,17,31,17,17}}", "{0x00C0u,{2,4,14,17,31,17,17}}"),
        ]
        for name, old, new in changes:
            assert base.count(old) == 1, (name, base.count(old))
            mutant = work / f"{name}.c"
            mutant.write_text(base.replace(old, new))
            binary = work / name
            built = subprocess.run(["gcc", *common, "-O2", work / "host.c", mutant, "-o", binary], cwd=HERE,
                                   capture_output=True)
            if built.returncode == 0:
                assert subprocess.run([binary], cwd=HERE, capture_output=True).stdout != expected, ("survived", name)
            mutations.append(name)
    admission = []
    if args.lean_root:
        assert args.cesta_source
        sys.path.insert(0, str(args.cesta_source / "tests"))
        import ownership_admission
        for index, plan in enumerate(plans.values()):
            record, = ownership_admission.parse(plan.read_bytes())
            admission.append(ownership_admission.admit_record(
                record, work / f"SexdisplayFontOwnership{index}.lean", args.lean_root))
    (work / "summary.json").write_text(json.dumps({
        "pin": inventory["pin"], "scalars": 0x110000, "builds": builds, "mutations": mutations,
        "admission": admission}, indent=2) + "\n")
    print("sexdisplay font:", 0x110000, "scalars,", len(builds), "builds,", len(mutations), "mutants,",
          sum(entry["functions"] for entry in admission), "admitted")


if __name__ == "__main__":
    main()
