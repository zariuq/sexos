#!/usr/bin/env python3
"""Real RAM and page-table controls for the program-record native loader."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
from layout_text import LayoutText

HERE = Path(__file__).resolve().parent.parent
UNITS = ("frame_lease", "kernel_memory", "program_records", "program_image",
         "program_image_bytes", "program_record_memory", "program_record_tasks")


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
    work = (args.work or Path(tempfile.mkdtemp(prefix="program-record-memory-", dir=HERE / "build"))).resolve()
    work.mkdir(parents=True, exist_ok=True)
    (work / "program_records.h").write_bytes(run([
        args.cesta, "export-c", "src/program_records_interface.cesta", "--"]))
    (work / "program_record_memory.h").write_bytes(run([
        args.cesta, "export-c", "src/program_record_memory_interface.cesta", "--"]))
    (work / "program_record_tasks.h").write_bytes(run([
        args.cesta, "export-c", "src/program_record_tasks_interface.cesta", "--"]))
    sources = {}
    for unit in UNITS:
        source = work / (unit + ".c")
        source.write_bytes(run([
            args.cesta, "export-c", "src/" + unit + ".cesta", "--std=c11", "--",
            "-Igenerated/include", "-Iruntime/kernel"]))
        sources[unit] = source
    flags = ["-std=c11", "-g0", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
             "-ffreestanding", "-fno-builtin", "-ffunction-sections", "-fdata-sections",
             "-DSEXOS_KERNEL_CURRENT_PDX=1", "-DPROGRAM_RECORD_CAPACITY=4",
             "-DPROGRAM_RECORD_POOL_FRAMES=64", "-I" + str(work), "-Itests/kernel_memory",
             "-Igenerated/include", "-Iruntime/kernel", "-Wl,--gc-sections"]

    def inputs(replacement=None):
        return [replacement if replacement and unit == "program_record_memory" else source
                for unit, source in sources.items() if unit != "kernel_memory"]

    def build(cc, extra, name, replacement=None):
        output = work / name
        run([cc, *flags, *extra, "tests/program_record_memory/host.c", *inputs(replacement), "-o", output])
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
            print(cc, mode, "record RAM/page-table controls passed", flush=True)

    changes = [
        ("zero_frame", "kmemset(phys_to_virt(physical), 0, PROGRAM_RECORD_PAGE_BYTES);",
         "(void)physical;"),
        ("segment_window", "memory_write(record, segment->window", "memory_write(record, 0"),
        ("map_absent", "*leaf = memory_expected(memory, entry, false);",
         "*leaf = memory_expected(memory, entry, true);"),
        ("rollback_leaf", "*leaf = 0;\n        cpu_invlpg(va);\n        memory->leaves[index][page] = 0;",
         "(void)leaf;\n        cpu_invlpg(va);\n        memory->leaves[index][page] = 0;"),
        ("dispatch_old", "if (old < table->count) memory_present(table, memory, old, false);\n    memory_present(table, memory, index, true);",
         "memory_present(table, memory, index, true);"),
        ("reap_detach", "*(volatile uint64_t *)(uintptr_t)at = 0;",
         "*(volatile uint64_t *)(uintptr_t)at = at;"),
        ("source_bounds", "segment->offset > image->source.len", "false"),
        ("relocation_write", "if (!relocation.valid ||\n            !memory_write(record, relocation.offset, (const uint8_t *)&relocation.value, 8))",
         "if (!relocation.valid)"),
        ("reap_stale", "snapshot->id != record->task", "false"),
        ("commit_metadata", "ProgramRecordReply committed = program_record_commit(table, pd, task);",
         "ProgramRecordReply committed = {PROGRAM_RECORD_OK, table->live, table->pool_free};"),
        ("hardware_status_check", "if ((actual & ~UINT64_C(0x60)) != memory_expected(memory, entry, entry->present))",
         "if (actual != memory_expected(memory, entry, entry->present))"),
        ("hardware_status_audit", "if ((*(volatile uint64_t *)(uintptr_t)at & ~UINT64_C(0x60)) !=",
         "if (*(volatile uint64_t *)(uintptr_t)at !="),
    ]
    caught = []
    if args.mutations:
        original = LayoutText(sources["program_record_memory"].read_text())
        for name, before, after in changes:
            assert original.count(before) == 1, (name, original.count(before))
            mutant = work / (name + ".c")
            mutant.write_text(original.replace(before, after, 1))
            binary = build("gcc", ["-O2"], name, mutant)
            result = subprocess.run([binary], cwd=HERE, capture_output=True)
            assert result.returncode != 0, ("mutation survived", name)
            caught.append(name)

    names = ["src/frame_lease.cesta", "src/kernel_memory.cesta", "src/kernel_interface.cesta",
             "src/program_records.cesta", "src/program_records_interface.cesta",
             "src/program_record_memory.cesta", "src/program_record_memory_interface.cesta",
             "src/program_record_tasks.cesta", "src/program_record_tasks_interface.cesta",
             "tests/program_record_memory.py", "tests/program_record_memory/host.c"]
    summary = {
        "builds": builds,
        "mutations_caught": caught,
        "source_sha256": {name: hashlib.sha256((HERE / name).read_bytes()).hexdigest()
                          for name in names},
        "scope": ("production record metadata and native loader on real RAM/page tables: absent mappings, "
                  "zeroing, segment/relocation writes, mapping rollback, tenant presence and detach/reuse; "
                  "no task rearm, BOOT grant, scheduler, user instructions or guest boot claim"),
    }
    (work / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")
    print("program-record memory qualification:", len(builds), "routes;", len(caught), "mutants", flush=True)


if __name__ == "__main__":
    main()
