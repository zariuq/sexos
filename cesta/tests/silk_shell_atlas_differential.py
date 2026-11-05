#!/usr/bin/env python3
"""Actual current Atlas bodies, full effects/state, and separate malformed controls."""
import argparse
import hashlib
import json
from pathlib import Path
import random
import re
import subprocess
import sys
import tempfile
from layout_text import LayoutText

from silk_shell_input_differential import declarations, identifiers, item_end

HERE = Path(__file__).resolve().parent.parent
ROOT = HERE.parent
FIX = HERE / "tests/silk_shell_atlas"
INVENTORY = HERE / "interfaces/silk_shell_atlas.source.json"
U64 = (1 << 64) - 1


def run(argv, data=None):
    result = subprocess.run(list(map(str, argv)), input=data, capture_output=True, cwd=HERE)
    if result.returncode:
        raise RuntimeError((list(map(str, argv)), result.returncode, result.stderr[-6000:]))
    return result.stdout


def prepare(work):
    inv = json.loads(INVENTORY.read_text())
    source = subprocess.check_output(["git", "show", inv["pin"] + ":servers/silk-shell/src/main.rs"], cwd=ROOT).decode()
    assert hashlib.sha256(source.encode()).hexdigest() == inv["source_sha256"]
    for dep in inv["dependencies"]:
        assert hashlib.sha256((ROOT / dep["path"]).read_bytes()).hexdigest() == dep["sha256"], dep["path"]
    catalog = declarations((ROOT / "crates/sex-pdx/src/lib.rs").read_text())
    catalog.update(declarations(source))
    bodies, needed, budgets = [], {"FocusRef", "TokenPreset"}, []
    for row in inv["functions"]:
        match = re.search(r"(?m)^(?:pub\s+)?(?:const\s+)?(?:unsafe\s+)?fn\s+" + row["name"] + r"\b", source)
        assert match, row["name"]
        body = source[match.start():item_end(source, match.start())]
        assert hashlib.sha256(body.encode()).hexdigest() == row["sha256"], row["name"]
        statics = re.findall(r"\bstatic mut\s+(\w+)\s*:\s*u32\s*=\s*(\d+)\s*;", body)
        assert [(x["name"], str(x["value"])) for x in row["diagnostics"]] == statics
        budgets.extend(row["diagnostics"])
        bodies.append(body)
        needed |= identifiers(body) & catalog.keys()
    pending, selected = list(needed), {}
    while pending:
        name = pending.pop()
        if name in selected:
            continue
        selected[name] = catalog[name]
        pending.extend((identifiers(catalog[name]) & catalog.keys()) - selected.keys())
    prefix = """#![allow(dead_code,unused_imports,unused_variables,unused_mut,static_mut_refs,unused_attributes)]
extern crate self as sex_pdx;
extern crate alloc;
use core::sync::atomic::{AtomicBool,AtomicU32,Ordering};
macro_rules! serial_println {($($arg:tt)*)=>{println!($($arg)*)};}
#[macro_export]macro_rules! serial_record {($($arg:tt)*)=>{println!($($arg)*)};}
"""
    for name, path in (("silkbar_model", "crates/silkbar-model/src/lib.rs"),
                       ("app_catalog", "crates/sex-pdx/src/app_catalog.rs"),
                       ("app_custody", "servers/silk-shell/src/app_custody.rs"),
                       ("app_lifecycle", "crates/sex-pdx/src/app_lifecycle.rs")):
        prefix += '#[path="' + str(ROOT / path) + '"] mod ' + name + ';\n'
    prefix += "use silkbar_model::{SILKBAR_WORKSPACE_COUNT,UpdateKind};\n"
    text = prefix + "\n".join(selected.values()) + "\n" + "\n".join(bodies) + "\n" + (FIX / "provider.rs").read_text()
    (work / "oracle.rs").write_text(text)
    (work / "budgets.h").write_text("static void init_budgets(void){\n" + "".join(
        " SILK_SHELL_STATE.atlas." + x["field"] + "=" + str(x["value"]) + ";\n" for x in budgets) + "}\n")
    return inv


