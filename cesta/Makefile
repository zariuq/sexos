CESTA ?= cesta
CESTA_SOURCE ?=
LEAN_ROOT ?=
JOBS ?= 4

.PHONY: check-silk-shell all build regenerate export plain export-plain arm64-build arm64-export arm64-plain check qualify check-services qualify-services check-sext qualify-sext
all: build

build:
	$(CESTA) build -j $(JOBS)

regenerate:
	@test -n "$(CESTA_SOURCE)" || { echo "set CESTA_SOURCE to the Cesta source directory"; exit 1; }
	python3 tools/regenerate_client.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)"

export:
	$(CESTA) build --export-c c
	$(CESTA) export-c --std=c11 -o c/src/silk_shell_models.c src/silk_shell_models.cesta -- -Igenerated/include
	$(CESTA) export-c -o c/generated/include/silk_shell.h src/silk_shell_interface.cesta --

plain:
	$(MAKE) -C c -j $(JOBS)

# Refresh the whole readable-C project before building it. Keep `plain`
# independently usable on machines without Cesta.
export-plain:
	$(MAKE) export
	$(MAKE) plain

arm64-build:
	$(CESTA) build -f cesta.arm64.project -j $(JOBS) arm64-phase2 arm64-phase3 arm64-phase4 arm64-phase5

arm64-export:
	$(CESTA) build -f cesta.arm64.project --export-c c-arm64 arm64-phase2 arm64-phase3 arm64-phase4 arm64-phase5

arm64-plain:
	$(MAKE) -C c-arm64 -j $(JOBS) CC=clang

.PHONY: check-arm64-phase2 qualify-arm64-phase2
check-arm64-phase2: arm64-build
	python3 tests/arm64_phase2.py --cesta "$(CESTA)" --image build-arm64/arm64-phase2/arm64-phase2 --mutations

qualify-arm64-phase2: arm64-build arm64-export arm64-plain
	@test -n "$(LEAN_ROOT)" -a -n "$(CESTA_SOURCE)" || { echo "set LEAN_ROOT and CESTA_SOURCE"; exit 1; }
	python3 tests/arm64_phase2.py --cesta "$(CESTA)" --image build-arm64/arm64-phase2/arm64-phase2 --plain-image c-arm64/build-arm64/arm64-phase2/arm64-phase2 --mutations --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"

.PHONY: check-arm64-phase3 qualify-arm64-phase3
check-arm64-phase3: arm64-build
	python3 tests/arm64_phase3.py --cesta "$(CESTA)" --image build-arm64/arm64-phase3/arm64-phase3 --mutations

qualify-arm64-phase3: arm64-build arm64-export arm64-plain
	@test -n "$(LEAN_ROOT)" -a -n "$(CESTA_SOURCE)" || { echo "set LEAN_ROOT and CESTA_SOURCE"; exit 1; }
	python3 tests/arm64_phase3.py --cesta "$(CESTA)" --image build-arm64/arm64-phase3/arm64-phase3 --plain-image c-arm64/build-arm64/arm64-phase3/arm64-phase3 --mutations --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"

.PHONY: check-arm64-phase4 qualify-arm64-phase4
check-arm64-phase4: arm64-build
	python3 tests/arm64_phase4_image.py --image build-arm64/arm64-phase4/arm64-phase4
	python3 tests/arm64_phase4.py --cesta "$(CESTA)" --mutations

qualify-arm64-phase4: arm64-build arm64-export arm64-plain
	@test -n "$(LEAN_ROOT)" -a -n "$(CESTA_SOURCE)" || { echo "set LEAN_ROOT and CESTA_SOURCE"; exit 1; }
	python3 tests/arm64_phase4_image.py --image build-arm64/arm64-phase4/arm64-phase4 --plain-image c-arm64/build-arm64/arm64-phase4/arm64-phase4
	python3 tests/arm64_phase4.py --cesta "$(CESTA)" --mutations --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"

.PHONY: check-arm64-phase5 qualify-arm64-phase5
check-arm64-phase5: arm64-build
	python3 tests/arm64_phase5_image.py --image build-arm64/arm64-phase5/arm64-phase5
	python3 tests/arm64_phase5.py --cesta "$(CESTA)" --mutations

qualify-arm64-phase5: arm64-build arm64-export arm64-plain
	@test -n "$(LEAN_ROOT)" -a -n "$(CESTA_SOURCE)" || { echo "set LEAN_ROOT and CESTA_SOURCE"; exit 1; }
	python3 tests/arm64_phase5_image.py --image build-arm64/arm64-phase5/arm64-phase5 --plain-image c-arm64/build-arm64/arm64-phase5/arm64-phase5
	python3 tests/arm64_phase5.py --cesta "$(CESTA)" --mutations --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"

.PHONY: check-arm64-runtime
check-arm64-runtime: arm64-build
	@work=$$(mktemp -d build/arm64-runtime.XXXXXX); \
	python3 tests/arm64_runtime.py --image build-arm64/arm64-phase2/arm64-phase2 --phase 2 --work $$work/phase2 && \
	python3 tests/arm64_runtime.py --image build-arm64/arm64-phase3/arm64-phase3 --phase 3 --work $$work/phase3 && \
	python3 tests/arm64_runtime.py --image build-arm64/arm64-phase4/arm64-phase4 --phase 4 --work $$work/phase4 && \
	python3 tests/arm64_runtime.py --image build-arm64/arm64-phase5/arm64-phase5 --phase 5 --work $$work/phase5

