#!/usr/bin/env python3
"""Qualify coherent document publication on existing BOOT Region views."""
import argparse
import json
import os
import subprocess
import tempfile
from pathlib import Path
from layout_text import LayoutText

HERE = Path(__file__).resolve().parent.parent


def run(argv, *, env=None):
    result = subprocess.run(list(map(str, argv)), cwd=HERE, capture_output=True, env=env)
    if result.returncode:
        raise RuntimeError((argv, result.returncode, result.stdout[-1000:], result.stderr[-5000:]))
    return result.stdout


def main():
    parser = argparse.ArgumentParser(); parser.add_argument("--cesta", type=Path, required=True)
    parser.add_argument("--work", type=Path); parser.add_argument("--mutations", action="store_true")
    args = parser.parse_args(); work = (args.work or Path(tempfile.mkdtemp(
        prefix="document-snapshot-", dir=HERE / "build"))).resolve(); work.mkdir(parents=True, exist_ok=True)
    builds = []; sources = {}; common = ["-std=c11", "-g0", "-Wall", "-Wextra", "-Wpedantic", "-Werror", "-Igenerated/include"]
    for compiler in ("gcc", "clang"):
        protocol = work / f"protocol-{compiler}.c"; snapshot = work / f"snapshot-{compiler}.c"
        run([args.cesta, "export-c", "src/document_protocol.cesta", "--cc", compiler, "--std=c11", "-o", protocol, "--", "-Igenerated/include"])
        run([args.cesta, "export-c", "src/document_snapshot.cesta", "--cc", compiler, "--std=c11", "-o", snapshot, "--", "-Igenerated/include"])
        sources[compiler] = snapshot
        for mode, extra in (("O0", ["-O0"]), ("O2", ["-O2"]), ("SAN", ["-O1", "-fsanitize=address,undefined", "-fno-sanitize-recover=all", "-fno-pie", "-no-pie"])):
            binary = work / f"{compiler}-{mode}"; run([compiler, *common, *extra, "tests/document_snapshot/host.c", protocol, snapshot, "-o", binary])
            run([binary], env={**os.environ, "ASAN_OPTIONS": "detect_leaks=1:abort_on_error=1"}); builds.append((compiler, mode))
    mutations = []
    if args.mutations:
        base = LayoutText(sources["gcc"].read_text()); protocol = work / "protocol-gcc.c"
        changes = [
            ("peer", "info.value2!=(writer?subject:11)", "false"),
            ("region", "info.value3!=id", "false"),
            ("writer-busy", "if(previous&1)", "if(false)"),
            ("stale-sequence", "token<=previous", "token<previous"),
            ("identity", "snapshot_load(region,3)!=((uint64_t)generation<<32|(uint64_t)object<<24|sequence)", "false"),
            ("coherence-recheck", "snapshot_load(region,0)!=token||sexos_document_hash(payload)!=hash", "false"),
            ("failure-scrub", "memset(payload,0,sizeof *payload)", "(void)payload"),
        ]
        for name, old, new in changes:
            assert base.count(old) >= 1, (name, base.count(old)); mutant = work / f"{name}.c"; mutant.write_text(base.replace(old, new, 1)); binary = work / name
            built = subprocess.run(["gcc", *common, "-O2", "tests/document_snapshot/host.c", protocol, mutant, "-o", binary], cwd=HERE, capture_output=True)
            if built.returncode == 0:
                result = subprocess.run([binary], cwd=HERE, capture_output=True); assert result.returncode != 0, ("survived", name)
            mutations.append(name)
    (work / "summary.json").write_text(json.dumps({"builds": builds, "mutations": mutations, "native_boundary": "C11 SC atomics over kernel-validated BOOT Region views"}, indent=2) + "\n")
    print("document snapshot:", len(builds), "builds,", len(mutations), "mutants")


if __name__ == "__main__": main()
