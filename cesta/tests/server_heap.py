#!/usr/bin/env python3
"""Qualify bounded service heap growth, adoption and allocation behavior."""
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
    parser.add_argument("--work", type=Path)
    parser.add_argument("--mutations", action="store_true")
    args = parser.parse_args()
    work = (args.work or Path(tempfile.mkdtemp(prefix="server-heap-", dir=HERE / "build"))).resolve()
    work.mkdir(parents=True, exist_ok=True)
    source = HERE / "runtime/server_runtime.c"
    common = ["-std=c11", "-g0", "-DSEXOS_HOST", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
              "-Igenerated/include"]
    builds = []
    for compiler in ("gcc", "clang"):
        for mode, extra in (("O0", ["-O0"]), ("O2", ["-O2"]),
                            ("SAN", ["-O1", "-fsanitize=address,undefined", "-fno-sanitize-recover=all", "-fno-pie", "-no-pie"])):
            binary = work / f"{compiler}-{mode}"
            run([compiler, *common, *extra, str(source), HERE / "tests/server_heap/host.c", "-o", binary])
            output = run([binary], env={**os.environ, "ASAN_OPTIONS": "detect_leaks=1:abort_on_error=1"})
            builds.append({"compiler": compiler, "mode": mode, "output": output.decode().strip()})
    mutations = []
    if args.mutations:
        original = LayoutText(source.read_text())
        changes = [
            ("fixed_denial", "if (sexos_heap_growth_state == SEXOS_HEAP_FIXED) return NULL;", "if (false) return NULL;"),
            ("split_bounds", "if (end > node_end) continue;", "if (false) continue;"),
            ("extend_top", "        last->next = node;\n    }\n    heap->top = end;", "        last->next = node;\n    }\n    heap->top = heap->bottom;"),
            ("restart_adoption", "if (reply.pages) {", "if (false) {"),
            ("realloc_preservation", "memcpy(result, pointer, old_size < new_size ? old_size : new_size);", "memcpy(result, pointer, 0);"),
        ]
        for name, before, after in changes:
            assert original.count(before) == 1, (name, original.count(before))
            mutant = work / f"{name}.c"
            mutant.write_text(original.replace(before, after, 1))
            binary = work / name
            built = subprocess.run(["gcc", *common, "-O2", mutant, HERE / "tests/server_heap/host.c", "-o", binary], cwd=HERE, capture_output=True)
            assert built.returncode == 0, (name, built.stderr[-2000:])
            result = subprocess.run([binary], cwd=HERE, capture_output=True)
            assert result.returncode != 0, ("mutation survived", name)
            mutations.append(name)
    summary = {
        "builds": builds,
        "mutations": mutations,
        "scope": "actual server runtime allocator; 128 KiB static arena, current-run page growth/adoption, refusal, alignment, coalescing and restart; no BOOT or provider qualification",
    }
    (work / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")
    print("server heap qualification:", len(builds), "routes;", len(mutations), "mutants")


if __name__ == "__main__":
    main()
