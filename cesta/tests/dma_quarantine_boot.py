#!/usr/bin/env python3
"""Fault-inject an unresolved NVMe read and prove its DMA lease stays quarantined."""
import argparse
import hashlib
import json
import re
import subprocess
import sys
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent.parent
REPLACEMENTS = {
    "apps/kaleidoscope": "kaleidoscope",
    "servers/silkbar": "silkbar",
    "servers/sexinput": "sexinput",
    "servers/sexstore": "sexstore",
    "servers/sexdisplay": "sexdisplay",
    "servers/sexbell": "sexbell",
    "servers/linen": "linen",
    "servers/quil": "quil",
    "apps/spindle": "spindle",
    "apps/purple-scanout": "purple-scanout",
    "servers/sexfiles": "sexfiles",
    "apps/sexdrive": "sexdrive-dma-quarantine-probe",
    "servers/sexusb": "sexusb",
    "servers/sexnet": "sexnet",
    "servers/sext": "sext",
    "servers/silk-shell": "silk-shell",
}


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def probe_record_complete(text):
    return re.search(r"\[sexdrive\.dma\.probe\.(?:recovered|failed)\][^\n]*\n", text) is not None


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--reference-tree", type=Path, required=True)
    parser.add_argument("--kernel", type=Path, required=True)
    parser.add_argument("--probe", type=Path, required=True)
    parser.add_argument("--limine", type=Path, required=True)
    parser.add_argument("--nvme-image", type=Path, required=True)
    parser.add_argument("--work", type=Path, required=True)
    parser.add_argument("--seconds", type=int, default=90)
    parser.add_argument("--build-root", type=Path, default=HERE / "build")
    parser.add_argument("--accel", choices=("tcg", "kvm"), default="tcg")
    parser.add_argument("--smp", type=int, default=1)
    args = parser.parse_args()
    work = args.work.resolve()
    work.mkdir(parents=True, exist_ok=False)
    disk = work / "nvme.img"
    disk.write_bytes(args.nvme_image.resolve().read_bytes())
    sys.path.insert(0, str(HERE.parent / "scripts"))
    from desktop_close_gate import QMP

    package = work / "package"
    command = [
        "python3", str(HERE / "tools/package_client.py"),
        "--reference-tree", str(args.reference_tree.resolve()),
    ]
    for path, target in REPLACEMENTS.items():
        binary = args.probe.resolve() if path == "apps/sexdrive" else (args.build_root / target / target).resolve()
        if not binary.is_file():
            raise FileNotFoundError(binary)
        command += ["--replace", path + "=" + str(binary)]
    command += ["--kernel", str(args.kernel.resolve()), "--limine", str(args.limine.resolve()),
                "--out", str(package)]
    with (work / "package.log").open("wb") as stream:
        subprocess.run(command, cwd=HERE, stdout=stream, stderr=subprocess.STDOUT, check=True)
    image = next(package.glob("*.iso"))
    serial = work / "serial.log"
    socket_path = work / "control.sock"
    run = [
        "bash", str(HERE / "oracle/run.sh"), str(image), str(serial), str(args.seconds),
        "--accel", args.accel, "--smp", str(args.smp),
        "--qmp-socket", str(socket_path), "--controller", "--nvme-image", str(disk),
    ]
    if args.accel == "tcg":
        run.append("--fixed-clock")
    rows = []
    with (work / "qemu.log").open("wb") as stream:
        process = subprocess.Popen(run, cwd=HERE, stdout=stream, stderr=subprocess.STDOUT)
        qmp = None
        armed = False
        restored = False
        try:
            deadline = time.monotonic() + args.seconds
            while not socket_path.exists():
                if process.poll() is not None or time.monotonic() > deadline:
                    raise RuntimeError("QMP startup failed")
                time.sleep(.02)
            qmp = QMP(socket_path)
            while time.monotonic() < deadline:
                text = serial.read_text(errors="replace") if serial.exists() else ""
                if re.search(r"KERNEL PANIC|fault\.kill|#PF|#GP", text):
                    raise AssertionError(text[-7000:])
                if not armed and "[sexdrive.dma.probe.arm]" in text:
                    blocks = qmp.cmd("query-block")
                    rows.append({"command": "query-block", "result": blocks})
                    backends = [row for row in blocks if row.get("device") == "nvme0"]
                    if len(backends) != 1:
                        raise AssertionError("owned NVMe backend absent: " + repr(blocks))
                    throttle = dict(device="nvme0", bps=0, bps_rd=1, bps_wr=0,
                                    iops=0, iops_rd=0, iops_wr=0, iops_size=0)
                    rows.append({"command": "block_set_io_throttle", "arguments": throttle,
                                 "result": qmp.cmd("block_set_io_throttle", throttle)})
                    armed = True
                if armed and not restored and "[sexdrive.dma.reset.quiescent]" in text:
                    restore = dict(device="nvme0", bps=0, bps_rd=0, bps_wr=0,
                                   iops=0, iops_rd=0, iops_wr=0, iops_size=0)
                    rows.append({"command": "block_set_io_throttle", "arguments": restore,
                                 "result": qmp.cmd("block_set_io_throttle", restore)})
                    restored = True
                if probe_record_complete(text):
                    break
                if process.poll() is not None:
                    raise RuntimeError("QEMU stopped before DMA probe result")
                time.sleep(.02)
            if not armed:
                raise AssertionError("DMA probe never armed")
            if not restored:
                raise AssertionError("DMA reset never reached observed quiescence")
            qmp.cmd("quit")
            if process.wait(timeout=10) != 0:
                raise RuntimeError("QEMU quit failed")
        finally:
            if qmp:
                if armed and not any(row.get("arguments", {}).get("bps_rd") == 0
                                     for row in rows if row["command"] == "block_set_io_throttle"):
                    try:
                        qmp.cmd("block_set_io_throttle", dict(device="nvme0", bps=0, bps_rd=0,
                            bps_wr=0, iops=0, iops_rd=0, iops_wr=0, iops_size=0))
                    except (OSError, EOFError, RuntimeError):
                        pass
                if process.poll() is None:
                    try:
                        qmp.cmd("quit")
                    except (OSError, EOFError, RuntimeError):
                        pass
                qmp.file.close()
                qmp.sock.close()
            if process.poll() is None:
                process.terminate()
                process.wait(timeout=10)

    text = serial.read_text(errors="replace")
    if "[sexdrive.dma.probe.failed]" in text:
        raise AssertionError(text[-7000:])
    fault = re.search(
        r"\[sexdrive\.dma\.probe\.fault\] first=(\d+) second=(\d+) third=(\d+) retry=(\d+) "
        r"phase=(\d+) tag=(\d+) ready=(\d+) cid_before_retry=(\d+) cid_after_retry=(\d+) "
        r"read_phys=(0x[0-9a-f]+)", text)
    recovered = re.search(
        r"\[sexdrive\.dma\.probe\.recovered\] status=(\d+) phase=(\d+) generation=(\d+) "
        r"device_generation=(\d+) ready=(\d+) read_phys=(0x[0-9a-f]+) resets=(\d+)", text)
    if not fault or not recovered:
        raise AssertionError(text[-7000:])
    values = [int(value, 0) for value in fault.groups()]
    first, second, third, retry, phase, tag, ready, before, after, read_phys = values
    recovered_values = [int(value, 0) for value in recovered.groups()]
    recovered_status, recovered_phase, lease_generation, device_generation, recovered_ready, recovered_phys, resets = recovered_values
    attempts = (first, second, third)
    if (5 not in attempts or any(status not in (0, 5, (1 << 64) - 1) for status in attempts) or
            retry != 5 or phase != 3 or tag != 0 or ready != 0 or before != after):
        raise AssertionError(values)
    if "[sexdrive.dma.reset.deferred] reason=interrupt_receipt" not in text[:fault.start()]:
        raise AssertionError("reset begin did not defer for the authentic queued interrupt")
    if text.count("[sexdrive.dma.quarantine] ready=0 reason=unresolved_submission") != 1:
        raise AssertionError("missing or repeated quarantine event")
    quarantine_at = text.find("[sexdrive.dma.quarantine] ready=0 reason=unresolved_submission")
    retry_refused_at = text.find("[sexdrive.block.read.handoff.err] reason=no_ioq_ready", quarantine_at)
    fault_at = text.find(fault.group(0), retry_refused_at)
    quiescent_at = text.find("[sexdrive.dma.reset.quiescent]", fault_at)
    resumed_at = text.find("[sexdrive.dma.reset.resumed]", quiescent_at)
    recovered_at = text.find(recovered.group(0), resumed_at)
    if not 0 <= quarantine_at < retry_refused_at < fault_at < quiescent_at < resumed_at < recovered_at:
        raise AssertionError("post-quarantine retry did not fail before submission")
    if recovered_values != [0, 0, 2, 2, 1, read_phys, 1]:
        raise AssertionError(recovered_values)
    reset_events = [tuple(int(value, 0) for value in row) for row in re.findall(
        r"\[device\.reset\.epoch\] pd=(\d+) action=(\d+) generation=(\d+) phase=(\d+) "
        r"vector=(\d+) interrupt_revision=(\d+) cc=(0x[0-9a-f]+) csts=(0x[0-9a-f]+) epoch=(\d+) "
        r"internal=([01]) recorded=([01])", text)]
    transitions = [event for event in reset_events if event[1] in (1, 2, 3)]
    acknowledgments = [event for event in reset_events if event[1] == 4]
    if len(transitions) != 3 or not acknowledgments:
        raise AssertionError(reset_events)
    for event, expected_action, expected_generation, expected_phase, expected_ready in zip(
            transitions, (1, 2, 3), (1, 1, 2), (1, 2, 0), (1, 0, 1)):
        pd, action, generation, event_phase, vector, interrupt, cc, csts, epoch = event[:9]
        if (pd, action, generation, event_phase, vector, cc & 1, csts & 1) != (
                2, expected_action, expected_generation, expected_phase, 0x22, expected_ready, expected_ready):
            raise AssertionError(event)
    transition_revision = transitions[0][5]
    begin_index = next(index for index, event in enumerate(reset_events) if event[1] == 1)
    pre_acknowledgments = [event for event in reset_events[:begin_index] if event[1] == 4]
    ack_revisions = [event[5] for event in acknowledgments]
    if (not pre_acknowledgments or pre_acknowledgments[-1][5] != transition_revision or
            len({event[5] for event in transitions}) != 1 or
            ack_revisions != list(range(1, len(ack_revisions) + 1)) or
            any(event[0] != 2 or event[4] != 0x22 or event[3] != 0
                for event in acknowledgments) or
            any(internal == recorded for *_, internal, recorded in reset_events) or
            any(left[8] > right[8] or (right[10] and left[8] == right[8])
                for left, right in zip(reset_events, reset_events[1:]))):
        raise AssertionError(reset_events)
    throttle_rows = [row for row in rows if row["command"] == "block_set_io_throttle"]
    if [row["arguments"]["bps_rd"] for row in throttle_rows] != [1, 0]:
        raise AssertionError(rows)
    summary = {
        "inputs": {"kernel": digest(args.kernel), "probe": digest(args.probe),
                   "image": digest(image), "nvme": digest(disk)},
        "attempt_statuses": attempts,
        "post_quarantine_retry": retry,
        "interrupted_begin_deferred": True,
        "quarantined": {"lease_phase": phase, "lease_tag": tag, "queue_ready": ready},
        "retry_cid_unchanged": before == after,
        "read_buffer": hex(read_phys),
        "recovered": {"status": recovered_status, "lease_phase": recovered_phase,
                      "lease_generation": lease_generation, "device_generation": device_generation,
                      "queue_ready": recovered_ready, "same_read_buffer": recovered_phys == read_phys,
                      "reset_count": resets},
        "kernel_device_epochs": reset_events,
        "reset_transition_revision": transition_revision,
        "acknowledged_interrupt_revisions": ack_revisions,
        "qmp": rows,
        "scope": "Real QEMU NVMe backend throttle after queue creation; submitted read timeout, quarantine and post-quarantine non-submission; authenticated interrupt receipts consumed in order before masked reset; observed CC.EN=0/CSTS.RDY=0; lease retirement into a new controller generation; queue rebuild, unmask and successful reuse of the same bounded buffer.",
    }
    (work / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")
    print("DMA reset probe passed: uncertain read quarantined, controller quiesced, queues rebuilt and same buffer retired into generation 2")


if __name__ == "__main__":
    main()
