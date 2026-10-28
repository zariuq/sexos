CESTA ?= cesta
CESTA_SOURCE ?=
LEAN_ROOT ?=
JOBS ?= 4

.PHONY: all build regenerate export plain check qualify check-services qualify-services
all: build

build:
	$(CESTA) build -j $(JOBS)

regenerate:
	@test -n "$(CESTA_SOURCE)" || { echo "set CESTA_SOURCE to the Cesta source directory"; exit 1; }
	python3 tools/regenerate_client.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)"

export:
	$(CESTA) build --export-c c
	$(CESTA) export-c -o c/src/silk_shell_models.c src/silk_shell_models.cesta --
	$(CESTA) export-c -o c/generated/include/silk_shell.h src/silk_shell_interface.cesta --

plain:
	$(MAKE) -C c -j $(JOBS)

check: check-services check-ownership
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
	python3 tests/sexdisplay_differential.py --cesta "$(CESTA)"
	python3 tests/sexbell_differential.py --cesta "$(CESTA)"
	python3 tests/linen_differential.py --cesta "$(CESTA)"
	python3 tests/quil_differential.py --cesta "$(CESTA)"
	python3 tests/sex_graphics_differential.py --cesta "$(CESTA)"
	python3 tests/spindle_differential.py --cesta "$(CESTA)"
	python3 tests/purple_scanout_differential.py --cesta "$(CESTA)"
	python3 tests/sexfiles_differential.py --cesta "$(CESTA)"
	python3 tests/sexdrive_differential.py --cesta "$(CESTA)"
	python3 tests/sexusb_differential.py --cesta "$(CESTA)"
	python3 tests/sexnet_differential.py --cesta "$(CESTA)"
	python3 tests/silk_shell_models_differential.py --cesta "$(CESTA)"

qualify: qualify-ownership
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
	python3 tests/sexdisplay_differential.py --cesta "$(CESTA)" --all-variants --cases 48
	python3 tests/sexbell_differential.py --cesta "$(CESTA)" --cases 144
	python3 tests/linen_differential.py --cesta "$(CESTA)" --cases 72
	python3 tests/quil_differential.py --cesta "$(CESTA)" --all-variants --cases 48
	python3 tests/sex_graphics_differential.py --cesta "$(CESTA)" --seeds 12
	python3 tests/spindle_differential.py --cesta "$(CESTA)" --all-variants --cases 18
	python3 tests/purple_scanout_differential.py --cesta "$(CESTA)"
	python3 tests/sexfiles_differential.py --cesta "$(CESTA)" --all-variants --cases 18
	python3 tests/sexdrive_differential.py --cesta "$(CESTA)" --all-variants --cases 12
	python3 tests/sexusb_differential.py --cesta "$(CESTA)" --all-variants --cases 12
	python3 tests/sexnet_differential.py --cesta "$(CESTA)" --all-variants --cases 12
	python3 tests/silk_shell_models_differential.py --cesta "$(CESTA)" --seeds 12 --cases 512

.PHONY: check-silk-shell-models
check-silk-shell-models:
	python3 tests/silk_shell_models_differential.py --cesta "$(CESTA)"

.PHONY: check-ownership qualify-ownership
check-ownership:
	python3 tests/sexbell_ownership.py --cesta "$(CESTA)"

qualify-ownership:
	@test -n "$(LEAN_ROOT)" -a -n "$(CESTA_SOURCE)" || { echo "set LEAN_ROOT and CESTA_SOURCE for ownership admission"; exit 1; }
	python3 tests/sexbell_ownership.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"

.PHONY: check-kernel-model
check-kernel-model:
	@test -n "$(LEAN_ROOT)" || { echo "set LEAN_ROOT to the Mettapedia Lean package"; exit 1; }
	python3 tests/kernel_model_differential.py --cesta "$(CESTA)" --lean-root "$(LEAN_ROOT)"

check-services:
	python3 tests/kernel_rpc.py --cesta "$(CESTA)"
	python3 tests/rpc_runtime.py --cesta "$(CESTA)"
	python3 tests/network_http.py --cesta "$(CESTA)"
	python3 tests/network_device.py --cesta "$(CESTA)"
	python3 tests/network_tcp.py --cesta "$(CESTA)"

qualify-services: check-services
	@test -n "$(LEAN_ROOT)" -a -n "$(CESTA_SOURCE)" || { echo "set LEAN_ROOT and CESTA_SOURCE"; exit 1; }
	python3 tests/kernel_rpc_model.py --cesta "$(CESTA)" --lean-root "$(LEAN_ROOT)"
	python3 "$(CESTA_SOURCE)/tests/protocol_loop_admission.py" --cesta "$(CESTA)" --lean-project "$(LEAN_ROOT)" --module "$(CURDIR)/interfaces/rpc_protocols.module" --module "$(CURDIR)/interfaces/network_tx_protocol.module" --module "$(CURDIR)/interfaces/network_rx_protocol.module"
	python3 tests/service_admission.py --cesta "$(CESTA)" --cesta-source "$(CESTA_SOURCE)" --lean-root "$(LEAN_ROOT)"
