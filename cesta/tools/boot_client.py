#!/usr/bin/env python3
"""Boot a new client image until the reference and browser milestones appear."""

import argparse
import hashlib
import ipaddress
import json
from pathlib import Path
import subprocess

HERE = Path(__file__).resolve().parent.parent


def digest(path):
    with path.open("rb") as stream:
        return hashlib.file_digest(stream, "sha256").hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("image", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--seconds", type=int, default=300)
    parser.add_argument("--nvme-image", type=Path, help="raw device input; writes stay in a disposable QEMU overlay")
    parser.add_argument("--network", action="store_true", help="e1000 on a restricted virtual network")
    parser.add_argument("--network-gateway", type=ipaddress.IPv4Address, help="guest-visible virtual router address")
    parser.add_argument("--usb-keyboard", action="store_true")
    args = parser.parse_args()
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    markers = [line for line in (HERE / "oracle/milestones.txt").read_text().splitlines() if line]
    markers.append("[browser.visible.polish.done]")
    log = output / "serial.log"
    command = [HERE / "oracle/run.sh", args.image.resolve(), log, str(args.seconds), "--fixed-clock"]
    network = args.network or args.network_gateway is not None
    identity = {"image_sha256": digest(args.image),
                "fixed_clock": True, "network": "restricted-e1000" if network else "none",
                "network_gateway": str(args.network_gateway or "10.0.2.2") if network else None,
                "usb_keyboard": args.usb_keyboard}
    if args.nvme_image:
        identity["nvme_sha256"] = digest(args.nvme_image)
        command += ["--nvme-image", args.nvme_image.resolve()]
    if network:
        command.append("--network")
    if args.network_gateway:
        command += ["--network-gateway", str(args.network_gateway)]
    if args.usb_keyboard:
        command.append("--usb-keyboard")
    for marker in markers:
        command += ["--until", marker]
    (output / "inputs.json").write_text(json.dumps(identity, indent=2) + "\n")
    with (output / "qemu.log").open("wb") as errors:
        subprocess.run(command, cwd=HERE.parent, stdout=errors, stderr=subprocess.STDOUT, check=True)
    if args.nvme_image and digest(args.nvme_image) != identity["nvme_sha256"]:
        raise RuntimeError("the NVMe input image changed during its snapshot boot")
    serial = log.read_text(errors="replace")
    missing = [marker for marker in markers if marker not in serial]
    observation = subprocess.run(["python3", "-I", HERE / "oracle/oracle.py", "observe", log,
        "--checker", HERE.parent / "scripts/check_bootgraph_log.py"], capture_output=True, check=True)
    (output / "observation.json").write_bytes(observation.stdout)
    (output / "markers.json").write_text(json.dumps({"missing": missing, "required": markers}, indent=2) + "\n")
    if missing:
        raise RuntimeError("boot did not reach: " + ", ".join(missing))
    print("boot reached", len(markers), "milestones")


if __name__ == "__main__":
    main()