check: check-silk-shell check-services check-ownership check-sext check-kernel-transport check-memory-regions check-dma-lease check-device-reset check-smp-park check-kernel-regions check-current-boot check-pager check-sources check-pci check-history check-handover check-program-image check-frame-leases check-program-frames check-program-memory check-server-heap check-program-workers check-program-records check-program-record-memory check-program-record-tasks check-epoch-journal check-epoch-files check-va-reservation
	python3 tests/boot_observation_controls.py
	python3 tests/kernel_boot_comparison.py
	python3 tests/kernel_boot_driver_controls.py
	python3 tests/client_abi.py
	python3 tests/runtime_boundaries.py
	python3 tests/memory_primitives.py
	python3 tests/kernel_memory_primitives.py --cesta "$(CESTA)"
	python3 tests/client_differential.py --cesta "$(CESTA)"
	python3 tests/silkbar_model_differential.py --cesta "$(CESTA)"
	python3 tests/silkbar_differential.py --cesta "$(CESTA)"
	python3 tests/sexinput_differential.py --cesta "$(CESTA)"
	python3 tests/sexstore_differential.py --cesta "$(CESTA)"
	python3 tests/sexstore_versions.py --cesta "$(CESTA)"
	python3 tests/sexdisplay_differential.py --cesta "$(CESTA)"
	python3 tests/sexdisplay_font.py --cesta "$(CESTA)"
	python3 tests/sexbell_differential.py --cesta "$(CESTA)"
	python3 tests/linen_differential.py --cesta "$(CESTA)"
	python3 tests/quil_differential.py --cesta "$(CESTA)"
	python3 tests/kaleidoscope_body.py --cesta "$(CESTA)"
	python3 tests/kaleidoscope_navigation.py --cesta "$(CESTA)"
	python3 tests/kaleidoscope_web_page.py --cesta "$(CESTA)" --mutations
	python3 tests/kaleidoscope_regex.py --cesta "$(CESTA)"
	python3 tests/kaleidoscope_decimal.py --cesta "$(CESTA)"
	python3 tests/kaleidoscope_script_lexer.py --cesta "$(CESTA)"
	python3 tests/kaleidoscope_script_core.py --cesta "$(CESTA)"
	python3 tests/kaleidoscope_html.py --cesta "$(CESTA)"
	python3 tests/kaleidoscope_html_live.py --cesta "$(CESTA)"
	python3 tests/kaleidoscope_qoi.py --cesta "$(CESTA)"
	python3 tests/kaleidoscope_bitmap.py --cesta "$(CESTA)"
	python3 tests/kaleidoscope_presentation.py --cesta "$(CESTA)"
	python3 tests/kaleidoscope_asset_view.py --cesta "$(CESTA)"
	python3 tests/kaleidoscope_png.py --cesta "$(CESTA)"
	python3 tests/kaleidoscope_asset_pipeline.py --cesta "$(CESTA)"
	python3 tests/quil_autosave.py --cesta "$(CESTA)"
	python3 tests/quil_lines_current.py --cesta "$(CESTA)"
	python3 tests/document_protocol.py --cesta "$(CESTA)"
	python3 tests/document_client.py --cesta "$(CESTA)"
	python3 tests/document_service.py --cesta "$(CESTA)"
	python3 tests/document_journal.py --cesta "$(CESTA)"
	python3 tests/document_growth.py --cesta "$(CESTA)"
	python3 tests/document_service_transport.py --cesta "$(CESTA)"
	python3 tests/document_snapshot.py --cesta "$(CESTA)"
	python3 tests/document_client_transport.py --cesta "$(CESTA)"
	python3 tests/sex_graphics_differential.py --cesta "$(CESTA)"
	python3 tests/spindle_complete.py --cesta "$(CESTA)"
	python3 tests/spindle_calc.py --cesta "$(CESTA)"
	python3 tests/spindle_calc_current.py --cesta "$(CESTA)"
	python3 tests/spindle_line_current.py --cesta "$(CESTA)"
	python3 tests/spindle_differential.py --cesta "$(CESTA)"
	python3 tests/purple_scanout_differential.py --cesta "$(CESTA)"
	python3 tests/sexfiles_differential.py --cesta "$(CESTA)"
	python3 tests/sexdrive_differential.py --cesta "$(CESTA)"
	python3 tests/sexusb_differential.py --cesta "$(CESTA)"
	python3 tests/sexnet_differential.py --cesta "$(CESTA)"
	python3 tests/silk_shell_models_differential.py --cesta "$(CESTA)"
	python3 tests/silk_shell_input_differential.py --cesta "$(CESTA)" --mutations
	python3 tests/silk_shell_atlas_differential.py --cesta "$(CESTA)" --mutations

