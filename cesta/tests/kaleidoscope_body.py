#!/usr/bin/env python3
"""Compare the bounded Cesta browser preview with pinned Rust."""
import argparse
import hashlib
import json
import os
import subprocess
import sys
import tempfile
from pathlib import Path
from layout_text import LayoutText

HERE = Path(__file__).resolve().parent.parent
REPO = HERE.parent
PIN = "44053ea9b017fe89008413f7cc8a6b1a4d28d015"
SOURCE_SHA256 = "f10a13fc887c52dd1ad973cbfaf42f354ca72c864f66607a0f9a465ef4e2d1b0"


def run(argv, *, env=None):
    result = subprocess.run(list(map(str, argv)), cwd=HERE, capture_output=True, env=env)
    if result.returncode:
        raise RuntimeError((argv, result.returncode, result.stdout[-1000:], result.stderr[-5000:]))
    return result.stdout


def cases():
    return [
        (b"", b""),
        (b"", b"network summary"),
        (b"  hello\n\tworld  ", b"unused"),
        (bytes([0, 1, 31, 32, 65, 127, 66, 128, 255]), b"unused"),
        (b"x" * 120, b"unused"),
        ((b"word " * 40)[:192], b"unused"),
        (bytes(range(128, 192)), b"unused"),
        (b"\r\n\t", b" fallback\r\nvalue "),
        (bytes((33 + i % 90) for i in range(192)), b"unused"),
        (b"body wins", b"summary loses"),
    ]


def c_array(data):
    return "{" + (",".join(map(str, data)) if data else "0") + "}"


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cesta", type=Path, required=True)
    parser.add_argument("--work", type=Path)
    parser.add_argument("--mutations", action="store_true")
    parser.add_argument("--cesta-source", type=Path)
    parser.add_argument("--lean-root", type=Path)
    args = parser.parse_args()
    work = (args.work or Path(tempfile.mkdtemp(
        prefix="kaleidoscope-body-", dir=HERE / "build"
    ))).resolve()
    work.mkdir(parents=True, exist_ok=True)
    rust_source = subprocess.check_output(
        ["git", "show", PIN + ":apps/kaleidoscope/src/body_text.rs"], cwd=REPO
    )
    assert hashlib.sha256(rust_source).hexdigest() == SOURCE_SHA256
    (work / "body_text.rs").write_bytes(rust_source)
    selected = cases()
    rust_cases = ",\n".join(
        "(&[" + ",".join(map(str, body)) + "],&[" + ",".join(map(str, summary)) + "])"
        for body, summary in selected
    )
    (work / "reference.rs").write_text(
        "mod body_text;\nuse std::io::Write;\n"
        "static CASES:&[(&[u8],&[u8])]=&[" + rust_cases + "];\n"
        "fn main(){let n:usize=std::env::args().nth(1).unwrap().parse().unwrap();"
        "let p=body_text::build(CASES[n].0,CASES[n].1);let b=p.as_bytes();"
        "print!(\"{}:\",b.len());for x in b{print!(\"{:02x}\",x);}std::io::stdout().flush().unwrap();}\n"
    )
    reference = work / "reference"
    run(["rustc", "--edition=2021", "-C", "opt-level=2", work / "reference.rs", "-o", reference])
    expected = [run([reference, i]) for i in range(len(selected))]
    case_rows = []
    for body, summary in selected:
        case_rows.append("{" + c_array(body) + f",{len(body)}," + c_array(summary) + f",{len(summary)}" + "}")
    (work / "cases.h").write_text(
        "typedef struct Case{uint8_t body[192];size_t body_len;uint8_t summary[192];size_t summary_len;}Case;\n"
        "static const Case cases[]={" + ",\n".join(case_rows) + "};\n"
    )
    (work / "host.c").write_text(
        "#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>\n#include \"kaleidoscope_body.h\"\n#include \"cases.h\"\n"
        "int main(int argc,char **argv){if(argc!=2)return 2;size_t n=strtoull(argv[1],0,10);if(n>=sizeof cases/sizeof *cases)return 2;"
        "KaleidoscopeBodySource b={0},s={0};KaleidoscopeBodyPreview p={0};b.len=cases[n].body_len;s.len=cases[n].summary_len;"
        "memcpy(b.bytes,cases[n].body,b.len);memcpy(s.bytes,cases[n].summary,s.len);kaleidoscope_body_build(&b,&s,&p);"
        "printf(\"%zu:\",p.len);for(size_t i=0;i<p.len;i++)printf(\"%02x\",p.bytes[i]);return 0;}\n"
    )
    common = ["-std=c11", "-g0", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
              "-Igenerated/include", "-I", work]
    sources = {}
    plans = {}
    builds = []
    for compiler in ("gcc", "clang"):
        source = work / f"kaleidoscope-body-{compiler}.c"
        plan = work / f"kaleidoscope-body-{compiler}.plan"
        run([args.cesta, "export-c", "src/kaleidoscope_body.cesta", "--cc", compiler,
             "--std=c11", "--plan", plan, "-o", source, "--", "-Igenerated/include"])
        sources[compiler] = source
        plans[hashlib.sha256(plan.read_bytes()).hexdigest()] = plan
        for mode, extra in (
            ("O0", ["-O0"]), ("O2", ["-O2"]),
            ("SAN", ["-O1", "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
                     "-fno-pie", "-no-pie"]),
        ):
            binary = work / f"{compiler}-{mode}"
            run([compiler, *common, *extra, work / "host.c", source, "-o", binary])
            for i, wanted in enumerate(expected):
                actual = run([binary, i], env={**os.environ, "ASAN_OPTIONS": "detect_leaks=1:abort_on_error=1"})
                assert actual == wanted, (compiler, mode, i, actual, wanted)
            builds.append((compiler, mode))

    mutations = []
    if args.mutations:
        base = LayoutText(sources["gcc"].read_text())
        changes = [
            ("control-bound", "byte <= 32", "byte <= 33"),
            ("delete-bound", "byte == 127", "byte == 126"),
            ("replacement", "byte <= 126 ? byte : 63", "byte <= 126 ? byte : 64"),
            ("truncation-length", "content->len = 96", "content->len = 95"),
            ("ellipsis", "size_t j = 93", "size_t j = 94"),
            ("fallback", "if (content.len == 0) kaleidoscope_body_clean", "if (false) kaleidoscope_body_clean"),
        ]
        for name, old, new in changes:
            assert base.count(old) >= 1, (name, base.count(old))
            mutant = work / f"{name}.c"
            mutant.write_text(base.replace(old, new, 1))
            binary = work / name
            built = subprocess.run(["gcc", *common, "-O2", work / "host.c", mutant, "-o", binary], cwd=HERE, capture_output=True)
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
                record, work / f"KaleidoscopeBodyOwnership{index}.lean", args.lean_root
            ))
    (work / "summary.json").write_text(json.dumps({
        "pin": PIN, "rust_sha256": SOURCE_SHA256, "cases": len(selected),
        "builds": builds, "mutations": mutations, "admission": admission,
    }, indent=2) + "\n")
    print("kaleidoscope body:", len(selected), "Rust cases,", len(builds), "builds,",
          len(mutations), "mutants,", sum(item["functions"] for item in admission), "admitted")


if __name__ == "__main__":
    main()
