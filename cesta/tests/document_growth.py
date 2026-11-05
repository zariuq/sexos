#!/usr/bin/env python3
"""Qualify the cooperative document-capacity publication protocol."""
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
PIN = "44053ea9b017fe89008413f7cc8a6b1a4d28d015"
SOURCE_SHA256 = "22606215a455d87292a283899c59f07dff337042e67eb3650563693940a32ad5"


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
        prefix="document-growth-", dir=HERE / "build"
    ))).resolve()
    work.mkdir(parents=True, exist_ok=True)
    rust = subprocess.check_output(
        ["git", "show", PIN + ":servers/sexfiles/src/vfs.rs"], cwd=HERE.parent
    )
    assert hashlib.sha256(rust).hexdigest() == SOURCE_SHA256
    sources = {}
    plans = {}
    builds = []
    common = ["-std=c11", "-g0", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
              "-Igenerated/include"]
    for compiler in ("gcc", "clang"):
        source = work / f"document-growth-{compiler}.c"
        plan = work / f"document-growth-{compiler}.plan"
        run([args.cesta, "export-c", "src/document_growth.cesta", "--cc", compiler,
             "--std=c11", "--plan", plan, "-o", source, "--", "-Igenerated/include"])
        sources[compiler] = source
        plans[hashlib.sha256(plan.read_bytes()).hexdigest()] = plan
        for mode, extra in (
            ("O0", ["-O0"]), ("O2", ["-O2"]),
            ("SAN", ["-O1", "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
                     "-fno-pie", "-no-pie"]),
        ):
            binary = work / f"{compiler}-{mode}"
            run([compiler, *common, *extra, "tests/document_growth/host.c", source,
                 "-o", binary])
            run([binary], env={**os.environ, "ASAN_OPTIONS": "detect_leaks=1:abort_on_error=1"})
            builds.append((compiler, mode))

    mutations = []
    if args.mutations:
        base = LayoutText(sources["gcc"].read_text())
        changes = [
            ("original-order", "original>count", "false&&original>count"),
            ("extent-bound", "count>DOCUMENT_GROWTH_MAX_EXTENTS", "false&&count>DOCUMENT_GROWTH_MAX_EXTENTS"),
            ("zero-extent", "!sector_count||", "false&&!sector_count||"),
            ("block-multiple", "sector_count%DOCUMENT_GROWTH_BLOCK_SECTORS", "false&&sector_count%DOCUMENT_GROWTH_BLOCK_SECTORS"),
            ("attempt-boundary", "plan->descriptor_attempted=true", "plan->descriptor_attempted=false"),
            ("zero-progress", "plan->sector+=DOCUMENT_GROWTH_BLOCK_SECTORS", "plan->sector+=1"),
            ("zero-barrier", "plan->phase=1", "plan->phase=2"),
            ("publication-done", "plan->phase=5;plan->active=false", "plan->phase=5;plan->active=true"),
            ("cancel-polarity", "!plan->descriptor_attempted", "plan->descriptor_attempted"),
            ("in-flight", "plan->in_flight=true", "plan->in_flight=false"),
        ]
        for name, old, new in changes:
            assert base.count(old) >= 1, (name, base.count(old))
            mutant = work / f"{name}.c"
            mutant.write_text(base.replace(old, new, 1))
            binary = work / name
            built = subprocess.run(["gcc", *common, "-O2", "tests/document_growth/host.c",
                                    mutant, "-o", binary], cwd=HERE, capture_output=True)
            if built.returncode == 0:
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
                record, work / f"DocumentGrowthOwnership{index}.lean", args.lean_root
            ))
    (work / "summary.json").write_text(json.dumps({
        "pin": PIN, "rust_sha256": SOURCE_SHA256,
        "builds": builds, "mutations": mutations, "admission": admission,
    }, indent=2) + "\n")
    print("document growth:", len(builds), "builds,", len(mutations), "mutants,",
          sum(item["functions"] for item in admission), "admitted")


if __name__ == "__main__":
    main()
