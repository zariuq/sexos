#!/usr/bin/env python3
"""Qualify the checked Cesta QOI decoder against pinned Rust."""

import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile

HERE = Path(__file__).resolve().parent.parent
REPO = HERE.parent
PIN = "44053ea9b017fe89008413f7cc8a6b1a4d28d015"
SOURCE_SHA256 = "b26f5a609f453dae74b5109b77e48724635f531ccb88e10699ce92112f46e705"
END = bytes([0, 0, 0, 0, 0, 0, 0, 1])


def run(argv, *, env=None):
    result = subprocess.run(list(map(str, argv)), cwd=HERE, capture_output=True, env=env)
    if result.returncode:
        raise RuntimeError((argv, result.returncode, result.stdout[-2000:], result.stderr[-5000:]))
    return result.stdout


def image(width, height, chunks, channels=4, colorspace=0):
    return b"qoif" + width.to_bytes(4, "big") + height.to_bytes(4, "big") + bytes(
        [channels, colorspace]) + bytes(chunks) + END


def corpus():
    rows = [
        (image(3, 1, [0xff, 200, 100, 50, 7, 0xfe, 1, 2, 3, 0xff, 9, 8, 7, 0]), 0),
        (image(3, 1, [0xc0, 0x00, 0x35]), 0),
        (image(4, 1, [0xff, 1, 2, 3, 4, 0xfe, 65, 2, 3, 14, 0xc0]), 0),
        (image(3, 1, [0xff, 0, 255, 1, 9, 0x4c, 0x7f]), 0),
        (image(3, 1, [0xff, 10, 250, 1, 99, 0x80, 0x0f, 0xbf, 0xf0]), 0),
    ]
    rows += [(image(1, 1, [tag]), 0) for tag in range(0x40, 0x80)]
    rows += [(image(1, 1, [0xff, 1, 2, 3, 4], c, s), 0)
             for c in (3, 4) for s in (0, 1)]
    rows += [(image(48, 48, [0xfd] * 37 + [0xc9]), 0),
             (image(3, 2, [0xfe, 1, 2, 3, 0xc2, 0xfe, 4, 5, 6, 0xc0]), 0)]
    for offset, value in ((0, ord("x")), (7, 0), (7, 49), (11, 0), (11, 49),
                          (12, 0), (12, 2), (12, 5), (13, 2)):
        data = bytearray(image(1, 1, [0xc0])); data[offset] = value; rows.append((bytes(data), 0))
    rows += [(image(w, h, [0xc0]), 0) for w, h in
             ((2**32 - 1, 1), (1, 2**32 - 1), (256, 1), (1, 256))]
    rows += [(bytes(n), 0) for n in range(22)]
    rows.append((bytes(12289), 0))
    rgba = bytearray()
    for _ in range(2304):
        rgba.extend((0xff, 1, 2, 3, 4))
    rows += [(image(48, 48, rgba), 0)]
    padded = bytearray(image(48, 48, rgba)); padded.extend(bytes(12288 - len(padded))); rows.append((bytes(padded), 0))
    for chunk in ([0xfe], [0xfe, 1], [0xfe, 1, 2], [0xff], [0xff, 1],
                  [0xff, 1, 2], [0xff, 1, 2, 3], [0x80], []):
        rows.append((image(1, 1, chunk), 0))
    rows += [(image(1, 1, [0xc1]), 0), (image(48, 1, [0xfd]), 0),
             (image(2, 1, [0xc0, 0xc1]), 0),
             (image(2, 1, [0, 0]), 0), (image(3, 1, [0, 0xc0, 0]), 0),
             (image(3, 1, [0, 1, 0]), 0)]
    for offset in range(8):
        data = bytearray(image(1, 1, [0xc0])); data[-8 + offset] ^= 1; rows.append((bytes(data), 0))
    rows.append((image(1, 1, [0xc0]) + b"\0", 0))
    rows.append((image(1, 1, [0xc0]), 1))
    rows.append((image(1, 1, [0xc0]), 2))
    return rows


