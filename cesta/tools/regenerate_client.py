#!/usr/bin/env python3
"""Regenerate the client's ordinary C interface and checked helper libraries."""

import argparse
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys

HERE = Path(__file__).resolve().parent.parent


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cesta", type=Path, required=True)
    parser.add_argument("--cesta-source", type=Path, required=True)
    args = parser.parse_args()
    binary = Path(shutil.which(str(args.cesta)) or args.cesta).resolve()
    source = args.cesta_source.resolve()
    include, runtime = HERE / "generated/include", HERE / "generated/runtime"
    arm_include = HERE / "generated/arm64/include"
    include.mkdir(parents=True, exist_ok=True)
    runtime.mkdir(parents=True, exist_ok=True)
    arm_include.mkdir(parents=True, exist_ok=True)
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
    subprocess.run(["python3", "tools/linen_capture_schema.py"], cwd=HERE, check=True)
    generate(["export-c", "src/linen_capture_interface.cesta", "--"], include / "linen_capture.h")
    generate(["export-c", "src/linen_capture_client_interface.cesta", "--"], include / "linen_capture_client.h")
    generate(["export-c", "src/linen_capture_transport_interface.cesta", "--"], include / "linen_capture_transport.h")
    generate(["export-c", "src/linen_capture_service_interface.cesta", "--"], include / "linen_capture_service.h")
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
    generate(["export-c", "src/silkbar_geometry_interface.cesta", "--"], include / "silkbar_geometry.h")
    generate(["export-c", "src/spindle_calc_interface.cesta", "--"], include / "spindle_calc.h")
    generate(["export-c", "src/spindle_complete_interface.cesta", "--"], include / "spindle_complete.h")
    generate(["export-c", "src/spindle_sextime_interface.cesta", "--"], include / "spindle_sextime.h")
    generate(["export-c", "src/kaleidoscope_body_interface.cesta", "--"], include / "kaleidoscope_body.h")
    generate(["export-c", "src/kaleidoscope_navigation_interface.cesta", "--"], include / "kaleidoscope_navigation.h")
    generate(["export-c", "src/kaleidoscope_regex_interface.cesta", "--"], include / "kaleidoscope_regex.h")
    generate(["export-c", "src/kaleidoscope_script_lexer_interface.cesta", "--"], include / "kaleidoscope_script_lexer.h")
    generate(["export-c", "src/kaleidoscope_script_core_interface.cesta", "--"], include / "kaleidoscope_script_core.h")
    generate(["export-c", "src/kaleidoscope_html_interface.cesta", "--"], include / "kaleidoscope_html.h")
    generate(["export-c", "src/kaleidoscope_qoi_interface.cesta", "--"], include / "kaleidoscope_qoi.h")
    generate(["export-c", "src/kaleidoscope_bitmap_interface.cesta", "--"], include / "kaleidoscope_bitmap.h")
    generate(["export-c", "src/kaleidoscope_presentation_interface.cesta", "--"], include / "kaleidoscope_presentation.h")
    generate(["export-c", "src/kaleidoscope_asset_view_interface.cesta", "--"], include / "kaleidoscope_asset_view.h")
    generate(["export-c", "src/kaleidoscope_png_interface.cesta", "--"], include / "kaleidoscope_png.h")
    generate(["export-c", "src/kaleidoscope_asset_pipeline_interface.cesta", "--"], include / "kaleidoscope_asset_pipeline.h")
    generate(["export-c", "src/sexdisplay_bitmap_interface.cesta", "--"], include / "sexdisplay_bitmap.h")
    generate(["export-c", "src/quil_autosave_interface.cesta", "--"], include / "quil_autosave.h")
    generate(["export-c", "src/document_protocol_interface.cesta", "--"], include / "sexos_document_protocol.h")
    generate(["export-c", "src/document_client_interface.cesta", "--"], include / "sexos_document_client.h")
    generate(["export-c", "src/document_service_interface.cesta", "--"], include / "sexos_document_service.h")
    generate(["export-c", "src/document_journal_interface.cesta", "--"], include / "sexos_document_journal.h")
    generate(["export-c", "src/document_growth_interface.cesta", "--"], include / "sexos_document_growth.h")
    generate(["export-c", "src/document_service_transport_interface.cesta", "--"], include / "sexos_document_service_transport.h")
    generate(["export-c", "src/document_snapshot_interface.cesta", "--"], include / "sexos_document_snapshot.h")
    generate(["export-c", "src/document_client_transport_interface.cesta", "--"], include / "sexos_document_client_transport.h")
    generate(["export-c", "src/network_job_abi_interface.cesta", "--"], include / "network_job_abi.h")
    generate(["export-c", "src/network_job_machine_interface.cesta", "--"], include / "network_job_machine.h")
    generate(["export-c", "src/network_dns_job_abi_interface.cesta", "--"], include / "network_dns_job_abi.h")
    generate(["export-c", "src/network_dns_job_machine_interface.cesta", "--"], include / "network_dns_job_machine.h")
    generate(["export-c", "src/network_engine_frame_interface.cesta", "--"], include / "network_engine_frame.h")
    generate(["export-c", "src/network_dns_codec_interface.cesta", "--"], include / "network_dns_codec.h")
    generate(["export-c", "src/network_engine_packet_interface.cesta", "--"], include / "network_engine_packet.h")
    generate(["export-c", "src/network_engine_neighbor_interface.cesta", "--"], include / "network_engine_neighbor.h")
    generate(["export-c", "src/network_engine_dns_interface.cesta", "--"], include / "network_engine_dns.h")
    generate(["export-c", "src/network_engine_tcp_packet_interface.cesta", "--"], include / "network_engine_tcp_packet.h")
    generate(["export-c", "src/network_tcp_client_interface.cesta", "--"], include / "network_tcp_client.h")
    generate(["export-c", "src/network_engine_transport_interface.cesta", "--"], include / "network_engine_transport.h")
    generate(["export-c", "src/network_job_engine_interface.cesta", "--"], include / "network_job_engine.h")
    generate(["export-c", "src/network_dns_job_engine_interface.cesta", "--"], include / "network_dns_job_engine.h")
    generate(["export-c", "src/network_tcp_listener_interface.cesta", "--"], include / "network_tcp_listener.h")
    generate(["export-c", "src/network_service_core_interface.cesta", "--"], include / "network_service_core.h")
    generate(["checked", "header", "src/sexstore_pages.cesta"], include / "sexstore_pages.h")
    generate(["export-c", "src/sexstore_interface.cesta", "--"], include / "sexstore.h")
    generate(["export-c", "src/sex_object_model_interface.cesta", "--"], include / "sex_object_model.h")
    generate(["export-c", "src/sex_graphics_interface.cesta", "--"], include / "sex_graphics.h")
    generate(["export-c", "src/sexfiles_interface.cesta", "--"], include / "sexfiles.h")
    generate(["export-c", "src/sexos_mmio_interface.cesta", "--std=c11", "--", "-Igenerated/include"], include / "sexos_mmio.h")
    subprocess.run([binary, "export-header", "interfaces/shell_apps.export", "-o", include / "shell_app_rows.h"], cwd=HERE, check=True)
    generate(["export-c", "src/silk_shell_custody_interface.cesta", "--"], include / "silk_shell_custody.h")
    generate(["export-c", "src/shell_text_delivery_interface.cesta", "--"], include / "shell_text_delivery.h")
    generate(["export-c", "src/shell_linen_snapshot_interface.cesta", "--"], include / "shell_linen_snapshot.h")
    generate(["export-c", "src/shell_linen_runtime_interface.cesta", "--"], include / "shell_linen_runtime.h")
    generate(["export-c", "src/shell_linen_list_interface.cesta", "--"], include / "shell_linen_list.h")
    generate(["export-c", "src/silk_shell_app_state_interface.cesta", "--"], include / "silk_shell_app_state.h")
    generate(["export-c", "src/silk_shell_interface.cesta", "--"], include / "silk_shell.h")
    generate(["export-c", "src/silk_shell_models_linen_interface.cesta", "--"], include / "silk_shell_models_linen.h")
    generate(["export-c", "src/silk_shell_models_bell_interface.cesta", "--"], include / "silk_shell_models_bell.h")
    generate(["export-c", "src/silk_shell_diagnostics_a_interface.cesta", "--"], include / "silk_shell_diagnostics_a.h")
    generate(["export-c", "src/silk_shell_proofs_a_interface.cesta", "--"], include / "silk_shell_proofs_a.h")
    generate(["export-c", "src/silk_shell_proofs_a_routes_interface.cesta", "--"], include / "silk_shell_proofs_a_routes.h")
    generate(["export-c", "src/shell_proof_lifecycle_fixture_interface.cesta", "--"], include / "shell_proof_lifecycle_fixture.h")
    generate(["export-c", "src/silk_shell_proofs_a_lifecycle_interface.cesta", "--"], include / "silk_shell_proofs_a_lifecycle.h")
    generate(["export-c", "src/silk_shell_proofs_b_palette_interface.cesta", "--"], include / "silk_shell_proofs_b_palette.h")
    generate(["export-c", "src/silk_shell_proofs_b_atlas_interface.cesta", "--"], include / "silk_shell_proofs_b_atlas.h")
    generate(["export-c", "src/silk_shell_proofs_b_input_interface.cesta", "--"], include / "silk_shell_proofs_b_input.h")
    generate(["export-c", "src/silk_shell_core_interface.cesta", "--"], include / "silk_shell_core.h")
    subprocess.run([sys.executable, HERE / "tools/shell_state_defaults.py", "--output", include / "silk_shell_state_defaults.h"], cwd=HERE, check=True)
    generate(["export-c", "src/silk_shell_input_interface.cesta", "--"], include / "silk_shell_input.h")
    generate(["export-c", "src/silk_shell_atlas_interface.cesta", "--"], include / "silk_shell_atlas.h")
    generate(["export-c", "src/silk_shell_viewport_interface.cesta", "--"], include / "silk_shell_viewport.h")
    generate(["export-c", "src/silk_shell_chrome_interface.cesta", "--"], include / "silk_shell_chrome.h")
    generate(["export-c", "src/silk_shell_models_render_interface.cesta", "--"], include / "silk_shell_models_render.h")
    generate(["export-c", "src/silk_shell_models_paint_interface.cesta", "--"], include / "silk_shell_models_paint.h")
    generate(["export-c", "src/silk_shell_models_workspace_interface.cesta", "--"], include / "silk_shell_models_workspace.h")
    generate(["export-c", "src/silk_shell_models_root_interface.cesta", "--"], include / "silk_shell_models_root.h")
    generate(["export-c", "src/silk_shell_surfaces_interface.cesta", "--"], include / "silk_shell_surfaces.h")
    generate(["export-c", "src/app_private_interface.cesta", "--"], include / "sexos_app_private.h")
    generate(["export-c", "src/pager_state_interface.cesta", "--"], include / "sexos_pager_state.h")
    generate(["export-c", "src/memory_regions_interface.cesta", "--"], include / "sexos_memory_regions.h")
    generate(["export-c", "src/region_writer_interface.cesta", "--"], include / "sexos_region_writer.h")
    generate(["export-c", "src/dma_lease_interface.cesta", "--"], include / "sexos_dma_lease.h")
    generate(["export-c", "src/device_reset_interface.cesta", "--"], include / "sexos_device_reset.h")
    generate(["export-c", "src/nvme_msix_interface.cesta", "--"], include / "sexos_nvme_msix.h")
    generate(["export-c", "src/smp_park_interface.cesta", "--"], include / "sexos_smp_park.h")
    generate(["export-c", "src/arm64_boot_interface.cesta", "--"], arm_include / "sexos_arm64_boot.h")
    generate(["export-c", "src/sext_interface.cesta", "--"], include / "sext.h")
    subprocess.run([binary, "export-header", "interfaces/app_catalog.export", "-o", include / "app_catalog_rows.h"], cwd=HERE, check=True)
    generate(["export-c", "src/app_catalog_interface.cesta", "--"], include / "sexos_app_catalog.h")
    generate(["export-c", "src/sext_service_interface.cesta", "--"], include / "sext_service.h")
    genesis = HERE / "build/kernel_genesis.h"
    subprocess.run([binary, "export-header", "interfaces/kernel.export", "-o", genesis], cwd=HERE, check=True)
    destination = include / "kernel_genesis.h"
    content = genesis.read_bytes().rstrip(b"\n") + b"\n"
    if not destination.exists() or destination.read_bytes() != content:
        destination.write_bytes(content)
    subprocess.run([binary, "export-header", "interfaces/kernel_current.export", "-o", include / "kernel_current.h"], cwd=HERE, check=True)
    subprocess.run([binary, "export-header", "interfaces/source_profile.export", "-o", include / "source_profile_rows.h"], cwd=HERE, check=True)
    generate(["export-c", "src/source_profile_interface.cesta", "--"], include / "sexos_source_profile.h")
    generate(["export-c", "src/source_policy_interface.cesta", "--"], include / "sexos_source_policy.h")
    generate(["export-c", "src/network_config_interface.cesta", "--"], include / "sexos_network_config.h")
    generate(["export-c", "src/pci_policy_interface.cesta", "--"], include / "sexos_pci_policy.h")
    generate(["export-c", "src/history_recovery_interface.cesta", "--"], include / "sexos_history_recovery.h")
    generate(["export-c", "src/kernel_interface.cesta", "--", "-Iruntime/kernel", "-Igenerated/include"], include / "sexos_kernel.h")
    # Machine-local generator identity belongs with build evidence, not source.
    evidence = HERE / "build/regeneration.json"
    evidence.parent.mkdir(parents=True, exist_ok=True)
    evidence.write_text(json.dumps(records, indent=2) + "\n")


if __name__ == "__main__":
    main()
