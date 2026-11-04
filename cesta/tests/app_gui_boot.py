#!/usr/bin/env python3
"""A frozen normal image: all Quil reserves and Browser retire via real input."""
import argparse
import json
from pathlib import Path
import re
import time

from history_disk_boot import Guest, ROOT, close_app, digest, startup
from check_desktop_pixels import font_from_source
from check_quil_ui_pixels import verify
from quil_reserve_catalog import quil_catalog


def capture(guest, name, target, expected, font):
    path = guest.work / (name + ".ppm")
    errors = ["not rendered"]
    deadline = time.monotonic() + 60
    while time.monotonic() < deadline:
        guest.q.cmd("screendump", {"filename": str(path)})
        text = guest.text()
        errors = verify(text, path.read_bytes(), font, expected, target=target)
        if not errors:
            return {"file": path.name, "sha256": digest(path.read_bytes()),
                    "serial_end": len(text), "serial_sha256": digest(text.encode()), "errors": []}
        time.sleep(.15)
    raise RuntimeError("Quil input glyphs: " + repr(errors))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--iso", type=Path, required=True)
    ap.add_argument("--work", type=Path, required=True)
    args = ap.parse_args()
    image, work = args.iso.resolve(), args.work.resolve()
    before = digest(image.read_bytes())
    font = font_from_source((ROOT / "servers/sexdisplay/src/main.rs").read_text())
    rows = quil_catalog(ROOT)
    steps = []
    with Guest(image, work) as guest:
        startup(guest)
        catalogs = [tuple(map(int, match)) for match in re.findall(
            r"\[boot.app.catalog\] target=(\d+) task=(\d+) surface=(\d+) logical=(\d+) incarnation=(\d+) predecessor=(\d+) ", guest.text())]
        tasks = {target: task for target, task, *_ in catalogs}
        assert len(catalogs) == len(tasks) and all(tasks.values())
        for index, row in enumerate(rows):
            found = [c for c in catalogs if c[0] == row["target"]]
            assert len(found) == 1
            assert found[0][2:] == (row["surface"], rows[0]["target"], index + 1,
                                    rows[index - 1]["target"] if index else 0)
        browser = close_app(guest, {"target": 15, "frame": 300}, tasks[15])
        for index, row in enumerate(rows):
            target, task = row["target"], tasks[row["target"]]
            start = len(guest.text())
            if index:
                guest.key("grave_accent", r"\[shell.palette.item\] idx=0")
                guest.key("j", r"\[shell.palette.select\] old=0 new=1")
                guest.key("ret", rf"\[shell.app.relaunch\] predecessor={rows[index-1]['target']} successor={target} task={task} ", 180)
                guest.wait(rf"\[app.surface.presented\] target={target} surface={row['surface']} .*ok=1", start)
            guest.wait(rf"\[quil.incarnation.start\] target={target} task={task} surface={row['surface']} fresh=1")
            guest.focus(row["frame"])
            begin = len(guest.text())
            guest.q.cmd("input-send-event", {"events": [{"type": "key", "data": {
                "down": down, "key": {"type": "qcode", "data": code}}}
                for code, down in [("ctrl", True), ("n", True), ("n", False), ("ctrl", False)]]})
            guest.wait(r"\[quil.new.ok\] bytes=0", begin)
            letter = chr(ord("a") + index)
            guest.key(letter, r"\[quil.document.edit\]")
            frame = capture(guest, "quil-" + str(target), target, letter + "#", font)
            refused_start = len(guest.text())
            guest.key("f11", rf"\[shell.app.close.request\] target={target} task={task} ")
            refused = guest.wait(r"\[quil.close.declined\] dirty=1 pending=0 revision=(\d+)", refused_start)
            guest.wait(rf"\[shell.app.close.declined\] target={target} task={task} reason=unsaved next_close=confirmed", refused_start)
            refused_end = len(guest.text())
            assert not re.search(rf"\[task.finish\] pd={target} |\[app.close.accepted\] target={target} ",
                                 guest.text()[refused_start:refused_end])
            closed = close_app(guest, row, task)
            guest.wait(r"\[quil.close.confirmed\] dirty=1 pending=0", refused_end)
            steps.append({"target": target, "task": task, "surface": row["surface"],
                          "typed": letter, "frame": frame, "closed": closed,
                          "unsaved_refused": {"revision": int(refused[1]), "serial_start": refused_start,
                                              "serial_end": refused_end, "task_retained": True}})
            print("Quil", target, "fresh input pixels and cleanup passed", flush=True)
        guest.key("grave_accent", r"\[shell.palette.item\] idx=0")
        guest.key("j", r"\[shell.palette.select\] old=0 new=1")
        guest.key("ret", r"\[launcher.execute.denied\] idx=1 .*reason=boot_reserve_exhausted")
    assert digest(image.read_bytes()) == before
    result = {"image_sha256": before, "browser_cleanup": browser, "quil": steps,
              "scope": "Actual normal-image Browser cleanup and complete source Quil reserve: native activation/presentation, fresh editor input pixels and exact five-service retirement. Explicit unsaved discard; no document persistence, power-loss or all-Cesta-userland claim."}
    (work / "summary.json").write_text(json.dumps(result, indent=2) + "\n")


if __name__ == "__main__":
    main()
