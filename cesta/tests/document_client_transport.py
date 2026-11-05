#!/usr/bin/env python3
"""Qualify correlated document transport without legacy fallback."""
import argparse
import json
import os
import subprocess
import sys
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
    parser.add_argument("--cesta-source", type=Path); parser.add_argument("--lean-root", type=Path)
    args = parser.parse_args(); work = (args.work or Path(tempfile.mkdtemp(
        prefix="document-client-transport-", dir=HERE / "build"))).resolve(); work.mkdir(parents=True, exist_ok=True)
    units = ["document_protocol", "document_client", "document_client_transport"]
    sources = {}; plans = {}; builds = []; common = ["-std=c11", "-g0", "-Wall", "-Wextra", "-Wpedantic", "-Werror", "-Igenerated/include"]
    for compiler in ("gcc", "clang"):
        exported = []
        for unit in units:
            source = work / f"{unit}-{compiler}.c"; command = [args.cesta, "export-c", f"src/{unit}.cesta", "--cc", compiler, "--std=c11"]
            if unit == "document_client_transport":
                plan = work / f"transport-{compiler}.plan"; command += ["--plan", plan]; plans[compiler] = plan
            command += ["-o", source, "--", "-Igenerated/include"]; run(command); exported.append(source)
        sources[compiler] = exported[-1]
        for mode, extra in (("O0", ["-O0"]), ("O2", ["-O2"]), ("SAN", ["-O1", "-fsanitize=address,undefined", "-fno-sanitize-recover=all", "-fno-pie", "-no-pie"])):
            binary = work / f"{compiler}-{mode}"; run([compiler, *common, *extra, "tests/document_client_transport/host.c", *exported, "-o", binary])
            run([binary], env={**os.environ, "ASAN_OPTIONS": "detect_leaks=1:abort_on_error=1"}); builds.append((compiler, mode))
    mutations = []
    if args.mutations:
        base = LayoutText(sources["gcc"].read_text()); dependencies = [work / "document_protocol-gcc.c", work / "document_client-gcc.c"]
        changes = [
            ("specific-poll", "sexos_rpc_poll(entry.request_id)", "sexos_rpc_poll(entry.request_id+1)"),
            ("busy-retained", "reply.status!=SEXOS_RPC_BUSY", "true"),
            ("startup-unsupported", "admitted.status==SEXOS_RPC_BUSY||admitted.status==SEXOS_RPC_UNSUPPORTED", "admitted.status==SEXOS_RPC_BUSY"),
            ("storage-slot", "SEXOS_SLOT_storage", "SEXOS_SLOT_storage+1"),
            ("exact-sequence", "action.request.seq,admitted.value", "action.request.seq+1,admitted.value"),
            ("close-cancel", "sexos_rpc_cancel(transport->entries[i].request_id)", "sexos_rpc_cancel(transport->entries[i].request_id+1)"),
        ]
        for name, old, new in changes:
            assert base.count(old) >= 1, (name, base.count(old)); mutant = work / f"{name}.c"; mutant.write_text(base.replace(old, new, 1)); binary = work / name
            built = subprocess.run(["gcc", *common, "-O2", "tests/document_client_transport/host.c", *dependencies, mutant, "-o", binary], cwd=HERE, capture_output=True)
            if built.returncode == 0:
                result = subprocess.run([binary], cwd=HERE, capture_output=True); assert result.returncode != 0, ("survived", name)
            mutations.append(name)
    admission = []
    if args.lean_root:
        assert args.cesta_source; sys.path.insert(0, str(args.cesta_source / "tests")); import ownership_admission
        for index, plan in enumerate(plans.values()):
            record, = ownership_admission.parse(plan.read_bytes()); admission.append(ownership_admission.admit_record(record, work / f"DocumentClientTransportOwnership{index}.lean", args.lean_root))
    (work / "summary.json").write_text(json.dumps({"builds": builds, "mutations": mutations, "admission": admission}, indent=2) + "\n")
    print("document client transport:", len(builds), "builds,", len(mutations), "mutants,", sum(item["functions"] for item in admission), "admitted")


if __name__ == "__main__": main()