qualify: qualify-ownership qualify-sext check-kernel-transport qualify-memory-regions qualify-dma-lease qualify-device-reset qualify-smp-park qualify-kernel-regions qualify-sources qualify-pci qualify-history qualify-handover qualify-program-image qualify-program-frames qualify-program-memory qualify-program-workers check-va-reservation
	python3 tests/boot_observation_controls.py
	python3 tests/kernel_boot_comparison.py
	python3 tests/kernel_boot_driver_controls.py
	python3 tests/client_abi.py
	python3 tests/runtime_boundaries.py
	python3 tests/memory_primitives.py
	python3 tests/kernel_memory_primitives.py --cesta "$(CESTA)"
	python3 tests/client_differential.py --cesta "$(CESTA)" --cases 512 --sanitizers --mutations
	python3 tests/silkbar_model_differential.py --cesta "$(CESTA)" --seeds 12
	python3 tests/silkbar_differential.py --cesta "$(CESTA)" --cases 96
	python3 tests/sexinput_differential.py --cesta "$(CESTA)" --all-variants --cases 48
	python3 tests/sexstore_differential.py --cesta "$(CESTA)" --cases 96 --durable-cases 20000
	python3 tests/sexstore_versions.py --cesta "$(CESTA)" --cases 10000
	python3 tests/sexdisplay_differential.py --cesta "$(CESTA)" --all-variants --cases 48
	python3 tests/sexdisplay_font.py --cesta "$(CESTA)" --mutations --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/sexbell_differential.py --cesta "$(CESTA)" --cases 144
	python3 tests/linen_differential.py --cesta "$(CESTA)" --cases 72
	python3 tests/quil_differential.py --cesta "$(CESTA)" --all-variants --cases 48
	python3 tests/kaleidoscope_body.py --cesta "$(CESTA)" --mutations --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/kaleidoscope_navigation.py --cesta "$(CESTA)" --mutations --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/kaleidoscope_regex.py --cesta "$(CESTA)" --mutations --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/kaleidoscope_decimal.py --cesta "$(CESTA)" --mutations --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/kaleidoscope_script_lexer.py --cesta "$(CESTA)" --mutations --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/kaleidoscope_script_core.py --cesta "$(CESTA)" --mutations
	python3 tests/kaleidoscope_html.py --cesta "$(CESTA)" --mutations
	python3 tests/kaleidoscope_html_live.py --cesta "$(CESTA)"
	python3 tests/kaleidoscope_qoi.py --cesta "$(CESTA)" --mutations --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/kaleidoscope_bitmap.py --cesta "$(CESTA)" --mutations --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/kaleidoscope_presentation.py --cesta "$(CESTA)" --mutations --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/kaleidoscope_asset_view.py --cesta "$(CESTA)" --mutations --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/kaleidoscope_png.py --cesta "$(CESTA)" --mutations
	python3 tests/kaleidoscope_asset_pipeline.py --cesta "$(CESTA)"
	python3 tests/quil_autosave.py --cesta "$(CESTA)" --mutations --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/quil_lines_current.py --cesta "$(CESTA)" --mutations
	python3 tests/document_protocol.py --cesta "$(CESTA)" --mutations --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/document_client.py --cesta "$(CESTA)" --mutations --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/document_service.py --cesta "$(CESTA)" --mutations --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/document_journal.py --cesta "$(CESTA)" --mutations --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/document_growth.py --cesta "$(CESTA)" --mutations --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/document_service_transport.py --cesta "$(CESTA)" --mutations --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/document_snapshot.py --cesta "$(CESTA)" --mutations
	python3 tests/document_client_transport.py --cesta "$(CESTA)" --mutations --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/sex_graphics_differential.py --cesta "$(CESTA)" --seeds 12
	python3 tests/spindle_complete.py --cesta "$(CESTA)" --mutations --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/spindle_calc.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/spindle_calc_current.py --cesta "$(CESTA)" --mutations --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/spindle_line_current.py --cesta "$(CESTA)" --mutations
	python3 tests/spindle_differential.py --cesta "$(CESTA)" --all-variants --cases 18
	python3 tests/purple_scanout_differential.py --cesta "$(CESTA)"
	python3 tests/sexfiles_differential.py --cesta "$(CESTA)" --all-variants --cases 18
	python3 tests/sexdrive_differential.py --cesta "$(CESTA)" --all-variants --cases 12
	python3 tests/sexusb_differential.py --cesta "$(CESTA)" --all-variants --cases 12
	python3 tests/sexnet_differential.py --cesta "$(CESTA)" --all-variants --cases 12
	python3 tests/silk_shell_models_differential.py --cesta "$(CESTA)" --seeds 12 --cases 512
	python3 tests/silk_shell_input_differential.py --cesta "$(CESTA)" --seeds 12 --cases 512 --mutations --all-profiles
	python3 tests/silk_shell_atlas_differential.py --cesta "$(CESTA)" --cases 256 --mutations

.PHONY: check-silk-shell-models check-silk-shell-models-historical
check-silk-shell: check-silk-shell-models check-silk-shell-chrome check-silk-shell-models-linen check-silk-shell-proofs-a check-silk-shell-proofs-a-ownership check-silk-shell-proofs-b-palette check-silk-shell-proofs-b-palette-ownership check-silk-shell-proofs-b-atlas check-silk-shell-proofs-b-atlas-ownership check-silk-shell-proofs-b check-silk-shell-proofs-b-ownership check-silk-shell-core check-silk-shell-startup check-silk-shell-startup-ownership check-silk-shell-models-render check-silk-shell-models-render-ownership check-silk-shell-diagnostics-a check-silk-shell-diagnostics-a-ownership check-silk-shell-surfaces check-silk-shell-surfaces-ownership check-silk-shell-chrome-ownership

check-silk-shell-models:
	python3 tests/silk_shell_models_current.py --cesta "$(CESTA)"
	python3 tests/silk_shell_models_root.py --cesta "$(CESTA)"
	python3 tests/silk_shell_models_workspace.py --cesta "$(CESTA)"
check-silk-shell-models-historical:
	python3 tests/silk_shell_models_differential.py --cesta "$(CESTA)"

.PHONY: check-silk-shell-input
check-silk-shell-input:
	python3 tests/silk_shell_input_differential.py --cesta "$(CESTA)" --mutations

.PHONY: check-silk-shell-atlas
check-silk-shell-atlas:
	python3 tests/silk_shell_atlas_differential.py --cesta "$(CESTA)" --mutations

.PHONY: check-silk-shell-chrome
check-silk-shell-chrome:
	python3 tools/shell_catalog.py --check
	python3 tests/silk_shell_custody.py --cesta "$(CESTA)" --mutations
	python3 tests/silk_shell_viewport.py --cesta "$(CESTA)" --mutations
	python3 tests/silk_shell_chrome.py --cesta "$(CESTA)" --mutations
	python3 tests/silk_shell_chrome.py --cesta "$(CESTA)" --mutations --proof-profile

