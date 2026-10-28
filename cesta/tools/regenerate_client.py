#!/usr/bin/env python3
"""Regenerate the client's ordinary C interface and checked helper libraries."""

import argparse
import hashlib
import json
from pathlib import Path
import shutil
import subprocess

HERE = Path(__file__).resolve().parent.parent


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cesta", type=Path, required=True)
    parser.add_argument("--cesta-source", type=Path, required=True)
    args = parser.parse_args()
    binary = Path(shutil.which(str(args.cesta)) or args.cesta).resolve()
    source = args.cesta_source.resolve()
    include, runtime = HERE / "generated/include", HERE / "generated/runtime"
    include.mkdir(parents=True, exist_ok=True)
    runtime.mkdir(parents=True, exist_ok=True)
    records = {"generator_sha256": hashlib.sha256(binary.read_bytes()).hexdigest(), "sources": {}}

    def generate(arguments, destination):
        result = subprocess.run([binary, *arguments], cwd=HERE, capture_output=True, check=True)
        content = result.stdout.rstrip(b"\n") + b"\n"
        if not destination.exists() or destination.read_bytes() != content:
            destination.write_bytes(content)

    for name, filename in (("PackedWords", "packed_words"), ("FormatBytes", "format_bytes")):
        module = source / "checked/examples" / (name + ".cesta")
        records["sources"][name] = hashlib.sha256(module.read_bytes()).hexdigest()
        generate(["checked", "header", module], include / (filename + ".h"))
        generate(["export-c", module, "--"], runtime / (filename + ".c"))
    generated = HERE / "build/client_constants.h"
    generated.parent.mkdir(parents=True, exist_ok=True)
    subprocess.run([binary, "export-header", "interfaces/client.export", "-o", generated], cwd=HERE, check=True)
    destination = include / "client_constants.h"
    content = generated.read_bytes().rstrip(b"\n") + b"\n"
    if not destination.exists() or destination.read_bytes() != content:
        destination.write_bytes(content)
    generate(["export-c", "src/client_interface.cesta", "--"], include / "sexos_client.h")
    generate(["export-c", "src/server_interface.cesta", "--"], include / "sexos_server.h")
    generate(["export-c", "src/rpc_interface.cesta", "--"], include / "sexos_rpc.h")
    subprocess.run([binary, "protocol", "export", "interfaces/rpc_protocols.module", "Protocols",
                    "-o", include / "rpc_protocols.h", "--plan", HERE / "build/rpc-protocols.plan"],
                   cwd=HERE, check=True)
    subprocess.run([binary, "protocol", "export", "interfaces/network_tx_protocol.module", "Protocols",
                    "-o", include / "network_tx_protocol.h", "--plan", HERE / "build/network-tx.plan"],
                   cwd=HERE, check=True)
    subprocess.run([binary, "protocol", "export", "interfaces/network_rx_protocol.module", "Protocols",
                    "-o", include / "network_rx_protocol.h", "--plan", HERE / "build/network-rx.plan"],
                   cwd=HERE, check=True)
    layout_header = HERE / "build/network_result.h"
    subprocess.run([binary, "layout", "header", "interfaces/network_layouts.module", "Layouts",
                    "-o", layout_header], cwd=HERE, check=True)
    layout_bytes = layout_header.read_bytes()
    layout_destination = include / "network_result.h"
    if not layout_destination.exists() or layout_destination.read_bytes() != layout_bytes:
        layout_destination.write_bytes(layout_bytes)
    generate(["export-c", "src/network_interface.cesta", "--"], include / "network_service.h")
    generate(["export-c", "src/silkbar_model_interface.cesta", "--"], include / "silkbar_model.h")
    generate(["checked", "header", "src/sexstore_pages.cesta"], include / "sexstore_pages.h")
    generate(["export-c", "src/sexstore_interface.cesta", "--"], include / "sexstore.h")
    generate(["export-c", "src/sex_object_model_interface.cesta", "--"], include / "sex_object_model.h")
    generate(["export-c", "src/sex_graphics_interface.cesta", "--"], include / "sex_graphics.h")
    generate(["export-c", "src/sexfiles_interface.cesta", "--"], include / "sexfiles.h")
    generate(["export-c", "src/sexos_mmio_interface.cesta", "--"], include / "sexos_mmio.h")
    generate(["export-c", "src/silk_shell_interface.cesta", "--"], include / "silk_shell.h")
    genesis = HERE / "build/kernel_genesis.h"
    subprocess.run([binary, "export-header", "interfaces/kernel.export", "-o", genesis], cwd=HERE, check=True)
    destination = include / "kernel_genesis.h"
    content = genesis.read_bytes().rstrip(b"\n") + b"\n"
    if not destination.exists() or destination.read_bytes() != content:
        destination.write_bytes(content)
    generate(["export-c", "src/kernel_interface.cesta", "--", "-Iruntime/kernel", "-Igenerated/include"], include / "sexos_kernel.h")
    # Machine-local generator identity belongs with build evidence, not source.
    evidence = HERE / "build/regeneration.json"
    evidence.parent.mkdir(parents=True, exist_ok=True)
    evidence.write_text(json.dumps(records, indent=2) + "\n")


if __name__ == "__main__":
    main()
