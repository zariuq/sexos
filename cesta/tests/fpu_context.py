#!/usr/bin/env python3
"""Qualify per-domain x86 FPU custody with a live control and a broken kernel."""

import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import time


HERE = Path(__file__).resolve().parent.parent
DONE_A = "[fpu.context.done] probe=1 turns=256 xmm=1 mxcsr=1"
DONE_B = "[fpu.context.done] probe=2 turns=256 xmm=1 mxcsr=1"
FAIL = "[fpu.context.fail]"


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def package(args, root: Path, kernel: Path) -> Path:
    command = [
        "python3", str(HERE / "tools/package_client.py"),
        "--reference-tree", str(args.reference_tree.resolve()),
        "--replace", "apps/kaleidoscope=" + str(args.probe_a.resolve()),
        "--replace", "servers/quil=" + str(args.probe_b.resolve()),
        "--kernel", str(kernel.resolve()),
        "--limine", str(args.limine.resolve()),
        "--out", str(root),
    ]
    result = subprocess.run(command, cwd=HERE, check=True, capture_output=True, text=True)
    image = Path(result.stdout.strip())
    if not image.is_file():
        raise RuntimeError("packager did not return an image")
    return image


def boot(image: Path, serial: Path, seconds: int, until: str | None = None) -> float:
    command = [
        "bash", str(HERE / "oracle/run.sh"), str(image), str(serial), str(seconds),
        "--accel", "kvm",
    ]
    if until:
        command += ["--until", until]
    started = time.monotonic()
    subprocess.run(command, cwd=HERE, check=True, capture_output=True)
    return time.monotonic() - started


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--reference-tree", type=Path, required=True)
    parser.add_argument("--kernel", type=Path, required=True)
    parser.add_argument("--corrupt-kernel", type=Path, required=True)
    parser.add_argument("--probe-a", type=Path, required=True)
    parser.add_argument("--probe-b", type=Path, required=True)
    parser.add_argument("--limine", type=Path, required=True)
    parser.add_argument("--work", type=Path, required=True)
    parser.add_argument("--seconds", type=int, default=15)
    args = parser.parse_args()
    if args.seconds < 1:
        raise RuntimeError("seconds must be positive")
    work = args.work.resolve()
    work.mkdir(parents=True, exist_ok=False)
    inputs = [args.kernel, args.corrupt_kernel, args.probe_a, args.probe_b]
    before = {str(path.resolve()): digest(path.resolve()) for path in inputs}

    positive_image = package(args, work / "positive-package", args.kernel)
    positive_serial = work / "positive.serial.log"
    positive_seconds = boot(positive_image, positive_serial, args.seconds)
    positive = positive_serial.read_text(errors="replace")
    if positive.count(DONE_A) != 1 or positive.count(DONE_B) != 1:
        raise RuntimeError("both live domains must finish exactly once")
    for forbidden in (FAIL, "KERNEL PANIC", "unknown syscall", "#GP"):
        if forbidden in positive:
            raise RuntimeError("positive image contains " + forbidden)

    negative_image = package(args, work / "negative-package", args.corrupt_kernel)
    negative_serial = work / "negative.serial.log"
    negative_seconds = boot(negative_image, negative_serial, args.seconds, FAIL)
    negative = negative_serial.read_text(errors="replace")
    if FAIL not in negative:
        raise RuntimeError("broken restore path escaped the live mutation control")

    after = {str(path.resolve()): digest(path.resolve()) for path in inputs}
    if after != before:
        raise RuntimeError("qualification inputs changed during the run")
    summary = {
        "inputs": before,
        "positive_image_sha256": digest(positive_image),
        "negative_image_sha256": digest(negative_image),
        "positive_seconds": positive_seconds,
        "negative_seconds": negative_seconds,
        "positive_markers": [DONE_A, DONE_B],
        "negative_marker": FAIL,
        "scope": "live two-domain XMM15/MXCSR save-restore custody and a kernel restore-omission mutation; not an instruction-level FXSAVE refinement proof",
    }
    (work / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")
    print("FPU context qualification passed: 2 live domains, 256 switches each, restore omission caught")


if __name__ == "__main__":
    main()
