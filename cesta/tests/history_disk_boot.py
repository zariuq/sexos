#!/usr/bin/env python3
"""Real Spindle incarnations, NVMe history and cold recovery on both kernels."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys
import time

HERE = Path(__file__).resolve().parent.parent
ROOT = HERE.parent
sys.path.insert(0, str(ROOT / "scripts"))
from desktop_close_gate import QMP
from history_roster_evidence import source_spindles, verify_boot_roster
from spindle_durable_reboot_gate import snapshot
from check_spindle_viewport_pixels import verify as pixels
from check_desktop_pixels import font_from_source


def digest(raw):
    return hashlib.sha256(raw).hexdigest()


def fnv(raw):
    value = 0xcbf29ce484222325
    for byte in raw:
        value = ((value ^ byte) * 0x100000001b3) & ((1 << 64) - 1)
    return value


def history_bytes(owner, commands):
    """Independent byte expectation from the commands actually entered."""
    data = b"".join(len(c.encode()).to_bytes(2, "little") + c.encode() for c in commands)
    header = bytearray(b"SHI1" + owner.to_bytes(4, "little")
                       + len(commands).to_bytes(2, "little")
                       + (24 + len(data)).to_bytes(2, "little") + bytes(12))
    header[16:24] = fnv(header + data).to_bytes(8, "little")
    return bytes(header) + data


def journal_reads(text, roster):
    """Join actual journal LBAs to their own submission, CQE and success."""
    records = []
    shapes = list(re.finditer(r"\[sexdrive.nvme.cmd.shape\] path=typed op=READ cid=(\d+) nsid=1 slba=(\d+) nlb=0 bytes=512 ", text))
    for index, shape in enumerate(shapes):
        cid, lba = map(int, shape.groups())
        if not roster["base"] <= lba < roster["base"] + roster["sectors"]:
            continue
        end = shapes[index + 1].start() if index + 1 < len(shapes) else len(text)
        span = text[shape.end():end]
        submitted = re.search(rf"\[sexdrive.nvme.submit.detail\] path=typed op=READ cid={cid} .*lba={lba} bytes=512", span)
        completed = re.search(rf"\[sexdrive.block.read.handoff.nvme.cqe\] cid={cid} ", span)
        success = re.search(rf"\[sexdrive.nvme.poll.end\] path=typed cid={cid} found=1 status=0 ", span)
        if not submitted or not completed or not success or not submitted.start() < completed.start() < success.start():
            raise ValueError("journal read lacks its own successful NVMe completion")
        records.append({"cid": cid, "lba": lba, "serial_start": shape.start(),
                        "serial_end": shape.end() + success.end()})
    if not {roster["base"], roster["base"] + 9} <= {r["lba"] for r in records}:
        raise ValueError("both journal headers require completed device reads")
    return records


class Guest:
    def __init__(self, image, work, seed=None):
        self.work = work
        work.mkdir(parents=True, exist_ok=False)
        self.serial = work / "serial.log"
        self.disk = work / "nvme.img"
        self.disk.write_bytes(bytes(8 * 1024 * 1024) if seed is None else seed)
        self.sock = work / "c.sock"
        if len(str(self.sock).encode()) >= 104:
            raise ValueError("choose a shorter evidence directory for the QMP socket")
        self.actions = []
        self.q = None
        self.process = None
        self.console = None
        self.command = ["qemu-system-x86_64", "-M", "q35", "-cpu", "max,+pku", "-m", "512M",
                        "-cdrom", str(image), "-drive", "if=none,id=nvm,file=" + str(self.disk) + ",format=raw",
                        "-device", "nvme,serial=sexos01,drive=nvm", "-serial", "file:" + str(self.serial),
                        "-qmp", "unix:" + str(self.sock) + ",server=on,wait=off", "-display", "none",
                        "-no-reboot", "-no-shutdown"]

    def __enter__(self):
        self.console = (self.work / "qemu.log").open("wb")
        self.process = subprocess.Popen(self.command, stdout=self.console, stderr=subprocess.STDOUT)
        deadline = time.monotonic() + 20
        try:
            while not self.sock.exists():
                if self.process.poll() is not None or time.monotonic() > deadline:
                    raise RuntimeError("QMP startup failed")
                time.sleep(.05)
            self.q = QMP(self.sock)
            return self
        except BaseException:
            self.__exit__(*sys.exc_info())
            raise

    def __exit__(self, kind, value, traceback):
        try:
            if self.q:
                if self.process.poll() is None:
                    try:
                        self.q.cmd("quit")
                    except (OSError, EOFError, RuntimeError):
                        pass
                self.q.file.close()
                self.q.sock.close()
            if self.process and self.process.poll() is None:
                try:
                    self.process.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    self.process.terminate()
                    self.process.wait(timeout=5)
        finally:
            if self.console:
                self.console.close()
            (self.work / "actions.json").write_text(json.dumps(self.actions, indent=2) + "\n")
            (self.work / "execution.json").write_text(json.dumps({"command": self.command,
                "error": str(value) if kind else None}, indent=2) + "\n")

    def text(self):
        return self.serial.read_text(errors="replace") if self.serial.exists() else ""

    def wait(self, pattern, start=0, timeout=180):
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            text = self.text()
            if re.search(r"KERNEL PANIC|KERNEL PAGE FAULT|DOUBLE FAULT|GP FAULT|panicked at|fault\.kill|\[sext.fatal\]", text):
                raise RuntimeError("guest fault: " + text[-2000:])
            match = re.search(pattern, text[start:])
            if match:
                return match
            if self.process.poll() is not None:
                raise RuntimeError("guest exited")
            time.sleep(.05)
        raise TimeoutError(pattern)

    def key(self, code, pattern=None, timeout=30):
        start = len(self.text())
        self.actions.append({"key": code, "serial_start": start})
        self.q.key(code)
        return self.wait(pattern, start, timeout) if pattern else start

    def focus(self, frame):
        spindle_frames = {row["frame"] for row in source_spindles(ROOT)}
        for _ in range(24):
            events = list(re.finditer(r"\[shell.focus.(?:set\] id=(\d+)|clear\])", self.text()))
            current = int(events[-1][1]) if events and events[-1][1] else 0
            if current == frame:
                return
            if frame in spindle_frames:
                self.key("scroll_lock", rf"\[shell.focus.set\] id={frame}")
                return
            start = len(self.text())
            self.q.cmd("input-send-event", {"events": [{"type": "key", "data": {
                "down": down, "key": {"type": "qcode", "data": key}}}
                for key, down in [("alt", True), ("tab", True), ("tab", False), ("alt", False)]]})
            self.wait(r"\[shell.focus.set\] id=\d+", start, 30)
        raise RuntimeError("cannot focus frame " + str(frame))

    def capture(self, name, target, prompt, font):
        path = self.work / (name + ".ppm")
        errors = ["not rendered"]
        deadline = time.monotonic() + 60
        while time.monotonic() < deadline:
            self.q.cmd("screendump", {"filename": str(path)})
            text = self.text()
            errors = pixels(text, path.read_bytes(), font, target=target,
                            require_prompt=prompt, require_echo=False, require_all_ink=False)
            if not errors:
                return {"file": path.name, "sha256": digest(path.read_bytes()),
                        "serial_end": len(text), "serial_sha256": digest(text.encode()), "errors": []}
            time.sleep(.15)
        raise RuntimeError("actual prompt pixels: " + repr(errors))


def startup(guest):
    guest.wait(r"\[app.surface.presented\] target=15 surface=300 sequence=1 ok=1")
    guest.wait(r"\[shell.app.registered.reply\] target=15 task=\d+ surface=300 committed=1")
    guest.wait(r"\[linen.disk.publish.done\]")
    return verify_boot_roster(ROOT, guest.text())


def receipt(guest, target, task, kind, expected, start=0):
    match = guest.wait(rf"\[spindle.history.receipt\] target={target} task={task} kind={kind} "
                       r"handle=([1-9]\d*) bytes=(\d+) count=(\d+) hash=(0x[0-9a-f]+) error=0", start)
    handle, size, count, fingerprint = match.groups()
    if (int(size), int(count), int(fingerprint, 16)) != (
            len(expected), int.from_bytes(expected[8:10], "little"), fnv(expected)):
        raise AssertionError((kind, target, match.group(0), expected.hex()))
    return {"handle": int(handle), "bytes": int(size), "count": int(count), "hash": fingerprint}


def close_app(guest, row, task):
    target = row["target"]
    guest.focus(row["frame"])
    start = len(guest.text())
    guest.key("f11", rf"\[shell.app.close.request\] target={target} task={task} ")
    guest.wait(rf"\[sext.app.pool.ready\] target={target} task={task} .*ok=1", start)
    span = guest.text()[start:]
    finished = re.search(rf"\[task.finish\] pd={target} task={task} reason=0 terminal=1", span)
    acked = list(re.finditer(rf"\[app.service.ack\] target={target} task={task} holder=(\d+) role=(\d+) acked=(\d+) required=31", span))
    retired = re.search(rf"\[app.private.retired\] target={target} task={task} .*acked=31 required=31 all_flushed=1 pool_reusable=1", span)
    pool = re.search(rf"\[sext.app.pool.ready\] target={target} task={task} .*ok=1", span)
    assert finished and retired and pool and len(acked) == 5
    assert {(int(a[1]), int(a[2])) for a in acked} == {(3, 1), (1, 2), (11, 4), (13, 8), (10, 16)}
    assert finished.start() < min(a.start() for a in acked) <= max(a.start() for a in acked) < retired.start() < pool.start()
    return {"serial_start": start, "serial_end": len(guest.text()), "roles": [1, 2, 4, 8, 16]}


def produce(image, work, font):
    commands, steps = [], []
    rows = source_spindles(ROOT)
    with Guest(image, work) as guest:
        roster = startup(guest)
        tasks = {p["target"]: p["task"] for p in roster["participants"]}
        previous = None
        for row in rows:
            target, task = row["target"], tasks[row["target"]]
            start = len(guest.text())
            if previous:
                guest.key("grave_accent", r"\[shell.palette.item\] idx=0")
                guest.key("ret", rf"\[shell.app.relaunch\] predecessor={previous['target']} successor={target} task={task} ", 180)
                guest.wait(rf"\[app.surface.presented\] target={target} surface={row['surface']} .*ok=1", start)
                guest.wait(rf"\[files.history.recovered\] logical=12 source={previous['target']} source_task={tasks[previous['target']]} target={target} ", start)
                loaded = receipt(guest, target, task, "loaded", history_bytes(target, commands), start)
            else:
                guest.wait(rf"\[spindle.history.receipt\] target={target} task={task} kind=missing .*error=4")
                loaded = None
            guest.focus(row["frame"])
            if commands:
                guest.key("up", rf"\[spindle.history.nav\] dir=up idx=0 len={len(commands[-1])} ok=1")
                recalled = guest.capture("recall-" + str(target), target, "> " + commands[-1], font)
                guest.key("down", r"\[spindle.history.nav\] dir=down idx=0 len=0 ok=1")
            else:
                recalled = None
            command = "echo cesta" + str(target)
            for length, char in enumerate(command, 1):
                guest.key("spc" if char == " " else char,
                          rf"\[spindle.cursor.view\] target={target} sequence=\d+ pos={length} len={length}(?:\s|$)")
            begin = guest.key("ret")
            guest.wait(rf"\[spindle.instance.command\] target={target} task={task} name=echo ok=1", begin)
            commands.append(command)
            expected = history_bytes(target, commands)
            saved = receipt(guest, target, task, "saved", expected, begin)
            durable = guest.wait(rf"\[files.history.durable\] logical=12 target={target} generation=([1-9]\d*) bank=([01]) bytes={len(expected)} hash={fnv(expected):#x} barriers=3 readback=1", begin)
            assert int(durable[1]) == len(commands)
            guest.key("up", rf"\[spindle.history.nav\] dir=up idx=0 len={len(command)} ok=1")
            frame = guest.capture("saved-" + str(target), target, "> " + command, font)
            closed = close_app(guest, row, task)
            steps.append({"target": target, "task": task, "loaded": loaded, "saved": saved,
                          "generation": int(durable[1]), "bank": int(durable[2]),
                          "recalled": recalled, "frame": frame, "closed": closed})
            previous = row
            print("durable app", target, "saved, recalled and retired", flush=True)
        guest.key("grave_accent", r"\[shell.palette.item\] idx=0")
        guest.key("ret", r"\[launcher.execute.denied\] idx=0 .*reason=boot_reserve_exhausted")
    seed = (work / "nvme.img").read_bytes()
    meta = snapshot(seed, roster)
    assert (meta["source"], meta["task"], meta["generation"]) == (rows[-1]["target"], tasks[rows[-1]["target"]], len(commands))
    offset = (roster["base"] + 9 * meta["bank"] + 1) * 512
    assert seed[offset:offset + meta["size"]] == history_bytes(meta["source"], commands)
    result = {"roster": roster, "steps": steps, "commands": commands, "snapshot": meta, "disk_sha256": digest(seed)}
    (work / "result.json").write_text(json.dumps(result, indent=2) + "\n")
    return seed, result


def recover(image, work, font, seed, parent, fault):
    raw = bytearray(seed)
    roster = parent["roster"]
    if fault in ("newest-header", "both-headers"):
        banks = [parent["snapshot"]["bank"]] if fault == "newest-header" else [0, 1]
        for bank in banks:
            raw[(roster["base"] + 9 * bank) * 512] ^= 1
    expected = None if fault == "both-headers" else snapshot(raw, roster)
    with Guest(image, work, raw) as guest:
        cold = startup(guest)
        assert cold == roster
        original = cold["participants"][0]
        target, task = original["target"], original["task"]
        guest.focus(source_spindles(ROOT)[0]["frame"])
        if expected is None:
            guest.wait(rf"\[spindle.history.receipt\] target={target} task={task} kind=failed .*error=6")
            guest.key("up", r"\[spindle.history.nav\] dir=up idx=0 len=0 ok=0")
            assert not re.search(r"\[files.history.recovered\]|\[spindle.history.receipt\].*kind=loaded", guest.text())
            frame = guest.capture("corrupt-empty", target, "> ", font)
            loaded = None
        else:
            commands = parent["commands"][:expected["generation"]]
            source_bytes = history_bytes(expected["source"], commands)
            owned_bytes = history_bytes(target, commands)
            guest.wait(rf"\[files.history.recovered\] logical=12 source={expected['source']} source_task={expected['task']} target={target} generation={expected['generation']} bytes={len(source_bytes)} source_hash={fnv(source_bytes):#x} owned_hash={fnv(owned_bytes):#x} data_hash={fnv(source_bytes[24:]):#x} boot_grant=53")
            loaded = receipt(guest, target, task, "loaded", owned_bytes)
            guest.key("up", rf"\[spindle.history.nav\] dir=up idx=0 len={len(commands[-1])} ok=1")
            frame = guest.capture("recovered", target, "> " + commands[-1], font)
        reads = journal_reads(guest.text(), cold)
    final = (work / "nvme.img").read_bytes()
    start, end = roster["base"] * 512, (roster["base"] + roster["sectors"]) * 512
    assert final[start:end] == raw[start:end], "cold recovery changed durable journal bytes"
    result = {"fault": fault, "expected": expected, "loaded": loaded, "frame": frame, "nvme_reads": reads,
              "journal_unchanged": True, "disk_sha256": digest(final)}
    (work / "result.json").write_text(json.dumps(result, indent=2) + "\n")
    print("cold recovery", fault, "passed", flush=True)
    return result


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--reference-iso", type=Path, required=True)
    ap.add_argument("--reference-tree", type=Path, required=True)
    ap.add_argument("--kernel", type=Path, required=True)
    ap.add_argument("--sext", type=Path, required=True)
    ap.add_argument("--limine", type=Path, required=True)
    ap.add_argument("--work", type=Path, required=True)
    ap.add_argument("--implementation", choices=("both", "rust", "cesta"), default="both")
    args = ap.parse_args()
    work = args.work.resolve()
    work.mkdir(parents=True, exist_ok=False)
    inputs = {name: digest(getattr(args, name).read_bytes()) for name in ("reference_iso", "kernel", "sext")}
    package = work / "package"
    command = ["python3", str(HERE / "tools/package_client.py"), "--reference-tree", str(args.reference_tree.resolve()),
               "--kernel", str(args.kernel.resolve()), "--replace", "servers/sext=" + str(args.sext.resolve()),
               "--limine", str(args.limine.resolve()), "--out", str(package)]
    with (work / "package.log").open("wb") as log:
        subprocess.run(command, cwd=ROOT, stdout=log, stderr=subprocess.STDOUT, check=True)
    manifest = json.loads((package / "manifest.json").read_text())
    assert sorted(manifest["changed_files"]) == ["servers/sext", "sexos-kernel"]
    font = font_from_source((ROOT / "servers/sexdisplay/src/main.rs").read_text())
    records = []
    for label, image in (("rust", args.reference_iso.resolve()), ("cesta", next(package.glob("*.iso")))):
        if args.implementation != "both" and args.implementation != label:
            continue
        seed, parent = produce(image, work / label / "A", font)
        cold = [recover(image, work / label / ("B" + str(i)), font, seed, parent, fault)
                for i, fault in enumerate(("none", "newest-header", "both-headers"))]
        records.append({"implementation": label, "image_sha256": digest(image.read_bytes()),
                        "producer": parent, "cold": cold})
    assert all(digest(getattr(args, name).read_bytes()) == value for name, value in inputs.items())
    summary = {"inputs": inputs, "records": records,
               "scope": "Actual current Rust services and NVMe for each recorded kernel; candidate swaps kernel and Sext. All eight Spindle lifetimes, exact independently expected history bytes, real prompt pixels, cold recovery and corrupt-header controls. Not all-Cesta services or epoch rollback."}
    (work / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")


if __name__ == "__main__":
    main()