.PHONY: check-silk-shell-surfaces check-silk-shell-surfaces-ownership
.PHONY: check-silk-shell-diagnostics-a check-silk-shell-diagnostics-a-ownership
.PHONY: check-shell-linen-snapshot check-shell-linen-snapshot-ownership
.PHONY: check-silk-shell-models-linen
check-silk-shell-models-linen:
	python3 tests/silk_shell_models_linen.py --cesta "$(CESTA)"

.PHONY: check-silk-shell-proofs-a check-silk-shell-proofs-a-ownership
check-silk-shell-proofs-a:
	python3 tests/silk_shell_proofs_a_complete.py
	python3 tests/silk_shell_proofs_a.py --cesta "$(CESTA)"
	python3 tests/silk_shell_proofs_a_routes.py --cesta "$(CESTA)"
	python3 tests/shell_proof_lifecycle_fixture.py --cesta "$(CESTA)"
	python3 tests/silk_shell_proofs_a_lifecycle.py --cesta "$(CESTA)"
check-silk-shell-proofs-a-ownership:
	python3 tests/silk_shell_proofs_a_complete.py
	python3 tests/silk_shell_proofs_a_ownership.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/silk_shell_proofs_a_routes.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/shell_proof_lifecycle_fixture.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/silk_shell_proofs_a_lifecycle.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"

.PHONY: check-silk-shell-proofs-b-palette check-silk-shell-proofs-b-palette-ownership
check-silk-shell-proofs-b-palette:
	python3 tests/silk_shell_proofs_b_palette.py --cesta "$(CESTA)"
check-silk-shell-proofs-b-palette-ownership:
	python3 tests/silk_shell_proofs_b_palette.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
.PHONY: check-silk-shell-proofs-b-atlas check-silk-shell-proofs-b-atlas-ownership
check-silk-shell-proofs-b-atlas:
	python3 tests/silk_shell_proofs_b_atlas.py --cesta "$(CESTA)"
check-silk-shell-proofs-b-atlas-ownership:
	python3 tests/silk_shell_proofs_b_atlas.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
.PHONY: check-silk-shell-proofs-b check-silk-shell-proofs-b-ownership
check-silk-shell-proofs-b:
	python3 tests/silk_shell_proofs_b_complete.py
	python3 tests/silk_shell_proofs_b_palette.py --cesta "$(CESTA)"
	python3 tests/silk_shell_proofs_b_atlas.py --cesta "$(CESTA)"
	python3 tests/silk_shell_proofs_b_input.py --cesta "$(CESTA)"
check-silk-shell-proofs-b-ownership:
	python3 tests/silk_shell_proofs_b_complete.py
	python3 tests/silk_shell_proofs_b_palette.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/silk_shell_proofs_b_atlas.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/silk_shell_proofs_b_input.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
.PHONY: check-silk-shell-core
check-silk-shell-core:
	python3 tests/silk_shell_core.py --cesta "$(CESTA)"
.PHONY: check-silk-shell-startup check-silk-shell-startup-ownership
check-silk-shell-startup:
	python3 tests/silk_shell_startup.py --cesta "$(CESTA)"
check-silk-shell-startup-ownership:
	python3 tests/silk_shell_startup.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
.PHONY: qualify-silk-shell-boot
qualify-silk-shell-boot:
	@test -n "$(REFERENCE_TREE)" -a -n "$(LIMINE)" -a -n "$(SHELL_BOOT_WORK)" || { echo "set REFERENCE_TREE, LIMINE and SHELL_BOOT_WORK"; exit 1; }
	python3 tests/silk_shell_boot.py --reference-tree "$(REFERENCE_TREE)" --limine "$(LIMINE)" --work "$(SHELL_BOOT_WORK)"

.PHONY: qualify-all-current-boot
qualify-all-current-boot:
	@test -n "$(REFERENCE_TREE)" -a -n "$(LIMINE)" -a -n "$(CURRENT_KERNEL)" -a -n "$(SHELL_BOOT_WORK)" || { echo "set REFERENCE_TREE, LIMINE, CURRENT_KERNEL and SHELL_BOOT_WORK"; exit 1; }
	python3 tests/silk_shell_boot.py --reference-tree "$(REFERENCE_TREE)" --limine "$(LIMINE)" --kernel "$(CURRENT_KERNEL)" --work "$(SHELL_BOOT_WORK)" --seconds 90

.PHONY: qualify-kernel-input-epoch-boot
qualify-kernel-input-epoch-boot:
	@test -n "$(REFERENCE_TREE)" -a -n "$(LIMINE)" -a -n "$(CURRENT_KERNEL)" -a -n "$(INPUT_BOOT_WORK)" || { echo "set REFERENCE_TREE, LIMINE, CURRENT_KERNEL and INPUT_BOOT_WORK"; exit 1; }
	python3 tests/kernel_input_epoch_boot.py --reference-tree "$(REFERENCE_TREE)" --kernel "$(CURRENT_KERNEL)" --limine "$(LIMINE)" --output "$(INPUT_BOOT_WORK)"

.PHONY: check-silk-shell-models-render check-silk-shell-models-render-ownership
check-silk-shell-models-render:
	python3 tests/shell_linen_list.py --cesta "$(CESTA)"
	python3 tests/shell_linen_presentation.py --cesta "$(CESTA)"
	python3 tests/silk_shell_models_render.py --cesta "$(CESTA)"
	python3 tests/silk_shell_models_publish.py --cesta "$(CESTA)"
check-silk-shell-models-render-ownership:
	python3 tests/silk_shell_models_render_ownership.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"

check-shell-linen-snapshot:
	python3 tests/shell_linen_snapshot.py --cesta "$(CESTA)" --mutations