def rust_slice(data):
    return "&[" + ",".join(map(str, data)) + "]"


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cesta", type=Path, required=True)
    parser.add_argument("--work", type=Path)
    parser.add_argument("--mutations", action="store_true")
    parser.add_argument("--cesta-source", type=Path)
    parser.add_argument("--lean-root", type=Path)
    args = parser.parse_args()
    work = (args.work or Path(tempfile.mkdtemp(prefix="kaleidoscope-qoi-", dir=HERE / "build"))).resolve()
    work.mkdir(parents=True, exist_ok=True)
    source = subprocess.check_output(["git", "show", f"{PIN}:crates/sex-graphics/src/qoi.rs"], cwd=REPO)
    assert hashlib.sha256(source).hexdigest() == SOURCE_SHA256
    rows = corpus()
    text = "mod qoi {\n" + source.decode().replace("//!", "//") + "\n}\n"
    text += "static CASES:&[(&[u8],u8)]=&[\n" + ",\n".join(
        f"({rust_slice(data)},{action})" for data, action in rows) + "\n];\n"
    text += r'''
fn code(e:qoi::Error)->u8{match e{qoi::Error::InputTooLarge=>1,qoi::Error::Truncated=>2,qoi::Error::InvalidHeader=>3,qoi::Error::InputChanged=>4,qoi::Error::RunOverflow=>5,qoi::Error::ConsecutiveIndex=>6,qoi::Error::InvalidEnd=>7,qoi::Error::TrailingData=>8,qoi::Error::Cancelled=>9}}
fn hash(p:&[u32])->u64{p.iter().flat_map(|x|x.to_le_bytes()).fold(0xcbf29ce484222325u64,|h,b|(h^b as u64).wrapping_mul(0x100000001b3))}
fn main(){let n:usize=std::env::args().nth(1).unwrap().parse().unwrap();let(mut input,action)=(CASES[n].0.to_vec(),CASES[n].1);let mut d=match qoi::Decoder::new(&input){Ok(d)=>d,Err(e)=>{println!("I {}",code(e));return}};let mut p=[0xdeadbeefu32;qoi::MAX_PIXELS];if action==1{input[0]^=1}if action==2{d.cancel(&mut p);let e=d.advance(&input,&mut p).unwrap_err();println!("C {} {} {:016x} {}",code(e),d.produced(),hash(&p),p.iter().all(|x|*x==0)as u8);return}print!("S");let mut failed=None;for _ in 0..32{match d.advance(&input,&mut p){Ok(s)=>{let state=(s==qoi::State::Ready)as u8;print!(":0:{}:{}",state,d.produced());if state==1{break}},Err(e)=>{print!(":{}:0:{}",code(e),d.produced());failed=Some(e);break}}}print!(" {} {} {:016x} {}",d.width(),d.height(),hash(&p),p.iter().all(|x|*x==0)as u8);if let Some(e)=failed{p[0]=1;let again=d.advance(&input,&mut p).unwrap_err();print!(" P{}:{}",code(again),p.iter().all(|x|*x==0)as u8);assert_eq!(code(e),code(again))}println!()}
'''
    (work / "reference.rs").write_text(text)
    reference = work / "reference"
    run(["rustc", "--edition=2021", "-C", "opt-level=2", work / "reference.rs", "-o", reference])
    expected = [run([reference, i]) for i in range(len(rows))]
    definitions, table = [], []
    for i, (data, action) in enumerate(rows):
        definitions.append(f"static const uint8_t case_{i}[]={{" +
                           (",".join(map(str, data)) if data else "0") + "};")
        table.append(f"{{case_{i},{len(data)},{action}}}")
    (work / "cases.h").write_text(
        "typedef struct QoiCase {const uint8_t *bytes;size_t length;uint8_t action;} QoiCase;\n" +
        "\n".join(definitions) + "\nstatic const QoiCase cases[]={\n" +
        ",\n".join(table) + "\n};\n")
    host = HERE / "tests/kaleidoscope_qoi/host.c"
    exports, plans, builds = {}, {}, []
    for compiler in ("gcc", "clang"):
        c_source = work / f"qoi-{compiler}.c"
        plan = work / f"qoi-{compiler}.plan"
        run([args.cesta, "export-c", "src/kaleidoscope_qoi.cesta", "--cc", compiler,
             "--std=c11", "-o", c_source, "--plan", plan, "--", "-Igenerated/include"])
        exports[compiler] = c_source
        plans[hashlib.sha256(plan.read_bytes()).hexdigest()] = plan
        common = ["-std=c11", "-g0", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
                  "-Igenerated/include", "-I", work]
        for mode, flags in (("O0", ["-O0"]), ("O2", ["-O2"]),
                            ("SAN", ["-O1", "-fsanitize=address,undefined",
                                     "-fno-sanitize-recover=all", "-fno-pie", "-no-pie"])):
            binary = work / f"{compiler}-{mode}"
            run([compiler, *common, *flags, host, c_source, "-o", binary])
            for i, wanted in enumerate(expected):
                actual = run([binary, i], env={**os.environ, "ASAN_OPTIONS": "detect_leaks=1:abort_on_error=1"})
                assert actual == wanted, (compiler, mode, i, actual, wanted)
            builds.append((compiler, mode))
    mutations = []
    if args.mutations:
        base = exports["gcc"].read_text()
        changes = [
            ("turn-budget", "KALEIDOSCOPE_QOI_PIXELS_PER_TURN", "(KALEIDOSCOPE_QOI_PIXELS_PER_TURN+1)"),
            ("header-change", "if(!same)return qoi_fail", "if(false)return qoi_fail"),
            ("run-overflow", "run>total-decoder->produced", "false"),
            ("consecutive-index", "previous_present&&previous==slot", "false"),
            ("end-marker", "source->bytes[data_end+i]!=qoi_end[i]", "false"),
            ("erase", "pixels->values[i]=0", "pixels->values[i]=pixels->values[i]"),
        ]
        common = ["-std=c11", "-O2", "-g0", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
                  "-Igenerated/include", "-I", work]
        for name, old, new in changes:
            assert base.count(old) >= 1, (name, base.count(old))
            mutant = work / f"mutant-{name}.c"
            mutant.write_text(base.replace(old, new, 1))
            binary = work / f"mutant-{name}"
            built = subprocess.run(["gcc", *common, host, mutant, "-o", binary],
                                   cwd=HERE, capture_output=True)
            if built.returncode == 0:
                assert any(subprocess.run([binary, str(i)], cwd=HERE, capture_output=True).stdout != wanted
                           for i, wanted in enumerate(expected)), ("survived", name)
            mutations.append(name)
    admission = []
    if args.lean_root:
        assert args.cesta_source
        sys.path.insert(0, str(args.cesta_source / "tests"))
        import ownership_admission
        for index, plan in enumerate(plans.values()):
            record, = ownership_admission.parse(plan.read_bytes())
            admission.append(ownership_admission.admit_record(
                record, work / f"KaleidoscopeQoiOwnership{index}.lean", args.lean_root))
    summary = {"pin": PIN, "source_sha256": SOURCE_SHA256, "cases": len(rows),
               "builds": builds, "mutations": mutations, "admission": admission}
    (work / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")
    print("kaleidoscope qoi:", len(rows), "Rust cases,", len(builds), "builds,",
          len(mutations), "mutants,", sum(item["functions"] for item in admission), "admitted")


if __name__ == "__main__":
    main()
