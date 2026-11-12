#!/usr/bin/env python3
"""Qualify persistent epoch recovery and replay on real QEMU boots."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import shutil
import struct
import subprocess

from silk_shell_boot import MARKERS, REPLACEMENTS, digest

HERE = Path(__file__).resolve().parent.parent
SECTOR = 512
CHECKSUM_OFFSET = 504
HEADER_SIZE = 24
EVENT_SIZE = 168
EVENT_MAGIC = b"SEXEPOCH"
COMMIT_MAGIC = b"SEXJHEAD"
SESSION_KIND = 255
BASE_LBA = 4096
DATA_SECTORS = 16384
META_SECTORS = 2
FORBIDDEN = ("[sexfiles.epoch.failed]", "[cesta.client.fault]", "KERNEL PANIC",
             "fault.kill", "paging.fault.terminal", "pdx.opcode.unknown")


def run(argv):
    result = subprocess.run(list(map(str, argv)), cwd=HERE, capture_output=True)
    if result.returncode:
        raise RuntimeError((argv, result.returncode, result.stdout[-2000:], result.stderr[-4000:]))
    return result.stdout


def checksum(sector):
    value = 14695981039346656037
    for byte in sector[:CHECKSUM_OFFSET]:
        value ^= byte
        value = (value * 1099511628211) & 0xFFFFFFFFFFFFFFFF
    return value


def commit_record(generation, next_sequence, sessions, next_event):
    sector = bytearray(SECTOR)
    sector[:8] = COMMIT_MAGIC
    struct.pack_into("<IIQQQQ", sector, 8, 1, 0, generation, next_sequence, sessions, next_event)
    struct.pack_into("<Q", sector, CHECKSUM_OFFSET, checksum(sector))
    return bytes(sector)


def event_record(sequence, payload):
    assert len(payload) == EVENT_SIZE and sequence
    sector = bytearray(SECTOR)
    sector[:8] = EVENT_MAGIC
    struct.pack_into("<IIQ", sector, 8, 1, 0, sequence)
    sector[HEADER_SIZE:HEADER_SIZE + EVENT_SIZE] = payload
    struct.pack_into("<Q", sector, CHECKSUM_OFFSET, checksum(sector))
    return bytes(sector)


def session_payload(session):
    payload = bytearray(EVENT_SIZE)
    payload[16] = SESSION_KIND
    struct.pack_into("<Q", payload, 24, session)
    return bytes(payload)


def ordinary_payload(epoch):
    payload = bytearray(EVENT_SIZE)
    struct.pack_into("<QII", payload, 0, epoch, 13, 0)
    payload[16] = 1
    struct.pack_into("<Q", payload, 24, epoch)
    return bytes(payload)


def inspect(image):
    image = Path(image)
    contents = image.read_bytes()
    start = BASE_LBA * SECTOR
    extent = (DATA_SECTORS + META_SECTORS) * SECTOR
    assert start <= len(contents) and extent <= len(contents) - start, "journal extent exceeds image"
    journal = contents[start:start + extent]
    candidates = []
    for slot in range(META_SECTORS):
        sector = journal[(DATA_SECTORS + slot) * SECTOR:(DATA_SECTORS + slot + 1) * SECTOR]
        if sector[:8] != COMMIT_MAGIC:
            continue
        version, flags, generation, next_sequence, sessions, next_event = struct.unpack_from("<IIQQQQ", sector, 8)
        if (version != 1 or flags or not generation or not next_sequence or
                next_sequence > DATA_SECTORS + 1 or any(sector[48:CHECKSUM_OFFSET]) or
                struct.unpack_from("<Q", sector, CHECKSUM_OFFSET)[0] != checksum(sector)):
            continue
        if ((sessions and next_sequence < 2) or (not sessions and (next_sequence != 1 or next_event != 1))):
            continue
        candidates.append((generation, slot, next_sequence, sessions, next_event))
    assert candidates, "no valid commit metadata"
    generation, slot, committed_next, committed_sessions, committed_event = max(candidates)
    sessions = []
    records = 0
    next_event = 1
    next_sequence = DATA_SECTORS + 1
    boundary = None
    if committed_next == 1:
        boundary = (0, 1, 0)
    for sequence in range(1, DATA_SECTORS + 1):
        sector = journal[(sequence - 1) * SECTOR:sequence * SECTOR]
        if not any(sector):
            next_sequence = sequence
            break
        assert sector[:8] == EVENT_MAGIC, f"bad event magic at sequence {sequence}"
        version, flags, actual_sequence = struct.unpack_from("<IIQ", sector, 8)
        assert version == 1 and not flags and actual_sequence == sequence, f"bad event header at {sequence}"
        assert struct.unpack_from("<Q", sector, CHECKSUM_OFFSET)[0] == checksum(sector), f"checksum at {sequence}"
        assert not any(sector[HEADER_SIZE + EVENT_SIZE:CHECKSUM_OFFSET]), f"event padding at {sequence}"
        payload = sector[HEADER_SIZE:HEADER_SIZE + EVENT_SIZE]
        epoch = struct.unpack_from("<Q", payload, 0)[0]
        kind = payload[16]
        if kind == SESSION_KIND:
            expected = len(sessions) + 1
            assert payload == session_payload(expected), f"session marker at {sequence}"
            sessions.append({"session": expected, "sequence": sequence, "events": 0, "max_epoch": 0})
            next_event = 1
        else:
            assert sessions and epoch == next_event, f"epoch order at {sequence}: {epoch} != {next_event}"
            sessions[-1]["events"] += 1
            sessions[-1]["max_epoch"] = epoch
            next_event = 0 if next_event == 0xFFFFFFFFFFFFFFFF else next_event + 1
        records += 1
        if sequence == committed_next - 1:
            boundary = (len(sessions), next_event, records)
    if next_sequence != DATA_SECTORS + 1:
        assert not any(journal[(next_sequence - 1) * SECTOR:DATA_SECTORS * SECTOR]), "nonblank gap"
    assert boundary is not None and boundary == (committed_sessions, committed_event,
                                                 (0 if committed_next == 1 else committed_next - 1)), \
        f"commit boundary mismatch: {boundary} != {(committed_sessions, committed_event)}"
    return {
        "image_sha256": hashlib.sha256(contents).hexdigest(),
        "commit_generation": generation,
        "commit_slot": slot,
        "committed_next_sequence": committed_next,
        "next_sequence": next_sequence,
        "sessions": sessions,
        "records": records,
        "next_event": next_event,
    }


def append_session(image, session, events):
    info = inspect(image)
    assert len(info["sessions"]) == session - 1
    first = info["next_sequence"]
    count = events + 1
    assert first >= 1 and count <= DATA_SECTORS - first + 1, "journal capacity"
    generation = info["commit_generation"] + 1
    with Path(image).open("r+b") as stream:
        stream.seek((BASE_LBA + first - 1) * SECTOR)
        stream.write(event_record(first, session_payload(session)))
        for epoch in range(1, events + 1):
            stream.write(event_record(first + epoch, ordinary_payload(epoch)))
        next_sequence = first + count
        record = commit_record(generation, next_sequence, session, events + 1)
        stream.seek((BASE_LBA + DATA_SECTORS) * SECTOR)
        stream.write(record * META_SECTORS)
        stream.flush()
    checked = inspect(image)
    assert checked["sessions"][-1]["events"] == events
    assert checked["sessions"][-1]["max_epoch"] == events
    return checked


def package(work, reference, limine, kernel):
    command = ["python3", HERE / "tools/package_client.py", "--reference-tree", reference,
               "--limine", limine, "--out", work / "package"]
    for path, target in REPLACEMENTS.items():
        build_target = "sext-worker-probe" if target == "sext" else target
        binary = (HERE / "build" / build_target / build_target).resolve()
        assert binary.is_file(), binary
        command += ["--replace", f"{path}={binary}"]
    command += ["--kernel", kernel]
    return Path(run(command).decode().strip())


def boot(iso, log, disk, seconds, markers, expected_failure=False):
    command = ["bash", HERE / "oracle/run.sh", iso, log, seconds, "--accel", "kvm",
               "--nvme-persistent", disk, "--network", "--usb-keyboard", "--smp", "4"]
    for marker in markers:
        command += ["--until", marker]
    run(command)
    text = Path(log).read_text(errors="replace")
    if expected_failure:
        assert expected_failure in text
        return text
    for marker in FORBIDDEN:
        assert marker not in text, (marker, log)
    return text


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--reference-tree", type=Path, required=True)
    parser.add_argument("--limine", type=Path, required=True)
    parser.add_argument("--kernel", type=Path, required=True)
    parser.add_argument("--work", type=Path, required=True)
    parser.add_argument("--nvme-image", type=Path, required=True)
    parser.add_argument("--seconds", type=int, default=180)
    parser.add_argument("--replay-seconds", type=int, default=900)
    parser.add_argument("--stress-events", type=int, default=8200)
    args = parser.parse_args()
    assert args.stress_events > 8192
    work = args.work.resolve()
    work.mkdir(parents=True, exist_ok=False)
    source = args.nvme_image.resolve()
    source_hash = digest(source)
    disk = work / "persistent.raw"
    shutil.copyfile(source, disk)
    iso = package(work, args.reference_tree.resolve(), args.limine.resolve(), args.kernel.resolve())
    full = list(MARKERS) + ["[sext.worker.probe]"]
    results = {}

    boot(iso, work / "boot1.log", disk, args.seconds, full)
    first = inspect(disk)
    assert len(first["sessions"]) == 1 and first["sessions"][0]["events"] > 0
    results["boot1"] = first

    boot(iso, work / "boot2.log", disk, args.seconds,
         ["[sexfiles.epoch.replay] sessions=1", "[sexfiles.epoch.flush] session=2"])
    second = inspect(disk)
    assert len(second["sessions"]) >= 2
    results["boot2"] = second

    recovery_disk = work / "generation-recovery.raw"
    shutil.copyfile(disk, recovery_disk)
    latest = inspect(recovery_disk)["commit_slot"]
    with recovery_disk.open("r+b") as stream:
        stream.seek((BASE_LBA + DATA_SECTORS + latest) * SECTOR)
        stream.write(bytes(SECTOR))
    boot(iso, work / "generation-recovery.log", recovery_disk, args.seconds,
         ["[sexfiles.epoch.replay] sessions=2", "[sexfiles.epoch.flush] session=3"])
    recovered = inspect(recovery_disk)
    assert len(recovered["sessions"]) >= 3
    results["generation_recovery"] = recovered

    synthetic = append_session(recovery_disk, 4, args.stress_events)
    assert synthetic["sessions"][3]["events"] == args.stress_events
    results["synthetic_prefix"] = synthetic
    replay_log = boot(iso, work / "long-replay.log", recovery_disk, args.replay_seconds,
                      ["[sexfiles.epoch.replay] sessions=4", "[sexfiles.epoch.flush] session=5"])
    replayed = inspect(recovery_disk)
    assert len(replayed["sessions"]) >= 5
    match = re.search(r"\[sexfiles\.epoch\.replay\] sessions=4 records=(\d+)", replay_log)
    assert match and int(match.group(1)) >= args.stress_events + 1
    results["long_replay"] = replayed
    results["long_replay"]["reported_records"] = int(match.group(1))

    corrupt = work / "corrupt.raw"
    shutil.copyfile(recovery_disk, corrupt)
    with corrupt.open("r+b") as stream:
        stream.seek((BASE_LBA + 10) * SECTOR + 37)
        original = stream.read(1)
        stream.seek((BASE_LBA + 10) * SECTOR + 37)
        stream.write(bytes([original[0] ^ 1]))
    boot(iso, work / "corrupt.log", corrupt, args.seconds,
         ["[sexfiles.epoch.failed] status=7"], expected_failure="[sexfiles.epoch.failed] status=7")
    results["corruption_refused"] = True

    gap = work / "gap.raw"
    shutil.copyfile(recovery_disk, gap)
    with gap.open("r+b") as stream:
        stream.seek((BASE_LBA + 10) * SECTOR)
        stream.write(bytes(SECTOR))
    try:
        inspect(gap)
    except AssertionError:
        results["gap_refused"] = True
    else:
        raise AssertionError("gap was accepted by independent inspector")

    assert digest(source) == source_hash
    summary = {
        "scope": "real persistent-NVMe boots with independent full-prefix inspection; no whole-OS checkpoint or effect replay claim",
        "iso_sha256": digest(iso),
        "source_nvme_sha256": source_hash,
        "stress_events": args.stress_events,
        "results": results,
    }
    (work / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")
    print("persistent epoch boots passed: two boots, generation recovery,",
          args.stress_events, "event prefix replay, corruption and gap refusal")


if __name__ == "__main__":
    main()