.PHONY: check-linen-capture qualify-linen-capture
check-linen-capture:
	python3 tools/linen_capture_schema.py --check
	python3 tests/linen_capture.py --cesta "$(CESTA)"
	python3 tests/linen_capture_client.py --cesta "$(CESTA)"
	python3 tests/linen_capture_kernel.py --cesta "$(CESTA)"
	python3 tests/linen_capture_live.py --cesta "$(CESTA)"
	python3 tests/linen_service_probe.py --cesta "$(CESTA)"
	python3 tests/linen_session_ids.py --cesta "$(CESTA)"

qualify-linen-capture:
	@test -n "$(LEAN_ROOT)" -a -n "$(CESTA_SOURCE)" || { echo "set LEAN_ROOT and CESTA_SOURCE"; exit 1; }
	python3 tools/linen_capture_schema.py --check
	python3 tests/linen_capture.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/linen_capture_client.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/linen_capture_kernel.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/linen_capture_live.py --cesta "$(CESTA)"
	python3 tests/linen_service_probe.py --cesta "$(CESTA)"
	python3 tests/linen_session_ids.py --cesta "$(CESTA)"

check-shell-linen-snapshot-ownership:
	python3 tests/shell_linen_snapshot_ownership.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"

check-silk-shell-diagnostics-a:
	python3 tests/rust_reference_items.py
	python3 tests/silk_shell_diagnostics_a.py --cesta "$(CESTA)" --mutations
	python3 tests/silk_shell_diagnostics_a.py --cesta "$(CESTA)" --mutations --joined --output build/shell-diagnostics-joined-tests

check-silk-shell-diagnostics-a-ownership:
	python3 tests/silk_shell_diagnostics_a_ownership.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"

check-silk-shell-surfaces:
	python3 tools/shell_catalog.py --check
	python3 tests/silk_shell_surfaces.py --cesta "$(CESTA)" --mutations
	python3 tests/silk_shell_surfaces_native.py --cesta "$(CESTA)"

check-silk-shell-surfaces-ownership:
	python3 tests/silk_shell_surfaces_ownership.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"

.PHONY: check-silk-shell-chrome-ownership
check-silk-shell-chrome-ownership:
	python3 tests/silkbar_geometry_ownership.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/silk_shell_custody_ownership.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/silk_shell_viewport_ownership.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/silk_shell_chrome_ownership.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"

check-sext:
	python3 tests/sext_differential.py --cesta "$(CESTA)" --mutations
	python3 tests/sext_workspace.py --cesta "$(CESTA)"
	python3 tests/sext_native.py
	python3 tests/sext_ownership.py --cesta "$(CESTA)"
	python3 tests/sext_catalog.py --cesta "$(CESTA)"

qualify-sext:
	@test -n "$(LEAN_ROOT)" -a -n "$(CESTA_SOURCE)" || { echo "set LEAN_ROOT and CESTA_SOURCE"; exit 1; }
	python3 tests/sext_differential.py --cesta "$(CESTA)" --counts 0,1,8,65 --mutations --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/sext_workspace.py --cesta "$(CESTA)"
	python3 tests/sext_native.py
	python3 tests/sext_ownership.py --cesta "$(CESTA)" --units service --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/sext_catalog.py --cesta "$(CESTA)"

.PHONY: check-ownership qualify-ownership
check-ownership:
	python3 tests/sexbell_ownership.py --cesta "$(CESTA)"
	python3 tests/sexstore_ownership.py --cesta "$(CESTA)"
	python3 tests/silk_shell_input_ownership.py --cesta "$(CESTA)"
	python3 tests/silk_shell_atlas_ownership.py --cesta "$(CESTA)"

qualify-ownership:
	@test -n "$(LEAN_ROOT)" -a -n "$(CESTA_SOURCE)" || { echo "set LEAN_ROOT and CESTA_SOURCE for ownership admission"; exit 1; }
	python3 tests/sexbell_ownership.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/sexstore_ownership.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/silk_shell_input_ownership.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/silk_shell_atlas_ownership.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	cd "$(LEAN_ROOT)" && lake env lean Mettapedia/Machines/SparseSnapshotValidation.lean

.PHONY: check-kernel-model
check-kernel-model:
	@test -n "$(LEAN_ROOT)" || { echo "set LEAN_ROOT to the Mettapedia Lean package"; exit 1; }
	python3 tests/kernel_model_differential.py --cesta "$(CESTA)" --lean-root "$(LEAN_ROOT)"

check-services:
	python3 tests/kernel_rpc.py --cesta "$(CESTA)"
	python3 tests/rpc_runtime.py --cesta "$(CESTA)"
	python3 tests/network_job_abi.py --cesta "$(CESTA)"
	python3 tests/network_job_machine.py --cesta "$(CESTA)"
	python3 tests/network_dns_job_machine.py --cesta "$(CESTA)"
	python3 tests/network_engine_frame.py --cesta "$(CESTA)"
	python3 tests/network_dns_codec.py --cesta "$(CESTA)"
	python3 tests/network_engine_packet.py --cesta "$(CESTA)"
	python3 tests/network_engine_neighbor.py --cesta "$(CESTA)"
	python3 tests/network_engine_dns.py --cesta "$(CESTA)"
	python3 tests/network_engine_tcp_packet.py --cesta "$(CESTA)"
	python3 tests/network_tcp_client.py --cesta "$(CESTA)"
	python3 tests/network_engine_transport.py --cesta "$(CESTA)"
	python3 tests/network_job_engine.py --cesta "$(CESTA)"
	python3 tests/network_tcp_listener.py --cesta "$(CESTA)"
	python3 tests/network_service_core.py --cesta "$(CESTA)"
	python3 tests/network_web.py --cesta "$(CESTA)" --mutations
	python3 tests/network_open.py --cesta "$(CESTA)" --mutations
	python3 tests/network_http.py --cesta "$(CESTA)"
	python3 tests/network_device.py --cesta "$(CESTA)"
	python3 tests/network_tcp.py --cesta "$(CESTA)"

