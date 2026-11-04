#!/usr/bin/env python3
"""Run one Cesta AArch64 phase image against the pinned upstream marker contract."""

import argparse
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import time


PHASE2 = (
    "[sex.arm.entry] current_el=2",
    "[sex.arm.el1] ok=1",
    "[sex.arm.dtb]",
    "[sex.arm.uart] pl011=ok",
    "SEX_ARM_ALIVE",
    "SEX_ARM_PHASE2_OK",
)

PHASE3 = PHASE2 + (
    "[sex.arm.mmu] tables=ok",
    "[sex.arm.mmu] on=1",
    "[sex.arm.mmu] wx=1",
    "[sex.arm.mmu] hhdm=ok",
    "SEX_ARM_MMU_OK",
    "[sex.arm.exc] vbar=ok",
    "[sex.arm.el0] enter=1",
    "[sex.arm.neg] el0_read_kernel=aborted",
    "[sex.arm.neg] el0_write_ro=aborted",
    "[sex.arm.neg] el0_exec_data=aborted",
    "[sex.arm.el0] done=1",
    "SEX_ARM_PHASE3_OK",
)

PHASE4 = PHASE3 + (
    "[sex.arm.p4] entry=1",
    "[sex.arm.gic] gicd=ok",
    "[sex.arm.gic] gicr=ok",
    "[sex.arm.gic] cpuif=ok",
    "[sex.arm.gic] ppi30=on",
    "[sex.arm.smp] ap_boot=1",
    "SEX_ARM_AP_OK",
    "[sex.arm.smp] ap_ack=1",
    "[sex.arm.timer] freq=",
    "[sex.arm.sched] start=1",
    "[sex.arm.sched] a_run=1",
    "[sex.arm.sched] a_run=2",
    "[sex.arm.sched] a_run=3",
    "[sex.arm.sched] b_run=1",
    "[sex.arm.sched] b_run=2",
    "[sex.arm.sched] b_run=3",
    "[sex.arm.neg] kill_task=contained",
    "[sex.arm.sched] a_after_dead=10",
    "SEX_ARM_PHASE4_OK",
)

PHASE5 = PHASE4 + (
    "[sex.arm.p5] entry=1",
    "[sex.arm.timer] off=1",
    "[sex.arm.el0] enter2=1",
    "[sex.arm.el0] done2=1",
    "[sex.arm.svc] echo=ok",
    "[sex.arm.svc] cap_bad=ERR_CAP_INVALID",
    "[sex.arm.svc] cap_good=ok",
    "SEX_ARM_PHASE5_OK",
    "[sex.arm.p6] entry=1",
    "[sex.arm.poe] id_aa64mmfr3=",
    "behavior=UNPROVEN",
    "s1poe=0",
    "[sex.arm.poe] isolation=none",
    "SEX_ARM_POE_TAGGED poindex_default=0 nonzero_tags=0",
    "SEX_ARM_PHASE6_OK",
    "[sex.arm.p7] deferred=1",
)

MARKERS = {2: PHASE2, 3: PHASE3, 4: PHASE4, 5: PHASE5}
FORBIDDEN = (
    "[sex.arm.panic]",
    "SEX_ARM_PHASE2_FAIL",
    "SEX_ARM_PHASE3_FAIL",
    "SEX_ARM_PHASE4_FAIL",
    "SEX_ARM_PHASE5_FAIL",
    "FAILCLOSED",
    "MISMATCH",
    "FATAL",
    "unexpected=1",
    "stray_irq",
    "kill_survivor",
    "ap_timeout",
    "psci_ret",
)


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def ordered_positions(text: str, markers: tuple[str, ...]) -> list[int]:
    positions = []
    cursor = -1
    for marker in markers:
        cursor = text.find(marker, cursor + 1)
        if cursor < 0:
            raise AssertionError("missing ordered marker: " + marker)
        positions.append(cursor)
    return positions


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--image", type=Path, required=True)
    parser.add_argument("--phase", type=int, choices=MARKERS, required=True)
    parser.add_argument("--work", type=Path, required=True)
    parser.add_argument("--seconds", type=float, default=12.0)
    args = parser.parse_args()
    if args.seconds <= 0:
        raise ValueError("--seconds must be positive")

    qemu = shutil.which("qemu-system-aarch64")
    if not qemu:
        raise RuntimeError("qemu-system-aarch64 is required for runtime qualification")
    image = args.image.resolve()
    work = args.work.resolve()
    work.mkdir(parents=True, exist_ok=False)
    log = work / "serial.log"
    command = [
        qemu,
        "-M", "virt,gic-version=3,virtualization=on",
        "-cpu", "max",
        "-smp", "2",
        "-m", "512M",
        "-nographic",
        "-no-reboot",
        "-kernel", str(image),
    ]
    started = time.monotonic()
    with log.open("wb") as stream:
        process = subprocess.Popen(command, stdout=stream, stderr=subprocess.STDOUT)
        try:
            deadline = started + args.seconds
            while time.monotonic() < deadline:
                text = log.read_text(errors="replace") if log.exists() else ""
                if any(marker in text for marker in FORBIDDEN):
                    raise AssertionError("forbidden ARM64 marker present")
                try:
                    ordered_positions(text, MARKERS[args.phase])
                    break
                except AssertionError:
                    pass
                if process.poll() is not None:
                    raise RuntimeError("QEMU stopped before the phase markers completed")
                time.sleep(0.02)
            else:
                raise TimeoutError("ARM64 phase marker deadline expired")
        finally:
            if process.poll() is None:
                process.terminate()
                try:
                    process.wait(timeout=3)
                except subprocess.TimeoutExpired:
                    process.kill()
                    process.wait(timeout=3)

    text = log.read_text(errors="replace")
    positions = ordered_positions(text, MARKERS[args.phase])
    forbidden = [marker for marker in FORBIDDEN if marker in text]
    if forbidden:
        raise AssertionError(forbidden)
    if text.count("[sex.arm.entry]") != 1:
        raise AssertionError("entry marker repeated")
    version = subprocess.run([qemu, "--version"], capture_output=True, text=True, check=True).stdout.splitlines()[0]
    summary = {
        "phase": args.phase,
        "image_sha256": digest(image),
        "serial_sha256": digest(log),
        "qemu": version,
        "seconds": time.monotonic() - started,
        "markers": list(MARKERS[args.phase]),
        "ordered_positions": positions,
        "forbidden": forbidden,
        "scope": "Cesta AArch64 runtime image checked against the pinned upstream phase marker contract; Phase 4 uses the current Phase-5 gate's observed scheduler order.",
    }
    (work / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")
    print(f"ARM64 phase {args.phase} runtime passed: {len(positions)} ordered markers under {version}")


if __name__ == "__main__":
    main()
