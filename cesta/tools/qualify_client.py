#!/usr/bin/env python3
"""Qualify hosted traces, plain-C exports and boots of the first client port."""

import argparse
import hashlib
import json
from pathlib import Path
import shutil
import struct
import subprocess

HERE = Path(__file__).resolve().parent.parent


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def allocated_sections(path):
    data = path.read_bytes()
    if data[:6] != b"\x7fELF\x02\x01":
        raise RuntimeError("expected a little-endian ELF64 client")
    offset = struct.unpack_from("<Q", data, 40)[0]
    entry_size, count, names_index = struct.unpack_from("<HHH", data, 58)
    headers = [struct.unpack_from("<IIQQQQIIQQ", data, offset + i * entry_size) for i in range(count)]
    names_header = headers[names_index]
    names = data[names_header[4]:names_header[4] + names_header[5]]
    result = {}
    for header in headers:
        name, kind, flags, address, file_offset, size, _, _, alignment, stride = header
        if not flags & 2:
            continue
        spelling = names[name:names.index(0, name)].decode("ascii")
        contents = b"" if kind == 8 else data[file_offset:file_offset + size]
        result[spelling] = {"type": kind, "flags": flags, "address": address, "size": size,
                            "alignment": alignment, "entry_size": stride,
                            "sha256": hashlib.sha256(contents).hexdigest()}
    return result


def require_clean_observation(observed):
    if observed["inconclusive"]:
        raise RuntimeError("boot observation was inconclusive")
    faults = observed["faults"]
    if set(faults) != {"#GP", "#PF", "fault.kill", "panic"} or any(
            type(count) is not int or count != 0 for count in faults.values()):
        raise RuntimeError("boot observation contains faults or an unknown fault schema")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cesta", required=True)
    parser.add_argument("--cesta-source", type=Path, required=True)
    parser.add_argument("--reference-tree", type=Path, required=True)
    parser.add_argument("--reference-image", type=Path, required=True)
    parser.add_argument("--limine", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    binary = Path(shutil.which(args.cesta) or args.cesta).resolve()
    output = args.out.resolve()
    if output.exists():
        raise RuntimeError("choose a new output directory to retain earlier evidence")
    output.mkdir(parents=True)
    step = 0

    def run(command, cwd=HERE):
        nonlocal step
        log = output / (str(step).zfill(2) + ".log")
        step += 1
        with log.open("wb") as stream:
            result = subprocess.run(list(map(str, command)), cwd=cwd, stdout=stream, stderr=subprocess.STDOUT)
        if result.returncode:
            raise RuntimeError((command, result.returncode, log))

    initial_binary = digest(binary)
    run(["python3", HERE / "tools/regenerate_client.py", "--cesta", binary,
         "--cesta-source", args.cesta_source.resolve()])
    run(["make", "qualify", "CESTA=" + str(binary)])
    run(["python3", HERE / "tools/boot_client.py", args.reference_image.resolve(), output / "rust-reference"])
    reference = json.loads((output / "rust-reference/observation.json").read_text())
    require_clean_observation(reference)
    summaries = []
    for compiler in ("gcc", "clang"):
        root = output / compiler
        root.mkdir()
        run([binary, "build", "--cc", compiler, "-j", "4"])
        artifact = root / "kaleidoscope"
        shutil.copy2(HERE / "build/kaleidoscope/kaleidoscope", artifact)
        exported = root / "plain-c"
        run([binary, "build", "--cc", compiler, "--export-c", exported])
        run(["make", "-j4", "CC=" + compiler], cwd=exported)
        plain = exported / "build/kaleidoscope/kaleidoscope"
        # Source-file symbols can differ between authored and exported inputs.
        # Every allocated ELF section must agree; debug/symbol tables are not
        # executable program behavior.
        sections = allocated_sections(artifact)
        if sections != allocated_sections(plain):
            raise RuntimeError("authored/plain C allocated sections differ")
        if artifact.read_bytes() != plain.read_bytes():
            raise RuntimeError("the client's authored/plain C executables are not byte-identical")
        run(["python3", HERE / "tools/package_client.py", "--reference-tree", args.reference_tree.resolve(),
             "--client", artifact, "--limine", args.limine.resolve(), "--out", root / "image"])
        image = root / "image/sexos-cesta-kaleidoscope.iso"
        run(["python3", HERE / "tools/boot_client.py", image, root / "boot"])
        run(["python3", HERE / "oracle/oracle.py", "compare", output / "rust-reference/observation.json",
             root / "boot/observation.json"])
        observed = json.loads((root / "boot/observation.json").read_text())
        require_clean_observation(observed)
        summaries.append({"compiler": compiler, "client_sha256": digest(artifact),
                          "plain_client_sha256": digest(plain), "whole_elf_identical": True, "sections_equal": sections,
                          "iso_sha256": digest(image), "behavior": "equal to pinned Rust reference"})
    if digest(binary) != initial_binary:
        raise RuntimeError("Cesta changed during client qualification")
    summary = {"cesta_sha256": initial_binary, "variants": summaries,
               "rust_reference_iso_sha256": digest(args.reference_image.resolve()),
               "runtime_faults": "none", "reference_checker": reference["checker"]}
    (output / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")
    print("client qualified: Rust traces, GCC/Clang authored/plain C and matching behavioral boots")


if __name__ == "__main__":
    main()
