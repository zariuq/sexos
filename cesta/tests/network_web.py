#!/usr/bin/env python3
"""Qualify growable provider flow/socket ownership against the real service heap."""
import argparse
import json
import os
from pathlib import Path
import subprocess
import tempfile
from layout_text import LayoutText

HERE = Path(__file__).resolve().parent.parent


def run(argv, *, env=None):
    result = subprocess.run(list(map(str, argv)), cwd=HERE, capture_output=True, env=env)
    if result.returncode:
        raise RuntimeError((argv, result.returncode, result.stdout[-2000:], result.stderr[-5000:]))
    return result.stdout


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cesta", type=Path, required=True)
    parser.add_argument("--work", type=Path)
    parser.add_argument("--mutations", action="store_true")
    args = parser.parse_args()
    work = (args.work or Path(tempfile.mkdtemp(prefix="network-web-", dir=HERE / "build"))).resolve()
    work.mkdir(parents=True, exist_ok=True)
    (work / "network_web.h").write_bytes(run([args.cesta, "export-c", "src/network_web_interface.cesta", "--"]))
    source = work / "network_web.c"
    source.write_bytes(run([args.cesta, "export-c", "src/network_web.cesta", "--std=c11", "--", "-Igenerated/include"]))
    common = ["-std=c11", "-g0", "-DSEXOS_HOST", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
              "-I" + str(work), "-Igenerated/include"]
    inputs = [source, "runtime/server_runtime.c", "tests/network_web/host.c"]
    builds = []
    for compiler in ("gcc", "clang"):
        for mode, extra in (("O0", ["-O0"]), ("O2", ["-O2"]),
                            ("SAN", ["-O1", "-fsanitize=address,undefined", "-fno-sanitize-recover=all", "-fno-pie", "-no-pie"])):
            binary = work / f"{compiler}-{mode}"
            run([compiler, *common, *extra, *inputs, "-o", binary])
            output = run([binary], env={**os.environ, "ASAN_OPTIONS": "detect_leaks=1:abort_on_error=1"})
            builds.append({"compiler": compiler, "mode": mode, "output": output.decode().strip()})
    mutations = []
    if args.mutations:
        original = LayoutText(source.read_text())
        changes = [
            ("growth_clears_live", "network_web_zero(flows + service->flow_capacity, 0,", "network_web_zero(flows, 0,"),
            ("flow_limit", "if (service->flow_count >= NETWORK_WEB_MAX_FLOWS ||\n        (service->flow_count == service->flow_capacity && !network_web_grow_flows(service)))", "if (false && network_web_grow_flows(service))"),
            ("flow_wrong_task", "flow->owner.task == owner.task)\n            return i;", "flow->owner.task != owner.task)\n            return i;"),
            ("resolver_excludes_other", "if (held < NETWORK_WEB_RESOLVE_HOLD_TICKS)", "if (held >= NETWORK_WEB_RESOLVE_HOLD_TICKS && false)"),
            ("retirement_report", "retired.resolving = true;", "retired.resolving = false;"),
        ]
        for name, before, after in changes:
            assert original.count(before) == 1, (name, original.count(before))
            mutant = work / f"{name}.c"
            mutant.write_text(original.replace(before, after, 1))
            binary = work / name
            built = subprocess.run(["gcc", *common, "-O2", mutant, "runtime/server_runtime.c",
                                    "tests/network_web/host.c", "-o", binary], cwd=HERE, capture_output=True)
            assert built.returncode == 0, (name, built.stderr[-2000:])
            result = subprocess.run([binary], cwd=HERE, capture_output=True)
            assert result.returncode != 0, ("mutation survived", name)
            mutations.append(name)
    summary = {
        "builds": builds,
        "mutations": mutations,
        "scope": "actual growable provider metadata and service heap; exact domain/task/flow ownership, socket namespace, resolver custody, refusal and retirement; no engine, wire, TLS or browser claim",
    }
    (work / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")
    print("network web qualification:", len(builds), "routes;", len(mutations), "mutants")


if __name__ == "__main__":
    main()