qualify-services: check-services
	@test -n "$(LEAN_ROOT)" -a -n "$(CESTA_SOURCE)" || { echo "set LEAN_ROOT and CESTA_SOURCE"; exit 1; }
	python3 tests/network_job_abi.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/network_job_machine.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/network_dns_job_machine.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/network_engine_frame.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/network_dns_codec.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/network_engine_packet.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/network_engine_neighbor.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/network_engine_dns.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/network_engine_tcp_packet.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/network_tcp_client.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/network_engine_transport.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/network_job_engine.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/network_tcp_listener.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/network_service_core.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/kernel_rpc_model.py --cesta "$(CESTA)" --lean-root "$(LEAN_ROOT)"
	python3 "$(CESTA_SOURCE)/tests/protocol_loop_admission.py" --cesta "$(CESTA)" --lean-project "$(LEAN_ROOT)" --module "$(CURDIR)/interfaces/rpc_protocols.module" --module "$(CURDIR)/interfaces/network_tx_protocol.module" --module "$(CURDIR)/interfaces/network_rx_protocol.module"
	python3 tests/service_admission.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"

.PHONY: qualify-network-dns-current-boot
qualify-network-dns-current-boot:
	@test -n "$(REFERENCE_TREE)" -a -n "$(LIMINE)" -a -n "$(NETWORK_DNS_BOOT_WORK)" || { echo "set REFERENCE_TREE, LIMINE and NETWORK_DNS_BOOT_WORK"; exit 1; }
	$(CESTA) build -j $(JOBS) network-dns-protocol-probe sexnet-network sexos-kernel-current
	python3 tests/network_dns_current_service_boot.py --reference-tree "$(REFERENCE_TREE)" --kernel build/sexos-kernel-current/sexos-kernel-current --service build/sexnet-network/sexnet-network --probe build/network-dns-protocol-probe/network-dns-protocol-probe --limine "$(LIMINE)" --output "$(NETWORK_DNS_BOOT_WORK)"

.PHONY: qualify-region-writer-boot
qualify-region-writer-boot:
	@test -n "$(REFERENCE_TREE)" -a -n "$(LIMINE)" -a -n "$(REGION_WRITER_BOOT_WORK)" || { echo "set REFERENCE_TREE, LIMINE and REGION_WRITER_BOOT_WORK"; exit 1; }
	$(CESTA) build -j $(JOBS) region-writer-probe sexos-kernel-current
	python3 tests/region_writer_boot.py --reference-tree "$(REFERENCE_TREE)" --kernel build/sexos-kernel-current/sexos-kernel-current --probe build/region-writer-probe/region-writer-probe --limine "$(LIMINE)" --work "$(REGION_WRITER_BOOT_WORK)"

.PHONY: check-kernel-transport
check-kernel-transport:
	python3 tests/kernel_transport.py --cesta "$(CESTA)" --mutations

.PHONY: check-memory-regions qualify-memory-regions check-kernel-regions qualify-kernel-regions
check-memory-regions:
	python3 tests/memory_regions.py --cesta "$(CESTA)" --mutations
qualify-memory-regions:
	@test -n "$(LEAN_ROOT)" -a -n "$(CESTA_SOURCE)" || { echo "set LEAN_ROOT and CESTA_SOURCE"; exit 1; }
	python3 tests/memory_regions.py --cesta "$(CESTA)" --mutations --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
.PHONY: check-dma-lease qualify-dma-lease
check-dma-lease:
	python3 tests/dma_lease.py --cesta "$(CESTA)" --mutations
qualify-dma-lease:
	@test -n "$(LEAN_ROOT)" -a -n "$(CESTA_SOURCE)" || { echo "set LEAN_ROOT and CESTA_SOURCE"; exit 1; }
	python3 tests/dma_lease.py --cesta "$(CESTA)" --mutations --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
.PHONY: check-device-reset qualify-device-reset
check-device-reset:
	python3 tests/device_reset.py --cesta "$(CESTA)" --mutations
	python3 tests/kernel_device_reset.py --cesta "$(CESTA)" --mutations
	python3 tests/nvme_recovery.py --cesta "$(CESTA)" --mutations
.PHONY: check-smp-park qualify-smp-park
check-smp-park:
	python3 tests/smp_park.py --cesta "$(CESTA)" --mutations
qualify-smp-park:
	@test -n "$(LEAN_ROOT)" -a -n "$(CESTA_SOURCE)" || { echo "set LEAN_ROOT and CESTA_SOURCE"; exit 1; }
	python3 tests/smp_park.py --cesta "$(CESTA)" --mutations --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	cd "$(LEAN_ROOT)" && lake env lean Mettapedia/Machines/Kernel/ParkedCoreReplay.lean
.PHONY: qualify-smp-current-boot
qualify-smp-current-boot:
	@test -n "$(REFERENCE_TREE)" -a -n "$(LIMINE)" -a -n "$(SMP_BOOT_WORK)" || { echo "set REFERENCE_TREE, LIMINE and SMP_BOOT_WORK"; exit 1; }
	$(CESTA) build -j $(JOBS) sexos-kernel-boot-audit sexos-kernel-smp-corrupt-probe
	python3 tests/current_boot.py --reference-tree "$(REFERENCE_TREE)" --kernel build/sexos-kernel-boot-audit/sexos-kernel-boot-audit --limine "$(LIMINE)" --work "$(SMP_BOOT_WORK)/positive" --smp 4
	python3 tests/smp_epoch_negative_boot.py --reference-tree "$(REFERENCE_TREE)" --kernel build/sexos-kernel-smp-corrupt-probe/sexos-kernel-smp-corrupt-probe --limine "$(LIMINE)" --work "$(SMP_BOOT_WORK)/negative"
