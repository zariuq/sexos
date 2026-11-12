#!/usr/bin/env python3
"""Lean record-task lifecycle and mutation controls."""
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
    work = (args.work or Path(tempfile.mkdtemp(prefix="program-record-tasks-", dir=HERE / "build"))).resolve()
    work.mkdir(parents=True, exist_ok=True)
    (work / "program_record_tasks.h").write_bytes(run([
        args.cesta, "export-c", "src/program_record_tasks_interface.cesta", "--"]))
    (work / "sexos_kernel.h").write_bytes(run([
        args.cesta, "export-c", "src/kernel_interface.cesta", "--",
        "-Igenerated/include", "-Iruntime/kernel"]))
    source = work / "program_record_tasks.c"
    source.write_bytes(run([
        args.cesta, "export-c", "src/program_record_tasks.cesta", "--std=c11", "--",
        "-Igenerated/include", "-Iruntime/kernel"]))
    flags = ["-std=c11", "-g0", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
             "-ffreestanding", "-fno-builtin", "-ffunction-sections", "-fdata-sections",
             "-DSEXOS_KERNEL_CURRENT_PDX=1", "-DPROGRAM_RECORD_CAPACITY=4",
             "-DPROGRAM_RECORD_QUEUE_CAPACITY=2", "-DKERNEL_RUN_QUEUE_CAPACITY=2",
             "-I" + str(work), "-Itests/kernel_memory",
             "-Igenerated/include", "-Iruntime/kernel", "-Wl,--gc-sections"]

    def build(cc, extra, name, input_source=source):
        output = work / name
        run([cc, *flags, *extra, "tests/program_record_tasks/host.c", input_source, "-o", output])
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
            print(cc, mode, "record-task controls passed", flush=True)

    changes = [
        ("rearm_zero", "kmemset(state->stack, 0, PROGRAM_RECORD_KSTACK_BYTES);", "(void)state->stack;"),
        ("id_floor", "task < PROGRAM_RECORD_TASK_FIRST", "false"),
        ("rearm_queued", "record->pd != PROGRAM_RECORD_BASE + index || record->queued ||",
         "record->pd != PROGRAM_RECORD_BASE + index || false ||"),
        ("queue_full", "!queue || !token || queue->count == PROGRAM_RECORD_QUEUE_CAPACITY",
         "!queue || !token || false"),
        ("duplicate_admission", "queue->entries[(queue->head + i) % PROGRAM_RECORD_QUEUE_CAPACITY] == token",
         "queue->entries[(queue->head + i) % PROGRAM_RECORD_QUEUE_CAPACITY] != token"),
        ("queue_phase", "task->phase = PROGRAM_WORKER_QUEUED;", "task->phase = PROGRAM_WORKER_DETACHED;"),
        ("attach_owner", "task->phase != PROGRAM_WORKER_DISPATCHING || task->owner_core != core",
         "task->phase != PROGRAM_WORKER_DISPATCHING || task->owner_core != (core + 1)"),
        ("wake_pending", "task->pending = true;\n    if (task->phase == PROGRAM_WORKER_DETACHED)",
         "task->pending = false;\n    if (task->phase == PROGRAM_WORKER_DETACHED)"),
        ("poll_deadline", "now >= task->wait_deadline", "now && false"),
        ("wake_backpressure", "return PROGRAM_RECORD_RETRY;", "return PROGRAM_RECORD_RECORDED;"),
        ("release_custody", "task->phase != PROGRAM_WORKER_SWITCHING_OUT || task->owner_core != core",
         "task->owner_core != core"),
        ("handoff_limit", "queue->handoff_streak < PROGRAM_RECORD_HANDOFF_STREAK", "false"),
        ("handoff_deferred", "if (!program_record_queue_push(queue, deferred)) queue->handoff = deferred;",
         "(void)deferred;"),
        ("handoff_duplicate", "if (queue->handoff == token) return true;", "if (false) return true;"),
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

    names = ["src/program_record_tasks.cesta", "src/program_record_tasks_interface.cesta",
             "tests/program_record_tasks.py", "tests/program_record_tasks/host.c"]
    summary = {
        "builds": builds,
        "mutations_caught": caught,
        "source_sha256": {name: hashlib.sha256((HERE / name).read_bytes()).hexdigest()
                          for name in names},
        "scope": ("lean record-task rearm, bounded queue custody, wait/wake and backpressure; "
                  "native storage and CPU execution remain separate evidence layers"),
    }
    (work / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")
    print("program-record task qualification:", len(builds), "routes;", len(caught), "mutants", flush=True)


if __name__ == "__main__":
    main()
