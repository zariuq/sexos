#!/usr/bin/env python3
"""Qualify the pinned generation-aware document wire protocol."""
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
PIN = "b6be1b451a6c21985d00d192674feedf66bb11dd"
SOURCE_SHA256 = "b16768a8983ba57d17b51949bede4bdb4b6dcfd3981d47d64420680edd45f655"


def run(argv, *, env=None):
    result = subprocess.run(list(map(str, argv)), cwd=HERE, capture_output=True, env=env)
    if result.returncode:
        raise RuntimeError((argv, result.returncode, result.stdout[-1000:], result.stderr[-5000:]))
    return result.stdout


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cesta", type=Path, required=True)
    parser.add_argument("--work", type=Path)
    parser.add_argument("--mutations", action="store_true")
    parser.add_argument("--cesta-source", type=Path)
    parser.add_argument("--lean-root", type=Path)
    args = parser.parse_args()
    work = (args.work or Path(tempfile.mkdtemp(
        prefix="document-protocol-", dir=HERE / "build"
    ))).resolve()
    work.mkdir(parents=True, exist_ok=True)

    rust = subprocess.check_output(
        ["git", "show", PIN + ":crates/sex-pdx/src/document.rs"], cwd=HERE.parent
    )
    assert hashlib.sha256(rust).hexdigest() == SOURCE_SHA256
    builds = []
    plans = {}
    sources = {}
    flags = ["-std=c11", "-g0", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
             "-Igenerated/include"]
    for compiler in ("gcc", "clang"):
        source = work / f"document-{compiler}.c"
        plan = work / f"document-{compiler}.plan"
        run([args.cesta, "export-c", "src/document_protocol.cesta", "--cc", compiler,
             "--std=c11", "--plan", plan, "-o", source, "--", "-Igenerated/include"])
        sources[compiler] = source
        plans[hashlib.sha256(plan.read_bytes()).hexdigest()] = plan
        for mode, extra in (
            ("O0", ["-O0"]),
            ("O2", ["-O2"]),
            ("SAN", ["-O1", "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
                     "-fno-pie", "-no-pie"]),
        ):
            binary = work / f"{compiler}-{mode}"
            run([compiler, *flags, *extra, "tests/document_protocol/host.c", source, "-o", binary])
            run([binary], env={**os.environ, "ASAN_OPTIONS": "detect_leaks=1:abort_on_error=1"})
            builds.append((compiler, mode))

    mutations = []
    if args.mutations:
        base = LayoutText(sources["gcc"].read_text())
        changes = [
            ("sequence-mask", "header&UINT64_C(0xffffff)", "header"),
            ("kind-shift", "header>>24", "header>>25"),
            ("object-shift", "header>>56", "header>>55"),
            ("offset-mask", "(header>>32)&UINT64_C(0xffffff)", "header>>32"),
            ("kind-bound", "r.kind<=SEXOS_DOCUMENT_DELETE", "true"),
            ("success-tag", "success?0x55u:0xe5u", "success?0xe5u:0x55u"),
            ("reply-sequence", "&UINT64_C(0xffffff))!=seq", ")!=seq"),
            ("hash-byte", "hash^payload->bytes[i]", "hash+payload->bytes[i]"),
            ("hash-prime", "UINT64_C(0x100000001b3)", "UINT64_C(0x100000001b1)"),
        ]
        for name, old, new in changes:
            assert base.count(old) >= 1, (name, base.count(old))
            mutant = work / f"{name}.c"
            mutant.write_text(base.replace(old, new, 1))
            binary = work / name
            run(["gcc", *flags, "-O2", "tests/document_protocol/host.c", mutant, "-o", binary])
            result = subprocess.run([binary], cwd=HERE, capture_output=True)
            assert result.returncode != 0, ("survived", name, result.stdout, result.stderr)
            mutations.append(name)

    admission = []
    if args.lean_root:
        assert args.cesta_source
        sys.path.insert(0, str(args.cesta_source / "tests"))
        import ownership_admission
        for index, plan in enumerate(plans.values()):
            record, = ownership_admission.parse(plan.read_bytes())
            admission.append(ownership_admission.admit_record(
                record, work / f"DocumentProtocolOwnership{index}.lean", args.lean_root
            ))

    summary = {
        "pin": PIN,
        "rust_sha256": SOURCE_SHA256,
        "builds": builds,
        "mutations": mutations,
        "admission": admission,
    }
    (work / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")
    print("document protocol:", len(builds), "builds,", len(mutations), "mutants,",
          sum(item["functions"] for item in admission), "admitted")


if __name__ == "__main__":
    main()