.PHONY: qualify-fpu-context
qualify-fpu-context:
	@test -n "$(REFERENCE_TREE)" -a -n "$(LIMINE)" -a -n "$(FPU_BOOT_WORK)" || { echo "set REFERENCE_TREE, LIMINE and FPU_BOOT_WORK"; exit 1; }
	$(CESTA) build -j $(JOBS) sexos-kernel-current sexos-kernel-fpu-corrupt-probe fpu-probe-a fpu-probe-b
	python3 tests/fpu_context.py --reference-tree "$(REFERENCE_TREE)" --kernel build/sexos-kernel-current/sexos-kernel-current --corrupt-kernel build/sexos-kernel-fpu-corrupt-probe/sexos-kernel-fpu-corrupt-probe --probe-a build/fpu-probe-a/fpu-probe-a --probe-b build/fpu-probe-b/fpu-probe-b --limine "$(LIMINE)" --work "$(FPU_BOOT_WORK)"
qualify-device-reset:
	@test -n "$(LEAN_ROOT)" -a -n "$(CESTA_SOURCE)" || { echo "set LEAN_ROOT and CESTA_SOURCE"; exit 1; }
	python3 tests/device_reset.py --cesta "$(CESTA)" --mutations --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/kernel_device_reset.py --cesta "$(CESTA)" --mutations
	python3 tests/nvme_recovery.py --cesta "$(CESTA)" --mutations
check-kernel-regions:
	python3 tests/kernel_regions.py --cesta "$(CESTA)" --mutations
	python3 tests/kernel_boot_memory.py --cesta "$(CESTA)" --mutations
qualify-kernel-regions:
	@test -n "$(LEAN_ROOT)" -a -n "$(CESTA_SOURCE)" || { echo "set LEAN_ROOT and CESTA_SOURCE"; exit 1; }
	python3 tests/kernel_regions.py --cesta "$(CESTA)" --mutations
	python3 tests/kernel_boot_memory.py --cesta "$(CESTA)" --mutations --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"

.PHONY: check-current-boot qualify-current-boot
check-current-boot:
	python3 tests/current_boot_catalog.py --cesta "$(CESTA)"
	python3 tests/app_private.py --cesta "$(CESTA)" --mutations
	python3 tests/kernel_private.py --cesta "$(CESTA)" --mutations
qualify-current-boot:
	@test -n "$(LEAN_ROOT)" -a -n "$(CESTA_SOURCE)" || { echo "set LEAN_ROOT and CESTA_SOURCE"; exit 1; }
	python3 tests/current_boot_admission.py --cesta "$(CESTA)" --lean-root "$(LEAN_ROOT)"
	python3 tests/app_private.py --cesta "$(CESTA)" --mutations --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/kernel_private.py --cesta "$(CESTA)" --mutations

.PHONY: check-frame-leases qualify-frame-leases
check-frame-leases:
	python3 tests/frame_lease.py --cesta "$(CESTA)" --mutations
qualify-frame-leases:
	@test -n "$(LEAN_ROOT)" -a -n "$(CESTA_SOURCE)" || { echo "set LEAN_ROOT and CESTA_SOURCE"; exit 1; }
	python3 tests/frame_lease.py --cesta "$(CESTA)" --mutations --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"

.PHONY: check-program-frames qualify-program-frames
check-program-frames:
	python3 tests/program_frames.py --cesta "$(CESTA)" --mutations
qualify-program-frames: qualify-frame-leases
	python3 tests/program_frames.py --cesta "$(CESTA)" --mutations

.PHONY: check-program-memory qualify-program-memory
check-program-memory:
	python3 tests/program_memory.py --cesta "$(CESTA)" --mutations
qualify-program-memory:
	@test -n "$(LEAN_ROOT)" -a -n "$(CESTA_SOURCE)" || { echo "set LEAN_ROOT and CESTA_SOURCE"; exit 1; }
	python3 tests/program_memory.py --cesta "$(CESTA)" --mutations --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"

.PHONY: check-server-heap
check-server-heap:
	python3 tests/server_heap.py --mutations

.PHONY: check-epoch-journal check-epoch-files
check-epoch-journal:
	python3 tests/epoch_journal.py --cesta "$(CESTA)" --mutations
	python3 tests/epoch_journal_format.py
check-epoch-files:
	python3 tests/epoch_files.py --cesta "$(CESTA)" --mutations

.PHONY: check-epoch-persistent
check-epoch-persistent:
	@test -n "$(REFERENCE_TREE)" -a -n "$(LIMINE)" -a -n "$(CURRENT_KERNEL)" -a -n "$(PERSISTENT_BOOT_WORK)" -a -n "$(NVME_IMAGE)" || { echo "set REFERENCE_TREE, LIMINE, CURRENT_KERNEL, PERSISTENT_BOOT_WORK and NVME_IMAGE"; exit 1; }
	python3 tests/epoch_persistent_boot.py --reference-tree "$(REFERENCE_TREE)" --limine "$(LIMINE)" --kernel "$(CURRENT_KERNEL)" --work "$(PERSISTENT_BOOT_WORK)" --seconds 180 --replay-seconds 900 --nvme-image "$(NVME_IMAGE)" --stress-events 8200

.PHONY: check-va-reservation
check-va-reservation:
	python3 tests/va_reservation.py --cesta "$(CESTA)" --mutations

.PHONY: check-program-workers qualify-program-workers check-kernel-workers check-program-records check-program-record-memory check-program-record-tasks
check-kernel-workers:
	python3 tests/kernel_workers.py --cesta "$(CESTA)" --work build/kernel-workers-native --mutations