def vectors(seed, cases):
    rows = []
    def add(*values):
        rows.append([v & U64 for v in values])
    def frame(slot, fid, scene, flags, active, tabs, surfaces, x=10, y=60, w=300, h=200):
        add(101, slot, fid, scene, flags, active, tabs, x, y, w, h, *surfaces)
    rng = random.Random(seed)
    # Every entry, with actual occupied frames; later operations cover diagnostics exhaustion.
    for scenario in range(cases):
        add(100)
        count = rng.randrange(1, 10)
        for slot in range(count):
            tabs = rng.randrange(1, 9)
            surfaces = [100 + slot * 8 + j for j in range(tabs)]
            frame(slot, 3 + slot * 7, rng.randrange(5), rng.randrange(8), rng.randrange(tabs), tabs, surfaces)
        active, selected = rng.randrange(5), rng.randrange(5)
        add(104, active, 100, 1, selected, 1, 3, scenario % 4 != 0)
        add(108, rng.randrange(3), rng.randrange(256), rng.randrange(5))
        for sid in range(100, 172):
            add(103, sid, rng.choice((253, 255, 252, 189, 245, 237, 125, 221)), rng.randrange(8))
        for op in range(40):
            if op == 6:
                add(op, rng.getrandbits(32))
            elif op in (1, 2):
                add(op, rng.choice((0, 100, 101, 156, 153, 201, 999)))
            elif op in (8, 9, 10, 11, 12):
                add(op, rng.choice((0, 1, 4, 5, 255)))
            elif op == 17:
                add(op, rng.choice((0, 2, 3, 4, (1 << 32) - 1, U64)), rng.choice((0, 640, 1280, (1 << 32) - 1)))
            elif op in (18, 37, 38):
                add(op, rng.choice((0, 286, 510, 754, (1 << 31) - 1, 1 << 31)), rng.choice((49, 80, 100, 300, 1 << 31)))
            elif op == 19:
                add(op, rng.choice((1, 2, 6, 0x4b, 0x4d, 0x48, 0x50, 0x1c, 0x1e, 0x2c, 0x1f, 0x11, 0x19, 0)))
            elif op in (22, 23):
                add(op, rng.choice((0, 1, 4, 5, 255)))
            elif op == 26:
                add(op, rng.choice((-1, 0, 1, -(1 << 31), (1 << 31) - 1)))
            elif op == 27:
                add(op, rng.choice((0, 3, 10, 17, (1 << 32) - 1)), rng.randrange(2))
            elif op == 35:
                add(op, 3, active, rng.randrange(5))
            elif op == 36:
                add(op, rng.choice((0, 3, 10, 17)), rng.choice((0, 1, 4, 5, 255)))
            else:
                add(op)
        # A successful restore has a live tab in every occupied slot. Empty/failed
        # restore and malformed records use a separate independent oracle.
        add(100)
        for slot in range(count):
            frame(slot, slot + 1, slot % 5, slot % 8, 0, 1, [100 + slot])
        add(41); add(40); add(42); add(43)
    for value in range(256):
        add(100); add(104, 0, 0, 1, value % 5, 0, 0, 1); add(19, value)
    for index in range(28):
        for dead, closing in ((0, 0), (1, 0), (0, 1)):
            add(100); add(107, index, 1000 + index, dead, closing)
            for sid in (153, 154, 156, 201, 300, 301, 302, 306, 338, 339):
                add(1, sid)
            # Original Quil's content and shell frame remain physically in
            # FRAMES while custody can already be terminal or close-pending.
            # The custody guard must run before a matching frame is accepted.
            frame(0, 71, 0, 0, 0, 2, [156, 201])
            add(1, 156); add(1, 201)
    for width in (0, 640, 1280, (1 << 31) - 1, 1 << 31, (1 << 32) - 1):
        for height in (0, 49, 50, 51, 720, 1 << 31, (1 << 31) - 1):
            add(100); add(105, width, height); add(18, 286, 100); add(20)
    return rows


