#!/usr/bin/env python3
"""Bounded program-record metadata and lifecycle mutation controls."""
import argparse
import hashlib
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
        raise RuntimeError((argv, result.returncode, result.stdout[-3000:], result.stderr[-6000:]))
    return result.stdout


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cesta", type=Path, required=True)
    parser.add_argument("--work", type=Path)
    parser.add_argument("--mutations", action="store_true")
    args = parser.parse_args()
    work = (args.work or Path(tempfile.mkdtemp(prefix="program-records-", dir=HERE / "build"))).resolve()
    work.mkdir(parents=True, exist_ok=True)
    (work / "program_records.h").write_bytes(run([
        args.cesta, "export-c", "src/program_records_interface.cesta", "--"]))
    source = work / "program_records.c"
    source.write_bytes(run([
        args.cesta, "export-c", "src/program_records.cesta", "--std=c11", "--", "-Igenerated/include"]))
    common = ["-std=c11", "-g0", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
              "-DPROGRAM_RECORD_CAPACITY=20", "-DPROGRAM_RECORD_POOL_FRAMES=256",
              "-I" + str(work), "-Igenerated/include"]

    def build(cc, extra, name, input_source=source):
        output = work / name
        run([cc, *common, *extra, "tests/program_records/host.c", input_source, "-o", output])
        return output

    builds = []
    for cc in ("gcc", "clang"):
        for mode, extra in (("O0", ["-O0"]), ("O2", ["-O2"]),
                            ("SAN", ["-O1", "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
                                     "-fno-pie", "-no-pie"])):
            binary = build(cc, extra, cc + "-" + mode)
            output = run([binary], env={**os.environ, "ASAN_OPTIONS": "detect_leaks=1:abort_on_error=1"})
            (work / (cc + "-" + mode + ".log")).write_bytes(output)
            builds.append({"compiler": cc, "mode": mode})
            print(cc, mode, "program-record controls passed", flush=True)

    changes = [
        ("stale_dispatch", "record->state != PROGRAM_RECORD_LIVE || record->task != task)\n        return PROGRAM_RECORD_DENIED;\n    if (record->killed) {",
         "record->state != PROGRAM_RECORD_LIVE || task == 0)\n        return PROGRAM_RECORD_DENIED;\n    if (record->killed) {"),
        ("stale_reap", "snapshot->id != record->task", "false"),
        ("owner_reap", "snapshot->owner_core != PROGRAM_RECORD_NO_CORE", "false"),
        ("tenant_eviction", "records_evict(table);\n    record_set_present(record, true);",
         "record_set_present(record, true);"),
        ("force_stop", "record->killed = force;", "record->killed = force && false;"),
        ("id_exhaustion", "if (!table->next_task || table->pool_free < plan->needed)",
         "if (table->pool_free < plan->needed)"),
        ("id_wrap", "table->next_task = table->next_task == UINT32_MAX ? 0 : table->next_task + 1;",
         "table->next_task = PROGRAM_RECORD_TASK_FIRST;"),
        ("reply_clear", "record->task = 0;\n        record->reply_pending = false;",
         "record->task = 0;"),
        ("page_permissions", ".frame = frame, .allocated = true, .writable = writable, .executable = executable",
         ".frame = frame, .allocated = true, .writable = writable, .executable = executable || writable"),
        ("free_return", "table->pool[table->pool_free++] = frame;",
         "table->pool[table->pool_free] = frame;"),
        ("start_batch", "uint64_t wanted = count < PROGRAM_RECORD_START_BATCH ? count : PROGRAM_RECORD_START_BATCH;",
         "uint64_t wanted = count;"),
        ("reap_batch", "freed < PROGRAM_RECORD_REAP_BATCH", "freed < PROGRAM_RECORD_CAPACITY"),
        ("stack_guard", "page = PROGRAM_RECORD_STACK_FIRST;", "page = PROGRAM_RECORD_WINDOW_PAGES;"),
    ]
    caught = []
    if args.mutations:
        original = LayoutText(source.read_text())
        for name, before, after in changes:
            assert original.count(before) == 1, (name, original.count(before))
            mutant = work / (name + ".c")
            mutant.write_text(original.replace(before, after, 1))
            binary = build("gcc", ["-O2"], name, mutant)
            result = subprocess.run([binary], cwd=HERE, capture_output=True)
            assert result.returncode != 0, ("mutation survived", name)
            caught.append(name)

    names = ["src/program_records.cesta", "src/program_records_interface.cesta",
             "tests/program_records.py", "tests/program_records/host.c"]
    summary = {
        "builds": builds,
        "mutations_caught": caught,
        "source_sha256": {name: hashlib.sha256((HERE / name).read_bytes()).hexdigest()
                          for name in names},
        "scope": ("bounded program-record metadata, frame-pool accounting, exact task incarnations, "
                  "page-presence tenancy, stop and bounded reaping; no ELF copying, real PTE writes, "
                  "task rearm, BOOT capability grant or guest execution claim"),
    }
    (work / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")
    print("program-record qualification:", len(builds), "routes;", len(caught), "mutants", flush=True)


if __name__ == "__main__":
    main()
