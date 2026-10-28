#!/usr/bin/env python3
"""Replace applications, servers or the kernel in a copied reference staging tree and build an ISO.

--client BINARY replaces apps/kaleidoscope; --replace PATH=BINARY (repeatable)
replaces any staged file, e.g. servers/silkbar=build/silkbar/silkbar; --kernel BINARY
replaces sexos-kernel with a static higher-half kernel.
"""

import argparse
import hashlib
import json
from pathlib import Path
import shutil
import struct
import subprocess


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def files(root):
    return {str(path.relative_to(root)): digest(path) for path in sorted(root.rglob("*")) if path.is_file()}


def check_elf(path):
    data = path.read_bytes()
    if data[:6] != b"\x7fELF\x02\x01" or struct.unpack_from("<HH", data, 16) != (3, 62):
        raise RuntimeError("the client must be an x86-64 little-endian static PIE")
    segments = subprocess.run(["readelf", "-W", "-l", path], capture_output=True, text=True, check=True).stdout
    if "INTERP" in segments:
        raise RuntimeError("the client requires a host dynamic loader")
    relocs = subprocess.run(["readelf", "-W", "-r", path], capture_output=True, text=True, check=True).stdout
    for line in relocs.splitlines():
        if "R_X86_64_" in line and "R_X86_64_RELATIVE" not in line:
            raise RuntimeError("the kernel cannot apply this relocation: " + line)
    return {"sha256": digest(path), "bytes": len(data), "segments": segments, "relocations": relocs}


def check_kernel(path):
    data = path.read_bytes()
    if data[:6] != b"\x7fELF\x02\x01" or struct.unpack_from("<HH", data, 16) != (2, 62):
        raise RuntimeError("the kernel must be an x86-64 little-endian static executable")
    entry = struct.unpack_from("<Q", data, 24)[0]
    if entry < 0xFFFFFFFF80000000:
        raise RuntimeError("the kernel must be linked in the higher half")
    segments = subprocess.run(["readelf", "-W", "-l", path], capture_output=True, text=True, check=True).stdout
    return {"sha256": digest(path), "bytes": len(data), "segments": segments, "entry": hex(entry)}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--reference-tree", type=Path, required=True)
    parser.add_argument("--client", type=Path)
    parser.add_argument("--replace", action="append", default=[], metavar="PATH=BINARY")
    parser.add_argument("--kernel", type=Path)
    parser.add_argument("--limine", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    base, limine, output = (x.resolve() for x in (args.reference_tree, args.limine, args.out))
    replacements = {}
    if args.client:
        replacements["apps/kaleidoscope"] = args.client.resolve()
    for item in args.replace:
        path, _, binary = item.partition("=")
        if not path or not binary or path in replacements:
            raise RuntimeError("--replace takes distinct PATH=BINARY pairs")
        replacements[path] = Path(binary).resolve()
    kernel = args.kernel.resolve() if args.kernel else None
    if not replacements and kernel is None:
        raise RuntimeError("name at least one replacement with --client, --replace or --kernel")
    if output.is_relative_to(base):
        raise RuntimeError("output must be outside the reference staging tree")
    if output.exists():
        raise RuntimeError("choose a new output directory; existing evidence is preserved")
    elves = {path: check_elf(binary) for path, binary in replacements.items()}
    if kernel is not None:
        elves["sexos-kernel"] = check_kernel(kernel)
        replacements["sexos-kernel"] = kernel
    before = files(base)
    missing = [path for path in replacements if path not in before]
    if missing:
        raise RuntimeError("reference staging tree has no " + ", ".join(missing))
    output.mkdir(parents=True)
    root = output / "iso_root"
    shutil.copytree(base, root)
    for path, binary in replacements.items():
        shutil.copy2(binary, root / path)
    changed = files(root)
    differences = sorted(name for name in before.keys() | changed.keys() if before.get(name) != changed.get(name))
    expected = sorted(path for path, binary in replacements.items() if before[path] != digest(binary))
    if differences != expected:
        raise RuntimeError("unexpected staging changes: " + str(differences))
    names = "-".join(Path(path).name for path in sorted(replacements))
    image = output / f"sexos-cesta-{names}.iso"
    with (output / "package.log").open("wb") as log:
        subprocess.run(["xorriso", "-as", "mkisofs", "-R", "-r", "-J",
            "-b", "boot/limine/limine-bios-cd.bin", "-no-emul-boot", "-boot-load-size", "4", "-boot-info-table",
            "--efi-boot", "boot/limine/limine-uefi-cd.bin", "-efi-boot-part", "--efi-boot-image",
            "--protective-msdos-label", root, "-o", image], stdout=log, stderr=subprocess.STDOUT, check=True)
        subprocess.run([limine / "limine", "bios-install", image], stdout=log, stderr=subprocess.STDOUT, check=True)
    if before != files(base):
        raise RuntimeError("reference staging inputs changed during packaging")
    summary = {"changed_files": differences, "reference_files": before, "candidate_files": changed,
               "replacements": elves, "image_sha256": digest(image)}
    (output / "manifest.json").write_text(json.dumps(summary, indent=2) + "\n")
    print(image)


if __name__ == "__main__":
    main()