def checksum(header, frames):
    value = 0
    for n in header[:4] + header[5:]:
        value ^= n & 255
    widths = [8, 32, 8, 8, 8, 32, 32, 32, 32, 32] + [64] * 8
    for frame in frames:
        for n, width in zip(frame, widths):
            for byte in range(width // 8):
                value ^= (n >> (8 * byte)) & 255
    return value


def valid_snapshot(header, frames):
    occupied = [f for f in frames if f[0]]
    return (header[0] == 83 and header[1] == 1 and header[2] < 5
            and header[3] == len(occupied) and len(occupied) <= 9
            and not any(header[5:]) and header[4] == checksum(header, frames)
            and len({f[1] for f in occupied}) == len(occupied)
            and all(f[0] == 1 and f[2] < 5 and 1 <= f[4] <= 8
                    and f[3] < f[4] and not f[5] & ~7 for f in occupied))


def malformed_controls(binary, work):
    cases = []
    for slot in range(9):
        for field, values in {0: [0, 1, 2, 255], 2: [0, 4, 5, 255],
                              3: [0, 1, 7, 8, 255], 4: [0, 1, 8, 9, 255],
                              5: [0, 7, 8, 1 << 31]}.items():
            for value in values:
                frames = [[0] * 18 for _ in range(9)]
                frames[slot] = [1, 73, 0, 0, 1, 0, 10, 60, 300, 200, 100] + [0] * 7
                frames[slot][field] = value
                header = [83, 1, 0, 1, 0, 0, 0, 0]
                header[4] = checksum(header, frames)
                cases.append((header, frames, None))
    for field, values in {0: [0, 82, 83, 255], 1: [0, 1, 2], 2: [0, 4, 5, 255],
                          3: [0, 1, 2, 9, 10, 255], 5: [0, 1, 255], 6: [1], 7: [1]}.items():
        for value in values:
            frames = [[0] * 18 for _ in range(9)]
            frames[8] = [1, 73, 0, 0, 1, 0, 10, 60, 300, 200, 100] + [0] * 7
            header = [83, 1, 0, 1, 0, 0, 0, 0]
            header[field] = value
            header[4] = checksum(header, frames)
            cases.append((header, frames, None))
    for duplicate in (False, True):
        frames = [[0] * 18 for _ in range(9)]
        for slot in (0, 8):
            frames[slot] = [1, 73 if duplicate else 73 + slot, 0, 0, 1, 0, 10, 60, 300, 200, 100] + [0] * 7
        header = [83, 1, 0, 2, 0, 0, 0, 0]
        header[4] = checksum(header, frames)
        cases.append((header, frames, None))
    for padding in (0, 1, 127, 255):
        frames = [[0] * 18 for _ in range(9)]
        frames[8] = [1, 73, 0, 0, 1, 0, 10, 60, 300, 200, 100] + [0] * 7
        header = [83, 1, 0, 1, 0, 0, 0, 0]
        header[4] = checksum(header, frames)
        cases.append((header, frames, padding))
        damaged = header.copy(); damaged[4] ^= 1
        cases.append((damaged, frames, None))
    rows, expected = [], []
    for header, frames, padding in cases:
        rows.append([109, *header])
        rows.extend([110, i, *frame] for i, frame in enumerate(frames))
        if padding is not None:
            rows.append([114, 8, padding])
        rows.extend([[40], [42]])
        expected += [checksum(header, frames), int(valid_snapshot(header, frames))]
    data = "".join(" ".join(str(v & U64) for v in row) + "\n" for row in rows).encode()
    out = run([binary], data)
    results = list(map(int, re.findall(rb"(?m)^R (\d+)$", out)))
    assert results == expected, (binary, "independent checksum/whole-slot validation")
    # Empty or entirely dead candidates must leave live frames and active scene
    # intact. Diagnostics are permitted; no publication or cleanup is permitted.
    for dead in (False, True):
        header = [83, 1, 0, int(dead), 0, 0, 0, 0]
        frames = [[0] * 18 for _ in range(9)]
        if dead:
            frames[8] = [1, 73, 0, 0, 1, 0, 10, 60, 300, 200, 0] + [0] * 7
        header[4] = checksum(header, frames)
        rows = [[100], [101, 0, 99, 2, 0, 0, 1, 10, 60, 333, 222, 100],
                [104, 2, 100, 0, 0, 1, 99, 1], [109, *header]]
        rows.extend([110, i, *f] for i, f in enumerate(frames))
        rows.append([43])
        out = run([binary], "".join(" ".join(map(str, row)) + "\n" for row in rows).encode())
        states = re.findall(rb"(?m)^STATE ([^\n]*)", out)
        before, after = states[-2], states[-1]
        def live(s):
            chunks = s.split(b"|")
            return chunks[0].split()[:2], [x for x in chunks if x.startswith(b"F ")]
        assert re.findall(rb"(?m)^R (\d+)$", out) == [b"0"] and live(before) == live(after)
        assert b"call " not in out and b"peer " not in out
    return {"independent_snapshots": len(cases), "failed_restore_preserves_live_layout": 2}


def mutations(original):
    def changed(before, after):
        assert original.count(before) == 1, before
        return original.replace(before, after)
    result = [
        ("ignore_terminal_custody", changed("if(shell_custody_has_surface(sid,true))return false;", "if(false)return false;")),
        ("unframed_native_visible", changed("if(shell_custody_has_surface(sid,false))return false;", "if(false)return false;")),
        ("invalid_scene_visible", changed("return FRAMES[i].scene_id==ACTIVE_SCENE_IDX;", "return FRAMES[i].scene_id!=ACTIVE_SCENE_IDX;")),
        ("stale_generation_visible", changed("if(make_focus_ref(sid,&ref) && !focus_ref_is_current(&ref))", "if(make_focus_ref(sid,&ref) && focus_ref_is_current(&ref))")),
    ]
    match = re.search(r"bool shell_atlas_snap_validate\([^;{}]*\)\s*\{", original)
    assert match
    end = item_end(original, match.start())
    body = LayoutText(original[match.start():end])
    before, after = original[:match.start()], original[end:]
    for name, old, new in (
        ("missing_occupied_count", "count==snap->frame_count", "count<=MAX_FRAMES"),
        ("unbounded_snapshot_tabs", "frame.tab_count>MAX_TABS_PER_FRAME", "false"),
        ("duplicate_snapshot_frame", "if(snap->frames[j].present && snap->frames[j].frame_id==frame.frame_id)return false;", "if(false)return false;"),
    ):
        assert body.count(old) == 1, (name, old)
        result.append((name, before + body.replace(old, new) + after))
    partial = body.replace("i<MAX_FRAMES", "i<snap->frame_count").replace("count==snap->frame_count", "count<=MAX_FRAMES")
    result.append(("original_sparse_prefix_bug", before + partial + after))
    result.append(("failed_restore_publishes", changed("if(!restored){", "for(size_t i=0;i<MAX_FRAMES;++i)FRAMES[i]=pending[i];\n    if(!restored){")))
    match = re.search(r"uint8_t shell_atlas_snap_compute_checksum\([^;{}]*\)\s*\{", original)
    assert match
    end = item_end(original, match.start())
    raw = """uint8_t shell_atlas_snap_compute_checksum(ShellAtlasContext *ctx,const SceneLayoutSnapshot *snap)
{
    (void)ctx;const unsigned char *bytes=(const unsigned char *)snap;uint8_t sum=atlas_xor_word(0);
    for(size_t i=0;i<sizeof *snap;++i)if(i!=4)sum^=bytes[i];
    return sum;
}"""
    result.append(("padding_affects_checksum", original[:match.start()] + raw + original[end:]))
    return result


def compare(binary, data, expected, work):
    actual = run([binary], data)
    if actual != expected:
        (work / (binary.name + ".out")).write_bytes(actual)
        a, b = actual.splitlines(), expected.splitlines()
        line = next((i for i, pair in enumerate(zip(a, b)) if pair[0] != pair[1]), min(len(a), len(b)))
        raise AssertionError((binary.name, line, a[line:line + 1], b[line:line + 1]))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--cesta", type=Path, required=True)
    ap.add_argument("--work", type=Path)
    ap.add_argument("--cases", type=int, default=64)
    ap.add_argument("--seed", type=int, default=20261101)
    ap.add_argument("--mutations", action="store_true")
    args = ap.parse_args()
    work = args.work or Path(tempfile.mkdtemp(prefix="shell-atlas-", dir=HERE / "build"))
    work.mkdir(parents=True, exist_ok=True)
    inv = prepare(work)
    run(["rustc", "--edition=2021", "--cap-lints", "allow", "-O", work / "oracle.rs", "-o", work / "rust"])
    counterexample = run([work / "rust"], b"120\n")
    assert counterexample.startswith(b"COUNTEREXAMPLE 1 255\n"), "original sparse-tail issue changed"
    (work / "source-counterexample.out").write_bytes(counterexample)
    rows = vectors(args.seed, args.cases)
    data = "".join(" ".join(map(str, row)) + "\n" for row in rows).encode()
    (work / "commands.txt").write_bytes(data)
    expected = run([work / "rust"], data)
    (work / "rust.out").write_bytes(expected)
    source, adapter, plan = work / "atlas.c", work / "adapter.c", work / "atlas.plan"
    run([args.cesta, "export-c", "src/silk_shell_atlas.cesta", "--std=c11", "-o", source, "--plan", plan, "--", "-Igenerated/include"])
    run([args.cesta, "export-c", "src/silk_shell_atlas_adapter.cesta", "--std=c11", "-o", adapter, "--", "-Igenerated/include"])
    custody=work/"custody.c"
    run([args.cesta, "export-c", "src/silk_shell_custody.cesta", "--std=c11", "-o", custody, "--", "-Igenerated/include"])
    builds = []
    def build(cc, extra, binary):
        run([cc, "-std=c11", "-g0", "-Wall", "-Wextra", "-Wpedantic", "-Werror", *extra,
             "-Igenerated/include", "-I" + str(work), FIX / "provider.c", source, adapter, custody, "-o", binary])
    for cc in ("gcc", "clang"):
        for mode in ("O0", "O2", "sanitize"):
            extra = ["-O1", "-fsanitize=address,undefined", "-fno-sanitize-recover=all"] if mode == "sanitize" else ["-" + mode]
            binary = work / (cc + "-" + mode)
            build(cc, extra, binary)
            compare(binary, data, expected, work)
            controls = malformed_controls(binary, work)
            builds.append({"compiler": cc, "mode": mode, "commands": len(rows), **controls})
            print(cc, mode, len(rows), "commands/effects/states match", flush=True)
    caught = []
    if args.mutations:
        original = LayoutText(source.read_text())
        try:
            for name, mutant in mutations(original):
                source.write_text(mutant)
                binary = work / name
                build("gcc", ["-O2"], binary)
                actual = subprocess.run([binary], input=data, capture_output=True)
                detected = actual.returncode != 0 or actual.stdout != expected
                if not detected:
                    try:
                        malformed_controls(binary, work)
                    except (AssertionError, RuntimeError):
                        detected = True
                assert detected, ("mutation survived", name)
                caught.append(name)
        finally:
            source.write_text(original)
    (work / "summary.json").write_text(json.dumps({"pin": inv["pin"], "builds": builds,
        "mutations_caught": caught,
        "source_counterexample": "original accepts slot8 with tab_count255 and frame_count1",
        "scope": "Actual 44 current Atlas bodies against scripted peer boundaries; full state and ordered diagnostics/calls. Corrected malformed snapshot rules require separate controls. Not whole-shell boot or current peer implementation qualification."}, indent=2) + "\n")


if __name__ == "__main__":
    main()