check-program-workers:
	python3 tests/program_workers.py --cesta "$(CESTA)" --mutations
check-program-records:
	python3 tests/program_records.py --cesta "$(CESTA)" --work build/program-records-native --mutations
check-program-record-memory:
	python3 tests/program_record_memory.py --cesta "$(CESTA)" --work build/program-record-memory-native --mutations
check-program-record-tasks:
	python3 tests/program_record_tasks.py --cesta "$(CESTA)" --work build/program-record-tasks-native --mutations
qualify-program-workers:
	@test -n "$(LEAN_ROOT)" -a -n "$(CESTA_SOURCE)" || { echo "set LEAN_ROOT and CESTA_SOURCE"; exit 1; }
	python3 tests/program_workers.py --cesta "$(CESTA)" --mutations --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"

.PHONY: check-pager qualify-pager check-partitioned-frames qualify-partitioned-frames
check-partitioned-frames:
	python3 tests/sext_partition.py --cesta "$(CESTA)" --mutations
	python3 tests/pager_partition.py --cesta "$(CESTA)" --mutations
qualify-partitioned-frames:
	@test -n "$(LEAN_ROOT)" -a -n "$(CESTA_SOURCE)" || { echo "set LEAN_ROOT and CESTA_SOURCE"; exit 1; }
	python3 tests/sext_partition.py --cesta "$(CESTA)" --mutations --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/pager_partition.py --cesta "$(CESTA)" --mutations --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	cd "$(LEAN_ROOT)" && lake env lean Mettapedia/Machines/Kernel/PartitionedFrameAllocation.lean
check-pager: check-partitioned-frames
	python3 tests/pager_state.py --cesta "$(CESTA)" --mutations
	python3 tests/kernel_paging.py --cesta "$(CESTA)" --mutations
qualify-pager: qualify-partitioned-frames
	@test -n "$(LEAN_ROOT)" -a -n "$(CESTA_SOURCE)" || { echo "set LEAN_ROOT and CESTA_SOURCE"; exit 1; }
	python3 tests/pager_state.py --cesta "$(CESTA)" --mutations --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/kernel_paging.py --cesta "$(CESTA)" --mutations

.PHONY: check-sources qualify-sources
check-sources:
	python3 tests/source_policy.py --cesta "$(CESTA)" --mutations
	python3 tests/kernel_sources.py --cesta "$(CESTA)" --mutations
qualify-sources:
	@test -n "$(LEAN_ROOT)" -a -n "$(CESTA_SOURCE)" || { echo "set LEAN_ROOT and CESTA_SOURCE"; exit 1; }
	python3 tests/source_policy.py --cesta "$(CESTA)" --mutations --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/kernel_sources.py --cesta "$(CESTA)" --mutations

.PHONY: check-pci qualify-pci
check-pci:
	python3 tests/network_config.py --cesta "$(CESTA)" --mutations
	python3 tests/kernel_pci.py --cesta "$(CESTA)" --mutations
qualify-pci:
	@test -n "$(LEAN_ROOT)" -a -n "$(CESTA_SOURCE)" || { echo "set LEAN_ROOT and CESTA_SOURCE"; exit 1; }
	python3 tests/network_config.py --cesta "$(CESTA)" --mutations --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 tests/kernel_pci.py --cesta "$(CESTA)" --mutations --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"

.PHONY: check-history qualify-history
check-history:
	python3 tests/history_recovery.py --cesta "$(CESTA)" --mutations
qualify-history:
	@test -n "$(LEAN_ROOT)" -a -n "$(CESTA_SOURCE)" || { echo "set LEAN_ROOT and CESTA_SOURCE"; exit 1; }
	python3 tests/history_recovery.py --cesta "$(CESTA)" --mutations --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"

.PHONY: check-handover qualify-handover
check-handover:
	python3 tests/app_handover.py --cesta "$(CESTA)" --mutations
qualify-handover:
	@test -n "$(LEAN_ROOT)" -a -n "$(CESTA_SOURCE)" || { echo "set LEAN_ROOT and CESTA_SOURCE"; exit 1; }
	python3 tests/app_handover.py --cesta "$(CESTA)" --mutations --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"

.PHONY: check-shell-text-delivery qualify-shell-text-delivery
check-shell-text-delivery:
	python3 tests/shell_text_delivery.py --cesta "$(CESTA)" --mutations
qualify-shell-text-delivery: check-shell-text-delivery
	python3 tests/shell_text_delivery_ownership.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"

.PHONY: check-program-image qualify-program-image qualify-program-image-hol
check-program-image:
	python3 -B tests/program_image.py --cesta "$(CESTA)"
	python3 -B tests/program_image_kernel.py --cesta "$(CESTA)"

qualify-program-image:
	@test -n "$(CESTA_SOURCE)" -a -n "$(LEAN_ROOT)" || { echo "set CESTA_SOURCE and LEAN_ROOT"; exit 1; }
	python3 -B tests/program_image.py --cesta "$(CESTA)" --admit --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
	python3 -B tests/program_image_kernel.py --cesta "$(CESTA)" --mutations

# Exact-machine qualification remains independent of ordinary build/check.
qualify-program-image-hol:
	@test -n "$(CESTA_SOURCE)" || { echo "set CESTA_SOURCE"; exit 1; }
	python3 -B tests/program_image_hol.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" $(PROGRAM_IMAGE_HOL_FLAGS)

.PHONY: qualify-frame-detach-hol
qualify-frame-detach-hol:
	@test -n "$(CESTA_SOURCE)" || { echo "set CESTA_SOURCE"; exit 1; }
	python3 -B tests/frame_lease_detach_hol.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" $(FRAME_DETACH_HOL_FLAGS)
