#!/usr/bin/env python3
"""Qualify the pinned nonblocking generation-aware document client."""
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
SOURCE_SHA256 = "fd2ce6070d81af83cef71e58c083038d3ae0cf604787635056d1ba5fbc47df67"


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
        prefix="document-client-", dir=HERE / "build"
    ))).resolve()
    work.mkdir(parents=True, exist_ok=True)

    rust = subprocess.check_output(
        ["git", "show", PIN + ":servers/quil/src/document_exchange.rs"], cwd=HERE.parent
    )
    assert hashlib.sha256(rust).hexdigest() == SOURCE_SHA256
    protocol = work / "document-protocol.c"
    run([args.cesta, "export-c", "src/document_protocol.cesta", "--std=c11", "-o", protocol,
         "--", "-Igenerated/include"])
    sources = {}
    plans = {}
    builds = []
    common = ["-std=c11", "-g0", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
              "-Igenerated/include"]
    for compiler in ("gcc", "clang"):
        source = work / f"document-client-{compiler}.c"
        plan = work / f"document-client-{compiler}.plan"
        run([args.cesta, "export-c", "src/document_client.cesta", "--cc", compiler,
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
            run([compiler, *common, *extra, "tests/document_client/host.c", protocol, source,
                 "-o", binary])
            run([binary], env={**os.environ, "ASAN_OPTIONS": "detect_leaks=1:abort_on_error=1"})
            builds.append((compiler, mode))

    mutations = []
    if args.mutations:
        base = LayoutText(sources["gcc"].read_text())
        changes = [
            ("load-range", "object<2||object>=15", "object<2&&object>=15"),
            ("known-delete", "||!state->known_valid[object]", ""),
            ("abort-reserve", "state->sequence>=limit", "state->sequence>limit"),
            ("reply-caller", "caller!=SEXOS_DOCUMENT_SERVER_PD", "(caller!=SEXOS_DOCUMENT_SERVER_PD&&false)"),
            ("known-generation", "state->known[state->object]!=v", "false"),
            ("write-count", "(size_t)reply.value!=entry.count", "false"),
            ("delete-retire", "state->known_valid[state->object]=false", "state->known_valid[state->object]=true"),
            ("cancel-quiescent", "state->quiescent=true", "state->quiescent=false"),
            ("timeout", "state->stall>=SEXOS_DOCUMENT_STALL_LIMIT", "false"),
            ("dead-load", "state->busy||state->fenced||state->dead", "state->busy||state->fenced"),
        ]
        for name, old, new in changes:
            assert base.count(old) >= 1, (name, base.count(old))
            mutant = work / f"{name}.c"
            mutant.write_text(base.replace(old, new, 1))
            binary = work / name
            run(["gcc", *common, "-O2", "tests/document_client/host.c", protocol, mutant,
                 "-o", binary])
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
                record, work / f"DocumentClientOwnership{index}.lean", args.lean_root
            ))

    (work / "summary.json").write_text(json.dumps({
        "pin": PIN, "rust_sha256": SOURCE_SHA256, "builds": builds,
        "mutations": mutations, "admission": admission,
    }, indent=2) + "\n")
    print("document client:", len(builds), "builds,", len(mutations), "mutants,",
          sum(item["functions"] for item in admission), "admitted")


if __name__ == "__main__":
    main()
