# Readable C exported by cesta build from cesta.project; builds without Cesta.
CC = gcc
AR = ar
.DEFAULT_GOAL := all
.DELETE_ON_ERROR:

.PHONY: all
all: build/kaleidoscope/kaleidoscope build/kaleidoscope-navigation/libkaleidoscope-navigation.a build/kaleidoscope-regex/libkaleidoscope-regex.a build/sexdisplay-font/libsexdisplay-font.a build/kaleidoscope-script-lexer/libkaleidoscope-script-lexer.a build/kaleidoscope-script-core/libkaleidoscope-script-core.a build/kaleidoscope-html/libkaleidoscope-html.a build/kaleidoscope-qoi/libkaleidoscope-qoi.a build/kaleidoscope-bitmap/libkaleidoscope-bitmap.a build/kaleidoscope-presentation/libkaleidoscope-presentation.a build/kaleidoscope-asset-view/libkaleidoscope-asset-view.a build/kaleidoscope-png/libkaleidoscope-png.a build/kaleidoscope-asset-pipeline/libkaleidoscope-asset-pipeline.a build/silkbar/silkbar build/sexinput/sexinput build/sexstore/sexstore build/sexdisplay/sexdisplay build/sexdisplay-bitmap/libsexdisplay-bitmap.a build/sexbell/sexbell build/linen/linen build/quil/quil build/quil-autosave/libquil-autosave.a build/document-protocol/libdocument-protocol.a build/document-protocol-probe/document-protocol-probe build/document-protocol-probe-save/document-protocol-probe-save build/document-protocol-probe-recover/document-protocol-probe-recover build/record-hum/record-hum build/spindle-record-probe/spindle-record-probe build/spindle/spindle build/purple-scanout/purple-scanout build/sexfiles/sexfiles build/sexdrive/sexdrive build/sexdrive-dma-quarantine-probe/sexdrive-dma-quarantine-probe build/sexusb/sexusb build/sexnet/sexnet build/kaleidoscope-network/kaleidoscope-network build/network-protocol-probe/network-protocol-probe build/network-dns-protocol-probe/network-dns-protocol-probe build/network-web-protocol-probe/network-web-protocol-probe build/sexnet-network/sexnet-network build/network-job-abi/libnetwork-job-abi.a build/network-job-machine/libnetwork-job-machine.a build/network-dns-job-machine/libnetwork-dns-job-machine.a build/network-engine-frame/libnetwork-engine-frame.a build/network-dns-codec/libnetwork-dns-codec.a build/network-engine-packet/libnetwork-engine-packet.a build/network-engine-neighbor/libnetwork-engine-neighbor.a build/network-engine-dns/libnetwork-engine-dns.a build/network-engine-tcp-packet/libnetwork-engine-tcp-packet.a build/network-tcp-client/libnetwork-tcp-client.a build/network-engine-transport/libnetwork-engine-transport.a build/network-job-engine/libnetwork-job-engine.a build/network-dns-job-engine/libnetwork-dns-job-engine.a build/network-tcp-listener/libnetwork-tcp-listener.a build/network-service-core/libnetwork-service-core.a build/source-policy/libsource-policy.a build/network-config/libnetwork-config.a build/history-recovery/libhistory-recovery.a build/pci-policy/libpci-policy.a build/frame-lease/libframe-lease.a build/program-workers/libprogram-workers.a build/program-records/libprogram-records.a build/program-record-memory/libprogram-record-memory.a build/program-record-tasks/libprogram-record-tasks.a build/program-memory/libprogram-memory.a build/epoch-journal/libepoch-journal.a build/epoch-files/libepoch-files.a build/pager-state/libpager-state.a build/app-private/libapp-private.a build/memory-regions/libmemory-regions.a build/region-writer/libregion-writer.a build/dma-lease/libdma-lease.a build/device-reset/libdevice-reset.a build/nvme-msix/libnvme-msix.a build/smp-park/libsmp-park.a build/arm64-boot-policy/libarm64-boot-policy.a build/sexos-kernel/sexos-kernel build/silk-shell-models/libsilk-shell-models.a build/linen-protocol-probe/linen-protocol-probe build/linen-capture/liblinen-capture.a build/shell-linen-snapshot/libshell-linen-snapshot.a build/silk-shell-models-linen/libsilk-shell-models-linen.a build/silk-shell-models-render/libsilk-shell-models-render.a build/silk-shell-diagnostics-a/libsilk-shell-diagnostics-a.a build/silk-shell-proofs-a/libsilk-shell-proofs-a.a build/silk-shell-proofs-b-palette/libsilk-shell-proofs-b-palette.a build/silk-shell-proofs-b-atlas/libsilk-shell-proofs-b-atlas.a build/silk-shell-proofs-b-input/libsilk-shell-proofs-b-input.a build/silk-shell-proofs-b/libsilk-shell-proofs-b.a build/silk-shell-core/libsilk-shell-core.a build/silk-shell/silk-shell build/shell-proof-lifecycle-fixture/libshell-proof-lifecycle-fixture.a build/silk-shell-input/libsilk-shell-input.a build/silk-shell-atlas/libsilk-shell-atlas.a build/silk-shell-custody/libsilk-shell-custody.a build/silk-shell-chrome/libsilk-shell-chrome.a build/silk-shell-viewport/libsilk-shell-viewport.a build/shell-text-delivery/libshell-text-delivery.a build/silk-shell-surfaces/libsilk-shell-surfaces.a build/pager-probe/pager-probe build/source-probe/source-probe build/pci-probe/pci-probe build/region-writer-probe/region-writer-probe build/fpu-probe-a/fpu-probe-a build/fpu-probe-b/fpu-probe-b build/history-probe/history-probe build/retire-probe/retire-probe build/sext/sext build/sext-worker-probe/sext-worker-probe build/sexos-kernel-current/sexos-kernel-current build/sexos-kernel-fpu-corrupt-probe/sexos-kernel-fpu-corrupt-probe build/sexos-kernel-boot-audit/sexos-kernel-boot-audit build/sexos-kernel-smp-corrupt-probe/sexos-kernel-smp-corrupt-probe build/sexos-kernel-network/sexos-kernel-network

build/.objects/3056f357363e2d28/src/kaleidoscope.cesta.o: src/kaleidoscope.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-msse2' '-mfpmath=sse' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_KALEIDOSCOPE_CURRENT=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kaleidoscope.c' -o "$@"

build/kaleidoscope/obj/src/kaleidoscope.cesta.o: build/.objects/3056f357363e2d28/src/kaleidoscope.cesta.o
	@mkdir -p 'build/kaleidoscope/obj/src'
	ln -f 'build/.objects/3056f357363e2d28/src/kaleidoscope.cesta.o' "$@"

build/.objects/1972bc0eb8330e3b/src/kaleidoscope_body.cesta.o: src/kaleidoscope_body.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-msse2' '-mfpmath=sse' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_KALEIDOSCOPE_CURRENT=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kaleidoscope_body.c' -o "$@"

build/kaleidoscope/obj/src/kaleidoscope_body.cesta.o: build/.objects/1972bc0eb8330e3b/src/kaleidoscope_body.cesta.o
	@mkdir -p 'build/kaleidoscope/obj/src'
	ln -f 'build/.objects/1972bc0eb8330e3b/src/kaleidoscope_body.cesta.o' "$@"

build/.objects/1c25e7c040b59cf7/src/kaleidoscope_web_page.cesta.o: src/kaleidoscope_web_page.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-msse2' '-mfpmath=sse' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_KALEIDOSCOPE_CURRENT=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kaleidoscope_web_page.c' -o "$@"

build/kaleidoscope/obj/src/kaleidoscope_web_page.cesta.o: build/.objects/1c25e7c040b59cf7/src/kaleidoscope_web_page.cesta.o
	@mkdir -p 'build/kaleidoscope/obj/src'
	ln -f 'build/.objects/1c25e7c040b59cf7/src/kaleidoscope_web_page.cesta.o' "$@"

build/.objects/a107cbf27579b4e8/src/kaleidoscope_decimal.cesta.o: src/kaleidoscope_decimal.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-msse2' '-mfpmath=sse' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_KALEIDOSCOPE_CURRENT=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kaleidoscope_decimal.c' -o "$@"

build/kaleidoscope/obj/src/kaleidoscope_decimal.cesta.o: build/.objects/a107cbf27579b4e8/src/kaleidoscope_decimal.cesta.o
	@mkdir -p 'build/kaleidoscope/obj/src'
	ln -f 'build/.objects/a107cbf27579b4e8/src/kaleidoscope_decimal.cesta.o' "$@"

build/.objects/2f3c865ef8a0b1cd/src/kaleidoscope_script_lexer.cesta.o: src/kaleidoscope_script_lexer.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-msse2' '-mfpmath=sse' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_KALEIDOSCOPE_CURRENT=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kaleidoscope_script_lexer.c' -o "$@"

build/kaleidoscope/obj/src/kaleidoscope_script_lexer.cesta.o: build/.objects/2f3c865ef8a0b1cd/src/kaleidoscope_script_lexer.cesta.o
	@mkdir -p 'build/kaleidoscope/obj/src'
	ln -f 'build/.objects/2f3c865ef8a0b1cd/src/kaleidoscope_script_lexer.cesta.o' "$@"

build/.objects/ca12216c321cdcc8/src/kaleidoscope_regex.cesta.o: src/kaleidoscope_regex.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-msse2' '-mfpmath=sse' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_KALEIDOSCOPE_CURRENT=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kaleidoscope_regex.c' -o "$@"

build/kaleidoscope/obj/src/kaleidoscope_regex.cesta.o: build/.objects/ca12216c321cdcc8/src/kaleidoscope_regex.cesta.o
	@mkdir -p 'build/kaleidoscope/obj/src'
	ln -f 'build/.objects/ca12216c321cdcc8/src/kaleidoscope_regex.cesta.o' "$@"

build/.objects/a4810fb4ef4fcafa/src/kaleidoscope_regex_match.cesta.o: src/kaleidoscope_regex_match.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-msse2' '-mfpmath=sse' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_KALEIDOSCOPE_CURRENT=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kaleidoscope_regex_match.c' -o "$@"

build/kaleidoscope/obj/src/kaleidoscope_regex_match.cesta.o: build/.objects/a4810fb4ef4fcafa/src/kaleidoscope_regex_match.cesta.o
	@mkdir -p 'build/kaleidoscope/obj/src'
	ln -f 'build/.objects/a4810fb4ef4fcafa/src/kaleidoscope_regex_match.cesta.o' "$@"

build/.objects/5a1b94b120a66954/src/kaleidoscope_script_core.cesta.o: src/kaleidoscope_script_core.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-msse2' '-mfpmath=sse' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_KALEIDOSCOPE_CURRENT=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kaleidoscope_script_core.c' -o "$@"

build/kaleidoscope/obj/src/kaleidoscope_script_core.cesta.o: build/.objects/5a1b94b120a66954/src/kaleidoscope_script_core.cesta.o
	@mkdir -p 'build/kaleidoscope/obj/src'
	ln -f 'build/.objects/5a1b94b120a66954/src/kaleidoscope_script_core.cesta.o' "$@"

build/.objects/1d1d75e5b87d331e/src/kaleidoscope_html.cesta.o: src/kaleidoscope_html.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-msse2' '-mfpmath=sse' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_KALEIDOSCOPE_CURRENT=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kaleidoscope_html.c' -o "$@"

build/kaleidoscope/obj/src/kaleidoscope_html.cesta.o: build/.objects/1d1d75e5b87d331e/src/kaleidoscope_html.cesta.o
	@mkdir -p 'build/kaleidoscope/obj/src'
	ln -f 'build/.objects/1d1d75e5b87d331e/src/kaleidoscope_html.cesta.o' "$@"

build/.objects/800aaa8ad5cc7e60/runtime/client_runtime.c.o: runtime/client_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-msse2' '-mfpmath=sse' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_KALEIDOSCOPE_CURRENT=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/kaleidoscope/obj/runtime/client_runtime.c.o: build/.objects/800aaa8ad5cc7e60/runtime/client_runtime.c.o
	@mkdir -p 'build/kaleidoscope/obj/runtime'
	ln -f 'build/.objects/800aaa8ad5cc7e60/runtime/client_runtime.c.o' "$@"

build/.objects/bc299f33d79e4eb4/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-msse2' '-mfpmath=sse' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_KALEIDOSCOPE_CURRENT=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/kaleidoscope/obj/runtime/traps_x86_64.c.o: build/.objects/bc299f33d79e4eb4/runtime/traps_x86_64.c.o
	@mkdir -p 'build/kaleidoscope/obj/runtime'
	ln -f 'build/.objects/bc299f33d79e4eb4/runtime/traps_x86_64.c.o' "$@"

build/.objects/eb0ca2bfcbe92530/runtime/start_x86_64.S.o: runtime/start_x86_64.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-msse2' '-mfpmath=sse' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_KALEIDOSCOPE_CURRENT=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/kaleidoscope/obj/runtime/start_x86_64.S.o: build/.objects/eb0ca2bfcbe92530/runtime/start_x86_64.S.o
	@mkdir -p 'build/kaleidoscope/obj/runtime'
	ln -f 'build/.objects/eb0ca2bfcbe92530/runtime/start_x86_64.S.o' "$@"

build/.objects/e20b5db439e268d1/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-msse2' '-mfpmath=sse' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_KALEIDOSCOPE_CURRENT=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/kaleidoscope/obj/generated/runtime/format_bytes.c.o: build/.objects/e20b5db439e268d1/generated/runtime/format_bytes.c.o
	@mkdir -p 'build/kaleidoscope/obj/generated/runtime'
	ln -f 'build/.objects/e20b5db439e268d1/generated/runtime/format_bytes.c.o' "$@"

build/.objects/ccefbd874e02dbc0/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-msse2' '-mfpmath=sse' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_KALEIDOSCOPE_CURRENT=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/kaleidoscope/obj/generated/runtime/packed_words.c.o: build/.objects/ccefbd874e02dbc0/generated/runtime/packed_words.c.o
	@mkdir -p 'build/kaleidoscope/obj/generated/runtime'
	ln -f 'build/.objects/ccefbd874e02dbc0/generated/runtime/packed_words.c.o' "$@"

build/kaleidoscope/kaleidoscope: GNUmakefile build/kaleidoscope/obj/src/kaleidoscope.cesta.o build/kaleidoscope/obj/src/kaleidoscope_body.cesta.o build/kaleidoscope/obj/src/kaleidoscope_web_page.cesta.o build/kaleidoscope/obj/src/kaleidoscope_decimal.cesta.o build/kaleidoscope/obj/src/kaleidoscope_script_lexer.cesta.o build/kaleidoscope/obj/src/kaleidoscope_regex.cesta.o build/kaleidoscope/obj/src/kaleidoscope_regex_match.cesta.o build/kaleidoscope/obj/src/kaleidoscope_script_core.cesta.o build/kaleidoscope/obj/src/kaleidoscope_html.cesta.o build/kaleidoscope/obj/runtime/client_runtime.c.o build/kaleidoscope/obj/runtime/traps_x86_64.c.o build/kaleidoscope/obj/runtime/start_x86_64.S.o build/kaleidoscope/obj/generated/runtime/format_bytes.c.o build/kaleidoscope/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/kaleidoscope/obj/src/kaleidoscope.cesta.o' 'build/kaleidoscope/obj/src/kaleidoscope_body.cesta.o' 'build/kaleidoscope/obj/src/kaleidoscope_web_page.cesta.o' 'build/kaleidoscope/obj/src/kaleidoscope_decimal.cesta.o' 'build/kaleidoscope/obj/src/kaleidoscope_script_lexer.cesta.o' 'build/kaleidoscope/obj/src/kaleidoscope_regex.cesta.o' 'build/kaleidoscope/obj/src/kaleidoscope_regex_match.cesta.o' 'build/kaleidoscope/obj/src/kaleidoscope_script_core.cesta.o' 'build/kaleidoscope/obj/src/kaleidoscope_html.cesta.o' 'build/kaleidoscope/obj/runtime/client_runtime.c.o' 'build/kaleidoscope/obj/runtime/traps_x86_64.c.o' 'build/kaleidoscope/obj/runtime/start_x86_64.S.o' 'build/kaleidoscope/obj/generated/runtime/format_bytes.c.o' 'build/kaleidoscope/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/.objects/3a47b50c3706c99c/src/kaleidoscope_navigation.cesta.o: src/kaleidoscope_navigation.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kaleidoscope_navigation.c' -o "$@"

build/kaleidoscope-navigation/obj/src/kaleidoscope_navigation.cesta.o: build/.objects/3a47b50c3706c99c/src/kaleidoscope_navigation.cesta.o
	@mkdir -p 'build/kaleidoscope-navigation/obj/src'
	ln -f 'build/.objects/3a47b50c3706c99c/src/kaleidoscope_navigation.cesta.o' "$@"

build/kaleidoscope-navigation/libkaleidoscope-navigation.a: GNUmakefile build/kaleidoscope-navigation/obj/src/kaleidoscope_navigation.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/kaleidoscope-navigation/obj/src/kaleidoscope_navigation.cesta.o'

build/.objects/1fad969ee0e0b087/src/kaleidoscope_regex.cesta.o: src/kaleidoscope_regex.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kaleidoscope_regex.c' -o "$@"

build/kaleidoscope-regex/obj/src/kaleidoscope_regex.cesta.o: build/.objects/1fad969ee0e0b087/src/kaleidoscope_regex.cesta.o
	@mkdir -p 'build/kaleidoscope-regex/obj/src'
	ln -f 'build/.objects/1fad969ee0e0b087/src/kaleidoscope_regex.cesta.o' "$@"

build/.objects/612754aef6f9a07d/src/kaleidoscope_regex_match.cesta.o: src/kaleidoscope_regex_match.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kaleidoscope_regex_match.c' -o "$@"

build/kaleidoscope-regex/obj/src/kaleidoscope_regex_match.cesta.o: build/.objects/612754aef6f9a07d/src/kaleidoscope_regex_match.cesta.o
	@mkdir -p 'build/kaleidoscope-regex/obj/src'
	ln -f 'build/.objects/612754aef6f9a07d/src/kaleidoscope_regex_match.cesta.o' "$@"

build/kaleidoscope-regex/libkaleidoscope-regex.a: GNUmakefile build/kaleidoscope-regex/obj/src/kaleidoscope_regex.cesta.o build/kaleidoscope-regex/obj/src/kaleidoscope_regex_match.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/kaleidoscope-regex/obj/src/kaleidoscope_regex.cesta.o' 'build/kaleidoscope-regex/obj/src/kaleidoscope_regex_match.cesta.o'

build/.objects/70e762eb5967f38a/src/sexdisplay_font.cesta.o: src/sexdisplay_font.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/sexdisplay_font.c' -o "$@"

build/sexdisplay-font/obj/src/sexdisplay_font.cesta.o: build/.objects/70e762eb5967f38a/src/sexdisplay_font.cesta.o
	@mkdir -p 'build/sexdisplay-font/obj/src'
	ln -f 'build/.objects/70e762eb5967f38a/src/sexdisplay_font.cesta.o' "$@"

build/sexdisplay-font/libsexdisplay-font.a: GNUmakefile build/sexdisplay-font/obj/src/sexdisplay_font.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/sexdisplay-font/obj/src/sexdisplay_font.cesta.o'

build/.objects/c6120ba0e3abde30/src/kaleidoscope_decimal.cesta.o: src/kaleidoscope_decimal.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-msse2' '-mfpmath=sse' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kaleidoscope_decimal.c' -o "$@"

build/kaleidoscope-script-lexer/obj/src/kaleidoscope_decimal.cesta.o: build/.objects/c6120ba0e3abde30/src/kaleidoscope_decimal.cesta.o
	@mkdir -p 'build/kaleidoscope-script-lexer/obj/src'
	ln -f 'build/.objects/c6120ba0e3abde30/src/kaleidoscope_decimal.cesta.o' "$@"

build/.objects/4c04085f5f58523f/src/kaleidoscope_script_lexer.cesta.o: src/kaleidoscope_script_lexer.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-msse2' '-mfpmath=sse' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kaleidoscope_script_lexer.c' -o "$@"

build/kaleidoscope-script-lexer/obj/src/kaleidoscope_script_lexer.cesta.o: build/.objects/4c04085f5f58523f/src/kaleidoscope_script_lexer.cesta.o
	@mkdir -p 'build/kaleidoscope-script-lexer/obj/src'
	ln -f 'build/.objects/4c04085f5f58523f/src/kaleidoscope_script_lexer.cesta.o' "$@"

build/kaleidoscope-script-lexer/libkaleidoscope-script-lexer.a: GNUmakefile build/kaleidoscope-script-lexer/obj/src/kaleidoscope_decimal.cesta.o build/kaleidoscope-script-lexer/obj/src/kaleidoscope_script_lexer.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/kaleidoscope-script-lexer/obj/src/kaleidoscope_decimal.cesta.o' 'build/kaleidoscope-script-lexer/obj/src/kaleidoscope_script_lexer.cesta.o'

build/kaleidoscope-script-core/obj/src/kaleidoscope_decimal.cesta.o: build/.objects/c6120ba0e3abde30/src/kaleidoscope_decimal.cesta.o
	@mkdir -p 'build/kaleidoscope-script-core/obj/src'
	ln -f 'build/.objects/c6120ba0e3abde30/src/kaleidoscope_decimal.cesta.o' "$@"

build/kaleidoscope-script-core/obj/src/kaleidoscope_script_lexer.cesta.o: build/.objects/4c04085f5f58523f/src/kaleidoscope_script_lexer.cesta.o
	@mkdir -p 'build/kaleidoscope-script-core/obj/src'
	ln -f 'build/.objects/4c04085f5f58523f/src/kaleidoscope_script_lexer.cesta.o' "$@"

build/.objects/6f1bd58338b211d0/src/kaleidoscope_regex.cesta.o: src/kaleidoscope_regex.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-msse2' '-mfpmath=sse' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kaleidoscope_regex.c' -o "$@"

build/kaleidoscope-script-core/obj/src/kaleidoscope_regex.cesta.o: build/.objects/6f1bd58338b211d0/src/kaleidoscope_regex.cesta.o
	@mkdir -p 'build/kaleidoscope-script-core/obj/src'
	ln -f 'build/.objects/6f1bd58338b211d0/src/kaleidoscope_regex.cesta.o' "$@"

build/.objects/07b8b4c281f4ed82/src/kaleidoscope_regex_match.cesta.o: src/kaleidoscope_regex_match.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-msse2' '-mfpmath=sse' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kaleidoscope_regex_match.c' -o "$@"

build/kaleidoscope-script-core/obj/src/kaleidoscope_regex_match.cesta.o: build/.objects/07b8b4c281f4ed82/src/kaleidoscope_regex_match.cesta.o
	@mkdir -p 'build/kaleidoscope-script-core/obj/src'
	ln -f 'build/.objects/07b8b4c281f4ed82/src/kaleidoscope_regex_match.cesta.o' "$@"

build/.objects/a4d68bbd3dc3d604/src/kaleidoscope_script_core.cesta.o: src/kaleidoscope_script_core.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-msse2' '-mfpmath=sse' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kaleidoscope_script_core.c' -o "$@"

build/kaleidoscope-script-core/obj/src/kaleidoscope_script_core.cesta.o: build/.objects/a4d68bbd3dc3d604/src/kaleidoscope_script_core.cesta.o
	@mkdir -p 'build/kaleidoscope-script-core/obj/src'
	ln -f 'build/.objects/a4d68bbd3dc3d604/src/kaleidoscope_script_core.cesta.o' "$@"

build/kaleidoscope-script-core/libkaleidoscope-script-core.a: GNUmakefile build/kaleidoscope-script-core/obj/src/kaleidoscope_decimal.cesta.o build/kaleidoscope-script-core/obj/src/kaleidoscope_script_lexer.cesta.o build/kaleidoscope-script-core/obj/src/kaleidoscope_regex.cesta.o build/kaleidoscope-script-core/obj/src/kaleidoscope_regex_match.cesta.o build/kaleidoscope-script-core/obj/src/kaleidoscope_script_core.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/kaleidoscope-script-core/obj/src/kaleidoscope_decimal.cesta.o' 'build/kaleidoscope-script-core/obj/src/kaleidoscope_script_lexer.cesta.o' 'build/kaleidoscope-script-core/obj/src/kaleidoscope_regex.cesta.o' 'build/kaleidoscope-script-core/obj/src/kaleidoscope_regex_match.cesta.o' 'build/kaleidoscope-script-core/obj/src/kaleidoscope_script_core.cesta.o'

build/kaleidoscope-html/obj/src/kaleidoscope_decimal.cesta.o: build/.objects/c6120ba0e3abde30/src/kaleidoscope_decimal.cesta.o
	@mkdir -p 'build/kaleidoscope-html/obj/src'
	ln -f 'build/.objects/c6120ba0e3abde30/src/kaleidoscope_decimal.cesta.o' "$@"

build/kaleidoscope-html/obj/src/kaleidoscope_script_lexer.cesta.o: build/.objects/4c04085f5f58523f/src/kaleidoscope_script_lexer.cesta.o
	@mkdir -p 'build/kaleidoscope-html/obj/src'
	ln -f 'build/.objects/4c04085f5f58523f/src/kaleidoscope_script_lexer.cesta.o' "$@"

build/kaleidoscope-html/obj/src/kaleidoscope_regex.cesta.o: build/.objects/6f1bd58338b211d0/src/kaleidoscope_regex.cesta.o
	@mkdir -p 'build/kaleidoscope-html/obj/src'
	ln -f 'build/.objects/6f1bd58338b211d0/src/kaleidoscope_regex.cesta.o' "$@"

build/kaleidoscope-html/obj/src/kaleidoscope_regex_match.cesta.o: build/.objects/07b8b4c281f4ed82/src/kaleidoscope_regex_match.cesta.o
	@mkdir -p 'build/kaleidoscope-html/obj/src'
	ln -f 'build/.objects/07b8b4c281f4ed82/src/kaleidoscope_regex_match.cesta.o' "$@"

build/kaleidoscope-html/obj/src/kaleidoscope_script_core.cesta.o: build/.objects/a4d68bbd3dc3d604/src/kaleidoscope_script_core.cesta.o
	@mkdir -p 'build/kaleidoscope-html/obj/src'
	ln -f 'build/.objects/a4d68bbd3dc3d604/src/kaleidoscope_script_core.cesta.o' "$@"

build/.objects/224eff0969acfd2e/src/kaleidoscope_html.cesta.o: src/kaleidoscope_html.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-msse2' '-mfpmath=sse' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kaleidoscope_html.c' -o "$@"

build/kaleidoscope-html/obj/src/kaleidoscope_html.cesta.o: build/.objects/224eff0969acfd2e/src/kaleidoscope_html.cesta.o
	@mkdir -p 'build/kaleidoscope-html/obj/src'
	ln -f 'build/.objects/224eff0969acfd2e/src/kaleidoscope_html.cesta.o' "$@"

build/kaleidoscope-html/libkaleidoscope-html.a: GNUmakefile build/kaleidoscope-html/obj/src/kaleidoscope_decimal.cesta.o build/kaleidoscope-html/obj/src/kaleidoscope_script_lexer.cesta.o build/kaleidoscope-html/obj/src/kaleidoscope_regex.cesta.o build/kaleidoscope-html/obj/src/kaleidoscope_regex_match.cesta.o build/kaleidoscope-html/obj/src/kaleidoscope_script_core.cesta.o build/kaleidoscope-html/obj/src/kaleidoscope_html.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/kaleidoscope-html/obj/src/kaleidoscope_decimal.cesta.o' 'build/kaleidoscope-html/obj/src/kaleidoscope_script_lexer.cesta.o' 'build/kaleidoscope-html/obj/src/kaleidoscope_regex.cesta.o' 'build/kaleidoscope-html/obj/src/kaleidoscope_regex_match.cesta.o' 'build/kaleidoscope-html/obj/src/kaleidoscope_script_core.cesta.o' 'build/kaleidoscope-html/obj/src/kaleidoscope_html.cesta.o'

build/.objects/be688e4577b7ad7f/src/kaleidoscope_qoi.cesta.o: src/kaleidoscope_qoi.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kaleidoscope_qoi.c' -o "$@"

build/kaleidoscope-qoi/obj/src/kaleidoscope_qoi.cesta.o: build/.objects/be688e4577b7ad7f/src/kaleidoscope_qoi.cesta.o
	@mkdir -p 'build/kaleidoscope-qoi/obj/src'
	ln -f 'build/.objects/be688e4577b7ad7f/src/kaleidoscope_qoi.cesta.o' "$@"

build/kaleidoscope-qoi/libkaleidoscope-qoi.a: GNUmakefile build/kaleidoscope-qoi/obj/src/kaleidoscope_qoi.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/kaleidoscope-qoi/obj/src/kaleidoscope_qoi.cesta.o'

build/.objects/bc54e5dcedfd5c53/src/kaleidoscope_bitmap.cesta.o: src/kaleidoscope_bitmap.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kaleidoscope_bitmap.c' -o "$@"

build/kaleidoscope-bitmap/obj/src/kaleidoscope_bitmap.cesta.o: build/.objects/bc54e5dcedfd5c53/src/kaleidoscope_bitmap.cesta.o
	@mkdir -p 'build/kaleidoscope-bitmap/obj/src'
	ln -f 'build/.objects/bc54e5dcedfd5c53/src/kaleidoscope_bitmap.cesta.o' "$@"

build/kaleidoscope-bitmap/libkaleidoscope-bitmap.a: GNUmakefile build/kaleidoscope-bitmap/obj/src/kaleidoscope_bitmap.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/kaleidoscope-bitmap/obj/src/kaleidoscope_bitmap.cesta.o'

build/.objects/29052c55faa1738e/src/kaleidoscope_presentation.cesta.o: src/kaleidoscope_presentation.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kaleidoscope_presentation.c' -o "$@"

build/kaleidoscope-presentation/obj/src/kaleidoscope_presentation.cesta.o: build/.objects/29052c55faa1738e/src/kaleidoscope_presentation.cesta.o
	@mkdir -p 'build/kaleidoscope-presentation/obj/src'
	ln -f 'build/.objects/29052c55faa1738e/src/kaleidoscope_presentation.cesta.o' "$@"

build/kaleidoscope-presentation/libkaleidoscope-presentation.a: GNUmakefile build/kaleidoscope-presentation/obj/src/kaleidoscope_presentation.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/kaleidoscope-presentation/obj/src/kaleidoscope_presentation.cesta.o'

build/.objects/527608ff2658c286/src/kaleidoscope_asset_view.cesta.o: src/kaleidoscope_asset_view.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kaleidoscope_asset_view.c' -o "$@"

build/kaleidoscope-asset-view/obj/src/kaleidoscope_asset_view.cesta.o: build/.objects/527608ff2658c286/src/kaleidoscope_asset_view.cesta.o
	@mkdir -p 'build/kaleidoscope-asset-view/obj/src'
	ln -f 'build/.objects/527608ff2658c286/src/kaleidoscope_asset_view.cesta.o' "$@"

build/.objects/1349797934031919/src/kaleidoscope_asset_view_render.cesta.o: src/kaleidoscope_asset_view_render.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kaleidoscope_asset_view_render.c' -o "$@"

build/kaleidoscope-asset-view/obj/src/kaleidoscope_asset_view_render.cesta.o: build/.objects/1349797934031919/src/kaleidoscope_asset_view_render.cesta.o
	@mkdir -p 'build/kaleidoscope-asset-view/obj/src'
	ln -f 'build/.objects/1349797934031919/src/kaleidoscope_asset_view_render.cesta.o' "$@"

build/kaleidoscope-asset-view/libkaleidoscope-asset-view.a: GNUmakefile build/kaleidoscope-asset-view/obj/src/kaleidoscope_asset_view.cesta.o build/kaleidoscope-asset-view/obj/src/kaleidoscope_asset_view_render.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/kaleidoscope-asset-view/obj/src/kaleidoscope_asset_view.cesta.o' 'build/kaleidoscope-asset-view/obj/src/kaleidoscope_asset_view_render.cesta.o'

build/.objects/4fe491afb201ac77/src/kaleidoscope_png.cesta.o: src/kaleidoscope_png.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kaleidoscope_png.c' -o "$@"

build/kaleidoscope-png/obj/src/kaleidoscope_png.cesta.o: build/.objects/4fe491afb201ac77/src/kaleidoscope_png.cesta.o
	@mkdir -p 'build/kaleidoscope-png/obj/src'
	ln -f 'build/.objects/4fe491afb201ac77/src/kaleidoscope_png.cesta.o' "$@"

build/kaleidoscope-png/libkaleidoscope-png.a: GNUmakefile build/kaleidoscope-png/obj/src/kaleidoscope_png.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/kaleidoscope-png/obj/src/kaleidoscope_png.cesta.o'

build/kaleidoscope-asset-pipeline/obj/src/kaleidoscope_qoi.cesta.o: build/.objects/be688e4577b7ad7f/src/kaleidoscope_qoi.cesta.o
	@mkdir -p 'build/kaleidoscope-asset-pipeline/obj/src'
	ln -f 'build/.objects/be688e4577b7ad7f/src/kaleidoscope_qoi.cesta.o' "$@"

build/kaleidoscope-asset-pipeline/obj/src/kaleidoscope_png.cesta.o: build/.objects/4fe491afb201ac77/src/kaleidoscope_png.cesta.o
	@mkdir -p 'build/kaleidoscope-asset-pipeline/obj/src'
	ln -f 'build/.objects/4fe491afb201ac77/src/kaleidoscope_png.cesta.o' "$@"

build/.objects/7f51cb38bd8f22fb/src/kaleidoscope_asset_pipeline.cesta.o: src/kaleidoscope_asset_pipeline.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kaleidoscope_asset_pipeline.c' -o "$@"

build/kaleidoscope-asset-pipeline/obj/src/kaleidoscope_asset_pipeline.cesta.o: build/.objects/7f51cb38bd8f22fb/src/kaleidoscope_asset_pipeline.cesta.o
	@mkdir -p 'build/kaleidoscope-asset-pipeline/obj/src'
	ln -f 'build/.objects/7f51cb38bd8f22fb/src/kaleidoscope_asset_pipeline.cesta.o' "$@"

build/kaleidoscope-asset-pipeline/libkaleidoscope-asset-pipeline.a: GNUmakefile build/kaleidoscope-asset-pipeline/obj/src/kaleidoscope_qoi.cesta.o build/kaleidoscope-asset-pipeline/obj/src/kaleidoscope_png.cesta.o build/kaleidoscope-asset-pipeline/obj/src/kaleidoscope_asset_pipeline.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/kaleidoscope-asset-pipeline/obj/src/kaleidoscope_qoi.cesta.o' 'build/kaleidoscope-asset-pipeline/obj/src/kaleidoscope_png.cesta.o' 'build/kaleidoscope-asset-pipeline/obj/src/kaleidoscope_asset_pipeline.cesta.o'

build/.objects/809a7111a7845347/src/silkbar.cesta.o: src/silkbar.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=silkbar_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silkbar.c' -o "$@"

build/silkbar/obj/src/silkbar.cesta.o: build/.objects/809a7111a7845347/src/silkbar.cesta.o
	@mkdir -p 'build/silkbar/obj/src'
	ln -f 'build/.objects/809a7111a7845347/src/silkbar.cesta.o' "$@"

build/.objects/644594c8a824e27d/src/silkbar_model.cesta.o: src/silkbar_model.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=silkbar_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silkbar_model.c' -o "$@"

build/silkbar/obj/src/silkbar_model.cesta.o: build/.objects/644594c8a824e27d/src/silkbar_model.cesta.o
	@mkdir -p 'build/silkbar/obj/src'
	ln -f 'build/.objects/644594c8a824e27d/src/silkbar_model.cesta.o' "$@"

build/.objects/c869434c2f4d31e2/runtime/client_runtime.c.o: runtime/client_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=silkbar_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/silkbar/obj/runtime/client_runtime.c.o: build/.objects/c869434c2f4d31e2/runtime/client_runtime.c.o
	@mkdir -p 'build/silkbar/obj/runtime'
	ln -f 'build/.objects/c869434c2f4d31e2/runtime/client_runtime.c.o' "$@"

build/.objects/c75551cc96199ab6/runtime/server_runtime.c.o: runtime/server_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=silkbar_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_runtime.c' -o "$@"

build/silkbar/obj/runtime/server_runtime.c.o: build/.objects/c75551cc96199ab6/runtime/server_runtime.c.o
	@mkdir -p 'build/silkbar/obj/runtime'
	ln -f 'build/.objects/c75551cc96199ab6/runtime/server_runtime.c.o' "$@"

build/.objects/01af3a7d099c537e/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=silkbar_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/silkbar/obj/runtime/traps_x86_64.c.o: build/.objects/01af3a7d099c537e/runtime/traps_x86_64.c.o
	@mkdir -p 'build/silkbar/obj/runtime'
	ln -f 'build/.objects/01af3a7d099c537e/runtime/traps_x86_64.c.o' "$@"

build/.objects/79bc4f70b09c47a2/runtime/server_traps_x86_64.c.o: runtime/server_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=silkbar_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_traps_x86_64.c' -o "$@"

build/silkbar/obj/runtime/server_traps_x86_64.c.o: build/.objects/79bc4f70b09c47a2/runtime/server_traps_x86_64.c.o
	@mkdir -p 'build/silkbar/obj/runtime'
	ln -f 'build/.objects/79bc4f70b09c47a2/runtime/server_traps_x86_64.c.o' "$@"

build/.objects/96f32a455f237b12/runtime/start_x86_64.S.o: runtime/start_x86_64.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=silkbar_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/silkbar/obj/runtime/start_x86_64.S.o: build/.objects/96f32a455f237b12/runtime/start_x86_64.S.o
	@mkdir -p 'build/silkbar/obj/runtime'
	ln -f 'build/.objects/96f32a455f237b12/runtime/start_x86_64.S.o' "$@"

build/.objects/b7384930acaa6d45/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=silkbar_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/silkbar/obj/generated/runtime/format_bytes.c.o: build/.objects/b7384930acaa6d45/generated/runtime/format_bytes.c.o
	@mkdir -p 'build/silkbar/obj/generated/runtime'
	ln -f 'build/.objects/b7384930acaa6d45/generated/runtime/format_bytes.c.o' "$@"

build/.objects/0fd89aeff9cdcc82/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=silkbar_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/silkbar/obj/generated/runtime/packed_words.c.o: build/.objects/0fd89aeff9cdcc82/generated/runtime/packed_words.c.o
	@mkdir -p 'build/silkbar/obj/generated/runtime'
	ln -f 'build/.objects/0fd89aeff9cdcc82/generated/runtime/packed_words.c.o' "$@"

build/silkbar/silkbar: GNUmakefile build/silkbar/obj/src/silkbar.cesta.o build/silkbar/obj/src/silkbar_model.cesta.o build/silkbar/obj/runtime/client_runtime.c.o build/silkbar/obj/runtime/server_runtime.c.o build/silkbar/obj/runtime/traps_x86_64.c.o build/silkbar/obj/runtime/server_traps_x86_64.c.o build/silkbar/obj/runtime/start_x86_64.S.o build/silkbar/obj/generated/runtime/format_bytes.c.o build/silkbar/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/silkbar/obj/src/silkbar.cesta.o' 'build/silkbar/obj/src/silkbar_model.cesta.o' 'build/silkbar/obj/runtime/client_runtime.c.o' 'build/silkbar/obj/runtime/server_runtime.c.o' 'build/silkbar/obj/runtime/traps_x86_64.c.o' 'build/silkbar/obj/runtime/server_traps_x86_64.c.o' 'build/silkbar/obj/runtime/start_x86_64.S.o' 'build/silkbar/obj/generated/runtime/format_bytes.c.o' 'build/silkbar/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/.objects/a4707781d0fe720b/src/sexinput.cesta.o: src/sexinput.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexinput_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/sexinput.c' -o "$@"

build/sexinput/obj/src/sexinput.cesta.o: build/.objects/a4707781d0fe720b/src/sexinput.cesta.o
	@mkdir -p 'build/sexinput/obj/src'
	ln -f 'build/.objects/a4707781d0fe720b/src/sexinput.cesta.o' "$@"

build/.objects/8e204911891a6d92/runtime/client_runtime.c.o: runtime/client_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexinput_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/sexinput/obj/runtime/client_runtime.c.o: build/.objects/8e204911891a6d92/runtime/client_runtime.c.o
	@mkdir -p 'build/sexinput/obj/runtime'
	ln -f 'build/.objects/8e204911891a6d92/runtime/client_runtime.c.o' "$@"

build/.objects/367aded7cdd027c6/runtime/server_runtime.c.o: runtime/server_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexinput_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_runtime.c' -o "$@"

build/sexinput/obj/runtime/server_runtime.c.o: build/.objects/367aded7cdd027c6/runtime/server_runtime.c.o
	@mkdir -p 'build/sexinput/obj/runtime'
	ln -f 'build/.objects/367aded7cdd027c6/runtime/server_runtime.c.o' "$@"

build/.objects/b1bc55baa915f5fe/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexinput_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/sexinput/obj/runtime/traps_x86_64.c.o: build/.objects/b1bc55baa915f5fe/runtime/traps_x86_64.c.o
	@mkdir -p 'build/sexinput/obj/runtime'
	ln -f 'build/.objects/b1bc55baa915f5fe/runtime/traps_x86_64.c.o' "$@"

build/.objects/733636f7ad779852/runtime/server_traps_x86_64.c.o: runtime/server_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexinput_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_traps_x86_64.c' -o "$@"

build/sexinput/obj/runtime/server_traps_x86_64.c.o: build/.objects/733636f7ad779852/runtime/server_traps_x86_64.c.o
	@mkdir -p 'build/sexinput/obj/runtime'
	ln -f 'build/.objects/733636f7ad779852/runtime/server_traps_x86_64.c.o' "$@"

build/.objects/92b82526b742e042/runtime/start_x86_64.S.o: runtime/start_x86_64.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexinput_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/sexinput/obj/runtime/start_x86_64.S.o: build/.objects/92b82526b742e042/runtime/start_x86_64.S.o
	@mkdir -p 'build/sexinput/obj/runtime'
	ln -f 'build/.objects/92b82526b742e042/runtime/start_x86_64.S.o' "$@"

build/.objects/f464c6d4274f99cb/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexinput_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/sexinput/obj/generated/runtime/format_bytes.c.o: build/.objects/f464c6d4274f99cb/generated/runtime/format_bytes.c.o
	@mkdir -p 'build/sexinput/obj/generated/runtime'
	ln -f 'build/.objects/f464c6d4274f99cb/generated/runtime/format_bytes.c.o' "$@"

build/.objects/f6f1dfeeae4fb732/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexinput_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/sexinput/obj/generated/runtime/packed_words.c.o: build/.objects/f6f1dfeeae4fb732/generated/runtime/packed_words.c.o
	@mkdir -p 'build/sexinput/obj/generated/runtime'
	ln -f 'build/.objects/f6f1dfeeae4fb732/generated/runtime/packed_words.c.o' "$@"

build/sexinput/sexinput: GNUmakefile build/sexinput/obj/src/sexinput.cesta.o build/sexinput/obj/runtime/client_runtime.c.o build/sexinput/obj/runtime/server_runtime.c.o build/sexinput/obj/runtime/traps_x86_64.c.o build/sexinput/obj/runtime/server_traps_x86_64.c.o build/sexinput/obj/runtime/start_x86_64.S.o build/sexinput/obj/generated/runtime/format_bytes.c.o build/sexinput/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/sexinput/obj/src/sexinput.cesta.o' 'build/sexinput/obj/runtime/client_runtime.c.o' 'build/sexinput/obj/runtime/server_runtime.c.o' 'build/sexinput/obj/runtime/traps_x86_64.c.o' 'build/sexinput/obj/runtime/server_traps_x86_64.c.o' 'build/sexinput/obj/runtime/start_x86_64.S.o' 'build/sexinput/obj/generated/runtime/format_bytes.c.o' 'build/sexinput/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/.objects/3da5541b3486e217/src/sexstore.cesta.o: src/sexstore.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexstore_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/sexstore.c' -o "$@"

build/sexstore/obj/src/sexstore.cesta.o: build/.objects/3da5541b3486e217/src/sexstore.cesta.o
	@mkdir -p 'build/sexstore/obj/src'
	ln -f 'build/.objects/3da5541b3486e217/src/sexstore.cesta.o' "$@"

build/.objects/29a1bf533b092787/src/sexstore_durable.cesta.o: src/sexstore_durable.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexstore_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/sexstore_durable.c' -o "$@"

build/sexstore/obj/src/sexstore_durable.cesta.o: build/.objects/29a1bf533b092787/src/sexstore_durable.cesta.o
	@mkdir -p 'build/sexstore/obj/src'
	ln -f 'build/.objects/29a1bf533b092787/src/sexstore_durable.cesta.o' "$@"

build/.objects/dfa365f626df1cec/src/sexstore_pages.cesta.o: src/sexstore_pages.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexstore_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/sexstore_pages.c' -o "$@"

build/sexstore/obj/src/sexstore_pages.cesta.o: build/.objects/dfa365f626df1cec/src/sexstore_pages.cesta.o
	@mkdir -p 'build/sexstore/obj/src'
	ln -f 'build/.objects/dfa365f626df1cec/src/sexstore_pages.cesta.o' "$@"

build/.objects/616a435a57c97fd7/runtime/client_runtime.c.o: runtime/client_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexstore_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/sexstore/obj/runtime/client_runtime.c.o: build/.objects/616a435a57c97fd7/runtime/client_runtime.c.o
	@mkdir -p 'build/sexstore/obj/runtime'
	ln -f 'build/.objects/616a435a57c97fd7/runtime/client_runtime.c.o' "$@"

build/.objects/f3bb2267bde9d17b/runtime/server_runtime.c.o: runtime/server_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexstore_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_runtime.c' -o "$@"

build/sexstore/obj/runtime/server_runtime.c.o: build/.objects/f3bb2267bde9d17b/runtime/server_runtime.c.o
	@mkdir -p 'build/sexstore/obj/runtime'
	ln -f 'build/.objects/f3bb2267bde9d17b/runtime/server_runtime.c.o' "$@"

build/.objects/a6447b1b0ca15303/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexstore_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/sexstore/obj/runtime/traps_x86_64.c.o: build/.objects/a6447b1b0ca15303/runtime/traps_x86_64.c.o
	@mkdir -p 'build/sexstore/obj/runtime'
	ln -f 'build/.objects/a6447b1b0ca15303/runtime/traps_x86_64.c.o' "$@"

build/.objects/eaeb859be8f1a397/runtime/server_traps_x86_64.c.o: runtime/server_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexstore_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_traps_x86_64.c' -o "$@"

build/sexstore/obj/runtime/server_traps_x86_64.c.o: build/.objects/eaeb859be8f1a397/runtime/server_traps_x86_64.c.o
	@mkdir -p 'build/sexstore/obj/runtime'
	ln -f 'build/.objects/eaeb859be8f1a397/runtime/server_traps_x86_64.c.o' "$@"

build/.objects/9388a08a4ba779c7/runtime/start_x86_64.S.o: runtime/start_x86_64.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexstore_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/sexstore/obj/runtime/start_x86_64.S.o: build/.objects/9388a08a4ba779c7/runtime/start_x86_64.S.o
	@mkdir -p 'build/sexstore/obj/runtime'
	ln -f 'build/.objects/9388a08a4ba779c7/runtime/start_x86_64.S.o' "$@"

build/.objects/2256bede4842d5b6/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexstore_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/sexstore/obj/generated/runtime/format_bytes.c.o: build/.objects/2256bede4842d5b6/generated/runtime/format_bytes.c.o
	@mkdir -p 'build/sexstore/obj/generated/runtime'
	ln -f 'build/.objects/2256bede4842d5b6/generated/runtime/format_bytes.c.o' "$@"

build/.objects/6ea72e92e9c9c277/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexstore_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/sexstore/obj/generated/runtime/packed_words.c.o: build/.objects/6ea72e92e9c9c277/generated/runtime/packed_words.c.o
	@mkdir -p 'build/sexstore/obj/generated/runtime'
	ln -f 'build/.objects/6ea72e92e9c9c277/generated/runtime/packed_words.c.o' "$@"

build/sexstore/sexstore: GNUmakefile build/sexstore/obj/src/sexstore.cesta.o build/sexstore/obj/src/sexstore_durable.cesta.o build/sexstore/obj/src/sexstore_pages.cesta.o build/sexstore/obj/runtime/client_runtime.c.o build/sexstore/obj/runtime/server_runtime.c.o build/sexstore/obj/runtime/traps_x86_64.c.o build/sexstore/obj/runtime/server_traps_x86_64.c.o build/sexstore/obj/runtime/start_x86_64.S.o build/sexstore/obj/generated/runtime/format_bytes.c.o build/sexstore/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/sexstore/obj/src/sexstore.cesta.o' 'build/sexstore/obj/src/sexstore_durable.cesta.o' 'build/sexstore/obj/src/sexstore_pages.cesta.o' 'build/sexstore/obj/runtime/client_runtime.c.o' 'build/sexstore/obj/runtime/server_runtime.c.o' 'build/sexstore/obj/runtime/traps_x86_64.c.o' 'build/sexstore/obj/runtime/server_traps_x86_64.c.o' 'build/sexstore/obj/runtime/start_x86_64.S.o' 'build/sexstore/obj/generated/runtime/format_bytes.c.o' 'build/sexstore/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/.objects/5a0ca417f3760ac3/src/sexdisplay.cesta.o: src/sexdisplay.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdisplay_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/sexdisplay.c' -o "$@"

build/sexdisplay/obj/src/sexdisplay.cesta.o: build/.objects/5a0ca417f3760ac3/src/sexdisplay.cesta.o
	@mkdir -p 'build/sexdisplay/obj/src'
	ln -f 'build/.objects/5a0ca417f3760ac3/src/sexdisplay.cesta.o' "$@"

build/.objects/d163714ba77f2519/src/sexdisplay_bitmap.cesta.o: src/sexdisplay_bitmap.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdisplay_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/sexdisplay_bitmap.c' -o "$@"

build/sexdisplay/obj/src/sexdisplay_bitmap.cesta.o: build/.objects/d163714ba77f2519/src/sexdisplay_bitmap.cesta.o
	@mkdir -p 'build/sexdisplay/obj/src'
	ln -f 'build/.objects/d163714ba77f2519/src/sexdisplay_bitmap.cesta.o' "$@"

build/.objects/51a0a620f55af961/src/silkbar_model.cesta.o: src/silkbar_model.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdisplay_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silkbar_model.c' -o "$@"

build/sexdisplay/obj/src/silkbar_model.cesta.o: build/.objects/51a0a620f55af961/src/silkbar_model.cesta.o
	@mkdir -p 'build/sexdisplay/obj/src'
	ln -f 'build/.objects/51a0a620f55af961/src/silkbar_model.cesta.o' "$@"

build/.objects/d4e2dae5387473b8/runtime/client_runtime.c.o: runtime/client_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdisplay_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/sexdisplay/obj/runtime/client_runtime.c.o: build/.objects/d4e2dae5387473b8/runtime/client_runtime.c.o
	@mkdir -p 'build/sexdisplay/obj/runtime'
	ln -f 'build/.objects/d4e2dae5387473b8/runtime/client_runtime.c.o' "$@"

build/.objects/f36b8bc462ecfa64/runtime/server_runtime.c.o: runtime/server_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdisplay_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_runtime.c' -o "$@"

build/sexdisplay/obj/runtime/server_runtime.c.o: build/.objects/f36b8bc462ecfa64/runtime/server_runtime.c.o
	@mkdir -p 'build/sexdisplay/obj/runtime'
	ln -f 'build/.objects/f36b8bc462ecfa64/runtime/server_runtime.c.o' "$@"

build/.objects/735c91cfb0ba6bec/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdisplay_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/sexdisplay/obj/runtime/traps_x86_64.c.o: build/.objects/735c91cfb0ba6bec/runtime/traps_x86_64.c.o
	@mkdir -p 'build/sexdisplay/obj/runtime'
	ln -f 'build/.objects/735c91cfb0ba6bec/runtime/traps_x86_64.c.o' "$@"

build/.objects/90661e54f0ff0e78/runtime/server_traps_x86_64.c.o: runtime/server_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdisplay_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_traps_x86_64.c' -o "$@"

build/sexdisplay/obj/runtime/server_traps_x86_64.c.o: build/.objects/90661e54f0ff0e78/runtime/server_traps_x86_64.c.o
	@mkdir -p 'build/sexdisplay/obj/runtime'
	ln -f 'build/.objects/90661e54f0ff0e78/runtime/server_traps_x86_64.c.o' "$@"

build/.objects/3b76cd050b006be8/runtime/start_x86_64.S.o: runtime/start_x86_64.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdisplay_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/sexdisplay/obj/runtime/start_x86_64.S.o: build/.objects/3b76cd050b006be8/runtime/start_x86_64.S.o
	@mkdir -p 'build/sexdisplay/obj/runtime'
	ln -f 'build/.objects/3b76cd050b006be8/runtime/start_x86_64.S.o' "$@"

build/.objects/8fdc6fb557af24e9/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdisplay_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/sexdisplay/obj/generated/runtime/format_bytes.c.o: build/.objects/8fdc6fb557af24e9/generated/runtime/format_bytes.c.o
	@mkdir -p 'build/sexdisplay/obj/generated/runtime'
	ln -f 'build/.objects/8fdc6fb557af24e9/generated/runtime/format_bytes.c.o' "$@"

build/.objects/29a4f676195a7f58/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdisplay_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/sexdisplay/obj/generated/runtime/packed_words.c.o: build/.objects/29a4f676195a7f58/generated/runtime/packed_words.c.o
	@mkdir -p 'build/sexdisplay/obj/generated/runtime'
	ln -f 'build/.objects/29a4f676195a7f58/generated/runtime/packed_words.c.o' "$@"

build/sexdisplay/sexdisplay: GNUmakefile build/sexdisplay/obj/src/sexdisplay.cesta.o build/sexdisplay/obj/src/sexdisplay_bitmap.cesta.o build/sexdisplay/obj/src/silkbar_model.cesta.o build/sexdisplay/obj/runtime/client_runtime.c.o build/sexdisplay/obj/runtime/server_runtime.c.o build/sexdisplay/obj/runtime/traps_x86_64.c.o build/sexdisplay/obj/runtime/server_traps_x86_64.c.o build/sexdisplay/obj/runtime/start_x86_64.S.o build/sexdisplay/obj/generated/runtime/format_bytes.c.o build/sexdisplay/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/sexdisplay/obj/src/sexdisplay.cesta.o' 'build/sexdisplay/obj/src/sexdisplay_bitmap.cesta.o' 'build/sexdisplay/obj/src/silkbar_model.cesta.o' 'build/sexdisplay/obj/runtime/client_runtime.c.o' 'build/sexdisplay/obj/runtime/server_runtime.c.o' 'build/sexdisplay/obj/runtime/traps_x86_64.c.o' 'build/sexdisplay/obj/runtime/server_traps_x86_64.c.o' 'build/sexdisplay/obj/runtime/start_x86_64.S.o' 'build/sexdisplay/obj/generated/runtime/format_bytes.c.o' 'build/sexdisplay/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/.objects/cbfa40cf4adae574/src/sexdisplay_bitmap.cesta.o: src/sexdisplay_bitmap.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/sexdisplay_bitmap.c' -o "$@"

build/sexdisplay-bitmap/obj/src/sexdisplay_bitmap.cesta.o: build/.objects/cbfa40cf4adae574/src/sexdisplay_bitmap.cesta.o
	@mkdir -p 'build/sexdisplay-bitmap/obj/src'
	ln -f 'build/.objects/cbfa40cf4adae574/src/sexdisplay_bitmap.cesta.o' "$@"

build/sexdisplay-bitmap/libsexdisplay-bitmap.a: GNUmakefile build/sexdisplay-bitmap/obj/src/sexdisplay_bitmap.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/sexdisplay-bitmap/obj/src/sexdisplay_bitmap.cesta.o'

build/.objects/c32e109efccfdf91/src/sexbell.cesta.o: src/sexbell.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexbell_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/sexbell.c' -o "$@"

build/sexbell/obj/src/sexbell.cesta.o: build/.objects/c32e109efccfdf91/src/sexbell.cesta.o
	@mkdir -p 'build/sexbell/obj/src'
	ln -f 'build/.objects/c32e109efccfdf91/src/sexbell.cesta.o' "$@"

build/.objects/e590269888e2aed5/runtime/client_runtime.c.o: runtime/client_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexbell_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/sexbell/obj/runtime/client_runtime.c.o: build/.objects/e590269888e2aed5/runtime/client_runtime.c.o
	@mkdir -p 'build/sexbell/obj/runtime'
	ln -f 'build/.objects/e590269888e2aed5/runtime/client_runtime.c.o' "$@"

build/.objects/3c786e56085eb211/runtime/server_runtime.c.o: runtime/server_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexbell_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_runtime.c' -o "$@"

build/sexbell/obj/runtime/server_runtime.c.o: build/.objects/3c786e56085eb211/runtime/server_runtime.c.o
	@mkdir -p 'build/sexbell/obj/runtime'
	ln -f 'build/.objects/3c786e56085eb211/runtime/server_runtime.c.o' "$@"

build/.objects/19d1d0dc6c4f3179/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexbell_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/sexbell/obj/runtime/traps_x86_64.c.o: build/.objects/19d1d0dc6c4f3179/runtime/traps_x86_64.c.o
	@mkdir -p 'build/sexbell/obj/runtime'
	ln -f 'build/.objects/19d1d0dc6c4f3179/runtime/traps_x86_64.c.o' "$@"

build/.objects/b75bfcf4e4ee0495/runtime/server_traps_x86_64.c.o: runtime/server_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexbell_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_traps_x86_64.c' -o "$@"

build/sexbell/obj/runtime/server_traps_x86_64.c.o: build/.objects/b75bfcf4e4ee0495/runtime/server_traps_x86_64.c.o
	@mkdir -p 'build/sexbell/obj/runtime'
	ln -f 'build/.objects/b75bfcf4e4ee0495/runtime/server_traps_x86_64.c.o' "$@"

build/.objects/30e9ad6134f44bc5/runtime/start_x86_64.S.o: runtime/start_x86_64.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexbell_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/sexbell/obj/runtime/start_x86_64.S.o: build/.objects/30e9ad6134f44bc5/runtime/start_x86_64.S.o
	@mkdir -p 'build/sexbell/obj/runtime'
	ln -f 'build/.objects/30e9ad6134f44bc5/runtime/start_x86_64.S.o' "$@"

build/.objects/e04ea3d5602080ba/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexbell_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/sexbell/obj/generated/runtime/format_bytes.c.o: build/.objects/e04ea3d5602080ba/generated/runtime/format_bytes.c.o
	@mkdir -p 'build/sexbell/obj/generated/runtime'
	ln -f 'build/.objects/e04ea3d5602080ba/generated/runtime/format_bytes.c.o' "$@"

build/.objects/dfbceadf90464d75/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexbell_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/sexbell/obj/generated/runtime/packed_words.c.o: build/.objects/dfbceadf90464d75/generated/runtime/packed_words.c.o
	@mkdir -p 'build/sexbell/obj/generated/runtime'
	ln -f 'build/.objects/dfbceadf90464d75/generated/runtime/packed_words.c.o' "$@"

build/sexbell/sexbell: GNUmakefile build/sexbell/obj/src/sexbell.cesta.o build/sexbell/obj/runtime/client_runtime.c.o build/sexbell/obj/runtime/server_runtime.c.o build/sexbell/obj/runtime/traps_x86_64.c.o build/sexbell/obj/runtime/server_traps_x86_64.c.o build/sexbell/obj/runtime/start_x86_64.S.o build/sexbell/obj/generated/runtime/format_bytes.c.o build/sexbell/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/sexbell/obj/src/sexbell.cesta.o' 'build/sexbell/obj/runtime/client_runtime.c.o' 'build/sexbell/obj/runtime/server_runtime.c.o' 'build/sexbell/obj/runtime/traps_x86_64.c.o' 'build/sexbell/obj/runtime/server_traps_x86_64.c.o' 'build/sexbell/obj/runtime/start_x86_64.S.o' 'build/sexbell/obj/generated/runtime/format_bytes.c.o' 'build/sexbell/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/.objects/b54de591e04b8483/src/linen.cesta.o: src/linen.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=linen_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/linen.c' -o "$@"

build/linen/obj/src/linen.cesta.o: build/.objects/b54de591e04b8483/src/linen.cesta.o
	@mkdir -p 'build/linen/obj/src'
	ln -f 'build/.objects/b54de591e04b8483/src/linen.cesta.o' "$@"

build/.objects/f2deeab1d24f3148/src/linen_capture.cesta.o: src/linen_capture.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=linen_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/linen_capture.c' -o "$@"

build/linen/obj/src/linen_capture.cesta.o: build/.objects/f2deeab1d24f3148/src/linen_capture.cesta.o
	@mkdir -p 'build/linen/obj/src'
	ln -f 'build/.objects/f2deeab1d24f3148/src/linen_capture.cesta.o' "$@"

build/.objects/35aeca1326bc696c/src/linen_capture_service.cesta.o: src/linen_capture_service.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=linen_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/linen_capture_service.c' -o "$@"

build/linen/obj/src/linen_capture_service.cesta.o: build/.objects/35aeca1326bc696c/src/linen_capture_service.cesta.o
	@mkdir -p 'build/linen/obj/src'
	ln -f 'build/.objects/35aeca1326bc696c/src/linen_capture_service.cesta.o' "$@"

build/.objects/5f82a4e6a0f85973/src/rpc_runtime.cesta.o: src/rpc_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=linen_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/rpc_runtime.c' -o "$@"

build/linen/obj/src/rpc_runtime.cesta.o: build/.objects/5f82a4e6a0f85973/src/rpc_runtime.cesta.o
	@mkdir -p 'build/linen/obj/src'
	ln -f 'build/.objects/5f82a4e6a0f85973/src/rpc_runtime.cesta.o' "$@"

build/.objects/861cde55e9a62468/src/rpc_wait.cesta.o: src/rpc_wait.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=linen_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/rpc_wait.c' -o "$@"

build/linen/obj/src/rpc_wait.cesta.o: build/.objects/861cde55e9a62468/src/rpc_wait.cesta.o
	@mkdir -p 'build/linen/obj/src'
	ln -f 'build/.objects/861cde55e9a62468/src/rpc_wait.cesta.o' "$@"

build/.objects/9e77e25e23e6f0d8/runtime/client_runtime.c.o: runtime/client_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=linen_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/linen/obj/runtime/client_runtime.c.o: build/.objects/9e77e25e23e6f0d8/runtime/client_runtime.c.o
	@mkdir -p 'build/linen/obj/runtime'
	ln -f 'build/.objects/9e77e25e23e6f0d8/runtime/client_runtime.c.o' "$@"

build/.objects/bf205fd1161e0994/runtime/server_runtime.c.o: runtime/server_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=linen_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_runtime.c' -o "$@"

build/linen/obj/runtime/server_runtime.c.o: build/.objects/bf205fd1161e0994/runtime/server_runtime.c.o
	@mkdir -p 'build/linen/obj/runtime'
	ln -f 'build/.objects/bf205fd1161e0994/runtime/server_runtime.c.o' "$@"

build/.objects/64ea149318e130cc/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=linen_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/linen/obj/runtime/traps_x86_64.c.o: build/.objects/64ea149318e130cc/runtime/traps_x86_64.c.o
	@mkdir -p 'build/linen/obj/runtime'
	ln -f 'build/.objects/64ea149318e130cc/runtime/traps_x86_64.c.o' "$@"

build/.objects/e2ef32fe3aa90c98/runtime/server_traps_x86_64.c.o: runtime/server_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=linen_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_traps_x86_64.c' -o "$@"

build/linen/obj/runtime/server_traps_x86_64.c.o: build/.objects/e2ef32fe3aa90c98/runtime/server_traps_x86_64.c.o
	@mkdir -p 'build/linen/obj/runtime'
	ln -f 'build/.objects/e2ef32fe3aa90c98/runtime/server_traps_x86_64.c.o' "$@"

build/.objects/ceaa54f05ca06720/runtime/rpc_traps_x86_64.c.o: runtime/rpc_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=linen_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/rpc_traps_x86_64.c' -o "$@"

build/linen/obj/runtime/rpc_traps_x86_64.c.o: build/.objects/ceaa54f05ca06720/runtime/rpc_traps_x86_64.c.o
	@mkdir -p 'build/linen/obj/runtime'
	ln -f 'build/.objects/ceaa54f05ca06720/runtime/rpc_traps_x86_64.c.o' "$@"

build/.objects/a7e6e981437c9708/runtime/start_x86_64.S.o: runtime/start_x86_64.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=linen_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/linen/obj/runtime/start_x86_64.S.o: build/.objects/a7e6e981437c9708/runtime/start_x86_64.S.o
	@mkdir -p 'build/linen/obj/runtime'
	ln -f 'build/.objects/a7e6e981437c9708/runtime/start_x86_64.S.o' "$@"

build/.objects/2e907a8b132a7513/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=linen_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/linen/obj/generated/runtime/format_bytes.c.o: build/.objects/2e907a8b132a7513/generated/runtime/format_bytes.c.o
	@mkdir -p 'build/linen/obj/generated/runtime'
	ln -f 'build/.objects/2e907a8b132a7513/generated/runtime/format_bytes.c.o' "$@"

build/.objects/8db73e788b389278/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=linen_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/linen/obj/generated/runtime/packed_words.c.o: build/.objects/8db73e788b389278/generated/runtime/packed_words.c.o
	@mkdir -p 'build/linen/obj/generated/runtime'
	ln -f 'build/.objects/8db73e788b389278/generated/runtime/packed_words.c.o' "$@"

build/linen/linen: GNUmakefile build/linen/obj/src/linen.cesta.o build/linen/obj/src/linen_capture.cesta.o build/linen/obj/src/linen_capture_service.cesta.o build/linen/obj/src/rpc_runtime.cesta.o build/linen/obj/src/rpc_wait.cesta.o build/linen/obj/runtime/client_runtime.c.o build/linen/obj/runtime/server_runtime.c.o build/linen/obj/runtime/traps_x86_64.c.o build/linen/obj/runtime/server_traps_x86_64.c.o build/linen/obj/runtime/rpc_traps_x86_64.c.o build/linen/obj/runtime/start_x86_64.S.o build/linen/obj/generated/runtime/format_bytes.c.o build/linen/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/linen/obj/src/linen.cesta.o' 'build/linen/obj/src/linen_capture.cesta.o' 'build/linen/obj/src/linen_capture_service.cesta.o' 'build/linen/obj/src/rpc_runtime.cesta.o' 'build/linen/obj/src/rpc_wait.cesta.o' 'build/linen/obj/runtime/client_runtime.c.o' 'build/linen/obj/runtime/server_runtime.c.o' 'build/linen/obj/runtime/traps_x86_64.c.o' 'build/linen/obj/runtime/server_traps_x86_64.c.o' 'build/linen/obj/runtime/rpc_traps_x86_64.c.o' 'build/linen/obj/runtime/start_x86_64.S.o' 'build/linen/obj/generated/runtime/format_bytes.c.o' 'build/linen/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/.objects/e6e743d2e51d98c1/src/quil.cesta.o: src/quil.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=quil_main' '-Dsexfiles_quil_persistence_proof' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/quil.c' -o "$@"

build/quil/obj/src/quil.cesta.o: build/.objects/e6e743d2e51d98c1/src/quil.cesta.o
	@mkdir -p 'build/quil/obj/src'
	ln -f 'build/.objects/e6e743d2e51d98c1/src/quil.cesta.o' "$@"

build/.objects/967b854a771372e4/src/quil_autosave.cesta.o: src/quil_autosave.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=quil_main' '-Dsexfiles_quil_persistence_proof' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/quil_autosave.c' -o "$@"

build/quil/obj/src/quil_autosave.cesta.o: build/.objects/967b854a771372e4/src/quil_autosave.cesta.o
	@mkdir -p 'build/quil/obj/src'
	ln -f 'build/.objects/967b854a771372e4/src/quil_autosave.cesta.o' "$@"

build/.objects/4af8542b27172520/src/document_protocol.cesta.o: src/document_protocol.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=quil_main' '-Dsexfiles_quil_persistence_proof' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/document_protocol.c' -o "$@"

build/quil/obj/src/document_protocol.cesta.o: build/.objects/4af8542b27172520/src/document_protocol.cesta.o
	@mkdir -p 'build/quil/obj/src'
	ln -f 'build/.objects/4af8542b27172520/src/document_protocol.cesta.o' "$@"

build/.objects/f6736279b51b0377/src/document_client.cesta.o: src/document_client.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=quil_main' '-Dsexfiles_quil_persistence_proof' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/document_client.c' -o "$@"

build/quil/obj/src/document_client.cesta.o: build/.objects/f6736279b51b0377/src/document_client.cesta.o
	@mkdir -p 'build/quil/obj/src'
	ln -f 'build/.objects/f6736279b51b0377/src/document_client.cesta.o' "$@"

build/.objects/7d9a3f61c646846d/src/document_client_transport.cesta.o: src/document_client_transport.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=quil_main' '-Dsexfiles_quil_persistence_proof' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/document_client_transport.c' -o "$@"

build/quil/obj/src/document_client_transport.cesta.o: build/.objects/7d9a3f61c646846d/src/document_client_transport.cesta.o
	@mkdir -p 'build/quil/obj/src'
	ln -f 'build/.objects/7d9a3f61c646846d/src/document_client_transport.cesta.o' "$@"

build/.objects/a3633f27b5a48530/src/document_snapshot.cesta.o: src/document_snapshot.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=quil_main' '-Dsexfiles_quil_persistence_proof' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/document_snapshot.c' -o "$@"

build/quil/obj/src/document_snapshot.cesta.o: build/.objects/a3633f27b5a48530/src/document_snapshot.cesta.o
	@mkdir -p 'build/quil/obj/src'
	ln -f 'build/.objects/a3633f27b5a48530/src/document_snapshot.cesta.o' "$@"

build/.objects/a67c74ccb841b23e/src/rpc_runtime.cesta.o: src/rpc_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=quil_main' '-Dsexfiles_quil_persistence_proof' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/rpc_runtime.c' -o "$@"

build/quil/obj/src/rpc_runtime.cesta.o: build/.objects/a67c74ccb841b23e/src/rpc_runtime.cesta.o
	@mkdir -p 'build/quil/obj/src'
	ln -f 'build/.objects/a67c74ccb841b23e/src/rpc_runtime.cesta.o' "$@"

build/.objects/ed8587918fa4593f/src/rpc_wait.cesta.o: src/rpc_wait.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=quil_main' '-Dsexfiles_quil_persistence_proof' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/rpc_wait.c' -o "$@"

build/quil/obj/src/rpc_wait.cesta.o: build/.objects/ed8587918fa4593f/src/rpc_wait.cesta.o
	@mkdir -p 'build/quil/obj/src'
	ln -f 'build/.objects/ed8587918fa4593f/src/rpc_wait.cesta.o' "$@"

build/.objects/567cdb011585538f/runtime/client_runtime.c.o: runtime/client_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=quil_main' '-Dsexfiles_quil_persistence_proof' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/quil/obj/runtime/client_runtime.c.o: build/.objects/567cdb011585538f/runtime/client_runtime.c.o
	@mkdir -p 'build/quil/obj/runtime'
	ln -f 'build/.objects/567cdb011585538f/runtime/client_runtime.c.o' "$@"

build/.objects/c372a10d7926abc3/runtime/server_runtime.c.o: runtime/server_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=quil_main' '-Dsexfiles_quil_persistence_proof' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_runtime.c' -o "$@"

build/quil/obj/runtime/server_runtime.c.o: build/.objects/c372a10d7926abc3/runtime/server_runtime.c.o
	@mkdir -p 'build/quil/obj/runtime'
	ln -f 'build/.objects/c372a10d7926abc3/runtime/server_runtime.c.o' "$@"

build/.objects/b2b435410352384b/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=quil_main' '-Dsexfiles_quil_persistence_proof' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/quil/obj/runtime/traps_x86_64.c.o: build/.objects/b2b435410352384b/runtime/traps_x86_64.c.o
	@mkdir -p 'build/quil/obj/runtime'
	ln -f 'build/.objects/b2b435410352384b/runtime/traps_x86_64.c.o' "$@"

build/.objects/919b8a939ffe92cf/runtime/server_traps_x86_64.c.o: runtime/server_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=quil_main' '-Dsexfiles_quil_persistence_proof' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_traps_x86_64.c' -o "$@"

build/quil/obj/runtime/server_traps_x86_64.c.o: build/.objects/919b8a939ffe92cf/runtime/server_traps_x86_64.c.o
	@mkdir -p 'build/quil/obj/runtime'
	ln -f 'build/.objects/919b8a939ffe92cf/runtime/server_traps_x86_64.c.o' "$@"

build/.objects/df7b10a9764042a7/runtime/rpc_traps_x86_64.c.o: runtime/rpc_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=quil_main' '-Dsexfiles_quil_persistence_proof' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/rpc_traps_x86_64.c' -o "$@"

build/quil/obj/runtime/rpc_traps_x86_64.c.o: build/.objects/df7b10a9764042a7/runtime/rpc_traps_x86_64.c.o
	@mkdir -p 'build/quil/obj/runtime'
	ln -f 'build/.objects/df7b10a9764042a7/runtime/rpc_traps_x86_64.c.o' "$@"

build/.objects/872afc0b3e76c564/runtime/region_writer_traps_x86_64.c.o: runtime/region_writer_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=quil_main' '-Dsexfiles_quil_persistence_proof' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/region_writer_traps_x86_64.c' -o "$@"

build/quil/obj/runtime/region_writer_traps_x86_64.c.o: build/.objects/872afc0b3e76c564/runtime/region_writer_traps_x86_64.c.o
	@mkdir -p 'build/quil/obj/runtime'
	ln -f 'build/.objects/872afc0b3e76c564/runtime/region_writer_traps_x86_64.c.o' "$@"

build/.objects/9846a42140dda85f/runtime/start_x86_64.S.o: runtime/start_x86_64.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=quil_main' '-Dsexfiles_quil_persistence_proof' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/quil/obj/runtime/start_x86_64.S.o: build/.objects/9846a42140dda85f/runtime/start_x86_64.S.o
	@mkdir -p 'build/quil/obj/runtime'
	ln -f 'build/.objects/9846a42140dda85f/runtime/start_x86_64.S.o' "$@"

build/.objects/57549f378afff75e/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=quil_main' '-Dsexfiles_quil_persistence_proof' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/quil/obj/generated/runtime/format_bytes.c.o: build/.objects/57549f378afff75e/generated/runtime/format_bytes.c.o
	@mkdir -p 'build/quil/obj/generated/runtime'
	ln -f 'build/.objects/57549f378afff75e/generated/runtime/format_bytes.c.o' "$@"

build/.objects/ca90890dd233acaf/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=quil_main' '-Dsexfiles_quil_persistence_proof' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/quil/obj/generated/runtime/packed_words.c.o: build/.objects/ca90890dd233acaf/generated/runtime/packed_words.c.o
	@mkdir -p 'build/quil/obj/generated/runtime'
	ln -f 'build/.objects/ca90890dd233acaf/generated/runtime/packed_words.c.o' "$@"

build/quil/quil: GNUmakefile build/quil/obj/src/quil.cesta.o build/quil/obj/src/quil_autosave.cesta.o build/quil/obj/src/document_protocol.cesta.o build/quil/obj/src/document_client.cesta.o build/quil/obj/src/document_client_transport.cesta.o build/quil/obj/src/document_snapshot.cesta.o build/quil/obj/src/rpc_runtime.cesta.o build/quil/obj/src/rpc_wait.cesta.o build/quil/obj/runtime/client_runtime.c.o build/quil/obj/runtime/server_runtime.c.o build/quil/obj/runtime/traps_x86_64.c.o build/quil/obj/runtime/server_traps_x86_64.c.o build/quil/obj/runtime/rpc_traps_x86_64.c.o build/quil/obj/runtime/region_writer_traps_x86_64.c.o build/quil/obj/runtime/start_x86_64.S.o build/quil/obj/generated/runtime/format_bytes.c.o build/quil/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/quil/obj/src/quil.cesta.o' 'build/quil/obj/src/quil_autosave.cesta.o' 'build/quil/obj/src/document_protocol.cesta.o' 'build/quil/obj/src/document_client.cesta.o' 'build/quil/obj/src/document_client_transport.cesta.o' 'build/quil/obj/src/document_snapshot.cesta.o' 'build/quil/obj/src/rpc_runtime.cesta.o' 'build/quil/obj/src/rpc_wait.cesta.o' 'build/quil/obj/runtime/client_runtime.c.o' 'build/quil/obj/runtime/server_runtime.c.o' 'build/quil/obj/runtime/traps_x86_64.c.o' 'build/quil/obj/runtime/server_traps_x86_64.c.o' 'build/quil/obj/runtime/rpc_traps_x86_64.c.o' 'build/quil/obj/runtime/region_writer_traps_x86_64.c.o' 'build/quil/obj/runtime/start_x86_64.S.o' 'build/quil/obj/generated/runtime/format_bytes.c.o' 'build/quil/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/.objects/8d61c75b319d78ae/src/quil_autosave.cesta.o: src/quil_autosave.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/quil_autosave.c' -o "$@"

build/quil-autosave/obj/src/quil_autosave.cesta.o: build/.objects/8d61c75b319d78ae/src/quil_autosave.cesta.o
	@mkdir -p 'build/quil-autosave/obj/src'
	ln -f 'build/.objects/8d61c75b319d78ae/src/quil_autosave.cesta.o' "$@"

build/quil-autosave/libquil-autosave.a: GNUmakefile build/quil-autosave/obj/src/quil_autosave.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/quil-autosave/obj/src/quil_autosave.cesta.o'

build/.objects/7fcbc0ca2406e722/src/document_protocol.cesta.o: src/document_protocol.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/document_protocol.c' -o "$@"

build/document-protocol/obj/src/document_protocol.cesta.o: build/.objects/7fcbc0ca2406e722/src/document_protocol.cesta.o
	@mkdir -p 'build/document-protocol/obj/src'
	ln -f 'build/.objects/7fcbc0ca2406e722/src/document_protocol.cesta.o' "$@"

build/.objects/69833e4346244ca7/src/document_client.cesta.o: src/document_client.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/document_client.c' -o "$@"

build/document-protocol/obj/src/document_client.cesta.o: build/.objects/69833e4346244ca7/src/document_client.cesta.o
	@mkdir -p 'build/document-protocol/obj/src'
	ln -f 'build/.objects/69833e4346244ca7/src/document_client.cesta.o' "$@"

build/.objects/6e57d784b623fd71/src/document_client_transport.cesta.o: src/document_client_transport.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/document_client_transport.c' -o "$@"

build/document-protocol/obj/src/document_client_transport.cesta.o: build/.objects/6e57d784b623fd71/src/document_client_transport.cesta.o
	@mkdir -p 'build/document-protocol/obj/src'
	ln -f 'build/.objects/6e57d784b623fd71/src/document_client_transport.cesta.o' "$@"

build/.objects/c8bd7e292e644bfb/src/document_service.cesta.o: src/document_service.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/document_service.c' -o "$@"

build/document-protocol/obj/src/document_service.cesta.o: build/.objects/c8bd7e292e644bfb/src/document_service.cesta.o
	@mkdir -p 'build/document-protocol/obj/src'
	ln -f 'build/.objects/c8bd7e292e644bfb/src/document_service.cesta.o' "$@"

build/.objects/933a7f8e0b3d2b47/src/document_journal.cesta.o: src/document_journal.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/document_journal.c' -o "$@"

build/document-protocol/obj/src/document_journal.cesta.o: build/.objects/933a7f8e0b3d2b47/src/document_journal.cesta.o
	@mkdir -p 'build/document-protocol/obj/src'
	ln -f 'build/.objects/933a7f8e0b3d2b47/src/document_journal.cesta.o' "$@"

build/.objects/93eaae1ca9c1c847/src/document_growth.cesta.o: src/document_growth.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/document_growth.c' -o "$@"

build/document-protocol/obj/src/document_growth.cesta.o: build/.objects/93eaae1ca9c1c847/src/document_growth.cesta.o
	@mkdir -p 'build/document-protocol/obj/src'
	ln -f 'build/.objects/93eaae1ca9c1c847/src/document_growth.cesta.o' "$@"

build/.objects/c5c998aa5253694d/src/document_service_transport.cesta.o: src/document_service_transport.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/document_service_transport.c' -o "$@"

build/document-protocol/obj/src/document_service_transport.cesta.o: build/.objects/c5c998aa5253694d/src/document_service_transport.cesta.o
	@mkdir -p 'build/document-protocol/obj/src'
	ln -f 'build/.objects/c5c998aa5253694d/src/document_service_transport.cesta.o' "$@"

build/.objects/e2ae61b39ea3c2f2/src/document_snapshot.cesta.o: src/document_snapshot.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/document_snapshot.c' -o "$@"

build/document-protocol/obj/src/document_snapshot.cesta.o: build/.objects/e2ae61b39ea3c2f2/src/document_snapshot.cesta.o
	@mkdir -p 'build/document-protocol/obj/src'
	ln -f 'build/.objects/e2ae61b39ea3c2f2/src/document_snapshot.cesta.o' "$@"

build/document-protocol/libdocument-protocol.a: GNUmakefile build/document-protocol/obj/src/document_protocol.cesta.o build/document-protocol/obj/src/document_client.cesta.o build/document-protocol/obj/src/document_client_transport.cesta.o build/document-protocol/obj/src/document_service.cesta.o build/document-protocol/obj/src/document_journal.cesta.o build/document-protocol/obj/src/document_growth.cesta.o build/document-protocol/obj/src/document_service_transport.cesta.o build/document-protocol/obj/src/document_snapshot.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/document-protocol/obj/src/document_protocol.cesta.o' 'build/document-protocol/obj/src/document_client.cesta.o' 'build/document-protocol/obj/src/document_client_transport.cesta.o' 'build/document-protocol/obj/src/document_service.cesta.o' 'build/document-protocol/obj/src/document_journal.cesta.o' 'build/document-protocol/obj/src/document_growth.cesta.o' 'build/document-protocol/obj/src/document_service_transport.cesta.o' 'build/document-protocol/obj/src/document_snapshot.cesta.o'

build/.objects/1da44c0c23c00b90/src/document_protocol_probe.cesta.o: src/document_protocol_probe.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=document_protocol_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/document_protocol_probe.c' -o "$@"

build/document-protocol-probe/obj/src/document_protocol_probe.cesta.o: build/.objects/1da44c0c23c00b90/src/document_protocol_probe.cesta.o
	@mkdir -p 'build/document-protocol-probe/obj/src'
	ln -f 'build/.objects/1da44c0c23c00b90/src/document_protocol_probe.cesta.o' "$@"

build/.objects/4941a6e194a218c3/src/document_protocol.cesta.o: src/document_protocol.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=document_protocol_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/document_protocol.c' -o "$@"

build/document-protocol-probe/obj/src/document_protocol.cesta.o: build/.objects/4941a6e194a218c3/src/document_protocol.cesta.o
	@mkdir -p 'build/document-protocol-probe/obj/src'
	ln -f 'build/.objects/4941a6e194a218c3/src/document_protocol.cesta.o' "$@"

build/.objects/6c2b96870a90f612/src/document_client.cesta.o: src/document_client.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=document_protocol_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/document_client.c' -o "$@"

build/document-protocol-probe/obj/src/document_client.cesta.o: build/.objects/6c2b96870a90f612/src/document_client.cesta.o
	@mkdir -p 'build/document-protocol-probe/obj/src'
	ln -f 'build/.objects/6c2b96870a90f612/src/document_client.cesta.o' "$@"

build/.objects/fb5a930cc96e8a28/src/document_client_transport.cesta.o: src/document_client_transport.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=document_protocol_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/document_client_transport.c' -o "$@"

build/document-protocol-probe/obj/src/document_client_transport.cesta.o: build/.objects/fb5a930cc96e8a28/src/document_client_transport.cesta.o
	@mkdir -p 'build/document-protocol-probe/obj/src'
	ln -f 'build/.objects/fb5a930cc96e8a28/src/document_client_transport.cesta.o' "$@"

build/.objects/b733e1665e827bb3/src/document_snapshot.cesta.o: src/document_snapshot.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=document_protocol_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/document_snapshot.c' -o "$@"

build/document-protocol-probe/obj/src/document_snapshot.cesta.o: build/.objects/b733e1665e827bb3/src/document_snapshot.cesta.o
	@mkdir -p 'build/document-protocol-probe/obj/src'
	ln -f 'build/.objects/b733e1665e827bb3/src/document_snapshot.cesta.o' "$@"

build/.objects/7c41d7cfa87d853a/src/rpc_wait.cesta.o: src/rpc_wait.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=document_protocol_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/rpc_wait.c' -o "$@"

build/document-protocol-probe/obj/src/rpc_wait.cesta.o: build/.objects/7c41d7cfa87d853a/src/rpc_wait.cesta.o
	@mkdir -p 'build/document-protocol-probe/obj/src'
	ln -f 'build/.objects/7c41d7cfa87d853a/src/rpc_wait.cesta.o' "$@"

build/.objects/d1d0f9eb1b544bea/runtime/client_runtime.c.o: runtime/client_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=document_protocol_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/document-protocol-probe/obj/runtime/client_runtime.c.o: build/.objects/d1d0f9eb1b544bea/runtime/client_runtime.c.o
	@mkdir -p 'build/document-protocol-probe/obj/runtime'
	ln -f 'build/.objects/d1d0f9eb1b544bea/runtime/client_runtime.c.o' "$@"

build/.objects/40a447f3259e3f56/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=document_protocol_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/document-protocol-probe/obj/runtime/traps_x86_64.c.o: build/.objects/40a447f3259e3f56/runtime/traps_x86_64.c.o
	@mkdir -p 'build/document-protocol-probe/obj/runtime'
	ln -f 'build/.objects/40a447f3259e3f56/runtime/traps_x86_64.c.o' "$@"

build/.objects/08e77be7fa965362/runtime/rpc_traps_x86_64.c.o: runtime/rpc_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=document_protocol_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/rpc_traps_x86_64.c' -o "$@"

build/document-protocol-probe/obj/runtime/rpc_traps_x86_64.c.o: build/.objects/08e77be7fa965362/runtime/rpc_traps_x86_64.c.o
	@mkdir -p 'build/document-protocol-probe/obj/runtime'
	ln -f 'build/.objects/08e77be7fa965362/runtime/rpc_traps_x86_64.c.o' "$@"

build/.objects/2635fe4b77574e67/runtime/region_writer_traps_x86_64.c.o: runtime/region_writer_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=document_protocol_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/region_writer_traps_x86_64.c' -o "$@"

build/document-protocol-probe/obj/runtime/region_writer_traps_x86_64.c.o: build/.objects/2635fe4b77574e67/runtime/region_writer_traps_x86_64.c.o
	@mkdir -p 'build/document-protocol-probe/obj/runtime'
	ln -f 'build/.objects/2635fe4b77574e67/runtime/region_writer_traps_x86_64.c.o' "$@"

build/.objects/c834544658f5d3da/runtime/start_x86_64.S.o: runtime/start_x86_64.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=document_protocol_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/document-protocol-probe/obj/runtime/start_x86_64.S.o: build/.objects/c834544658f5d3da/runtime/start_x86_64.S.o
	@mkdir -p 'build/document-protocol-probe/obj/runtime'
	ln -f 'build/.objects/c834544658f5d3da/runtime/start_x86_64.S.o' "$@"

build/.objects/63cf55c2f3f07271/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=document_protocol_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/document-protocol-probe/obj/generated/runtime/format_bytes.c.o: build/.objects/63cf55c2f3f07271/generated/runtime/format_bytes.c.o
	@mkdir -p 'build/document-protocol-probe/obj/generated/runtime'
	ln -f 'build/.objects/63cf55c2f3f07271/generated/runtime/format_bytes.c.o' "$@"

build/.objects/6d833c8edacc9e8a/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=document_protocol_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/document-protocol-probe/obj/generated/runtime/packed_words.c.o: build/.objects/6d833c8edacc9e8a/generated/runtime/packed_words.c.o
	@mkdir -p 'build/document-protocol-probe/obj/generated/runtime'
	ln -f 'build/.objects/6d833c8edacc9e8a/generated/runtime/packed_words.c.o' "$@"

build/document-protocol-probe/document-protocol-probe: GNUmakefile build/document-protocol-probe/obj/src/document_protocol_probe.cesta.o build/document-protocol-probe/obj/src/document_protocol.cesta.o build/document-protocol-probe/obj/src/document_client.cesta.o build/document-protocol-probe/obj/src/document_client_transport.cesta.o build/document-protocol-probe/obj/src/document_snapshot.cesta.o build/document-protocol-probe/obj/src/rpc_wait.cesta.o build/document-protocol-probe/obj/runtime/client_runtime.c.o build/document-protocol-probe/obj/runtime/traps_x86_64.c.o build/document-protocol-probe/obj/runtime/rpc_traps_x86_64.c.o build/document-protocol-probe/obj/runtime/region_writer_traps_x86_64.c.o build/document-protocol-probe/obj/runtime/start_x86_64.S.o build/document-protocol-probe/obj/generated/runtime/format_bytes.c.o build/document-protocol-probe/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/document-protocol-probe/obj/src/document_protocol_probe.cesta.o' 'build/document-protocol-probe/obj/src/document_protocol.cesta.o' 'build/document-protocol-probe/obj/src/document_client.cesta.o' 'build/document-protocol-probe/obj/src/document_client_transport.cesta.o' 'build/document-protocol-probe/obj/src/document_snapshot.cesta.o' 'build/document-protocol-probe/obj/src/rpc_wait.cesta.o' 'build/document-protocol-probe/obj/runtime/client_runtime.c.o' 'build/document-protocol-probe/obj/runtime/traps_x86_64.c.o' 'build/document-protocol-probe/obj/runtime/rpc_traps_x86_64.c.o' 'build/document-protocol-probe/obj/runtime/region_writer_traps_x86_64.c.o' 'build/document-protocol-probe/obj/runtime/start_x86_64.S.o' 'build/document-protocol-probe/obj/generated/runtime/format_bytes.c.o' 'build/document-protocol-probe/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/.objects/d785b029746545d1/src/document_protocol_probe.cesta.o: src/document_protocol_probe.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=document_protocol_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DDOCUMENT_PROTOCOL_PROBE_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/document_protocol_probe.c' -o "$@"

build/document-protocol-probe-save/obj/src/document_protocol_probe.cesta.o: build/.objects/d785b029746545d1/src/document_protocol_probe.cesta.o
	@mkdir -p 'build/document-protocol-probe-save/obj/src'
	ln -f 'build/.objects/d785b029746545d1/src/document_protocol_probe.cesta.o' "$@"

build/.objects/7dec28beb62b6864/src/document_protocol.cesta.o: src/document_protocol.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=document_protocol_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DDOCUMENT_PROTOCOL_PROBE_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/document_protocol.c' -o "$@"

build/document-protocol-probe-save/obj/src/document_protocol.cesta.o: build/.objects/7dec28beb62b6864/src/document_protocol.cesta.o
	@mkdir -p 'build/document-protocol-probe-save/obj/src'
	ln -f 'build/.objects/7dec28beb62b6864/src/document_protocol.cesta.o' "$@"

build/.objects/e398c10d6c59ec97/src/document_client.cesta.o: src/document_client.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=document_protocol_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DDOCUMENT_PROTOCOL_PROBE_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/document_client.c' -o "$@"

build/document-protocol-probe-save/obj/src/document_client.cesta.o: build/.objects/e398c10d6c59ec97/src/document_client.cesta.o
	@mkdir -p 'build/document-protocol-probe-save/obj/src'
	ln -f 'build/.objects/e398c10d6c59ec97/src/document_client.cesta.o' "$@"

build/.objects/1be682a7a1074799/src/document_client_transport.cesta.o: src/document_client_transport.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=document_protocol_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DDOCUMENT_PROTOCOL_PROBE_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/document_client_transport.c' -o "$@"

build/document-protocol-probe-save/obj/src/document_client_transport.cesta.o: build/.objects/1be682a7a1074799/src/document_client_transport.cesta.o
	@mkdir -p 'build/document-protocol-probe-save/obj/src'
	ln -f 'build/.objects/1be682a7a1074799/src/document_client_transport.cesta.o' "$@"

build/.objects/11139b4a9cbb0bd4/src/document_snapshot.cesta.o: src/document_snapshot.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=document_protocol_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DDOCUMENT_PROTOCOL_PROBE_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/document_snapshot.c' -o "$@"

build/document-protocol-probe-save/obj/src/document_snapshot.cesta.o: build/.objects/11139b4a9cbb0bd4/src/document_snapshot.cesta.o
	@mkdir -p 'build/document-protocol-probe-save/obj/src'
	ln -f 'build/.objects/11139b4a9cbb0bd4/src/document_snapshot.cesta.o' "$@"

build/.objects/f65ba791e6bb22af/src/rpc_wait.cesta.o: src/rpc_wait.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=document_protocol_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DDOCUMENT_PROTOCOL_PROBE_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/rpc_wait.c' -o "$@"

build/document-protocol-probe-save/obj/src/rpc_wait.cesta.o: build/.objects/f65ba791e6bb22af/src/rpc_wait.cesta.o
	@mkdir -p 'build/document-protocol-probe-save/obj/src'
	ln -f 'build/.objects/f65ba791e6bb22af/src/rpc_wait.cesta.o' "$@"

build/.objects/d63925bdebff395f/runtime/client_runtime.c.o: runtime/client_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=document_protocol_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DDOCUMENT_PROTOCOL_PROBE_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/document-protocol-probe-save/obj/runtime/client_runtime.c.o: build/.objects/d63925bdebff395f/runtime/client_runtime.c.o
	@mkdir -p 'build/document-protocol-probe-save/obj/runtime'
	ln -f 'build/.objects/d63925bdebff395f/runtime/client_runtime.c.o' "$@"

build/.objects/df96042043fed73b/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=document_protocol_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DDOCUMENT_PROTOCOL_PROBE_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/document-protocol-probe-save/obj/runtime/traps_x86_64.c.o: build/.objects/df96042043fed73b/runtime/traps_x86_64.c.o
	@mkdir -p 'build/document-protocol-probe-save/obj/runtime'
	ln -f 'build/.objects/df96042043fed73b/runtime/traps_x86_64.c.o' "$@"

build/.objects/c580a0a18a826d67/runtime/rpc_traps_x86_64.c.o: runtime/rpc_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=document_protocol_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DDOCUMENT_PROTOCOL_PROBE_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/rpc_traps_x86_64.c' -o "$@"

build/document-protocol-probe-save/obj/runtime/rpc_traps_x86_64.c.o: build/.objects/c580a0a18a826d67/runtime/rpc_traps_x86_64.c.o
	@mkdir -p 'build/document-protocol-probe-save/obj/runtime'
	ln -f 'build/.objects/c580a0a18a826d67/runtime/rpc_traps_x86_64.c.o' "$@"

build/.objects/97802513469cb058/runtime/region_writer_traps_x86_64.c.o: runtime/region_writer_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=document_protocol_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DDOCUMENT_PROTOCOL_PROBE_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/region_writer_traps_x86_64.c' -o "$@"

build/document-protocol-probe-save/obj/runtime/region_writer_traps_x86_64.c.o: build/.objects/97802513469cb058/runtime/region_writer_traps_x86_64.c.o
	@mkdir -p 'build/document-protocol-probe-save/obj/runtime'
	ln -f 'build/.objects/97802513469cb058/runtime/region_writer_traps_x86_64.c.o' "$@"

build/.objects/b1bb41c86efb000f/runtime/start_x86_64.S.o: runtime/start_x86_64.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=document_protocol_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DDOCUMENT_PROTOCOL_PROBE_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/document-protocol-probe-save/obj/runtime/start_x86_64.S.o: build/.objects/b1bb41c86efb000f/runtime/start_x86_64.S.o
	@mkdir -p 'build/document-protocol-probe-save/obj/runtime'
	ln -f 'build/.objects/b1bb41c86efb000f/runtime/start_x86_64.S.o' "$@"

build/.objects/998a0cfbe8acaf56/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=document_protocol_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DDOCUMENT_PROTOCOL_PROBE_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/document-protocol-probe-save/obj/generated/runtime/format_bytes.c.o: build/.objects/998a0cfbe8acaf56/generated/runtime/format_bytes.c.o
	@mkdir -p 'build/document-protocol-probe-save/obj/generated/runtime'
	ln -f 'build/.objects/998a0cfbe8acaf56/generated/runtime/format_bytes.c.o' "$@"

build/.objects/be25464ea86ec63f/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=document_protocol_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DDOCUMENT_PROTOCOL_PROBE_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/document-protocol-probe-save/obj/generated/runtime/packed_words.c.o: build/.objects/be25464ea86ec63f/generated/runtime/packed_words.c.o
	@mkdir -p 'build/document-protocol-probe-save/obj/generated/runtime'
	ln -f 'build/.objects/be25464ea86ec63f/generated/runtime/packed_words.c.o' "$@"

build/document-protocol-probe-save/document-protocol-probe-save: GNUmakefile build/document-protocol-probe-save/obj/src/document_protocol_probe.cesta.o build/document-protocol-probe-save/obj/src/document_protocol.cesta.o build/document-protocol-probe-save/obj/src/document_client.cesta.o build/document-protocol-probe-save/obj/src/document_client_transport.cesta.o build/document-protocol-probe-save/obj/src/document_snapshot.cesta.o build/document-protocol-probe-save/obj/src/rpc_wait.cesta.o build/document-protocol-probe-save/obj/runtime/client_runtime.c.o build/document-protocol-probe-save/obj/runtime/traps_x86_64.c.o build/document-protocol-probe-save/obj/runtime/rpc_traps_x86_64.c.o build/document-protocol-probe-save/obj/runtime/region_writer_traps_x86_64.c.o build/document-protocol-probe-save/obj/runtime/start_x86_64.S.o build/document-protocol-probe-save/obj/generated/runtime/format_bytes.c.o build/document-protocol-probe-save/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/document-protocol-probe-save/obj/src/document_protocol_probe.cesta.o' 'build/document-protocol-probe-save/obj/src/document_protocol.cesta.o' 'build/document-protocol-probe-save/obj/src/document_client.cesta.o' 'build/document-protocol-probe-save/obj/src/document_client_transport.cesta.o' 'build/document-protocol-probe-save/obj/src/document_snapshot.cesta.o' 'build/document-protocol-probe-save/obj/src/rpc_wait.cesta.o' 'build/document-protocol-probe-save/obj/runtime/client_runtime.c.o' 'build/document-protocol-probe-save/obj/runtime/traps_x86_64.c.o' 'build/document-protocol-probe-save/obj/runtime/rpc_traps_x86_64.c.o' 'build/document-protocol-probe-save/obj/runtime/region_writer_traps_x86_64.c.o' 'build/document-protocol-probe-save/obj/runtime/start_x86_64.S.o' 'build/document-protocol-probe-save/obj/generated/runtime/format_bytes.c.o' 'build/document-protocol-probe-save/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/.objects/d78fb029746d9a56/src/document_protocol_probe.cesta.o: src/document_protocol_probe.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=document_protocol_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DDOCUMENT_PROTOCOL_PROBE_MODE=2' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/document_protocol_probe.c' -o "$@"

build/document-protocol-probe-recover/obj/src/document_protocol_probe.cesta.o: build/.objects/d78fb029746d9a56/src/document_protocol_probe.cesta.o
	@mkdir -p 'build/document-protocol-probe-recover/obj/src'
	ln -f 'build/.objects/d78fb029746d9a56/src/document_protocol_probe.cesta.o' "$@"

build/.objects/7df028beb62f513b/src/document_protocol.cesta.o: src/document_protocol.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=document_protocol_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DDOCUMENT_PROTOCOL_PROBE_MODE=2' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/document_protocol.c' -o "$@"

build/document-protocol-probe-recover/obj/src/document_protocol.cesta.o: build/.objects/7df028beb62f513b/src/document_protocol.cesta.o
	@mkdir -p 'build/document-protocol-probe-recover/obj/src'
	ln -f 'build/.objects/7df028beb62f513b/src/document_protocol.cesta.o' "$@"

build/.objects/e394c10d6c5603c0/src/document_client.cesta.o: src/document_client.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=document_protocol_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DDOCUMENT_PROTOCOL_PROBE_MODE=2' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/document_client.c' -o "$@"

build/document-protocol-probe-recover/obj/src/document_client.cesta.o: build/.objects/e394c10d6c5603c0/src/document_client.cesta.o
	@mkdir -p 'build/document-protocol-probe-recover/obj/src'
	ln -f 'build/.objects/e394c10d6c5603c0/src/document_client.cesta.o' "$@"

build/.objects/1bf082a7a10f9c1e/src/document_client_transport.cesta.o: src/document_client_transport.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=document_protocol_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DDOCUMENT_PROTOCOL_PROBE_MODE=2' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/document_client_transport.c' -o "$@"

build/document-protocol-probe-recover/obj/src/document_client_transport.cesta.o: build/.objects/1bf082a7a10f9c1e/src/document_client_transport.cesta.o
	@mkdir -p 'build/document-protocol-probe-recover/obj/src'
	ln -f 'build/.objects/1bf082a7a10f9c1e/src/document_client_transport.cesta.o' "$@"

build/.objects/11159b4a9cbb8eab/src/document_snapshot.cesta.o: src/document_snapshot.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=document_protocol_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DDOCUMENT_PROTOCOL_PROBE_MODE=2' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/document_snapshot.c' -o "$@"

build/document-protocol-probe-recover/obj/src/document_snapshot.cesta.o: build/.objects/11159b4a9cbb8eab/src/document_snapshot.cesta.o
	@mkdir -p 'build/document-protocol-probe-recover/obj/src'
	ln -f 'build/.objects/11159b4a9cbb8eab/src/document_snapshot.cesta.o' "$@"

build/.objects/f659a791e6ba9fd8/src/rpc_wait.cesta.o: src/rpc_wait.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=document_protocol_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DDOCUMENT_PROTOCOL_PROBE_MODE=2' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/rpc_wait.c' -o "$@"

build/document-protocol-probe-recover/obj/src/rpc_wait.cesta.o: build/.objects/f659a791e6ba9fd8/src/rpc_wait.cesta.o
	@mkdir -p 'build/document-protocol-probe-recover/obj/src'
	ln -f 'build/.objects/f659a791e6ba9fd8/src/rpc_wait.cesta.o' "$@"

build/.objects/d63525bdebfb5088/runtime/client_runtime.c.o: runtime/client_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=document_protocol_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DDOCUMENT_PROTOCOL_PROBE_MODE=2' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/document-protocol-probe-recover/obj/runtime/client_runtime.c.o: build/.objects/d63525bdebfb5088/runtime/client_runtime.c.o
	@mkdir -p 'build/document-protocol-probe-recover/obj/runtime'
	ln -f 'build/.objects/d63525bdebfb5088/runtime/client_runtime.c.o' "$@"

build/.objects/df92042043faee64/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=document_protocol_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DDOCUMENT_PROTOCOL_PROBE_MODE=2' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/document-protocol-probe-recover/obj/runtime/traps_x86_64.c.o: build/.objects/df92042043faee64/runtime/traps_x86_64.c.o
	@mkdir -p 'build/document-protocol-probe-recover/obj/runtime'
	ln -f 'build/.objects/df92042043faee64/runtime/traps_x86_64.c.o' "$@"

build/.objects/c57ca0a18a7e8490/runtime/rpc_traps_x86_64.c.o: runtime/rpc_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=document_protocol_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DDOCUMENT_PROTOCOL_PROBE_MODE=2' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/rpc_traps_x86_64.c' -o "$@"

build/document-protocol-probe-recover/obj/runtime/rpc_traps_x86_64.c.o: build/.objects/c57ca0a18a7e8490/runtime/rpc_traps_x86_64.c.o
	@mkdir -p 'build/document-protocol-probe-recover/obj/runtime'
	ln -f 'build/.objects/c57ca0a18a7e8490/runtime/rpc_traps_x86_64.c.o' "$@"

build/.objects/9784251346a0992f/runtime/region_writer_traps_x86_64.c.o: runtime/region_writer_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=document_protocol_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DDOCUMENT_PROTOCOL_PROBE_MODE=2' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/region_writer_traps_x86_64.c' -o "$@"

build/document-protocol-probe-recover/obj/runtime/region_writer_traps_x86_64.c.o: build/.objects/9784251346a0992f/runtime/region_writer_traps_x86_64.c.o
	@mkdir -p 'build/document-protocol-probe-recover/obj/runtime'
	ln -f 'build/.objects/9784251346a0992f/runtime/region_writer_traps_x86_64.c.o' "$@"

build/.objects/b1b941c86efa7d38/runtime/start_x86_64.S.o: runtime/start_x86_64.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=document_protocol_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DDOCUMENT_PROTOCOL_PROBE_MODE=2' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/document-protocol-probe-recover/obj/runtime/start_x86_64.S.o: build/.objects/b1b941c86efa7d38/runtime/start_x86_64.S.o
	@mkdir -p 'build/document-protocol-probe-recover/obj/runtime'
	ln -f 'build/.objects/b1b941c86efa7d38/runtime/start_x86_64.S.o' "$@"

build/.objects/99800cfbe8a45ad1/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=document_protocol_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DDOCUMENT_PROTOCOL_PROBE_MODE=2' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/document-protocol-probe-recover/obj/generated/runtime/format_bytes.c.o: build/.objects/99800cfbe8a45ad1/generated/runtime/format_bytes.c.o
	@mkdir -p 'build/document-protocol-probe-recover/obj/generated/runtime'
	ln -f 'build/.objects/99800cfbe8a45ad1/generated/runtime/format_bytes.c.o' "$@"

build/.objects/be23464ea86e4368/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=document_protocol_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DDOCUMENT_PROTOCOL_PROBE_MODE=2' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/document-protocol-probe-recover/obj/generated/runtime/packed_words.c.o: build/.objects/be23464ea86e4368/generated/runtime/packed_words.c.o
	@mkdir -p 'build/document-protocol-probe-recover/obj/generated/runtime'
	ln -f 'build/.objects/be23464ea86e4368/generated/runtime/packed_words.c.o' "$@"

build/document-protocol-probe-recover/document-protocol-probe-recover: GNUmakefile build/document-protocol-probe-recover/obj/src/document_protocol_probe.cesta.o build/document-protocol-probe-recover/obj/src/document_protocol.cesta.o build/document-protocol-probe-recover/obj/src/document_client.cesta.o build/document-protocol-probe-recover/obj/src/document_client_transport.cesta.o build/document-protocol-probe-recover/obj/src/document_snapshot.cesta.o build/document-protocol-probe-recover/obj/src/rpc_wait.cesta.o build/document-protocol-probe-recover/obj/runtime/client_runtime.c.o build/document-protocol-probe-recover/obj/runtime/traps_x86_64.c.o build/document-protocol-probe-recover/obj/runtime/rpc_traps_x86_64.c.o build/document-protocol-probe-recover/obj/runtime/region_writer_traps_x86_64.c.o build/document-protocol-probe-recover/obj/runtime/start_x86_64.S.o build/document-protocol-probe-recover/obj/generated/runtime/format_bytes.c.o build/document-protocol-probe-recover/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/document-protocol-probe-recover/obj/src/document_protocol_probe.cesta.o' 'build/document-protocol-probe-recover/obj/src/document_protocol.cesta.o' 'build/document-protocol-probe-recover/obj/src/document_client.cesta.o' 'build/document-protocol-probe-recover/obj/src/document_client_transport.cesta.o' 'build/document-protocol-probe-recover/obj/src/document_snapshot.cesta.o' 'build/document-protocol-probe-recover/obj/src/rpc_wait.cesta.o' 'build/document-protocol-probe-recover/obj/runtime/client_runtime.c.o' 'build/document-protocol-probe-recover/obj/runtime/traps_x86_64.c.o' 'build/document-protocol-probe-recover/obj/runtime/rpc_traps_x86_64.c.o' 'build/document-protocol-probe-recover/obj/runtime/region_writer_traps_x86_64.c.o' 'build/document-protocol-probe-recover/obj/runtime/start_x86_64.S.o' 'build/document-protocol-probe-recover/obj/generated/runtime/format_bytes.c.o' 'build/document-protocol-probe-recover/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/.objects/8e2e67d20378e132/src/record_hum.cesta.o: src/record_hum.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=record_hum_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/record_hum.c' -o "$@"

build/record-hum/obj/src/record_hum.cesta.o: build/.objects/8e2e67d20378e132/src/record_hum.cesta.o
	@mkdir -p 'build/record-hum/obj/src'
	ln -f 'build/.objects/8e2e67d20378e132/src/record_hum.cesta.o' "$@"

build/.objects/757081c64bd6cb57/runtime/client_runtime.c.o: runtime/client_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=record_hum_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/record-hum/obj/runtime/client_runtime.c.o: build/.objects/757081c64bd6cb57/runtime/client_runtime.c.o
	@mkdir -p 'build/record-hum/obj/runtime'
	ln -f 'build/.objects/757081c64bd6cb57/runtime/client_runtime.c.o' "$@"

build/.objects/e563e048b79b0ebb/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=record_hum_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/record-hum/obj/runtime/traps_x86_64.c.o: build/.objects/e563e048b79b0ebb/runtime/traps_x86_64.c.o
	@mkdir -p 'build/record-hum/obj/runtime'
	ln -f 'build/.objects/e563e048b79b0ebb/runtime/traps_x86_64.c.o' "$@"

build/.objects/0cacde62a2c593c7/runtime/start_x86_64.S.o: runtime/start_x86_64.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=record_hum_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/record-hum/obj/runtime/start_x86_64.S.o: build/.objects/0cacde62a2c593c7/runtime/start_x86_64.S.o
	@mkdir -p 'build/record-hum/obj/runtime'
	ln -f 'build/.objects/0cacde62a2c593c7/runtime/start_x86_64.S.o' "$@"

build/.objects/ab2be8f41e7f3fde/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=record_hum_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/record-hum/obj/generated/runtime/format_bytes.c.o: build/.objects/ab2be8f41e7f3fde/generated/runtime/format_bytes.c.o
	@mkdir -p 'build/record-hum/obj/generated/runtime'
	ln -f 'build/.objects/ab2be8f41e7f3fde/generated/runtime/format_bytes.c.o' "$@"

build/.objects/66f8f98cff1ead37/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=record_hum_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/record-hum/obj/generated/runtime/packed_words.c.o: build/.objects/66f8f98cff1ead37/generated/runtime/packed_words.c.o
	@mkdir -p 'build/record-hum/obj/generated/runtime'
	ln -f 'build/.objects/66f8f98cff1ead37/generated/runtime/packed_words.c.o' "$@"

build/record-hum/record-hum: GNUmakefile build/record-hum/obj/src/record_hum.cesta.o build/record-hum/obj/runtime/client_runtime.c.o build/record-hum/obj/runtime/traps_x86_64.c.o build/record-hum/obj/runtime/start_x86_64.S.o build/record-hum/obj/generated/runtime/format_bytes.c.o build/record-hum/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/record-hum/obj/src/record_hum.cesta.o' 'build/record-hum/obj/runtime/client_runtime.c.o' 'build/record-hum/obj/runtime/traps_x86_64.c.o' 'build/record-hum/obj/runtime/start_x86_64.S.o' 'build/record-hum/obj/generated/runtime/format_bytes.c.o' 'build/record-hum/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/.objects/52fc098079fcb95c/src/spindle.cesta.o: src/spindle.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=spindle_main' '-DSEXOS_SPINDLE_CURRENT=1' '-DSEXOS_RECORD_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/spindle.c' -o "$@"

build/spindle-record-probe/obj/src/spindle.cesta.o: build/.objects/52fc098079fcb95c/src/spindle.cesta.o
	@mkdir -p 'build/spindle-record-probe/obj/src'
	ln -f 'build/.objects/52fc098079fcb95c/src/spindle.cesta.o' "$@"

build/.objects/75db67f3e9848c96/src/spindle_calc.cesta.o: src/spindle_calc.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=spindle_main' '-DSEXOS_SPINDLE_CURRENT=1' '-DSEXOS_RECORD_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/spindle_calc.c' -o "$@"

build/spindle-record-probe/obj/src/spindle_calc.cesta.o: build/.objects/75db67f3e9848c96/src/spindle_calc.cesta.o
	@mkdir -p 'build/spindle-record-probe/obj/src'
	ln -f 'build/.objects/75db67f3e9848c96/src/spindle_calc.cesta.o' "$@"

build/.objects/d7be94a0a029098c/src/spindle_complete.cesta.o: src/spindle_complete.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=spindle_main' '-DSEXOS_SPINDLE_CURRENT=1' '-DSEXOS_RECORD_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/spindle_complete.c' -o "$@"

build/spindle-record-probe/obj/src/spindle_complete.cesta.o: build/.objects/d7be94a0a029098c/src/spindle_complete.cesta.o
	@mkdir -p 'build/spindle-record-probe/obj/src'
	ln -f 'build/.objects/d7be94a0a029098c/src/spindle_complete.cesta.o' "$@"

build/.objects/23cce5806f327186/src/spindle_sextime.cesta.o: src/spindle_sextime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=spindle_main' '-DSEXOS_SPINDLE_CURRENT=1' '-DSEXOS_RECORD_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/spindle_sextime.c' -o "$@"

build/spindle-record-probe/obj/src/spindle_sextime.cesta.o: build/.objects/23cce5806f327186/src/spindle_sextime.cesta.o
	@mkdir -p 'build/spindle-record-probe/obj/src'
	ln -f 'build/.objects/23cce5806f327186/src/spindle_sextime.cesta.o' "$@"

build/.objects/dddc42035b51937b/src/sex_graphics.cesta.o: src/sex_graphics.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=spindle_main' '-DSEXOS_SPINDLE_CURRENT=1' '-DSEXOS_RECORD_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/sex_graphics.c' -o "$@"

build/spindle-record-probe/obj/src/sex_graphics.cesta.o: build/.objects/dddc42035b51937b/src/sex_graphics.cesta.o
	@mkdir -p 'build/spindle-record-probe/obj/src'
	ln -f 'build/.objects/dddc42035b51937b/src/sex_graphics.cesta.o' "$@"

build/.objects/27be57ed3e65269a/runtime/client_runtime.c.o: runtime/client_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=spindle_main' '-DSEXOS_SPINDLE_CURRENT=1' '-DSEXOS_RECORD_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/spindle-record-probe/obj/runtime/client_runtime.c.o: build/.objects/27be57ed3e65269a/runtime/client_runtime.c.o
	@mkdir -p 'build/spindle-record-probe/obj/runtime'
	ln -f 'build/.objects/27be57ed3e65269a/runtime/client_runtime.c.o' "$@"

build/.objects/2c22230e9be0fc46/runtime/server_runtime.c.o: runtime/server_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=spindle_main' '-DSEXOS_SPINDLE_CURRENT=1' '-DSEXOS_RECORD_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_runtime.c' -o "$@"

build/spindle-record-probe/obj/runtime/server_runtime.c.o: build/.objects/2c22230e9be0fc46/runtime/server_runtime.c.o
	@mkdir -p 'build/spindle-record-probe/obj/runtime'
	ln -f 'build/.objects/2c22230e9be0fc46/runtime/server_runtime.c.o' "$@"

build/.objects/18850ed1142857de/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=spindle_main' '-DSEXOS_SPINDLE_CURRENT=1' '-DSEXOS_RECORD_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/spindle-record-probe/obj/runtime/traps_x86_64.c.o: build/.objects/18850ed1142857de/runtime/traps_x86_64.c.o
	@mkdir -p 'build/spindle-record-probe/obj/runtime'
	ln -f 'build/.objects/18850ed1142857de/runtime/traps_x86_64.c.o' "$@"

build/.objects/5e509e36fc953c5a/runtime/server_traps_x86_64.c.o: runtime/server_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=spindle_main' '-DSEXOS_SPINDLE_CURRENT=1' '-DSEXOS_RECORD_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_traps_x86_64.c' -o "$@"

build/spindle-record-probe/obj/runtime/server_traps_x86_64.c.o: build/.objects/5e509e36fc953c5a/runtime/server_traps_x86_64.c.o
	@mkdir -p 'build/spindle-record-probe/obj/runtime'
	ln -f 'build/.objects/5e509e36fc953c5a/runtime/server_traps_x86_64.c.o' "$@"

build/.objects/4e134fb9c40170aa/runtime/start_x86_64.S.o: runtime/start_x86_64.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=spindle_main' '-DSEXOS_SPINDLE_CURRENT=1' '-DSEXOS_RECORD_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/spindle-record-probe/obj/runtime/start_x86_64.S.o: build/.objects/4e134fb9c40170aa/runtime/start_x86_64.S.o
	@mkdir -p 'build/spindle-record-probe/obj/runtime'
	ln -f 'build/.objects/4e134fb9c40170aa/runtime/start_x86_64.S.o' "$@"

build/.objects/5d6bafa70b42dddf/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=spindle_main' '-DSEXOS_SPINDLE_CURRENT=1' '-DSEXOS_RECORD_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/spindle-record-probe/obj/generated/runtime/format_bytes.c.o: build/.objects/5d6bafa70b42dddf/generated/runtime/format_bytes.c.o
	@mkdir -p 'build/spindle-record-probe/obj/generated/runtime'
	ln -f 'build/.objects/5d6bafa70b42dddf/generated/runtime/format_bytes.c.o' "$@"

build/.objects/327bb8f3cd58df7a/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=spindle_main' '-DSEXOS_SPINDLE_CURRENT=1' '-DSEXOS_RECORD_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/spindle-record-probe/obj/generated/runtime/packed_words.c.o: build/.objects/327bb8f3cd58df7a/generated/runtime/packed_words.c.o
	@mkdir -p 'build/spindle-record-probe/obj/generated/runtime'
	ln -f 'build/.objects/327bb8f3cd58df7a/generated/runtime/packed_words.c.o' "$@"

build/spindle-record-probe/spindle-record-probe: GNUmakefile build/spindle-record-probe/obj/src/spindle.cesta.o build/spindle-record-probe/obj/src/spindle_calc.cesta.o build/spindle-record-probe/obj/src/spindle_complete.cesta.o build/spindle-record-probe/obj/src/spindle_sextime.cesta.o build/spindle-record-probe/obj/src/sex_graphics.cesta.o build/spindle-record-probe/obj/runtime/client_runtime.c.o build/spindle-record-probe/obj/runtime/server_runtime.c.o build/spindle-record-probe/obj/runtime/traps_x86_64.c.o build/spindle-record-probe/obj/runtime/server_traps_x86_64.c.o build/spindle-record-probe/obj/runtime/start_x86_64.S.o build/spindle-record-probe/obj/generated/runtime/format_bytes.c.o build/spindle-record-probe/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/spindle-record-probe/obj/src/spindle.cesta.o' 'build/spindle-record-probe/obj/src/spindle_calc.cesta.o' 'build/spindle-record-probe/obj/src/spindle_complete.cesta.o' 'build/spindle-record-probe/obj/src/spindle_sextime.cesta.o' 'build/spindle-record-probe/obj/src/sex_graphics.cesta.o' 'build/spindle-record-probe/obj/runtime/client_runtime.c.o' 'build/spindle-record-probe/obj/runtime/server_runtime.c.o' 'build/spindle-record-probe/obj/runtime/traps_x86_64.c.o' 'build/spindle-record-probe/obj/runtime/server_traps_x86_64.c.o' 'build/spindle-record-probe/obj/runtime/start_x86_64.S.o' 'build/spindle-record-probe/obj/generated/runtime/format_bytes.c.o' 'build/spindle-record-probe/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/.objects/e8d2d6b86865479b/src/spindle.cesta.o: src/spindle.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=spindle_main' '-DSEXOS_SPINDLE_CURRENT=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/spindle.c' -o "$@"

build/spindle/obj/src/spindle.cesta.o: build/.objects/e8d2d6b86865479b/src/spindle.cesta.o
	@mkdir -p 'build/spindle/obj/src'
	ln -f 'build/.objects/e8d2d6b86865479b/src/spindle.cesta.o' "$@"

build/.objects/5595892e3f8886e5/src/spindle_calc.cesta.o: src/spindle_calc.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=spindle_main' '-DSEXOS_SPINDLE_CURRENT=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/spindle_calc.c' -o "$@"

build/spindle/obj/src/spindle_calc.cesta.o: build/.objects/5595892e3f8886e5/src/spindle_calc.cesta.o
	@mkdir -p 'build/spindle/obj/src'
	ln -f 'build/.objects/5595892e3f8886e5/src/spindle_calc.cesta.o' "$@"

build/.objects/4e8ccaa66276deeb/src/spindle_complete.cesta.o: src/spindle_complete.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=spindle_main' '-DSEXOS_SPINDLE_CURRENT=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/spindle_complete.c' -o "$@"

build/spindle/obj/src/spindle_complete.cesta.o: build/.objects/4e8ccaa66276deeb/src/spindle_complete.cesta.o
	@mkdir -p 'build/spindle/obj/src'
	ln -f 'build/.objects/4e8ccaa66276deeb/src/spindle_complete.cesta.o' "$@"

build/.objects/183770f0daf9e2b5/src/spindle_sextime.cesta.o: src/spindle_sextime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=spindle_main' '-DSEXOS_SPINDLE_CURRENT=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/spindle_sextime.c' -o "$@"

build/spindle/obj/src/spindle_sextime.cesta.o: build/.objects/183770f0daf9e2b5/src/spindle_sextime.cesta.o
	@mkdir -p 'build/spindle/obj/src'
	ln -f 'build/.objects/183770f0daf9e2b5/src/spindle_sextime.cesta.o' "$@"

build/.objects/f4801b387e8f4576/src/sex_graphics.cesta.o: src/sex_graphics.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=spindle_main' '-DSEXOS_SPINDLE_CURRENT=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/sex_graphics.c' -o "$@"

build/spindle/obj/src/sex_graphics.cesta.o: build/.objects/f4801b387e8f4576/src/sex_graphics.cesta.o
	@mkdir -p 'build/spindle/obj/src'
	ln -f 'build/.objects/f4801b387e8f4576/src/sex_graphics.cesta.o' "$@"

build/.objects/668f501f0f732f79/runtime/client_runtime.c.o: runtime/client_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=spindle_main' '-DSEXOS_SPINDLE_CURRENT=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/spindle/obj/runtime/client_runtime.c.o: build/.objects/668f501f0f732f79/runtime/client_runtime.c.o
	@mkdir -p 'build/spindle/obj/runtime'
	ln -f 'build/.objects/668f501f0f732f79/runtime/client_runtime.c.o' "$@"

build/.objects/dacd0e8e92b959f5/runtime/server_runtime.c.o: runtime/server_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=spindle_main' '-DSEXOS_SPINDLE_CURRENT=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_runtime.c' -o "$@"

build/spindle/obj/runtime/server_runtime.c.o: build/.objects/dacd0e8e92b959f5/runtime/server_runtime.c.o
	@mkdir -p 'build/spindle/obj/runtime'
	ln -f 'build/.objects/dacd0e8e92b959f5/runtime/server_runtime.c.o' "$@"

build/.objects/56587e500d19c58d/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=spindle_main' '-DSEXOS_SPINDLE_CURRENT=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/spindle/obj/runtime/traps_x86_64.c.o: build/.objects/56587e500d19c58d/runtime/traps_x86_64.c.o
	@mkdir -p 'build/spindle/obj/runtime'
	ln -f 'build/.objects/56587e500d19c58d/runtime/traps_x86_64.c.o' "$@"

build/.objects/b2a6ca10e76c75b9/runtime/server_traps_x86_64.c.o: runtime/server_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=spindle_main' '-DSEXOS_SPINDLE_CURRENT=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_traps_x86_64.c' -o "$@"

build/spindle/obj/runtime/server_traps_x86_64.c.o: build/.objects/b2a6ca10e76c75b9/runtime/server_traps_x86_64.c.o
	@mkdir -p 'build/spindle/obj/runtime'
	ln -f 'build/.objects/b2a6ca10e76c75b9/runtime/server_traps_x86_64.c.o' "$@"

build/.objects/a48a511cb9bab169/runtime/start_x86_64.S.o: runtime/start_x86_64.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=spindle_main' '-DSEXOS_SPINDLE_CURRENT=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/spindle/obj/runtime/start_x86_64.S.o: build/.objects/a48a511cb9bab169/runtime/start_x86_64.S.o
	@mkdir -p 'build/spindle/obj/runtime'
	ln -f 'build/.objects/a48a511cb9bab169/runtime/start_x86_64.S.o' "$@"

build/.objects/daa01083cfab945a/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=spindle_main' '-DSEXOS_SPINDLE_CURRENT=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/spindle/obj/generated/runtime/format_bytes.c.o: build/.objects/daa01083cfab945a/generated/runtime/format_bytes.c.o
	@mkdir -p 'build/spindle/obj/generated/runtime'
	ln -f 'build/.objects/daa01083cfab945a/generated/runtime/format_bytes.c.o' "$@"

build/.objects/0363f0644b24ca99/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=spindle_main' '-DSEXOS_SPINDLE_CURRENT=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/spindle/obj/generated/runtime/packed_words.c.o: build/.objects/0363f0644b24ca99/generated/runtime/packed_words.c.o
	@mkdir -p 'build/spindle/obj/generated/runtime'
	ln -f 'build/.objects/0363f0644b24ca99/generated/runtime/packed_words.c.o' "$@"

build/spindle/spindle: GNUmakefile build/spindle/obj/src/spindle.cesta.o build/spindle/obj/src/spindle_calc.cesta.o build/spindle/obj/src/spindle_complete.cesta.o build/spindle/obj/src/spindle_sextime.cesta.o build/spindle/obj/src/sex_graphics.cesta.o build/spindle/obj/runtime/client_runtime.c.o build/spindle/obj/runtime/server_runtime.c.o build/spindle/obj/runtime/traps_x86_64.c.o build/spindle/obj/runtime/server_traps_x86_64.c.o build/spindle/obj/runtime/start_x86_64.S.o build/spindle/obj/generated/runtime/format_bytes.c.o build/spindle/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/spindle/obj/src/spindle.cesta.o' 'build/spindle/obj/src/spindle_calc.cesta.o' 'build/spindle/obj/src/spindle_complete.cesta.o' 'build/spindle/obj/src/spindle_sextime.cesta.o' 'build/spindle/obj/src/sex_graphics.cesta.o' 'build/spindle/obj/runtime/client_runtime.c.o' 'build/spindle/obj/runtime/server_runtime.c.o' 'build/spindle/obj/runtime/traps_x86_64.c.o' 'build/spindle/obj/runtime/server_traps_x86_64.c.o' 'build/spindle/obj/runtime/start_x86_64.S.o' 'build/spindle/obj/generated/runtime/format_bytes.c.o' 'build/spindle/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/.objects/d86b5d7c92ac583b/src/purple_scanout.cesta.o: src/purple_scanout.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=purple_scanout_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/purple_scanout.c' -o "$@"

build/purple-scanout/obj/src/purple_scanout.cesta.o: build/.objects/d86b5d7c92ac583b/src/purple_scanout.cesta.o
	@mkdir -p 'build/purple-scanout/obj/src'
	ln -f 'build/.objects/d86b5d7c92ac583b/src/purple_scanout.cesta.o' "$@"

build/.objects/198bfc2a6887f5d4/runtime/client_runtime.c.o: runtime/client_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=purple_scanout_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/purple-scanout/obj/runtime/client_runtime.c.o: build/.objects/198bfc2a6887f5d4/runtime/client_runtime.c.o
	@mkdir -p 'build/purple-scanout/obj/runtime'
	ln -f 'build/.objects/198bfc2a6887f5d4/runtime/client_runtime.c.o' "$@"

build/.objects/721d3e11079126b8/runtime/server_runtime.c.o: runtime/server_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=purple_scanout_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_runtime.c' -o "$@"

build/purple-scanout/obj/runtime/server_runtime.c.o: build/.objects/721d3e11079126b8/runtime/server_runtime.c.o
	@mkdir -p 'build/purple-scanout/obj/runtime'
	ln -f 'build/.objects/721d3e11079126b8/runtime/server_runtime.c.o' "$@"

build/.objects/579d8bde38143d80/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=purple_scanout_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/purple-scanout/obj/runtime/traps_x86_64.c.o: build/.objects/579d8bde38143d80/runtime/traps_x86_64.c.o
	@mkdir -p 'build/purple-scanout/obj/runtime'
	ln -f 'build/.objects/579d8bde38143d80/runtime/traps_x86_64.c.o' "$@"

build/.objects/e2fd53136869b094/runtime/server_traps_x86_64.c.o: runtime/server_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=purple_scanout_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_traps_x86_64.c' -o "$@"

build/purple-scanout/obj/runtime/server_traps_x86_64.c.o: build/.objects/e2fd53136869b094/runtime/server_traps_x86_64.c.o
	@mkdir -p 'build/purple-scanout/obj/runtime'
	ln -f 'build/.objects/e2fd53136869b094/runtime/server_traps_x86_64.c.o' "$@"

build/.objects/ababdd2e7be5ac24/runtime/start_x86_64.S.o: runtime/start_x86_64.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=purple_scanout_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/purple-scanout/obj/runtime/start_x86_64.S.o: build/.objects/ababdd2e7be5ac24/runtime/start_x86_64.S.o
	@mkdir -p 'build/purple-scanout/obj/runtime'
	ln -f 'build/.objects/ababdd2e7be5ac24/runtime/start_x86_64.S.o' "$@"

build/.objects/5b78eb1fd76d46e9/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=purple_scanout_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/purple-scanout/obj/generated/runtime/format_bytes.c.o: build/.objects/5b78eb1fd76d46e9/generated/runtime/format_bytes.c.o
	@mkdir -p 'build/purple-scanout/obj/generated/runtime'
	ln -f 'build/.objects/5b78eb1fd76d46e9/generated/runtime/format_bytes.c.o' "$@"

build/.objects/e073844c54607b34/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=purple_scanout_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/purple-scanout/obj/generated/runtime/packed_words.c.o: build/.objects/e073844c54607b34/generated/runtime/packed_words.c.o
	@mkdir -p 'build/purple-scanout/obj/generated/runtime'
	ln -f 'build/.objects/e073844c54607b34/generated/runtime/packed_words.c.o' "$@"

build/purple-scanout/purple-scanout: GNUmakefile build/purple-scanout/obj/src/purple_scanout.cesta.o build/purple-scanout/obj/runtime/client_runtime.c.o build/purple-scanout/obj/runtime/server_runtime.c.o build/purple-scanout/obj/runtime/traps_x86_64.c.o build/purple-scanout/obj/runtime/server_traps_x86_64.c.o build/purple-scanout/obj/runtime/start_x86_64.S.o build/purple-scanout/obj/generated/runtime/format_bytes.c.o build/purple-scanout/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/purple-scanout/obj/src/purple_scanout.cesta.o' 'build/purple-scanout/obj/runtime/client_runtime.c.o' 'build/purple-scanout/obj/runtime/server_runtime.c.o' 'build/purple-scanout/obj/runtime/traps_x86_64.c.o' 'build/purple-scanout/obj/runtime/server_traps_x86_64.c.o' 'build/purple-scanout/obj/runtime/start_x86_64.S.o' 'build/purple-scanout/obj/generated/runtime/format_bytes.c.o' 'build/purple-scanout/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/.objects/a71dcfa714c2611a/src/sexfiles.cesta.o: src/sexfiles.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexfiles_main' '-DSEXOS_EPOCH_LOG=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/sexfiles.c' -o "$@"

build/sexfiles/obj/src/sexfiles.cesta.o: build/.objects/a71dcfa714c2611a/src/sexfiles.cesta.o
	@mkdir -p 'build/sexfiles/obj/src'
	ln -f 'build/.objects/a71dcfa714c2611a/src/sexfiles.cesta.o' "$@"

build/.objects/67e6322d79d7d8d5/src/sexfiles_program.cesta.o: src/sexfiles_program.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexfiles_main' '-DSEXOS_EPOCH_LOG=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/sexfiles_program.c' -o "$@"

build/sexfiles/obj/src/sexfiles_program.cesta.o: build/.objects/67e6322d79d7d8d5/src/sexfiles_program.cesta.o
	@mkdir -p 'build/sexfiles/obj/src'
	ln -f 'build/.objects/67e6322d79d7d8d5/src/sexfiles_program.cesta.o' "$@"

build/.objects/652b5208ddec474f/src/sexfiles_diskfs.cesta.o: src/sexfiles_diskfs.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexfiles_main' '-DSEXOS_EPOCH_LOG=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/sexfiles_diskfs.c' -o "$@"

build/sexfiles/obj/src/sexfiles_diskfs.cesta.o: build/.objects/652b5208ddec474f/src/sexfiles_diskfs.cesta.o
	@mkdir -p 'build/sexfiles/obj/src'
	ln -f 'build/.objects/652b5208ddec474f/src/sexfiles_diskfs.cesta.o' "$@"

build/.objects/f2dee059fde3f3db/src/sexfiles_proof.cesta.o: src/sexfiles_proof.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexfiles_main' '-DSEXOS_EPOCH_LOG=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/sexfiles_proof.c' -o "$@"

build/sexfiles/obj/src/sexfiles_proof.cesta.o: build/.objects/f2dee059fde3f3db/src/sexfiles_proof.cesta.o
	@mkdir -p 'build/sexfiles/obj/src'
	ln -f 'build/.objects/f2dee059fde3f3db/src/sexfiles_proof.cesta.o' "$@"

build/.objects/61aac9c69a721938/src/epoch_journal.cesta.o: src/epoch_journal.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexfiles_main' '-DSEXOS_EPOCH_LOG=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/epoch_journal.c' -o "$@"

build/sexfiles/obj/src/epoch_journal.cesta.o: build/.objects/61aac9c69a721938/src/epoch_journal.cesta.o
	@mkdir -p 'build/sexfiles/obj/src'
	ln -f 'build/.objects/61aac9c69a721938/src/epoch_journal.cesta.o' "$@"

build/.objects/d8612c2ee33724f0/src/epoch_files.cesta.o: src/epoch_files.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexfiles_main' '-DSEXOS_EPOCH_LOG=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/epoch_files.c' -o "$@"

build/sexfiles/obj/src/epoch_files.cesta.o: build/.objects/d8612c2ee33724f0/src/epoch_files.cesta.o
	@mkdir -p 'build/sexfiles/obj/src'
	ln -f 'build/.objects/d8612c2ee33724f0/src/epoch_files.cesta.o' "$@"

build/.objects/3f4c10f006f3c47b/src/document_protocol.cesta.o: src/document_protocol.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexfiles_main' '-DSEXOS_EPOCH_LOG=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/document_protocol.c' -o "$@"

build/sexfiles/obj/src/document_protocol.cesta.o: build/.objects/3f4c10f006f3c47b/src/document_protocol.cesta.o
	@mkdir -p 'build/sexfiles/obj/src'
	ln -f 'build/.objects/3f4c10f006f3c47b/src/document_protocol.cesta.o' "$@"

build/.objects/e6b803bb93331c00/src/document_service.cesta.o: src/document_service.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexfiles_main' '-DSEXOS_EPOCH_LOG=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/document_service.c' -o "$@"

build/sexfiles/obj/src/document_service.cesta.o: build/.objects/e6b803bb93331c00/src/document_service.cesta.o
	@mkdir -p 'build/sexfiles/obj/src'
	ln -f 'build/.objects/e6b803bb93331c00/src/document_service.cesta.o' "$@"

build/.objects/b05d122bd97a0eca/src/document_service_transport.cesta.o: src/document_service_transport.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexfiles_main' '-DSEXOS_EPOCH_LOG=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/document_service_transport.c' -o "$@"

build/sexfiles/obj/src/document_service_transport.cesta.o: build/.objects/b05d122bd97a0eca/src/document_service_transport.cesta.o
	@mkdir -p 'build/sexfiles/obj/src'
	ln -f 'build/.objects/b05d122bd97a0eca/src/document_service_transport.cesta.o' "$@"

build/.objects/3c37a074f28fce1c/src/document_journal.cesta.o: src/document_journal.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexfiles_main' '-DSEXOS_EPOCH_LOG=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/document_journal.c' -o "$@"

build/sexfiles/obj/src/document_journal.cesta.o: build/.objects/3c37a074f28fce1c/src/document_journal.cesta.o
	@mkdir -p 'build/sexfiles/obj/src'
	ln -f 'build/.objects/3c37a074f28fce1c/src/document_journal.cesta.o' "$@"

build/.objects/34261f61d9037d1c/src/document_growth.cesta.o: src/document_growth.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexfiles_main' '-DSEXOS_EPOCH_LOG=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/document_growth.c' -o "$@"

build/sexfiles/obj/src/document_growth.cesta.o: build/.objects/34261f61d9037d1c/src/document_growth.cesta.o
	@mkdir -p 'build/sexfiles/obj/src'
	ln -f 'build/.objects/34261f61d9037d1c/src/document_growth.cesta.o' "$@"

build/.objects/f9f4117f3919a0cb/src/document_snapshot.cesta.o: src/document_snapshot.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexfiles_main' '-DSEXOS_EPOCH_LOG=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/document_snapshot.c' -o "$@"

build/sexfiles/obj/src/document_snapshot.cesta.o: build/.objects/f9f4117f3919a0cb/src/document_snapshot.cesta.o
	@mkdir -p 'build/sexfiles/obj/src'
	ln -f 'build/.objects/f9f4117f3919a0cb/src/document_snapshot.cesta.o' "$@"

build/.objects/27568d615786cf8d/src/rpc_runtime.cesta.o: src/rpc_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexfiles_main' '-DSEXOS_EPOCH_LOG=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/rpc_runtime.c' -o "$@"

build/sexfiles/obj/src/rpc_runtime.cesta.o: build/.objects/27568d615786cf8d/src/rpc_runtime.cesta.o
	@mkdir -p 'build/sexfiles/obj/src'
	ln -f 'build/.objects/27568d615786cf8d/src/rpc_runtime.cesta.o' "$@"

build/.objects/68b44764937f6e24/src/rpc_wait.cesta.o: src/rpc_wait.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexfiles_main' '-DSEXOS_EPOCH_LOG=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/rpc_wait.c' -o "$@"

build/sexfiles/obj/src/rpc_wait.cesta.o: build/.objects/68b44764937f6e24/src/rpc_wait.cesta.o
	@mkdir -p 'build/sexfiles/obj/src'
	ln -f 'build/.objects/68b44764937f6e24/src/rpc_wait.cesta.o' "$@"

build/.objects/d903d6cff3919f34/runtime/client_runtime.c.o: runtime/client_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexfiles_main' '-DSEXOS_EPOCH_LOG=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/sexfiles/obj/runtime/client_runtime.c.o: build/.objects/d903d6cff3919f34/runtime/client_runtime.c.o
	@mkdir -p 'build/sexfiles/obj/runtime'
	ln -f 'build/.objects/d903d6cff3919f34/runtime/client_runtime.c.o' "$@"

build/.objects/5ef762284da990b0/runtime/server_runtime.c.o: runtime/server_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexfiles_main' '-DSEXOS_EPOCH_LOG=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_runtime.c' -o "$@"

build/sexfiles/obj/runtime/server_runtime.c.o: build/.objects/5ef762284da990b0/runtime/server_runtime.c.o
	@mkdir -p 'build/sexfiles/obj/runtime'
	ln -f 'build/.objects/5ef762284da990b0/runtime/server_runtime.c.o' "$@"

build/.objects/9690092e4e97d9f8/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexfiles_main' '-DSEXOS_EPOCH_LOG=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/sexfiles/obj/runtime/traps_x86_64.c.o: build/.objects/9690092e4e97d9f8/runtime/traps_x86_64.c.o
	@mkdir -p 'build/sexfiles/obj/runtime'
	ln -f 'build/.objects/9690092e4e97d9f8/runtime/traps_x86_64.c.o' "$@"

build/.objects/13b8f6b879b84a74/runtime/server_traps_x86_64.c.o: runtime/server_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexfiles_main' '-DSEXOS_EPOCH_LOG=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_traps_x86_64.c' -o "$@"

build/sexfiles/obj/runtime/server_traps_x86_64.c.o: build/.objects/13b8f6b879b84a74/runtime/server_traps_x86_64.c.o
	@mkdir -p 'build/sexfiles/obj/runtime'
	ln -f 'build/.objects/13b8f6b879b84a74/runtime/server_traps_x86_64.c.o' "$@"

build/.objects/2ba6e3868998506c/runtime/rpc_traps_x86_64.c.o: runtime/rpc_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexfiles_main' '-DSEXOS_EPOCH_LOG=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/rpc_traps_x86_64.c' -o "$@"

build/sexfiles/obj/runtime/rpc_traps_x86_64.c.o: build/.objects/2ba6e3868998506c/runtime/rpc_traps_x86_64.c.o
	@mkdir -p 'build/sexfiles/obj/runtime'
	ln -f 'build/.objects/2ba6e3868998506c/runtime/rpc_traps_x86_64.c.o' "$@"

build/.objects/20dd3d9935959ae7/runtime/region_writer_traps_x86_64.c.o: runtime/region_writer_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexfiles_main' '-DSEXOS_EPOCH_LOG=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/region_writer_traps_x86_64.c' -o "$@"

build/sexfiles/obj/runtime/region_writer_traps_x86_64.c.o: build/.objects/20dd3d9935959ae7/runtime/region_writer_traps_x86_64.c.o
	@mkdir -p 'build/sexfiles/obj/runtime'
	ln -f 'build/.objects/20dd3d9935959ae7/runtime/region_writer_traps_x86_64.c.o' "$@"

build/.objects/8ee070b022ad5d02/runtime/epoch_journal_traps_x86_64.c.o: runtime/epoch_journal_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexfiles_main' '-DSEXOS_EPOCH_LOG=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/epoch_journal_traps_x86_64.c' -o "$@"

build/sexfiles/obj/runtime/epoch_journal_traps_x86_64.c.o: build/.objects/8ee070b022ad5d02/runtime/epoch_journal_traps_x86_64.c.o
	@mkdir -p 'build/sexfiles/obj/runtime'
	ln -f 'build/.objects/8ee070b022ad5d02/runtime/epoch_journal_traps_x86_64.c.o' "$@"

build/.objects/d38c6856f3db1c44/runtime/start_x86_64.S.o: runtime/start_x86_64.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexfiles_main' '-DSEXOS_EPOCH_LOG=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/sexfiles/obj/runtime/start_x86_64.S.o: build/.objects/d38c6856f3db1c44/runtime/start_x86_64.S.o
	@mkdir -p 'build/sexfiles/obj/runtime'
	ln -f 'build/.objects/d38c6856f3db1c44/runtime/start_x86_64.S.o' "$@"

build/.objects/d112c55955f5cfad/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexfiles_main' '-DSEXOS_EPOCH_LOG=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/sexfiles/obj/generated/runtime/format_bytes.c.o: build/.objects/d112c55955f5cfad/generated/runtime/format_bytes.c.o
	@mkdir -p 'build/sexfiles/obj/generated/runtime'
	ln -f 'build/.objects/d112c55955f5cfad/generated/runtime/format_bytes.c.o' "$@"

build/.objects/0497411883005dd4/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexfiles_main' '-DSEXOS_EPOCH_LOG=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/sexfiles/obj/generated/runtime/packed_words.c.o: build/.objects/0497411883005dd4/generated/runtime/packed_words.c.o
	@mkdir -p 'build/sexfiles/obj/generated/runtime'
	ln -f 'build/.objects/0497411883005dd4/generated/runtime/packed_words.c.o' "$@"

build/sexfiles/sexfiles: GNUmakefile build/sexfiles/obj/src/sexfiles.cesta.o build/sexfiles/obj/src/sexfiles_program.cesta.o build/sexfiles/obj/src/sexfiles_diskfs.cesta.o build/sexfiles/obj/src/sexfiles_proof.cesta.o build/sexfiles/obj/src/epoch_journal.cesta.o build/sexfiles/obj/src/epoch_files.cesta.o build/sexfiles/obj/src/document_protocol.cesta.o build/sexfiles/obj/src/document_service.cesta.o build/sexfiles/obj/src/document_service_transport.cesta.o build/sexfiles/obj/src/document_journal.cesta.o build/sexfiles/obj/src/document_growth.cesta.o build/sexfiles/obj/src/document_snapshot.cesta.o build/sexfiles/obj/src/rpc_runtime.cesta.o build/sexfiles/obj/src/rpc_wait.cesta.o build/sexfiles/obj/runtime/client_runtime.c.o build/sexfiles/obj/runtime/server_runtime.c.o build/sexfiles/obj/runtime/traps_x86_64.c.o build/sexfiles/obj/runtime/server_traps_x86_64.c.o build/sexfiles/obj/runtime/rpc_traps_x86_64.c.o build/sexfiles/obj/runtime/region_writer_traps_x86_64.c.o build/sexfiles/obj/runtime/epoch_journal_traps_x86_64.c.o build/sexfiles/obj/runtime/start_x86_64.S.o build/sexfiles/obj/generated/runtime/format_bytes.c.o build/sexfiles/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/sexfiles/obj/src/sexfiles.cesta.o' 'build/sexfiles/obj/src/sexfiles_program.cesta.o' 'build/sexfiles/obj/src/sexfiles_diskfs.cesta.o' 'build/sexfiles/obj/src/sexfiles_proof.cesta.o' 'build/sexfiles/obj/src/epoch_journal.cesta.o' 'build/sexfiles/obj/src/epoch_files.cesta.o' 'build/sexfiles/obj/src/document_protocol.cesta.o' 'build/sexfiles/obj/src/document_service.cesta.o' 'build/sexfiles/obj/src/document_service_transport.cesta.o' 'build/sexfiles/obj/src/document_journal.cesta.o' 'build/sexfiles/obj/src/document_growth.cesta.o' 'build/sexfiles/obj/src/document_snapshot.cesta.o' 'build/sexfiles/obj/src/rpc_runtime.cesta.o' 'build/sexfiles/obj/src/rpc_wait.cesta.o' 'build/sexfiles/obj/runtime/client_runtime.c.o' 'build/sexfiles/obj/runtime/server_runtime.c.o' 'build/sexfiles/obj/runtime/traps_x86_64.c.o' 'build/sexfiles/obj/runtime/server_traps_x86_64.c.o' 'build/sexfiles/obj/runtime/rpc_traps_x86_64.c.o' 'build/sexfiles/obj/runtime/region_writer_traps_x86_64.c.o' 'build/sexfiles/obj/runtime/epoch_journal_traps_x86_64.c.o' 'build/sexfiles/obj/runtime/start_x86_64.S.o' 'build/sexfiles/obj/generated/runtime/format_bytes.c.o' 'build/sexfiles/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/.objects/ded14289bd6ffb6a/src/sexdrive.cesta.o: src/sexdrive.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdrive_main' '-DSEXOS_CURRENT_REGION_WRITER=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/sexdrive.c' -o "$@"

build/sexdrive/obj/src/sexdrive.cesta.o: build/.objects/ded14289bd6ffb6a/src/sexdrive.cesta.o
	@mkdir -p 'build/sexdrive/obj/src'
	ln -f 'build/.objects/ded14289bd6ffb6a/src/sexdrive.cesta.o' "$@"

build/.objects/f10ea48f21e5f97d/src/dma_lease.cesta.o: src/dma_lease.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdrive_main' '-DSEXOS_CURRENT_REGION_WRITER=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/dma_lease.c' -o "$@"

build/sexdrive/obj/src/dma_lease.cesta.o: build/.objects/f10ea48f21e5f97d/src/dma_lease.cesta.o
	@mkdir -p 'build/sexdrive/obj/src'
	ln -f 'build/.objects/f10ea48f21e5f97d/src/dma_lease.cesta.o' "$@"

build/.objects/b2ab4820a183cf6f/runtime/client_runtime.c.o: runtime/client_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdrive_main' '-DSEXOS_CURRENT_REGION_WRITER=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/sexdrive/obj/runtime/client_runtime.c.o: build/.objects/b2ab4820a183cf6f/runtime/client_runtime.c.o
	@mkdir -p 'build/sexdrive/obj/runtime'
	ln -f 'build/.objects/b2ab4820a183cf6f/runtime/client_runtime.c.o' "$@"

build/.objects/273a41d80e61617b/runtime/server_runtime.c.o: runtime/server_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdrive_main' '-DSEXOS_CURRENT_REGION_WRITER=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_runtime.c' -o "$@"

build/sexdrive/obj/runtime/server_runtime.c.o: build/.objects/273a41d80e61617b/runtime/server_runtime.c.o
	@mkdir -p 'build/sexdrive/obj/runtime'
	ln -f 'build/.objects/273a41d80e61617b/runtime/server_runtime.c.o' "$@"

build/.objects/dd141473aba81703/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdrive_main' '-DSEXOS_CURRENT_REGION_WRITER=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/sexdrive/obj/runtime/traps_x86_64.c.o: build/.objects/dd141473aba81703/runtime/traps_x86_64.c.o
	@mkdir -p 'build/sexdrive/obj/runtime'
	ln -f 'build/.objects/dd141473aba81703/runtime/traps_x86_64.c.o' "$@"

build/.objects/6d5a207547ad60af/runtime/server_traps_x86_64.c.o: runtime/server_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdrive_main' '-DSEXOS_CURRENT_REGION_WRITER=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_traps_x86_64.c' -o "$@"

build/sexdrive/obj/runtime/server_traps_x86_64.c.o: build/.objects/6d5a207547ad60af/runtime/server_traps_x86_64.c.o
	@mkdir -p 'build/sexdrive/obj/runtime'
	ln -f 'build/.objects/6d5a207547ad60af/runtime/server_traps_x86_64.c.o' "$@"

build/.objects/e3594cccb735b440/runtime/mmio_traps_x86_64.c.o: runtime/mmio_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdrive_main' '-DSEXOS_CURRENT_REGION_WRITER=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/mmio_traps_x86_64.c' -o "$@"

build/sexdrive/obj/runtime/mmio_traps_x86_64.c.o: build/.objects/e3594cccb735b440/runtime/mmio_traps_x86_64.c.o
	@mkdir -p 'build/sexdrive/obj/runtime'
	ln -f 'build/.objects/e3594cccb735b440/runtime/mmio_traps_x86_64.c.o' "$@"

build/.objects/6335af194ac06a10/runtime/region_writer_traps_x86_64.c.o: runtime/region_writer_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdrive_main' '-DSEXOS_CURRENT_REGION_WRITER=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/region_writer_traps_x86_64.c' -o "$@"

build/sexdrive/obj/runtime/region_writer_traps_x86_64.c.o: build/.objects/6335af194ac06a10/runtime/region_writer_traps_x86_64.c.o
	@mkdir -p 'build/sexdrive/obj/runtime'
	ln -f 'build/.objects/6335af194ac06a10/runtime/region_writer_traps_x86_64.c.o' "$@"

build/.objects/ffd66f93799ff460/runtime/device_reset_traps_x86_64.c.o: runtime/device_reset_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdrive_main' '-DSEXOS_CURRENT_REGION_WRITER=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/device_reset_traps_x86_64.c' -o "$@"

build/sexdrive/obj/runtime/device_reset_traps_x86_64.c.o: build/.objects/ffd66f93799ff460/runtime/device_reset_traps_x86_64.c.o
	@mkdir -p 'build/sexdrive/obj/runtime'
	ln -f 'build/.objects/ffd66f93799ff460/runtime/device_reset_traps_x86_64.c.o' "$@"

build/.objects/47af5b783d9f5d5f/runtime/start_x86_64.S.o: runtime/start_x86_64.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdrive_main' '-DSEXOS_CURRENT_REGION_WRITER=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/sexdrive/obj/runtime/start_x86_64.S.o: build/.objects/47af5b783d9f5d5f/runtime/start_x86_64.S.o
	@mkdir -p 'build/sexdrive/obj/runtime'
	ln -f 'build/.objects/47af5b783d9f5d5f/runtime/start_x86_64.S.o' "$@"

build/.objects/ec7e430756ff7eea/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdrive_main' '-DSEXOS_CURRENT_REGION_WRITER=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/sexdrive/obj/generated/runtime/format_bytes.c.o: build/.objects/ec7e430756ff7eea/generated/runtime/format_bytes.c.o
	@mkdir -p 'build/sexdrive/obj/generated/runtime'
	ln -f 'build/.objects/ec7e430756ff7eea/generated/runtime/format_bytes.c.o' "$@"

build/.objects/19026752e82df00f/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdrive_main' '-DSEXOS_CURRENT_REGION_WRITER=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/sexdrive/obj/generated/runtime/packed_words.c.o: build/.objects/19026752e82df00f/generated/runtime/packed_words.c.o
	@mkdir -p 'build/sexdrive/obj/generated/runtime'
	ln -f 'build/.objects/19026752e82df00f/generated/runtime/packed_words.c.o' "$@"

build/sexdrive/sexdrive: GNUmakefile build/sexdrive/obj/src/sexdrive.cesta.o build/sexdrive/obj/src/dma_lease.cesta.o build/sexdrive/obj/runtime/client_runtime.c.o build/sexdrive/obj/runtime/server_runtime.c.o build/sexdrive/obj/runtime/traps_x86_64.c.o build/sexdrive/obj/runtime/server_traps_x86_64.c.o build/sexdrive/obj/runtime/mmio_traps_x86_64.c.o build/sexdrive/obj/runtime/region_writer_traps_x86_64.c.o build/sexdrive/obj/runtime/device_reset_traps_x86_64.c.o build/sexdrive/obj/runtime/start_x86_64.S.o build/sexdrive/obj/generated/runtime/format_bytes.c.o build/sexdrive/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/sexdrive/obj/src/sexdrive.cesta.o' 'build/sexdrive/obj/src/dma_lease.cesta.o' 'build/sexdrive/obj/runtime/client_runtime.c.o' 'build/sexdrive/obj/runtime/server_runtime.c.o' 'build/sexdrive/obj/runtime/traps_x86_64.c.o' 'build/sexdrive/obj/runtime/server_traps_x86_64.c.o' 'build/sexdrive/obj/runtime/mmio_traps_x86_64.c.o' 'build/sexdrive/obj/runtime/region_writer_traps_x86_64.c.o' 'build/sexdrive/obj/runtime/device_reset_traps_x86_64.c.o' 'build/sexdrive/obj/runtime/start_x86_64.S.o' 'build/sexdrive/obj/generated/runtime/format_bytes.c.o' 'build/sexdrive/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/.objects/c58c3ef33fbf0241/src/sexdrive.cesta.o: src/sexdrive.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdrive_main' '-DSEXOS_CURRENT_REGION_WRITER=1' '-DSEXOS_DMA_QUARANTINE_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/sexdrive.c' -o "$@"

build/sexdrive-dma-quarantine-probe/obj/src/sexdrive.cesta.o: build/.objects/c58c3ef33fbf0241/src/sexdrive.cesta.o
	@mkdir -p 'build/sexdrive-dma-quarantine-probe/obj/src'
	ln -f 'build/.objects/c58c3ef33fbf0241/src/sexdrive.cesta.o' "$@"

build/.objects/45534eaf6053e878/src/dma_lease.cesta.o: src/dma_lease.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdrive_main' '-DSEXOS_CURRENT_REGION_WRITER=1' '-DSEXOS_DMA_QUARANTINE_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/dma_lease.c' -o "$@"

build/sexdrive-dma-quarantine-probe/obj/src/dma_lease.cesta.o: build/.objects/45534eaf6053e878/src/dma_lease.cesta.o
	@mkdir -p 'build/sexdrive-dma-quarantine-probe/obj/src'
	ln -f 'build/.objects/45534eaf6053e878/src/dma_lease.cesta.o' "$@"

build/.objects/00ed6375ad789e52/runtime/client_runtime.c.o: runtime/client_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdrive_main' '-DSEXOS_CURRENT_REGION_WRITER=1' '-DSEXOS_DMA_QUARANTINE_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/sexdrive-dma-quarantine-probe/obj/runtime/client_runtime.c.o: build/.objects/00ed6375ad789e52/runtime/client_runtime.c.o
	@mkdir -p 'build/sexdrive-dma-quarantine-probe/obj/runtime'
	ln -f 'build/.objects/00ed6375ad789e52/runtime/client_runtime.c.o' "$@"

build/.objects/febfd909519f4546/runtime/server_runtime.c.o: runtime/server_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdrive_main' '-DSEXOS_CURRENT_REGION_WRITER=1' '-DSEXOS_DMA_QUARANTINE_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_runtime.c' -o "$@"

build/sexdrive-dma-quarantine-probe/obj/runtime/server_runtime.c.o: build/.objects/febfd909519f4546/runtime/server_runtime.c.o
	@mkdir -p 'build/sexdrive-dma-quarantine-probe/obj/runtime'
	ln -f 'build/.objects/febfd909519f4546/runtime/server_runtime.c.o' "$@"

build/.objects/3f46cb5331d3980e/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdrive_main' '-DSEXOS_CURRENT_REGION_WRITER=1' '-DSEXOS_DMA_QUARANTINE_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/sexdrive-dma-quarantine-probe/obj/runtime/traps_x86_64.c.o: build/.objects/3f46cb5331d3980e/runtime/traps_x86_64.c.o
	@mkdir -p 'build/sexdrive-dma-quarantine-probe/obj/runtime'
	ln -f 'build/.objects/3f46cb5331d3980e/runtime/traps_x86_64.c.o' "$@"

build/.objects/f9e9e5a0c33a6912/runtime/server_traps_x86_64.c.o: runtime/server_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdrive_main' '-DSEXOS_CURRENT_REGION_WRITER=1' '-DSEXOS_DMA_QUARANTINE_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_traps_x86_64.c' -o "$@"

build/sexdrive-dma-quarantine-probe/obj/runtime/server_traps_x86_64.c.o: build/.objects/f9e9e5a0c33a6912/runtime/server_traps_x86_64.c.o
	@mkdir -p 'build/sexdrive-dma-quarantine-probe/obj/runtime'
	ln -f 'build/.objects/f9e9e5a0c33a6912/runtime/server_traps_x86_64.c.o' "$@"

build/.objects/8a07b2959889e697/runtime/mmio_traps_x86_64.c.o: runtime/mmio_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdrive_main' '-DSEXOS_CURRENT_REGION_WRITER=1' '-DSEXOS_DMA_QUARANTINE_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/mmio_traps_x86_64.c' -o "$@"

build/sexdrive-dma-quarantine-probe/obj/runtime/mmio_traps_x86_64.c.o: build/.objects/8a07b2959889e697/runtime/mmio_traps_x86_64.c.o
	@mkdir -p 'build/sexdrive-dma-quarantine-probe/obj/runtime'
	ln -f 'build/.objects/8a07b2959889e697/runtime/mmio_traps_x86_64.c.o' "$@"

build/.objects/53fc252d5a204da7/runtime/region_writer_traps_x86_64.c.o: runtime/region_writer_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdrive_main' '-DSEXOS_CURRENT_REGION_WRITER=1' '-DSEXOS_DMA_QUARANTINE_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/region_writer_traps_x86_64.c' -o "$@"

build/sexdrive-dma-quarantine-probe/obj/runtime/region_writer_traps_x86_64.c.o: build/.objects/53fc252d5a204da7/runtime/region_writer_traps_x86_64.c.o
	@mkdir -p 'build/sexdrive-dma-quarantine-probe/obj/runtime'
	ln -f 'build/.objects/53fc252d5a204da7/runtime/region_writer_traps_x86_64.c.o' "$@"

build/.objects/d009dc578e808a77/runtime/device_reset_traps_x86_64.c.o: runtime/device_reset_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdrive_main' '-DSEXOS_CURRENT_REGION_WRITER=1' '-DSEXOS_DMA_QUARANTINE_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/device_reset_traps_x86_64.c' -o "$@"

build/sexdrive-dma-quarantine-probe/obj/runtime/device_reset_traps_x86_64.c.o: build/.objects/d009dc578e808a77/runtime/device_reset_traps_x86_64.c.o
	@mkdir -p 'build/sexdrive-dma-quarantine-probe/obj/runtime'
	ln -f 'build/.objects/d009dc578e808a77/runtime/device_reset_traps_x86_64.c.o' "$@"

build/.objects/8aa8337660d78a42/runtime/start_x86_64.S.o: runtime/start_x86_64.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdrive_main' '-DSEXOS_CURRENT_REGION_WRITER=1' '-DSEXOS_DMA_QUARANTINE_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/sexdrive-dma-quarantine-probe/obj/runtime/start_x86_64.S.o: build/.objects/8aa8337660d78a42/runtime/start_x86_64.S.o
	@mkdir -p 'build/sexdrive-dma-quarantine-probe/obj/runtime'
	ln -f 'build/.objects/8aa8337660d78a42/runtime/start_x86_64.S.o' "$@"

build/.objects/23464f8895a6b5c1/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdrive_main' '-DSEXOS_CURRENT_REGION_WRITER=1' '-DSEXOS_DMA_QUARANTINE_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/sexdrive-dma-quarantine-probe/obj/generated/runtime/format_bytes.c.o: build/.objects/23464f8895a6b5c1/generated/runtime/format_bytes.c.o
	@mkdir -p 'build/sexdrive-dma-quarantine-probe/obj/generated/runtime'
	ln -f 'build/.objects/23464f8895a6b5c1/generated/runtime/format_bytes.c.o' "$@"

build/.objects/fc900b04fb7fc832/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdrive_main' '-DSEXOS_CURRENT_REGION_WRITER=1' '-DSEXOS_DMA_QUARANTINE_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/sexdrive-dma-quarantine-probe/obj/generated/runtime/packed_words.c.o: build/.objects/fc900b04fb7fc832/generated/runtime/packed_words.c.o
	@mkdir -p 'build/sexdrive-dma-quarantine-probe/obj/generated/runtime'
	ln -f 'build/.objects/fc900b04fb7fc832/generated/runtime/packed_words.c.o' "$@"

build/sexdrive-dma-quarantine-probe/sexdrive-dma-quarantine-probe: GNUmakefile build/sexdrive-dma-quarantine-probe/obj/src/sexdrive.cesta.o build/sexdrive-dma-quarantine-probe/obj/src/dma_lease.cesta.o build/sexdrive-dma-quarantine-probe/obj/runtime/client_runtime.c.o build/sexdrive-dma-quarantine-probe/obj/runtime/server_runtime.c.o build/sexdrive-dma-quarantine-probe/obj/runtime/traps_x86_64.c.o build/sexdrive-dma-quarantine-probe/obj/runtime/server_traps_x86_64.c.o build/sexdrive-dma-quarantine-probe/obj/runtime/mmio_traps_x86_64.c.o build/sexdrive-dma-quarantine-probe/obj/runtime/region_writer_traps_x86_64.c.o build/sexdrive-dma-quarantine-probe/obj/runtime/device_reset_traps_x86_64.c.o build/sexdrive-dma-quarantine-probe/obj/runtime/start_x86_64.S.o build/sexdrive-dma-quarantine-probe/obj/generated/runtime/format_bytes.c.o build/sexdrive-dma-quarantine-probe/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/sexdrive-dma-quarantine-probe/obj/src/sexdrive.cesta.o' 'build/sexdrive-dma-quarantine-probe/obj/src/dma_lease.cesta.o' 'build/sexdrive-dma-quarantine-probe/obj/runtime/client_runtime.c.o' 'build/sexdrive-dma-quarantine-probe/obj/runtime/server_runtime.c.o' 'build/sexdrive-dma-quarantine-probe/obj/runtime/traps_x86_64.c.o' 'build/sexdrive-dma-quarantine-probe/obj/runtime/server_traps_x86_64.c.o' 'build/sexdrive-dma-quarantine-probe/obj/runtime/mmio_traps_x86_64.c.o' 'build/sexdrive-dma-quarantine-probe/obj/runtime/region_writer_traps_x86_64.c.o' 'build/sexdrive-dma-quarantine-probe/obj/runtime/device_reset_traps_x86_64.c.o' 'build/sexdrive-dma-quarantine-probe/obj/runtime/start_x86_64.S.o' 'build/sexdrive-dma-quarantine-probe/obj/generated/runtime/format_bytes.c.o' 'build/sexdrive-dma-quarantine-probe/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/.objects/f6a25d0e84faf1c7/src/sexusb.cesta.o: src/sexusb.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexusb_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/sexusb.c' -o "$@"

build/sexusb/obj/src/sexusb.cesta.o: build/.objects/f6a25d0e84faf1c7/src/sexusb.cesta.o
	@mkdir -p 'build/sexusb/obj/src'
	ln -f 'build/.objects/f6a25d0e84faf1c7/src/sexusb.cesta.o' "$@"

build/.objects/ec5fcc705520ae04/runtime/client_runtime.c.o: runtime/client_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexusb_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/sexusb/obj/runtime/client_runtime.c.o: build/.objects/ec5fcc705520ae04/runtime/client_runtime.c.o
	@mkdir -p 'build/sexusb/obj/runtime'
	ln -f 'build/.objects/ec5fcc705520ae04/runtime/client_runtime.c.o' "$@"

build/.objects/4567609367161408/runtime/server_runtime.c.o: runtime/server_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexusb_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_runtime.c' -o "$@"

build/sexusb/obj/runtime/server_runtime.c.o: build/.objects/4567609367161408/runtime/server_runtime.c.o
	@mkdir -p 'build/sexusb/obj/runtime'
	ln -f 'build/.objects/4567609367161408/runtime/server_runtime.c.o' "$@"

build/.objects/c2c4c2609090b560/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexusb_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/sexusb/obj/runtime/traps_x86_64.c.o: build/.objects/c2c4c2609090b560/runtime/traps_x86_64.c.o
	@mkdir -p 'build/sexusb/obj/runtime'
	ln -f 'build/.objects/c2c4c2609090b560/runtime/traps_x86_64.c.o' "$@"

build/.objects/ae45c1d7f694b9c4/runtime/server_traps_x86_64.c.o: runtime/server_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexusb_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_traps_x86_64.c' -o "$@"

build/sexusb/obj/runtime/server_traps_x86_64.c.o: build/.objects/ae45c1d7f694b9c4/runtime/server_traps_x86_64.c.o
	@mkdir -p 'build/sexusb/obj/runtime'
	ln -f 'build/.objects/ae45c1d7f694b9c4/runtime/server_traps_x86_64.c.o' "$@"

build/.objects/3ea50ea4a0c6ed63/runtime/mmio_traps_x86_64.c.o: runtime/mmio_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexusb_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/mmio_traps_x86_64.c' -o "$@"

build/sexusb/obj/runtime/mmio_traps_x86_64.c.o: build/.objects/3ea50ea4a0c6ed63/runtime/mmio_traps_x86_64.c.o
	@mkdir -p 'build/sexusb/obj/runtime'
	ln -f 'build/.objects/3ea50ea4a0c6ed63/runtime/mmio_traps_x86_64.c.o' "$@"

build/.objects/f3bdc7dbddccc9d4/runtime/start_x86_64.S.o: runtime/start_x86_64.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexusb_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/sexusb/obj/runtime/start_x86_64.S.o: build/.objects/f3bdc7dbddccc9d4/runtime/start_x86_64.S.o
	@mkdir -p 'build/sexusb/obj/runtime'
	ln -f 'build/.objects/f3bdc7dbddccc9d4/runtime/start_x86_64.S.o' "$@"

build/.objects/e1b1fc4fbeebe9e9/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexusb_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/sexusb/obj/generated/runtime/format_bytes.c.o: build/.objects/e1b1fc4fbeebe9e9/generated/runtime/format_bytes.c.o
	@mkdir -p 'build/sexusb/obj/generated/runtime'
	ln -f 'build/.objects/e1b1fc4fbeebe9e9/generated/runtime/format_bytes.c.o' "$@"

build/.objects/403db740e8ab71a4/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexusb_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/sexusb/obj/generated/runtime/packed_words.c.o: build/.objects/403db740e8ab71a4/generated/runtime/packed_words.c.o
	@mkdir -p 'build/sexusb/obj/generated/runtime'
	ln -f 'build/.objects/403db740e8ab71a4/generated/runtime/packed_words.c.o' "$@"

build/sexusb/sexusb: GNUmakefile build/sexusb/obj/src/sexusb.cesta.o build/sexusb/obj/runtime/client_runtime.c.o build/sexusb/obj/runtime/server_runtime.c.o build/sexusb/obj/runtime/traps_x86_64.c.o build/sexusb/obj/runtime/server_traps_x86_64.c.o build/sexusb/obj/runtime/mmio_traps_x86_64.c.o build/sexusb/obj/runtime/start_x86_64.S.o build/sexusb/obj/generated/runtime/format_bytes.c.o build/sexusb/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/sexusb/obj/src/sexusb.cesta.o' 'build/sexusb/obj/runtime/client_runtime.c.o' 'build/sexusb/obj/runtime/server_runtime.c.o' 'build/sexusb/obj/runtime/traps_x86_64.c.o' 'build/sexusb/obj/runtime/server_traps_x86_64.c.o' 'build/sexusb/obj/runtime/mmio_traps_x86_64.c.o' 'build/sexusb/obj/runtime/start_x86_64.S.o' 'build/sexusb/obj/generated/runtime/format_bytes.c.o' 'build/sexusb/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/.objects/8e915d7bafb6cc07/src/sexnet.cesta.o: src/sexnet.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexnet_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/sexnet.c' -o "$@"

build/sexnet/obj/src/sexnet.cesta.o: build/.objects/8e915d7bafb6cc07/src/sexnet.cesta.o
	@mkdir -p 'build/sexnet/obj/src'
	ln -f 'build/.objects/8e915d7bafb6cc07/src/sexnet.cesta.o' "$@"

build/.objects/3eceb5b4af6d5333/src/network_job_abi.cesta.o: src/network_job_abi.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexnet_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_job_abi.c' -o "$@"

build/sexnet/obj/src/network_job_abi.cesta.o: build/.objects/3eceb5b4af6d5333/src/network_job_abi.cesta.o
	@mkdir -p 'build/sexnet/obj/src'
	ln -f 'build/.objects/3eceb5b4af6d5333/src/network_job_abi.cesta.o' "$@"

build/.objects/9f5695410c728086/src/network_job_wire.cesta.o: src/network_job_wire.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexnet_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_job_wire.c' -o "$@"

build/sexnet/obj/src/network_job_wire.cesta.o: build/.objects/9f5695410c728086/src/network_job_wire.cesta.o
	@mkdir -p 'build/sexnet/obj/src'
	ln -f 'build/.objects/9f5695410c728086/src/network_job_wire.cesta.o' "$@"

build/.objects/92248878f05028ac/src/network_job_machine.cesta.o: src/network_job_machine.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexnet_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_job_machine.c' -o "$@"

build/sexnet/obj/src/network_job_machine.cesta.o: build/.objects/92248878f05028ac/src/network_job_machine.cesta.o
	@mkdir -p 'build/sexnet/obj/src'
	ln -f 'build/.objects/92248878f05028ac/src/network_job_machine.cesta.o' "$@"

build/.objects/ef555c840897231f/src/network_dns_job_abi.cesta.o: src/network_dns_job_abi.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexnet_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_dns_job_abi.c' -o "$@"

build/sexnet/obj/src/network_dns_job_abi.cesta.o: build/.objects/ef555c840897231f/src/network_dns_job_abi.cesta.o
	@mkdir -p 'build/sexnet/obj/src'
	ln -f 'build/.objects/ef555c840897231f/src/network_dns_job_abi.cesta.o' "$@"

build/.objects/2f1cad95d02ec6ba/src/network_dns_job_wire.cesta.o: src/network_dns_job_wire.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexnet_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_dns_job_wire.c' -o "$@"

build/sexnet/obj/src/network_dns_job_wire.cesta.o: build/.objects/2f1cad95d02ec6ba/src/network_dns_job_wire.cesta.o
	@mkdir -p 'build/sexnet/obj/src'
	ln -f 'build/.objects/2f1cad95d02ec6ba/src/network_dns_job_wire.cesta.o' "$@"

build/.objects/68690a23e82ab490/src/network_dns_job_machine.cesta.o: src/network_dns_job_machine.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexnet_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_dns_job_machine.c' -o "$@"

build/sexnet/obj/src/network_dns_job_machine.cesta.o: build/.objects/68690a23e82ab490/src/network_dns_job_machine.cesta.o
	@mkdir -p 'build/sexnet/obj/src'
	ln -f 'build/.objects/68690a23e82ab490/src/network_dns_job_machine.cesta.o' "$@"

build/.objects/627cca4e7a94f945/src/network_config.cesta.o: src/network_config.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexnet_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_config.c' -o "$@"

build/sexnet/obj/src/network_config.cesta.o: build/.objects/627cca4e7a94f945/src/network_config.cesta.o
	@mkdir -p 'build/sexnet/obj/src'
	ln -f 'build/.objects/627cca4e7a94f945/src/network_config.cesta.o' "$@"

build/.objects/180d67fa00ac4d6f/src/network_engine_frame.cesta.o: src/network_engine_frame.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexnet_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_engine_frame.c' -o "$@"

build/sexnet/obj/src/network_engine_frame.cesta.o: build/.objects/180d67fa00ac4d6f/src/network_engine_frame.cesta.o
	@mkdir -p 'build/sexnet/obj/src'
	ln -f 'build/.objects/180d67fa00ac4d6f/src/network_engine_frame.cesta.o' "$@"

build/.objects/0e577519b3af1656/src/network_engine_packet.cesta.o: src/network_engine_packet.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexnet_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_engine_packet.c' -o "$@"

build/sexnet/obj/src/network_engine_packet.cesta.o: build/.objects/0e577519b3af1656/src/network_engine_packet.cesta.o
	@mkdir -p 'build/sexnet/obj/src'
	ln -f 'build/.objects/0e577519b3af1656/src/network_engine_packet.cesta.o' "$@"

build/.objects/bfb9b98f70f2e79e/src/network_engine_tcp_packet.cesta.o: src/network_engine_tcp_packet.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexnet_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_engine_tcp_packet.c' -o "$@"

build/sexnet/obj/src/network_engine_tcp_packet.cesta.o: build/.objects/bfb9b98f70f2e79e/src/network_engine_tcp_packet.cesta.o
	@mkdir -p 'build/sexnet/obj/src'
	ln -f 'build/.objects/bfb9b98f70f2e79e/src/network_engine_tcp_packet.cesta.o' "$@"

build/.objects/bf0af8f1cf595e28/src/network_tcp_client.cesta.o: src/network_tcp_client.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexnet_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_tcp_client.c' -o "$@"

build/sexnet/obj/src/network_tcp_client.cesta.o: build/.objects/bf0af8f1cf595e28/src/network_tcp_client.cesta.o
	@mkdir -p 'build/sexnet/obj/src'
	ln -f 'build/.objects/bf0af8f1cf595e28/src/network_tcp_client.cesta.o' "$@"

build/.objects/90030264f91ff89f/src/network_tcp_listener.cesta.o: src/network_tcp_listener.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexnet_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_tcp_listener.c' -o "$@"

build/sexnet/obj/src/network_tcp_listener.cesta.o: build/.objects/90030264f91ff89f/src/network_tcp_listener.cesta.o
	@mkdir -p 'build/sexnet/obj/src'
	ln -f 'build/.objects/90030264f91ff89f/src/network_tcp_listener.cesta.o' "$@"

build/.objects/7d46a94af557ea20/src/network_engine_neighbor.cesta.o: src/network_engine_neighbor.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexnet_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_engine_neighbor.c' -o "$@"

build/sexnet/obj/src/network_engine_neighbor.cesta.o: build/.objects/7d46a94af557ea20/src/network_engine_neighbor.cesta.o
	@mkdir -p 'build/sexnet/obj/src'
	ln -f 'build/.objects/7d46a94af557ea20/src/network_engine_neighbor.cesta.o' "$@"

build/.objects/007a6cdb4c056c6f/src/network_dns_codec.cesta.o: src/network_dns_codec.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexnet_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_dns_codec.c' -o "$@"

build/sexnet/obj/src/network_dns_codec.cesta.o: build/.objects/007a6cdb4c056c6f/src/network_dns_codec.cesta.o
	@mkdir -p 'build/sexnet/obj/src'
	ln -f 'build/.objects/007a6cdb4c056c6f/src/network_dns_codec.cesta.o' "$@"

build/.objects/3d1826956711f7c9/src/network_engine_dns.cesta.o: src/network_engine_dns.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexnet_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_engine_dns.c' -o "$@"

build/sexnet/obj/src/network_engine_dns.cesta.o: build/.objects/3d1826956711f7c9/src/network_engine_dns.cesta.o
	@mkdir -p 'build/sexnet/obj/src'
	ln -f 'build/.objects/3d1826956711f7c9/src/network_engine_dns.cesta.o' "$@"

build/.objects/251617bc120f4e67/src/network_engine_transport.cesta.o: src/network_engine_transport.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexnet_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_engine_transport.c' -o "$@"

build/sexnet/obj/src/network_engine_transport.cesta.o: build/.objects/251617bc120f4e67/src/network_engine_transport.cesta.o
	@mkdir -p 'build/sexnet/obj/src'
	ln -f 'build/.objects/251617bc120f4e67/src/network_engine_transport.cesta.o' "$@"

build/.objects/b826d5b0cd4a9c54/src/network_job_engine_policy.cesta.o: src/network_job_engine_policy.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexnet_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_job_engine_policy.c' -o "$@"

build/sexnet/obj/src/network_job_engine_policy.cesta.o: build/.objects/b826d5b0cd4a9c54/src/network_job_engine_policy.cesta.o
	@mkdir -p 'build/sexnet/obj/src'
	ln -f 'build/.objects/b826d5b0cd4a9c54/src/network_job_engine_policy.cesta.o' "$@"

build/.objects/8d741e2cb603a265/src/network_job_engine_adapter.cesta.o: src/network_job_engine_adapter.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexnet_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_job_engine_adapter.c' -o "$@"

build/sexnet/obj/src/network_job_engine_adapter.cesta.o: build/.objects/8d741e2cb603a265/src/network_job_engine_adapter.cesta.o
	@mkdir -p 'build/sexnet/obj/src'
	ln -f 'build/.objects/8d741e2cb603a265/src/network_job_engine_adapter.cesta.o' "$@"

build/.objects/343a1fa44876c3d1/src/network_dns_job_engine_adapter.cesta.o: src/network_dns_job_engine_adapter.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexnet_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_dns_job_engine_adapter.c' -o "$@"

build/sexnet/obj/src/network_dns_job_engine_adapter.cesta.o: build/.objects/343a1fa44876c3d1/src/network_dns_job_engine_adapter.cesta.o
	@mkdir -p 'build/sexnet/obj/src'
	ln -f 'build/.objects/343a1fa44876c3d1/src/network_dns_job_engine_adapter.cesta.o' "$@"

build/.objects/5b6b39e572778898/src/network_service_native.cesta.o: src/network_service_native.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexnet_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_service_native.c' -o "$@"

build/sexnet/obj/src/network_service_native.cesta.o: build/.objects/5b6b39e572778898/src/network_service_native.cesta.o
	@mkdir -p 'build/sexnet/obj/src'
	ln -f 'build/.objects/5b6b39e572778898/src/network_service_native.cesta.o' "$@"

build/.objects/4844fe243adc1f24/src/network_service_core.cesta.o: src/network_service_core.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexnet_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_service_core.c' -o "$@"

build/sexnet/obj/src/network_service_core.cesta.o: build/.objects/4844fe243adc1f24/src/network_service_core.cesta.o
	@mkdir -p 'build/sexnet/obj/src'
	ln -f 'build/.objects/4844fe243adc1f24/src/network_service_core.cesta.o' "$@"

build/.objects/9b96c825ed2f0ada/src/network_dns_job_liveness.cesta.o: src/network_dns_job_liveness.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexnet_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_dns_job_liveness.c' -o "$@"

build/sexnet/obj/src/network_dns_job_liveness.cesta.o: build/.objects/9b96c825ed2f0ada/src/network_dns_job_liveness.cesta.o
	@mkdir -p 'build/sexnet/obj/src'
	ln -f 'build/.objects/9b96c825ed2f0ada/src/network_dns_job_liveness.cesta.o' "$@"

build/.objects/334680c75bbc97b5/runtime/client_runtime.c.o: runtime/client_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexnet_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/sexnet/obj/runtime/client_runtime.c.o: build/.objects/334680c75bbc97b5/runtime/client_runtime.c.o
	@mkdir -p 'build/sexnet/obj/runtime'
	ln -f 'build/.objects/334680c75bbc97b5/runtime/client_runtime.c.o' "$@"

build/.objects/51b5e008a03562e9/runtime/server_runtime.c.o: runtime/server_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexnet_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_runtime.c' -o "$@"

build/sexnet/obj/runtime/server_runtime.c.o: build/.objects/51b5e008a03562e9/runtime/server_runtime.c.o
	@mkdir -p 'build/sexnet/obj/runtime'
	ln -f 'build/.objects/51b5e008a03562e9/runtime/server_runtime.c.o' "$@"

build/.objects/4db7939580c31531/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexnet_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/sexnet/obj/runtime/traps_x86_64.c.o: build/.objects/4db7939580c31531/runtime/traps_x86_64.c.o
	@mkdir -p 'build/sexnet/obj/runtime'
	ln -f 'build/.objects/4db7939580c31531/runtime/traps_x86_64.c.o' "$@"

build/.objects/9f4aebdd267a9f75/runtime/server_traps_x86_64.c.o: runtime/server_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexnet_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_traps_x86_64.c' -o "$@"

build/sexnet/obj/runtime/server_traps_x86_64.c.o: build/.objects/9f4aebdd267a9f75/runtime/server_traps_x86_64.c.o
	@mkdir -p 'build/sexnet/obj/runtime'
	ln -f 'build/.objects/9f4aebdd267a9f75/runtime/server_traps_x86_64.c.o' "$@"

build/.objects/318879ba7803ad1a/runtime/mmio_traps_x86_64.c.o: runtime/mmio_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexnet_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/mmio_traps_x86_64.c' -o "$@"

build/sexnet/obj/runtime/mmio_traps_x86_64.c.o: build/.objects/318879ba7803ad1a/runtime/mmio_traps_x86_64.c.o
	@mkdir -p 'build/sexnet/obj/runtime'
	ln -f 'build/.objects/318879ba7803ad1a/runtime/mmio_traps_x86_64.c.o' "$@"

build/.objects/31690fe2979d5065/runtime/start_x86_64.S.o: runtime/start_x86_64.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexnet_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/sexnet/obj/runtime/start_x86_64.S.o: build/.objects/31690fe2979d5065/runtime/start_x86_64.S.o
	@mkdir -p 'build/sexnet/obj/runtime'
	ln -f 'build/.objects/31690fe2979d5065/runtime/start_x86_64.S.o' "$@"

build/.objects/e2878123870062f4/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexnet_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/sexnet/obj/generated/runtime/format_bytes.c.o: build/.objects/e2878123870062f4/generated/runtime/format_bytes.c.o
	@mkdir -p 'build/sexnet/obj/generated/runtime'
	ln -f 'build/.objects/e2878123870062f4/generated/runtime/format_bytes.c.o' "$@"

build/.objects/87246b97ef475b55/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexnet_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/sexnet/obj/generated/runtime/packed_words.c.o: build/.objects/87246b97ef475b55/generated/runtime/packed_words.c.o
	@mkdir -p 'build/sexnet/obj/generated/runtime'
	ln -f 'build/.objects/87246b97ef475b55/generated/runtime/packed_words.c.o' "$@"

build/sexnet/sexnet: GNUmakefile build/sexnet/obj/src/sexnet.cesta.o build/sexnet/obj/src/network_job_abi.cesta.o build/sexnet/obj/src/network_job_wire.cesta.o build/sexnet/obj/src/network_job_machine.cesta.o build/sexnet/obj/src/network_dns_job_abi.cesta.o build/sexnet/obj/src/network_dns_job_wire.cesta.o build/sexnet/obj/src/network_dns_job_machine.cesta.o build/sexnet/obj/src/network_config.cesta.o build/sexnet/obj/src/network_engine_frame.cesta.o build/sexnet/obj/src/network_engine_packet.cesta.o build/sexnet/obj/src/network_engine_tcp_packet.cesta.o build/sexnet/obj/src/network_tcp_client.cesta.o build/sexnet/obj/src/network_tcp_listener.cesta.o build/sexnet/obj/src/network_engine_neighbor.cesta.o build/sexnet/obj/src/network_dns_codec.cesta.o build/sexnet/obj/src/network_engine_dns.cesta.o build/sexnet/obj/src/network_engine_transport.cesta.o build/sexnet/obj/src/network_job_engine_policy.cesta.o build/sexnet/obj/src/network_job_engine_adapter.cesta.o build/sexnet/obj/src/network_dns_job_engine_adapter.cesta.o build/sexnet/obj/src/network_service_native.cesta.o build/sexnet/obj/src/network_service_core.cesta.o build/sexnet/obj/src/network_dns_job_liveness.cesta.o build/sexnet/obj/runtime/client_runtime.c.o build/sexnet/obj/runtime/server_runtime.c.o build/sexnet/obj/runtime/traps_x86_64.c.o build/sexnet/obj/runtime/server_traps_x86_64.c.o build/sexnet/obj/runtime/mmio_traps_x86_64.c.o build/sexnet/obj/runtime/start_x86_64.S.o build/sexnet/obj/generated/runtime/format_bytes.c.o build/sexnet/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/sexnet/obj/src/sexnet.cesta.o' 'build/sexnet/obj/src/network_job_abi.cesta.o' 'build/sexnet/obj/src/network_job_wire.cesta.o' 'build/sexnet/obj/src/network_job_machine.cesta.o' 'build/sexnet/obj/src/network_dns_job_abi.cesta.o' 'build/sexnet/obj/src/network_dns_job_wire.cesta.o' 'build/sexnet/obj/src/network_dns_job_machine.cesta.o' 'build/sexnet/obj/src/network_config.cesta.o' 'build/sexnet/obj/src/network_engine_frame.cesta.o' 'build/sexnet/obj/src/network_engine_packet.cesta.o' 'build/sexnet/obj/src/network_engine_tcp_packet.cesta.o' 'build/sexnet/obj/src/network_tcp_client.cesta.o' 'build/sexnet/obj/src/network_tcp_listener.cesta.o' 'build/sexnet/obj/src/network_engine_neighbor.cesta.o' 'build/sexnet/obj/src/network_dns_codec.cesta.o' 'build/sexnet/obj/src/network_engine_dns.cesta.o' 'build/sexnet/obj/src/network_engine_transport.cesta.o' 'build/sexnet/obj/src/network_job_engine_policy.cesta.o' 'build/sexnet/obj/src/network_job_engine_adapter.cesta.o' 'build/sexnet/obj/src/network_dns_job_engine_adapter.cesta.o' 'build/sexnet/obj/src/network_service_native.cesta.o' 'build/sexnet/obj/src/network_service_core.cesta.o' 'build/sexnet/obj/src/network_dns_job_liveness.cesta.o' 'build/sexnet/obj/runtime/client_runtime.c.o' 'build/sexnet/obj/runtime/server_runtime.c.o' 'build/sexnet/obj/runtime/traps_x86_64.c.o' 'build/sexnet/obj/runtime/server_traps_x86_64.c.o' 'build/sexnet/obj/runtime/mmio_traps_x86_64.c.o' 'build/sexnet/obj/runtime/start_x86_64.S.o' 'build/sexnet/obj/generated/runtime/format_bytes.c.o' 'build/sexnet/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/.objects/fb5d138511f21649/src/network_browser.cesta.o: src/network_browser.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_browser_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_browser.c' -o "$@"

build/kaleidoscope-network/obj/src/network_browser.cesta.o: build/.objects/fb5d138511f21649/src/network_browser.cesta.o
	@mkdir -p 'build/kaleidoscope-network/obj/src'
	ln -f 'build/.objects/fb5d138511f21649/src/network_browser.cesta.o' "$@"

build/.objects/d1f208141784c2b3/src/network_observation.cesta.o: src/network_observation.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_browser_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_observation.c' -o "$@"

build/kaleidoscope-network/obj/src/network_observation.cesta.o: build/.objects/d1f208141784c2b3/src/network_observation.cesta.o
	@mkdir -p 'build/kaleidoscope-network/obj/src'
	ln -f 'build/.objects/d1f208141784c2b3/src/network_observation.cesta.o' "$@"

build/.objects/6f996727ff1d4d6e/src/rpc_runtime.cesta.o: src/rpc_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_browser_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/rpc_runtime.c' -o "$@"

build/kaleidoscope-network/obj/src/rpc_runtime.cesta.o: build/.objects/6f996727ff1d4d6e/src/rpc_runtime.cesta.o
	@mkdir -p 'build/kaleidoscope-network/obj/src'
	ln -f 'build/.objects/6f996727ff1d4d6e/src/rpc_runtime.cesta.o' "$@"

build/.objects/35ba9b4754952b21/src/rpc_wait.cesta.o: src/rpc_wait.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_browser_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/rpc_wait.c' -o "$@"

build/kaleidoscope-network/obj/src/rpc_wait.cesta.o: build/.objects/35ba9b4754952b21/src/rpc_wait.cesta.o
	@mkdir -p 'build/kaleidoscope-network/obj/src'
	ln -f 'build/.objects/35ba9b4754952b21/src/rpc_wait.cesta.o' "$@"

build/.objects/0622bc66be197e91/runtime/client_runtime.c.o: runtime/client_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_browser_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/kaleidoscope-network/obj/runtime/client_runtime.c.o: build/.objects/0622bc66be197e91/runtime/client_runtime.c.o
	@mkdir -p 'build/kaleidoscope-network/obj/runtime'
	ln -f 'build/.objects/0622bc66be197e91/runtime/client_runtime.c.o' "$@"

build/.objects/9f80bccf9067b7fd/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_browser_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/kaleidoscope-network/obj/runtime/traps_x86_64.c.o: build/.objects/9f80bccf9067b7fd/runtime/traps_x86_64.c.o
	@mkdir -p 'build/kaleidoscope-network/obj/runtime'
	ln -f 'build/.objects/9f80bccf9067b7fd/runtime/traps_x86_64.c.o' "$@"

build/.objects/5f0134b7a12a8351/runtime/server_traps_x86_64.c.o: runtime/server_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_browser_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_traps_x86_64.c' -o "$@"

build/kaleidoscope-network/obj/runtime/server_traps_x86_64.c.o: build/.objects/5f0134b7a12a8351/runtime/server_traps_x86_64.c.o
	@mkdir -p 'build/kaleidoscope-network/obj/runtime'
	ln -f 'build/.objects/5f0134b7a12a8351/runtime/server_traps_x86_64.c.o' "$@"

build/.objects/6b8e8352ea7353a9/runtime/rpc_traps_x86_64.c.o: runtime/rpc_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_browser_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/rpc_traps_x86_64.c' -o "$@"

build/kaleidoscope-network/obj/runtime/rpc_traps_x86_64.c.o: build/.objects/6b8e8352ea7353a9/runtime/rpc_traps_x86_64.c.o
	@mkdir -p 'build/kaleidoscope-network/obj/runtime'
	ln -f 'build/.objects/6b8e8352ea7353a9/runtime/rpc_traps_x86_64.c.o' "$@"

build/.objects/a1ff3500248a8601/runtime/start_x86_64.S.o: runtime/start_x86_64.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_browser_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/kaleidoscope-network/obj/runtime/start_x86_64.S.o: build/.objects/a1ff3500248a8601/runtime/start_x86_64.S.o
	@mkdir -p 'build/kaleidoscope-network/obj/runtime'
	ln -f 'build/.objects/a1ff3500248a8601/runtime/start_x86_64.S.o' "$@"

build/.objects/9d7649c9c3c893ce/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_browser_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/kaleidoscope-network/obj/generated/runtime/format_bytes.c.o: build/.objects/9d7649c9c3c893ce/generated/runtime/format_bytes.c.o
	@mkdir -p 'build/kaleidoscope-network/obj/generated/runtime'
	ln -f 'build/.objects/9d7649c9c3c893ce/generated/runtime/format_bytes.c.o' "$@"

build/.objects/df75ded7d79ff471/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_browser_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/kaleidoscope-network/obj/generated/runtime/packed_words.c.o: build/.objects/df75ded7d79ff471/generated/runtime/packed_words.c.o
	@mkdir -p 'build/kaleidoscope-network/obj/generated/runtime'
	ln -f 'build/.objects/df75ded7d79ff471/generated/runtime/packed_words.c.o' "$@"

build/kaleidoscope-network/kaleidoscope-network: GNUmakefile build/kaleidoscope-network/obj/src/network_browser.cesta.o build/kaleidoscope-network/obj/src/network_observation.cesta.o build/kaleidoscope-network/obj/src/rpc_runtime.cesta.o build/kaleidoscope-network/obj/src/rpc_wait.cesta.o build/kaleidoscope-network/obj/runtime/client_runtime.c.o build/kaleidoscope-network/obj/runtime/traps_x86_64.c.o build/kaleidoscope-network/obj/runtime/server_traps_x86_64.c.o build/kaleidoscope-network/obj/runtime/rpc_traps_x86_64.c.o build/kaleidoscope-network/obj/runtime/start_x86_64.S.o build/kaleidoscope-network/obj/generated/runtime/format_bytes.c.o build/kaleidoscope-network/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/kaleidoscope-network/obj/src/network_browser.cesta.o' 'build/kaleidoscope-network/obj/src/network_observation.cesta.o' 'build/kaleidoscope-network/obj/src/rpc_runtime.cesta.o' 'build/kaleidoscope-network/obj/src/rpc_wait.cesta.o' 'build/kaleidoscope-network/obj/runtime/client_runtime.c.o' 'build/kaleidoscope-network/obj/runtime/traps_x86_64.c.o' 'build/kaleidoscope-network/obj/runtime/server_traps_x86_64.c.o' 'build/kaleidoscope-network/obj/runtime/rpc_traps_x86_64.c.o' 'build/kaleidoscope-network/obj/runtime/start_x86_64.S.o' 'build/kaleidoscope-network/obj/generated/runtime/format_bytes.c.o' 'build/kaleidoscope-network/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/.objects/e41d83cef6424d0f/src/network_protocol_probe.cesta.o: src/network_protocol_probe.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_protocol_probe_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_protocol_probe.c' -o "$@"

build/network-protocol-probe/obj/src/network_protocol_probe.cesta.o: build/.objects/e41d83cef6424d0f/src/network_protocol_probe.cesta.o
	@mkdir -p 'build/network-protocol-probe/obj/src'
	ln -f 'build/.objects/e41d83cef6424d0f/src/network_protocol_probe.cesta.o' "$@"

build/.objects/757f1163dd766626/src/network_job_abi.cesta.o: src/network_job_abi.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_protocol_probe_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_job_abi.c' -o "$@"

build/network-protocol-probe/obj/src/network_job_abi.cesta.o: build/.objects/757f1163dd766626/src/network_job_abi.cesta.o
	@mkdir -p 'build/network-protocol-probe/obj/src'
	ln -f 'build/.objects/757f1163dd766626/src/network_job_abi.cesta.o' "$@"

build/.objects/2065401b6e4a3754/runtime/client_runtime.c.o: runtime/client_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_protocol_probe_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/network-protocol-probe/obj/runtime/client_runtime.c.o: build/.objects/2065401b6e4a3754/runtime/client_runtime.c.o
	@mkdir -p 'build/network-protocol-probe/obj/runtime'
	ln -f 'build/.objects/2065401b6e4a3754/runtime/client_runtime.c.o' "$@"

build/.objects/fb850bf0b3040b20/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_protocol_probe_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/network-protocol-probe/obj/runtime/traps_x86_64.c.o: build/.objects/fb850bf0b3040b20/runtime/traps_x86_64.c.o
	@mkdir -p 'build/network-protocol-probe/obj/runtime'
	ln -f 'build/.objects/fb850bf0b3040b20/runtime/traps_x86_64.c.o' "$@"

build/.objects/62b68b96fb4136cc/runtime/rpc_traps_x86_64.c.o: runtime/rpc_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_protocol_probe_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/rpc_traps_x86_64.c' -o "$@"

build/network-protocol-probe/obj/runtime/rpc_traps_x86_64.c.o: build/.objects/62b68b96fb4136cc/runtime/rpc_traps_x86_64.c.o
	@mkdir -p 'build/network-protocol-probe/obj/runtime'
	ln -f 'build/.objects/62b68b96fb4136cc/runtime/rpc_traps_x86_64.c.o' "$@"

build/.objects/5c5874eb51449524/runtime/start_x86_64.S.o: runtime/start_x86_64.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_protocol_probe_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/network-protocol-probe/obj/runtime/start_x86_64.S.o: build/.objects/5c5874eb51449524/runtime/start_x86_64.S.o
	@mkdir -p 'build/network-protocol-probe/obj/runtime'
	ln -f 'build/.objects/5c5874eb51449524/runtime/start_x86_64.S.o' "$@"

build/.objects/ab68de999b00d795/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_protocol_probe_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/network-protocol-probe/obj/generated/runtime/format_bytes.c.o: build/.objects/ab68de999b00d795/generated/runtime/format_bytes.c.o
	@mkdir -p 'build/network-protocol-probe/obj/generated/runtime'
	ln -f 'build/.objects/ab68de999b00d795/generated/runtime/format_bytes.c.o' "$@"

build/.objects/7df85f02daf82174/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_protocol_probe_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/network-protocol-probe/obj/generated/runtime/packed_words.c.o: build/.objects/7df85f02daf82174/generated/runtime/packed_words.c.o
	@mkdir -p 'build/network-protocol-probe/obj/generated/runtime'
	ln -f 'build/.objects/7df85f02daf82174/generated/runtime/packed_words.c.o' "$@"

build/network-protocol-probe/network-protocol-probe: GNUmakefile build/network-protocol-probe/obj/src/network_protocol_probe.cesta.o build/network-protocol-probe/obj/src/network_job_abi.cesta.o build/network-protocol-probe/obj/runtime/client_runtime.c.o build/network-protocol-probe/obj/runtime/traps_x86_64.c.o build/network-protocol-probe/obj/runtime/rpc_traps_x86_64.c.o build/network-protocol-probe/obj/runtime/start_x86_64.S.o build/network-protocol-probe/obj/generated/runtime/format_bytes.c.o build/network-protocol-probe/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/network-protocol-probe/obj/src/network_protocol_probe.cesta.o' 'build/network-protocol-probe/obj/src/network_job_abi.cesta.o' 'build/network-protocol-probe/obj/runtime/client_runtime.c.o' 'build/network-protocol-probe/obj/runtime/traps_x86_64.c.o' 'build/network-protocol-probe/obj/runtime/rpc_traps_x86_64.c.o' 'build/network-protocol-probe/obj/runtime/start_x86_64.S.o' 'build/network-protocol-probe/obj/generated/runtime/format_bytes.c.o' 'build/network-protocol-probe/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/.objects/a07cec5d3fb9aabb/src/network_dns_protocol_probe.cesta.o: src/network_dns_protocol_probe.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_dns_protocol_probe_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_dns_protocol_probe.c' -o "$@"

build/network-dns-protocol-probe/obj/src/network_dns_protocol_probe.cesta.o: build/.objects/a07cec5d3fb9aabb/src/network_dns_protocol_probe.cesta.o
	@mkdir -p 'build/network-dns-protocol-probe/obj/src'
	ln -f 'build/.objects/a07cec5d3fb9aabb/src/network_dns_protocol_probe.cesta.o' "$@"

build/.objects/1ba90a6b9ea3a2ca/src/network_job_abi.cesta.o: src/network_job_abi.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_dns_protocol_probe_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_job_abi.c' -o "$@"

build/network-dns-protocol-probe/obj/src/network_job_abi.cesta.o: build/.objects/1ba90a6b9ea3a2ca/src/network_job_abi.cesta.o
	@mkdir -p 'build/network-dns-protocol-probe/obj/src'
	ln -f 'build/.objects/1ba90a6b9ea3a2ca/src/network_job_abi.cesta.o' "$@"

build/.objects/47acb57384d55ede/src/network_dns_job_abi.cesta.o: src/network_dns_job_abi.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_dns_protocol_probe_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_dns_job_abi.c' -o "$@"

build/network-dns-protocol-probe/obj/src/network_dns_job_abi.cesta.o: build/.objects/47acb57384d55ede/src/network_dns_job_abi.cesta.o
	@mkdir -p 'build/network-dns-protocol-probe/obj/src'
	ln -f 'build/.objects/47acb57384d55ede/src/network_dns_job_abi.cesta.o' "$@"

build/.objects/be689d27fca8f7cc/runtime/client_runtime.c.o: runtime/client_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_dns_protocol_probe_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/network-dns-protocol-probe/obj/runtime/client_runtime.c.o: build/.objects/be689d27fca8f7cc/runtime/client_runtime.c.o
	@mkdir -p 'build/network-dns-protocol-probe/obj/runtime'
	ln -f 'build/.objects/be689d27fca8f7cc/runtime/client_runtime.c.o' "$@"

build/.objects/ad95784d31e23250/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_dns_protocol_probe_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/network-dns-protocol-probe/obj/runtime/traps_x86_64.c.o: build/.objects/ad95784d31e23250/runtime/traps_x86_64.c.o
	@mkdir -p 'build/network-dns-protocol-probe/obj/runtime'
	ln -f 'build/.objects/ad95784d31e23250/runtime/traps_x86_64.c.o' "$@"

build/.objects/6b01f4ff1fc05e84/runtime/rpc_traps_x86_64.c.o: runtime/rpc_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_dns_protocol_probe_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/rpc_traps_x86_64.c' -o "$@"

build/network-dns-protocol-probe/obj/runtime/rpc_traps_x86_64.c.o: build/.objects/6b01f4ff1fc05e84/runtime/rpc_traps_x86_64.c.o
	@mkdir -p 'build/network-dns-protocol-probe/obj/runtime'
	ln -f 'build/.objects/6b01f4ff1fc05e84/runtime/rpc_traps_x86_64.c.o' "$@"

build/.objects/c95c4058e3224e9c/runtime/start_x86_64.S.o: runtime/start_x86_64.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_dns_protocol_probe_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/network-dns-protocol-probe/obj/runtime/start_x86_64.S.o: build/.objects/c95c4058e3224e9c/runtime/start_x86_64.S.o
	@mkdir -p 'build/network-dns-protocol-probe/obj/runtime'
	ln -f 'build/.objects/c95c4058e3224e9c/runtime/start_x86_64.S.o' "$@"

build/.objects/a25f538c8426ed69/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_dns_protocol_probe_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/network-dns-protocol-probe/obj/generated/runtime/format_bytes.c.o: build/.objects/a25f538c8426ed69/generated/runtime/format_bytes.c.o
	@mkdir -p 'build/network-dns-protocol-probe/obj/generated/runtime'
	ln -f 'build/.objects/a25f538c8426ed69/generated/runtime/format_bytes.c.o' "$@"

build/.objects/4cf545cfffc6ad2c/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_dns_protocol_probe_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/network-dns-protocol-probe/obj/generated/runtime/packed_words.c.o: build/.objects/4cf545cfffc6ad2c/generated/runtime/packed_words.c.o
	@mkdir -p 'build/network-dns-protocol-probe/obj/generated/runtime'
	ln -f 'build/.objects/4cf545cfffc6ad2c/generated/runtime/packed_words.c.o' "$@"

build/network-dns-protocol-probe/network-dns-protocol-probe: GNUmakefile build/network-dns-protocol-probe/obj/src/network_dns_protocol_probe.cesta.o build/network-dns-protocol-probe/obj/src/network_job_abi.cesta.o build/network-dns-protocol-probe/obj/src/network_dns_job_abi.cesta.o build/network-dns-protocol-probe/obj/runtime/client_runtime.c.o build/network-dns-protocol-probe/obj/runtime/traps_x86_64.c.o build/network-dns-protocol-probe/obj/runtime/rpc_traps_x86_64.c.o build/network-dns-protocol-probe/obj/runtime/start_x86_64.S.o build/network-dns-protocol-probe/obj/generated/runtime/format_bytes.c.o build/network-dns-protocol-probe/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/network-dns-protocol-probe/obj/src/network_dns_protocol_probe.cesta.o' 'build/network-dns-protocol-probe/obj/src/network_job_abi.cesta.o' 'build/network-dns-protocol-probe/obj/src/network_dns_job_abi.cesta.o' 'build/network-dns-protocol-probe/obj/runtime/client_runtime.c.o' 'build/network-dns-protocol-probe/obj/runtime/traps_x86_64.c.o' 'build/network-dns-protocol-probe/obj/runtime/rpc_traps_x86_64.c.o' 'build/network-dns-protocol-probe/obj/runtime/start_x86_64.S.o' 'build/network-dns-protocol-probe/obj/generated/runtime/format_bytes.c.o' 'build/network-dns-protocol-probe/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/.objects/df60facc18c94c03/src/network_web_protocol_probe.cesta.o: src/network_web_protocol_probe.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_web_protocol_probe_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_web_protocol_probe.c' -o "$@"

build/network-web-protocol-probe/obj/src/network_web_protocol_probe.cesta.o: build/.objects/df60facc18c94c03/src/network_web_protocol_probe.cesta.o
	@mkdir -p 'build/network-web-protocol-probe/obj/src'
	ln -f 'build/.objects/df60facc18c94c03/src/network_web_protocol_probe.cesta.o' "$@"

build/.objects/8c984f9b72b9fac7/runtime/client_runtime.c.o: runtime/client_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_web_protocol_probe_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/network-web-protocol-probe/obj/runtime/client_runtime.c.o: build/.objects/8c984f9b72b9fac7/runtime/client_runtime.c.o
	@mkdir -p 'build/network-web-protocol-probe/obj/runtime'
	ln -f 'build/.objects/8c984f9b72b9fac7/runtime/client_runtime.c.o' "$@"

build/.objects/faedc500b2bd3d4b/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_web_protocol_probe_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/network-web-protocol-probe/obj/runtime/traps_x86_64.c.o: build/.objects/faedc500b2bd3d4b/runtime/traps_x86_64.c.o
	@mkdir -p 'build/network-web-protocol-probe/obj/runtime'
	ln -f 'build/.objects/faedc500b2bd3d4b/runtime/traps_x86_64.c.o' "$@"

build/.objects/91c9bae0bd9b384f/runtime/rpc_traps_x86_64.c.o: runtime/rpc_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_web_protocol_probe_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/rpc_traps_x86_64.c' -o "$@"

build/network-web-protocol-probe/obj/runtime/rpc_traps_x86_64.c.o: build/.objects/91c9bae0bd9b384f/runtime/rpc_traps_x86_64.c.o
	@mkdir -p 'build/network-web-protocol-probe/obj/runtime'
	ln -f 'build/.objects/91c9bae0bd9b384f/runtime/rpc_traps_x86_64.c.o' "$@"

build/.objects/5dd3e89018e476c8/runtime/region_writer_traps_x86_64.c.o: runtime/region_writer_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_web_protocol_probe_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/region_writer_traps_x86_64.c' -o "$@"

build/network-web-protocol-probe/obj/runtime/region_writer_traps_x86_64.c.o: build/.objects/5dd3e89018e476c8/runtime/region_writer_traps_x86_64.c.o
	@mkdir -p 'build/network-web-protocol-probe/obj/runtime'
	ln -f 'build/.objects/5dd3e89018e476c8/runtime/region_writer_traps_x86_64.c.o' "$@"

build/.objects/481a512481549417/runtime/start_x86_64.S.o: runtime/start_x86_64.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_web_protocol_probe_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/network-web-protocol-probe/obj/runtime/start_x86_64.S.o: build/.objects/481a512481549417/runtime/start_x86_64.S.o
	@mkdir -p 'build/network-web-protocol-probe/obj/runtime'
	ln -f 'build/.objects/481a512481549417/runtime/start_x86_64.S.o' "$@"

build/.objects/8214953be79f5b9a/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_web_protocol_probe_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/network-web-protocol-probe/obj/generated/runtime/format_bytes.c.o: build/.objects/8214953be79f5b9a/generated/runtime/format_bytes.c.o
	@mkdir -p 'build/network-web-protocol-probe/obj/generated/runtime'
	ln -f 'build/.objects/8214953be79f5b9a/generated/runtime/format_bytes.c.o' "$@"

build/.objects/f37ae42ff52bd3a7/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_web_protocol_probe_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/network-web-protocol-probe/obj/generated/runtime/packed_words.c.o: build/.objects/f37ae42ff52bd3a7/generated/runtime/packed_words.c.o
	@mkdir -p 'build/network-web-protocol-probe/obj/generated/runtime'
	ln -f 'build/.objects/f37ae42ff52bd3a7/generated/runtime/packed_words.c.o' "$@"

build/network-web-protocol-probe/network-web-protocol-probe: GNUmakefile build/network-web-protocol-probe/obj/src/network_web_protocol_probe.cesta.o build/network-web-protocol-probe/obj/runtime/client_runtime.c.o build/network-web-protocol-probe/obj/runtime/traps_x86_64.c.o build/network-web-protocol-probe/obj/runtime/rpc_traps_x86_64.c.o build/network-web-protocol-probe/obj/runtime/region_writer_traps_x86_64.c.o build/network-web-protocol-probe/obj/runtime/start_x86_64.S.o build/network-web-protocol-probe/obj/generated/runtime/format_bytes.c.o build/network-web-protocol-probe/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/network-web-protocol-probe/obj/src/network_web_protocol_probe.cesta.o' 'build/network-web-protocol-probe/obj/runtime/client_runtime.c.o' 'build/network-web-protocol-probe/obj/runtime/traps_x86_64.c.o' 'build/network-web-protocol-probe/obj/runtime/rpc_traps_x86_64.c.o' 'build/network-web-protocol-probe/obj/runtime/region_writer_traps_x86_64.c.o' 'build/network-web-protocol-probe/obj/runtime/start_x86_64.S.o' 'build/network-web-protocol-probe/obj/generated/runtime/format_bytes.c.o' 'build/network-web-protocol-probe/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/.objects/2565dd9b70a0b331/src/network_service.cesta.o: src/network_service.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_service.c' -o "$@"

build/sexnet-network/obj/src/network_service.cesta.o: build/.objects/2565dd9b70a0b331/src/network_service.cesta.o
	@mkdir -p 'build/sexnet-network/obj/src'
	ln -f 'build/.objects/2565dd9b70a0b331/src/network_service.cesta.o' "$@"

build/.objects/f628801e19b283de/src/network_job_abi.cesta.o: src/network_job_abi.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_job_abi.c' -o "$@"

build/sexnet-network/obj/src/network_job_abi.cesta.o: build/.objects/f628801e19b283de/src/network_job_abi.cesta.o
	@mkdir -p 'build/sexnet-network/obj/src'
	ln -f 'build/.objects/f628801e19b283de/src/network_job_abi.cesta.o' "$@"

build/.objects/54199538d8dd3d59/src/network_job_wire.cesta.o: src/network_job_wire.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_job_wire.c' -o "$@"

build/sexnet-network/obj/src/network_job_wire.cesta.o: build/.objects/54199538d8dd3d59/src/network_job_wire.cesta.o
	@mkdir -p 'build/sexnet-network/obj/src'
	ln -f 'build/.objects/54199538d8dd3d59/src/network_job_wire.cesta.o' "$@"

build/.objects/69535348fb8c5667/src/network_job_machine.cesta.o: src/network_job_machine.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_job_machine.c' -o "$@"

build/sexnet-network/obj/src/network_job_machine.cesta.o: build/.objects/69535348fb8c5667/src/network_job_machine.cesta.o
	@mkdir -p 'build/sexnet-network/obj/src'
	ln -f 'build/.objects/69535348fb8c5667/src/network_job_machine.cesta.o' "$@"

build/.objects/0fc56426d0f730f2/src/network_dns_job_abi.cesta.o: src/network_dns_job_abi.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_dns_job_abi.c' -o "$@"

build/sexnet-network/obj/src/network_dns_job_abi.cesta.o: build/.objects/0fc56426d0f730f2/src/network_dns_job_abi.cesta.o
	@mkdir -p 'build/sexnet-network/obj/src'
	ln -f 'build/.objects/0fc56426d0f730f2/src/network_dns_job_abi.cesta.o' "$@"

build/.objects/0b3273d33499b445/src/network_dns_job_wire.cesta.o: src/network_dns_job_wire.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_dns_job_wire.c' -o "$@"

build/sexnet-network/obj/src/network_dns_job_wire.cesta.o: build/.objects/0b3273d33499b445/src/network_dns_job_wire.cesta.o
	@mkdir -p 'build/sexnet-network/obj/src'
	ln -f 'build/.objects/0b3273d33499b445/src/network_dns_job_wire.cesta.o' "$@"

build/.objects/699f78e15ecc21e3/src/network_dns_job_machine.cesta.o: src/network_dns_job_machine.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_dns_job_machine.c' -o "$@"

build/sexnet-network/obj/src/network_dns_job_machine.cesta.o: build/.objects/699f78e15ecc21e3/src/network_dns_job_machine.cesta.o
	@mkdir -p 'build/sexnet-network/obj/src'
	ln -f 'build/.objects/699f78e15ecc21e3/src/network_dns_job_machine.cesta.o' "$@"

build/.objects/332737ee2fe018d8/src/network_config.cesta.o: src/network_config.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_config.c' -o "$@"

build/sexnet-network/obj/src/network_config.cesta.o: build/.objects/332737ee2fe018d8/src/network_config.cesta.o
	@mkdir -p 'build/sexnet-network/obj/src'
	ln -f 'build/.objects/332737ee2fe018d8/src/network_config.cesta.o' "$@"

build/.objects/14dbd5787041ff42/src/network_engine_frame.cesta.o: src/network_engine_frame.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_engine_frame.c' -o "$@"

build/sexnet-network/obj/src/network_engine_frame.cesta.o: build/.objects/14dbd5787041ff42/src/network_engine_frame.cesta.o
	@mkdir -p 'build/sexnet-network/obj/src'
	ln -f 'build/.objects/14dbd5787041ff42/src/network_engine_frame.cesta.o' "$@"

build/.objects/142af6a4f489b2e9/src/network_engine_packet.cesta.o: src/network_engine_packet.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_engine_packet.c' -o "$@"

build/sexnet-network/obj/src/network_engine_packet.cesta.o: build/.objects/142af6a4f489b2e9/src/network_engine_packet.cesta.o
	@mkdir -p 'build/sexnet-network/obj/src'
	ln -f 'build/.objects/142af6a4f489b2e9/src/network_engine_packet.cesta.o' "$@"

build/.objects/dea3277b5387a731/src/network_engine_tcp_packet.cesta.o: src/network_engine_tcp_packet.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_engine_tcp_packet.c' -o "$@"

build/sexnet-network/obj/src/network_engine_tcp_packet.cesta.o: build/.objects/dea3277b5387a731/src/network_engine_tcp_packet.cesta.o
	@mkdir -p 'build/sexnet-network/obj/src'
	ln -f 'build/.objects/dea3277b5387a731/src/network_engine_tcp_packet.cesta.o' "$@"

build/.objects/50e34292d452047b/src/network_tcp_client.cesta.o: src/network_tcp_client.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_tcp_client.c' -o "$@"

build/sexnet-network/obj/src/network_tcp_client.cesta.o: build/.objects/50e34292d452047b/src/network_tcp_client.cesta.o
	@mkdir -p 'build/sexnet-network/obj/src'
	ln -f 'build/.objects/50e34292d452047b/src/network_tcp_client.cesta.o' "$@"

build/.objects/dbeb7d6943ff3572/src/network_tcp_listener.cesta.o: src/network_tcp_listener.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_tcp_listener.c' -o "$@"

build/sexnet-network/obj/src/network_tcp_listener.cesta.o: build/.objects/dbeb7d6943ff3572/src/network_tcp_listener.cesta.o
	@mkdir -p 'build/sexnet-network/obj/src'
	ln -f 'build/.objects/dbeb7d6943ff3572/src/network_tcp_listener.cesta.o' "$@"

build/.objects/e2472e0d064a9df3/src/network_engine_neighbor.cesta.o: src/network_engine_neighbor.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_engine_neighbor.c' -o "$@"

build/sexnet-network/obj/src/network_engine_neighbor.cesta.o: build/.objects/e2472e0d064a9df3/src/network_engine_neighbor.cesta.o
	@mkdir -p 'build/sexnet-network/obj/src'
	ln -f 'build/.objects/e2472e0d064a9df3/src/network_engine_neighbor.cesta.o' "$@"

build/.objects/0130c0d83dd4ec42/src/network_dns_codec.cesta.o: src/network_dns_codec.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_dns_codec.c' -o "$@"

build/sexnet-network/obj/src/network_dns_codec.cesta.o: build/.objects/0130c0d83dd4ec42/src/network_dns_codec.cesta.o
	@mkdir -p 'build/sexnet-network/obj/src'
	ln -f 'build/.objects/0130c0d83dd4ec42/src/network_dns_codec.cesta.o' "$@"

build/.objects/e036755d896258f4/src/network_engine_dns.cesta.o: src/network_engine_dns.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_engine_dns.c' -o "$@"

build/sexnet-network/obj/src/network_engine_dns.cesta.o: build/.objects/e036755d896258f4/src/network_engine_dns.cesta.o
	@mkdir -p 'build/sexnet-network/obj/src'
	ln -f 'build/.objects/e036755d896258f4/src/network_engine_dns.cesta.o' "$@"

build/.objects/0c3ebca6f73775fa/src/network_engine_transport.cesta.o: src/network_engine_transport.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_engine_transport.c' -o "$@"

build/sexnet-network/obj/src/network_engine_transport.cesta.o: build/.objects/0c3ebca6f73775fa/src/network_engine_transport.cesta.o
	@mkdir -p 'build/sexnet-network/obj/src'
	ln -f 'build/.objects/0c3ebca6f73775fa/src/network_engine_transport.cesta.o' "$@"

build/.objects/e409a38a58f96baf/src/network_job_engine_policy.cesta.o: src/network_job_engine_policy.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_job_engine_policy.c' -o "$@"

build/sexnet-network/obj/src/network_job_engine_policy.cesta.o: build/.objects/e409a38a58f96baf/src/network_job_engine_policy.cesta.o
	@mkdir -p 'build/sexnet-network/obj/src'
	ln -f 'build/.objects/e409a38a58f96baf/src/network_job_engine_policy.cesta.o' "$@"

build/.objects/ca6caa9541a5b878/src/network_job_engine_adapter.cesta.o: src/network_job_engine_adapter.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_job_engine_adapter.c' -o "$@"

build/sexnet-network/obj/src/network_job_engine_adapter.cesta.o: build/.objects/ca6caa9541a5b878/src/network_job_engine_adapter.cesta.o
	@mkdir -p 'build/sexnet-network/obj/src'
	ln -f 'build/.objects/ca6caa9541a5b878/src/network_job_engine_adapter.cesta.o' "$@"

build/.objects/9ed3f9531faad2dc/src/network_dns_job_engine_adapter.cesta.o: src/network_dns_job_engine_adapter.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_dns_job_engine_adapter.c' -o "$@"

build/sexnet-network/obj/src/network_dns_job_engine_adapter.cesta.o: build/.objects/9ed3f9531faad2dc/src/network_dns_job_engine_adapter.cesta.o
	@mkdir -p 'build/sexnet-network/obj/src'
	ln -f 'build/.objects/9ed3f9531faad2dc/src/network_dns_job_engine_adapter.cesta.o' "$@"

build/.objects/8ded30649fa5c82b/src/network_service_native.cesta.o: src/network_service_native.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_service_native.c' -o "$@"

build/sexnet-network/obj/src/network_service_native.cesta.o: build/.objects/8ded30649fa5c82b/src/network_service_native.cesta.o
	@mkdir -p 'build/sexnet-network/obj/src'
	ln -f 'build/.objects/8ded30649fa5c82b/src/network_service_native.cesta.o' "$@"

build/.objects/4b3ea6c898a05d7f/src/network_service_core.cesta.o: src/network_service_core.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_service_core.c' -o "$@"

build/sexnet-network/obj/src/network_service_core.cesta.o: build/.objects/4b3ea6c898a05d7f/src/network_service_core.cesta.o
	@mkdir -p 'build/sexnet-network/obj/src'
	ln -f 'build/.objects/4b3ea6c898a05d7f/src/network_service_core.cesta.o' "$@"

build/.objects/2e7297d1bfc0eb86/src/network_observation.cesta.o: src/network_observation.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_observation.c' -o "$@"

build/sexnet-network/obj/src/network_observation.cesta.o: build/.objects/2e7297d1bfc0eb86/src/network_observation.cesta.o
	@mkdir -p 'build/sexnet-network/obj/src'
	ln -f 'build/.objects/2e7297d1bfc0eb86/src/network_observation.cesta.o' "$@"

build/.objects/2eca027f28e860b4/src/network_device.cesta.o: src/network_device.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_device.c' -o "$@"

build/sexnet-network/obj/src/network_device.cesta.o: build/.objects/2eca027f28e860b4/src/network_device.cesta.o
	@mkdir -p 'build/sexnet-network/obj/src'
	ln -f 'build/.objects/2eca027f28e860b4/src/network_device.cesta.o' "$@"

build/.objects/c0525384d4ebc8a8/src/network_send.cesta.o: src/network_send.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_send.c' -o "$@"

build/sexnet-network/obj/src/network_send.cesta.o: build/.objects/c0525384d4ebc8a8/src/network_send.cesta.o
	@mkdir -p 'build/sexnet-network/obj/src'
	ln -f 'build/.objects/c0525384d4ebc8a8/src/network_send.cesta.o' "$@"

build/.objects/2c5ae818c12d805d/src/network_tcp.cesta.o: src/network_tcp.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_tcp.c' -o "$@"

build/sexnet-network/obj/src/network_tcp.cesta.o: build/.objects/2c5ae818c12d805d/src/network_tcp.cesta.o
	@mkdir -p 'build/sexnet-network/obj/src'
	ln -f 'build/.objects/2c5ae818c12d805d/src/network_tcp.cesta.o' "$@"

build/.objects/d595698e080c773b/src/network_service_current.cesta.o: src/network_service_current.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_service_current.c' -o "$@"

build/sexnet-network/obj/src/network_service_current.cesta.o: build/.objects/d595698e080c773b/src/network_service_current.cesta.o
	@mkdir -p 'build/sexnet-network/obj/src'
	ln -f 'build/.objects/d595698e080c773b/src/network_service_current.cesta.o' "$@"

build/.objects/013604783d72683a/src/network_device_current.cesta.o: src/network_device_current.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_device_current.c' -o "$@"

build/sexnet-network/obj/src/network_device_current.cesta.o: build/.objects/013604783d72683a/src/network_device_current.cesta.o
	@mkdir -p 'build/sexnet-network/obj/src'
	ln -f 'build/.objects/013604783d72683a/src/network_device_current.cesta.o' "$@"

build/.objects/28f04fbb785ec5e5/src/network_dns_job_liveness.cesta.o: src/network_dns_job_liveness.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_dns_job_liveness.c' -o "$@"

build/sexnet-network/obj/src/network_dns_job_liveness.cesta.o: build/.objects/28f04fbb785ec5e5/src/network_dns_job_liveness.cesta.o
	@mkdir -p 'build/sexnet-network/obj/src'
	ln -f 'build/.objects/28f04fbb785ec5e5/src/network_dns_job_liveness.cesta.o' "$@"

build/.objects/61c64007a77b2c02/src/network_open.cesta.o: src/network_open.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_open.c' -o "$@"

build/sexnet-network/obj/src/network_open.cesta.o: build/.objects/61c64007a77b2c02/src/network_open.cesta.o
	@mkdir -p 'build/sexnet-network/obj/src'
	ln -f 'build/.objects/61c64007a77b2c02/src/network_open.cesta.o' "$@"

build/.objects/3ac8744a3128946a/src/network_web.cesta.o: src/network_web.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_web.c' -o "$@"

build/sexnet-network/obj/src/network_web.cesta.o: build/.objects/3ac8744a3128946a/src/network_web.cesta.o
	@mkdir -p 'build/sexnet-network/obj/src'
	ln -f 'build/.objects/3ac8744a3128946a/src/network_web.cesta.o' "$@"

build/.objects/ea2e92f3eb2369b7/src/network_web_open.cesta.o: src/network_web_open.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_web_open.c' -o "$@"

build/sexnet-network/obj/src/network_web_open.cesta.o: build/.objects/ea2e92f3eb2369b7/src/network_web_open.cesta.o
	@mkdir -p 'build/sexnet-network/obj/src'
	ln -f 'build/.objects/ea2e92f3eb2369b7/src/network_web_open.cesta.o' "$@"

build/.objects/df1c96f29e62b5b6/src/network_http.cesta.o: src/network_http.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_http.c' -o "$@"

build/sexnet-network/obj/src/network_http.cesta.o: build/.objects/df1c96f29e62b5b6/src/network_http.cesta.o
	@mkdir -p 'build/sexnet-network/obj/src'
	ln -f 'build/.objects/df1c96f29e62b5b6/src/network_http.cesta.o' "$@"

build/.objects/b2b5feb99edd962f/src/rpc_runtime.cesta.o: src/rpc_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/rpc_runtime.c' -o "$@"

build/sexnet-network/obj/src/rpc_runtime.cesta.o: build/.objects/b2b5feb99edd962f/src/rpc_runtime.cesta.o
	@mkdir -p 'build/sexnet-network/obj/src'
	ln -f 'build/.objects/b2b5feb99edd962f/src/rpc_runtime.cesta.o' "$@"

build/.objects/4117e732fa7ec958/src/rpc_wait.cesta.o: src/rpc_wait.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/rpc_wait.c' -o "$@"

build/sexnet-network/obj/src/rpc_wait.cesta.o: build/.objects/4117e732fa7ec958/src/rpc_wait.cesta.o
	@mkdir -p 'build/sexnet-network/obj/src'
	ln -f 'build/.objects/4117e732fa7ec958/src/rpc_wait.cesta.o' "$@"

build/.objects/520d834280a17088/runtime/client_runtime.c.o: runtime/client_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/sexnet-network/obj/runtime/client_runtime.c.o: build/.objects/520d834280a17088/runtime/client_runtime.c.o
	@mkdir -p 'build/sexnet-network/obj/runtime'
	ln -f 'build/.objects/520d834280a17088/runtime/client_runtime.c.o' "$@"

build/.objects/8d95a521a7d69314/runtime/server_runtime.c.o: runtime/server_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_runtime.c' -o "$@"

build/sexnet-network/obj/runtime/server_runtime.c.o: build/.objects/8d95a521a7d69314/runtime/server_runtime.c.o
	@mkdir -p 'build/sexnet-network/obj/runtime'
	ln -f 'build/.objects/8d95a521a7d69314/runtime/server_runtime.c.o' "$@"

build/.objects/bfbd66c63d674ebc/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/sexnet-network/obj/runtime/traps_x86_64.c.o: build/.objects/bfbd66c63d674ebc/runtime/traps_x86_64.c.o
	@mkdir -p 'build/sexnet-network/obj/runtime'
	ln -f 'build/.objects/bfbd66c63d674ebc/runtime/traps_x86_64.c.o' "$@"

build/.objects/747b0553b0799748/runtime/server_traps_x86_64.c.o: runtime/server_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_traps_x86_64.c' -o "$@"

build/sexnet-network/obj/runtime/server_traps_x86_64.c.o: build/.objects/747b0553b0799748/runtime/server_traps_x86_64.c.o
	@mkdir -p 'build/sexnet-network/obj/runtime'
	ln -f 'build/.objects/747b0553b0799748/runtime/server_traps_x86_64.c.o' "$@"

build/.objects/db9c855e5f7eaf60/runtime/rpc_traps_x86_64.c.o: runtime/rpc_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/rpc_traps_x86_64.c' -o "$@"

build/sexnet-network/obj/runtime/rpc_traps_x86_64.c.o: build/.objects/db9c855e5f7eaf60/runtime/rpc_traps_x86_64.c.o
	@mkdir -p 'build/sexnet-network/obj/runtime'
	ln -f 'build/.objects/db9c855e5f7eaf60/runtime/rpc_traps_x86_64.c.o' "$@"

build/.objects/f155396b47b68e25/runtime/mmio_traps_x86_64.c.o: runtime/mmio_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/mmio_traps_x86_64.c' -o "$@"

build/sexnet-network/obj/runtime/mmio_traps_x86_64.c.o: build/.objects/f155396b47b68e25/runtime/mmio_traps_x86_64.c.o
	@mkdir -p 'build/sexnet-network/obj/runtime'
	ln -f 'build/.objects/f155396b47b68e25/runtime/mmio_traps_x86_64.c.o' "$@"

build/.objects/90d48cf0db1adc75/runtime/region_writer_traps_x86_64.c.o: runtime/region_writer_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/region_writer_traps_x86_64.c' -o "$@"

build/sexnet-network/obj/runtime/region_writer_traps_x86_64.c.o: build/.objects/90d48cf0db1adc75/runtime/region_writer_traps_x86_64.c.o
	@mkdir -p 'build/sexnet-network/obj/runtime'
	ln -f 'build/.objects/90d48cf0db1adc75/runtime/region_writer_traps_x86_64.c.o' "$@"

build/.objects/b07f5bf2d462e278/runtime/start_x86_64.S.o: runtime/start_x86_64.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/sexnet-network/obj/runtime/start_x86_64.S.o: build/.objects/b07f5bf2d462e278/runtime/start_x86_64.S.o
	@mkdir -p 'build/sexnet-network/obj/runtime'
	ln -f 'build/.objects/b07f5bf2d462e278/runtime/start_x86_64.S.o' "$@"

build/.objects/e550ec13e0bae2cf/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/sexnet-network/obj/generated/runtime/format_bytes.c.o: build/.objects/e550ec13e0bae2cf/generated/runtime/format_bytes.c.o
	@mkdir -p 'build/sexnet-network/obj/generated/runtime'
	ln -f 'build/.objects/e550ec13e0bae2cf/generated/runtime/format_bytes.c.o' "$@"

build/.objects/9aa710a4d2226628/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_current_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/sexnet-network/obj/generated/runtime/packed_words.c.o: build/.objects/9aa710a4d2226628/generated/runtime/packed_words.c.o
	@mkdir -p 'build/sexnet-network/obj/generated/runtime'
	ln -f 'build/.objects/9aa710a4d2226628/generated/runtime/packed_words.c.o' "$@"

build/sexnet-network/sexnet-network: GNUmakefile build/sexnet-network/obj/src/network_service.cesta.o build/sexnet-network/obj/src/network_job_abi.cesta.o build/sexnet-network/obj/src/network_job_wire.cesta.o build/sexnet-network/obj/src/network_job_machine.cesta.o build/sexnet-network/obj/src/network_dns_job_abi.cesta.o build/sexnet-network/obj/src/network_dns_job_wire.cesta.o build/sexnet-network/obj/src/network_dns_job_machine.cesta.o build/sexnet-network/obj/src/network_config.cesta.o build/sexnet-network/obj/src/network_engine_frame.cesta.o build/sexnet-network/obj/src/network_engine_packet.cesta.o build/sexnet-network/obj/src/network_engine_tcp_packet.cesta.o build/sexnet-network/obj/src/network_tcp_client.cesta.o build/sexnet-network/obj/src/network_tcp_listener.cesta.o build/sexnet-network/obj/src/network_engine_neighbor.cesta.o build/sexnet-network/obj/src/network_dns_codec.cesta.o build/sexnet-network/obj/src/network_engine_dns.cesta.o build/sexnet-network/obj/src/network_engine_transport.cesta.o build/sexnet-network/obj/src/network_job_engine_policy.cesta.o build/sexnet-network/obj/src/network_job_engine_adapter.cesta.o build/sexnet-network/obj/src/network_dns_job_engine_adapter.cesta.o build/sexnet-network/obj/src/network_service_native.cesta.o build/sexnet-network/obj/src/network_service_core.cesta.o build/sexnet-network/obj/src/network_observation.cesta.o build/sexnet-network/obj/src/network_device.cesta.o build/sexnet-network/obj/src/network_send.cesta.o build/sexnet-network/obj/src/network_tcp.cesta.o build/sexnet-network/obj/src/network_service_current.cesta.o build/sexnet-network/obj/src/network_device_current.cesta.o build/sexnet-network/obj/src/network_dns_job_liveness.cesta.o build/sexnet-network/obj/src/network_open.cesta.o build/sexnet-network/obj/src/network_web.cesta.o build/sexnet-network/obj/src/network_web_open.cesta.o build/sexnet-network/obj/src/network_http.cesta.o build/sexnet-network/obj/src/rpc_runtime.cesta.o build/sexnet-network/obj/src/rpc_wait.cesta.o build/sexnet-network/obj/runtime/client_runtime.c.o build/sexnet-network/obj/runtime/server_runtime.c.o build/sexnet-network/obj/runtime/traps_x86_64.c.o build/sexnet-network/obj/runtime/server_traps_x86_64.c.o build/sexnet-network/obj/runtime/rpc_traps_x86_64.c.o build/sexnet-network/obj/runtime/mmio_traps_x86_64.c.o build/sexnet-network/obj/runtime/region_writer_traps_x86_64.c.o build/sexnet-network/obj/runtime/start_x86_64.S.o build/sexnet-network/obj/generated/runtime/format_bytes.c.o build/sexnet-network/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/sexnet-network/obj/src/network_service.cesta.o' 'build/sexnet-network/obj/src/network_job_abi.cesta.o' 'build/sexnet-network/obj/src/network_job_wire.cesta.o' 'build/sexnet-network/obj/src/network_job_machine.cesta.o' 'build/sexnet-network/obj/src/network_dns_job_abi.cesta.o' 'build/sexnet-network/obj/src/network_dns_job_wire.cesta.o' 'build/sexnet-network/obj/src/network_dns_job_machine.cesta.o' 'build/sexnet-network/obj/src/network_config.cesta.o' 'build/sexnet-network/obj/src/network_engine_frame.cesta.o' 'build/sexnet-network/obj/src/network_engine_packet.cesta.o' 'build/sexnet-network/obj/src/network_engine_tcp_packet.cesta.o' 'build/sexnet-network/obj/src/network_tcp_client.cesta.o' 'build/sexnet-network/obj/src/network_tcp_listener.cesta.o' 'build/sexnet-network/obj/src/network_engine_neighbor.cesta.o' 'build/sexnet-network/obj/src/network_dns_codec.cesta.o' 'build/sexnet-network/obj/src/network_engine_dns.cesta.o' 'build/sexnet-network/obj/src/network_engine_transport.cesta.o' 'build/sexnet-network/obj/src/network_job_engine_policy.cesta.o' 'build/sexnet-network/obj/src/network_job_engine_adapter.cesta.o' 'build/sexnet-network/obj/src/network_dns_job_engine_adapter.cesta.o' 'build/sexnet-network/obj/src/network_service_native.cesta.o' 'build/sexnet-network/obj/src/network_service_core.cesta.o' 'build/sexnet-network/obj/src/network_observation.cesta.o' 'build/sexnet-network/obj/src/network_device.cesta.o' 'build/sexnet-network/obj/src/network_send.cesta.o' 'build/sexnet-network/obj/src/network_tcp.cesta.o' 'build/sexnet-network/obj/src/network_service_current.cesta.o' 'build/sexnet-network/obj/src/network_device_current.cesta.o' 'build/sexnet-network/obj/src/network_dns_job_liveness.cesta.o' 'build/sexnet-network/obj/src/network_open.cesta.o' 'build/sexnet-network/obj/src/network_web.cesta.o' 'build/sexnet-network/obj/src/network_web_open.cesta.o' 'build/sexnet-network/obj/src/network_http.cesta.o' 'build/sexnet-network/obj/src/rpc_runtime.cesta.o' 'build/sexnet-network/obj/src/rpc_wait.cesta.o' 'build/sexnet-network/obj/runtime/client_runtime.c.o' 'build/sexnet-network/obj/runtime/server_runtime.c.o' 'build/sexnet-network/obj/runtime/traps_x86_64.c.o' 'build/sexnet-network/obj/runtime/server_traps_x86_64.c.o' 'build/sexnet-network/obj/runtime/rpc_traps_x86_64.c.o' 'build/sexnet-network/obj/runtime/mmio_traps_x86_64.c.o' 'build/sexnet-network/obj/runtime/region_writer_traps_x86_64.c.o' 'build/sexnet-network/obj/runtime/start_x86_64.S.o' 'build/sexnet-network/obj/generated/runtime/format_bytes.c.o' 'build/sexnet-network/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/.objects/882cdb57c9f38e61/src/network_job_abi.cesta.o: src/network_job_abi.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_job_abi.c' -o "$@"

build/network-job-abi/obj/src/network_job_abi.cesta.o: build/.objects/882cdb57c9f38e61/src/network_job_abi.cesta.o
	@mkdir -p 'build/network-job-abi/obj/src'
	ln -f 'build/.objects/882cdb57c9f38e61/src/network_job_abi.cesta.o' "$@"

build/network-job-abi/libnetwork-job-abi.a: GNUmakefile build/network-job-abi/obj/src/network_job_abi.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/network-job-abi/obj/src/network_job_abi.cesta.o'

build/network-job-machine/obj/src/network_job_abi.cesta.o: build/.objects/882cdb57c9f38e61/src/network_job_abi.cesta.o
	@mkdir -p 'build/network-job-machine/obj/src'
	ln -f 'build/.objects/882cdb57c9f38e61/src/network_job_abi.cesta.o' "$@"

build/.objects/9a8ea406c8ee08f2/src/network_job_wire.cesta.o: src/network_job_wire.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_job_wire.c' -o "$@"

build/network-job-machine/obj/src/network_job_wire.cesta.o: build/.objects/9a8ea406c8ee08f2/src/network_job_wire.cesta.o
	@mkdir -p 'build/network-job-machine/obj/src'
	ln -f 'build/.objects/9a8ea406c8ee08f2/src/network_job_wire.cesta.o' "$@"

build/.objects/8cf505d61d8c47fc/src/network_job_machine.cesta.o: src/network_job_machine.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_job_machine.c' -o "$@"

build/network-job-machine/obj/src/network_job_machine.cesta.o: build/.objects/8cf505d61d8c47fc/src/network_job_machine.cesta.o
	@mkdir -p 'build/network-job-machine/obj/src'
	ln -f 'build/.objects/8cf505d61d8c47fc/src/network_job_machine.cesta.o' "$@"

build/network-job-machine/libnetwork-job-machine.a: GNUmakefile build/network-job-machine/obj/src/network_job_abi.cesta.o build/network-job-machine/obj/src/network_job_wire.cesta.o build/network-job-machine/obj/src/network_job_machine.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/network-job-machine/obj/src/network_job_abi.cesta.o' 'build/network-job-machine/obj/src/network_job_wire.cesta.o' 'build/network-job-machine/obj/src/network_job_machine.cesta.o'

build/network-dns-job-machine/obj/src/network_job_abi.cesta.o: build/.objects/882cdb57c9f38e61/src/network_job_abi.cesta.o
	@mkdir -p 'build/network-dns-job-machine/obj/src'
	ln -f 'build/.objects/882cdb57c9f38e61/src/network_job_abi.cesta.o' "$@"

build/network-dns-job-machine/obj/src/network_job_wire.cesta.o: build/.objects/9a8ea406c8ee08f2/src/network_job_wire.cesta.o
	@mkdir -p 'build/network-dns-job-machine/obj/src'
	ln -f 'build/.objects/9a8ea406c8ee08f2/src/network_job_wire.cesta.o' "$@"

build/network-dns-job-machine/obj/src/network_job_machine.cesta.o: build/.objects/8cf505d61d8c47fc/src/network_job_machine.cesta.o
	@mkdir -p 'build/network-dns-job-machine/obj/src'
	ln -f 'build/.objects/8cf505d61d8c47fc/src/network_job_machine.cesta.o' "$@"

build/.objects/0659b3adc088e6d5/src/network_dns_codec.cesta.o: src/network_dns_codec.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_dns_codec.c' -o "$@"

build/network-dns-job-machine/obj/src/network_dns_codec.cesta.o: build/.objects/0659b3adc088e6d5/src/network_dns_codec.cesta.o
	@mkdir -p 'build/network-dns-job-machine/obj/src'
	ln -f 'build/.objects/0659b3adc088e6d5/src/network_dns_codec.cesta.o' "$@"

build/.objects/6c2f8c263229c465/src/network_dns_job_abi.cesta.o: src/network_dns_job_abi.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_dns_job_abi.c' -o "$@"

build/network-dns-job-machine/obj/src/network_dns_job_abi.cesta.o: build/.objects/6c2f8c263229c465/src/network_dns_job_abi.cesta.o
	@mkdir -p 'build/network-dns-job-machine/obj/src'
	ln -f 'build/.objects/6c2f8c263229c465/src/network_dns_job_abi.cesta.o' "$@"

build/.objects/3f1f9e1b3175245e/src/network_dns_job_wire.cesta.o: src/network_dns_job_wire.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_dns_job_wire.c' -o "$@"

build/network-dns-job-machine/obj/src/network_dns_job_wire.cesta.o: build/.objects/3f1f9e1b3175245e/src/network_dns_job_wire.cesta.o
	@mkdir -p 'build/network-dns-job-machine/obj/src'
	ln -f 'build/.objects/3f1f9e1b3175245e/src/network_dns_job_wire.cesta.o' "$@"

build/.objects/e195fe6a39f61bd8/src/network_dns_job_machine.cesta.o: src/network_dns_job_machine.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_dns_job_machine.c' -o "$@"

build/network-dns-job-machine/obj/src/network_dns_job_machine.cesta.o: build/.objects/e195fe6a39f61bd8/src/network_dns_job_machine.cesta.o
	@mkdir -p 'build/network-dns-job-machine/obj/src'
	ln -f 'build/.objects/e195fe6a39f61bd8/src/network_dns_job_machine.cesta.o' "$@"

build/.objects/fe7b6b3b3451bb7e/src/network_dns_job_liveness.cesta.o: src/network_dns_job_liveness.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_dns_job_liveness.c' -o "$@"

build/network-dns-job-machine/obj/src/network_dns_job_liveness.cesta.o: build/.objects/fe7b6b3b3451bb7e/src/network_dns_job_liveness.cesta.o
	@mkdir -p 'build/network-dns-job-machine/obj/src'
	ln -f 'build/.objects/fe7b6b3b3451bb7e/src/network_dns_job_liveness.cesta.o' "$@"

build/network-dns-job-machine/libnetwork-dns-job-machine.a: GNUmakefile build/network-dns-job-machine/obj/src/network_job_abi.cesta.o build/network-dns-job-machine/obj/src/network_job_wire.cesta.o build/network-dns-job-machine/obj/src/network_job_machine.cesta.o build/network-dns-job-machine/obj/src/network_dns_codec.cesta.o build/network-dns-job-machine/obj/src/network_dns_job_abi.cesta.o build/network-dns-job-machine/obj/src/network_dns_job_wire.cesta.o build/network-dns-job-machine/obj/src/network_dns_job_machine.cesta.o build/network-dns-job-machine/obj/src/network_dns_job_liveness.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/network-dns-job-machine/obj/src/network_job_abi.cesta.o' 'build/network-dns-job-machine/obj/src/network_job_wire.cesta.o' 'build/network-dns-job-machine/obj/src/network_job_machine.cesta.o' 'build/network-dns-job-machine/obj/src/network_dns_codec.cesta.o' 'build/network-dns-job-machine/obj/src/network_dns_job_abi.cesta.o' 'build/network-dns-job-machine/obj/src/network_dns_job_wire.cesta.o' 'build/network-dns-job-machine/obj/src/network_dns_job_machine.cesta.o' 'build/network-dns-job-machine/obj/src/network_dns_job_liveness.cesta.o'

build/.objects/919a04ca0db8978f/src/network_config.cesta.o: src/network_config.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_config.c' -o "$@"

build/network-engine-frame/obj/src/network_config.cesta.o: build/.objects/919a04ca0db8978f/src/network_config.cesta.o
	@mkdir -p 'build/network-engine-frame/obj/src'
	ln -f 'build/.objects/919a04ca0db8978f/src/network_config.cesta.o' "$@"

build/.objects/024cac7ba69fe1d5/src/network_engine_frame.cesta.o: src/network_engine_frame.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_engine_frame.c' -o "$@"

build/network-engine-frame/obj/src/network_engine_frame.cesta.o: build/.objects/024cac7ba69fe1d5/src/network_engine_frame.cesta.o
	@mkdir -p 'build/network-engine-frame/obj/src'
	ln -f 'build/.objects/024cac7ba69fe1d5/src/network_engine_frame.cesta.o' "$@"

build/network-engine-frame/libnetwork-engine-frame.a: GNUmakefile build/network-engine-frame/obj/src/network_config.cesta.o build/network-engine-frame/obj/src/network_engine_frame.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/network-engine-frame/obj/src/network_config.cesta.o' 'build/network-engine-frame/obj/src/network_engine_frame.cesta.o'

build/network-dns-codec/obj/src/network_dns_codec.cesta.o: build/.objects/0659b3adc088e6d5/src/network_dns_codec.cesta.o
	@mkdir -p 'build/network-dns-codec/obj/src'
	ln -f 'build/.objects/0659b3adc088e6d5/src/network_dns_codec.cesta.o' "$@"

build/network-dns-codec/libnetwork-dns-codec.a: GNUmakefile build/network-dns-codec/obj/src/network_dns_codec.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/network-dns-codec/obj/src/network_dns_codec.cesta.o'

build/network-engine-packet/obj/src/network_config.cesta.o: build/.objects/919a04ca0db8978f/src/network_config.cesta.o
	@mkdir -p 'build/network-engine-packet/obj/src'
	ln -f 'build/.objects/919a04ca0db8978f/src/network_config.cesta.o' "$@"

build/network-engine-packet/obj/src/network_engine_frame.cesta.o: build/.objects/024cac7ba69fe1d5/src/network_engine_frame.cesta.o
	@mkdir -p 'build/network-engine-packet/obj/src'
	ln -f 'build/.objects/024cac7ba69fe1d5/src/network_engine_frame.cesta.o' "$@"

build/.objects/e88be3f5062b9662/src/network_engine_packet.cesta.o: src/network_engine_packet.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_engine_packet.c' -o "$@"

build/network-engine-packet/obj/src/network_engine_packet.cesta.o: build/.objects/e88be3f5062b9662/src/network_engine_packet.cesta.o
	@mkdir -p 'build/network-engine-packet/obj/src'
	ln -f 'build/.objects/e88be3f5062b9662/src/network_engine_packet.cesta.o' "$@"

build/network-engine-packet/libnetwork-engine-packet.a: GNUmakefile build/network-engine-packet/obj/src/network_config.cesta.o build/network-engine-packet/obj/src/network_engine_frame.cesta.o build/network-engine-packet/obj/src/network_engine_packet.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/network-engine-packet/obj/src/network_config.cesta.o' 'build/network-engine-packet/obj/src/network_engine_frame.cesta.o' 'build/network-engine-packet/obj/src/network_engine_packet.cesta.o'

build/network-engine-neighbor/obj/src/network_config.cesta.o: build/.objects/919a04ca0db8978f/src/network_config.cesta.o
	@mkdir -p 'build/network-engine-neighbor/obj/src'
	ln -f 'build/.objects/919a04ca0db8978f/src/network_config.cesta.o' "$@"

build/network-engine-neighbor/obj/src/network_engine_frame.cesta.o: build/.objects/024cac7ba69fe1d5/src/network_engine_frame.cesta.o
	@mkdir -p 'build/network-engine-neighbor/obj/src'
	ln -f 'build/.objects/024cac7ba69fe1d5/src/network_engine_frame.cesta.o' "$@"

build/network-engine-neighbor/obj/src/network_engine_packet.cesta.o: build/.objects/e88be3f5062b9662/src/network_engine_packet.cesta.o
	@mkdir -p 'build/network-engine-neighbor/obj/src'
	ln -f 'build/.objects/e88be3f5062b9662/src/network_engine_packet.cesta.o' "$@"

build/.objects/85f56b2ae0eb5d88/src/network_engine_neighbor.cesta.o: src/network_engine_neighbor.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_engine_neighbor.c' -o "$@"

build/network-engine-neighbor/obj/src/network_engine_neighbor.cesta.o: build/.objects/85f56b2ae0eb5d88/src/network_engine_neighbor.cesta.o
	@mkdir -p 'build/network-engine-neighbor/obj/src'
	ln -f 'build/.objects/85f56b2ae0eb5d88/src/network_engine_neighbor.cesta.o' "$@"

build/network-engine-neighbor/libnetwork-engine-neighbor.a: GNUmakefile build/network-engine-neighbor/obj/src/network_config.cesta.o build/network-engine-neighbor/obj/src/network_engine_frame.cesta.o build/network-engine-neighbor/obj/src/network_engine_packet.cesta.o build/network-engine-neighbor/obj/src/network_engine_neighbor.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/network-engine-neighbor/obj/src/network_config.cesta.o' 'build/network-engine-neighbor/obj/src/network_engine_frame.cesta.o' 'build/network-engine-neighbor/obj/src/network_engine_packet.cesta.o' 'build/network-engine-neighbor/obj/src/network_engine_neighbor.cesta.o'

build/network-engine-dns/obj/src/network_config.cesta.o: build/.objects/919a04ca0db8978f/src/network_config.cesta.o
	@mkdir -p 'build/network-engine-dns/obj/src'
	ln -f 'build/.objects/919a04ca0db8978f/src/network_config.cesta.o' "$@"

build/network-engine-dns/obj/src/network_engine_frame.cesta.o: build/.objects/024cac7ba69fe1d5/src/network_engine_frame.cesta.o
	@mkdir -p 'build/network-engine-dns/obj/src'
	ln -f 'build/.objects/024cac7ba69fe1d5/src/network_engine_frame.cesta.o' "$@"

build/network-engine-dns/obj/src/network_engine_packet.cesta.o: build/.objects/e88be3f5062b9662/src/network_engine_packet.cesta.o
	@mkdir -p 'build/network-engine-dns/obj/src'
	ln -f 'build/.objects/e88be3f5062b9662/src/network_engine_packet.cesta.o' "$@"

build/network-engine-dns/obj/src/network_engine_neighbor.cesta.o: build/.objects/85f56b2ae0eb5d88/src/network_engine_neighbor.cesta.o
	@mkdir -p 'build/network-engine-dns/obj/src'
	ln -f 'build/.objects/85f56b2ae0eb5d88/src/network_engine_neighbor.cesta.o' "$@"

build/network-engine-dns/obj/src/network_dns_codec.cesta.o: build/.objects/0659b3adc088e6d5/src/network_dns_codec.cesta.o
	@mkdir -p 'build/network-engine-dns/obj/src'
	ln -f 'build/.objects/0659b3adc088e6d5/src/network_dns_codec.cesta.o' "$@"

build/.objects/6b81973b41eac44b/src/network_engine_dns.cesta.o: src/network_engine_dns.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_engine_dns.c' -o "$@"

build/network-engine-dns/obj/src/network_engine_dns.cesta.o: build/.objects/6b81973b41eac44b/src/network_engine_dns.cesta.o
	@mkdir -p 'build/network-engine-dns/obj/src'
	ln -f 'build/.objects/6b81973b41eac44b/src/network_engine_dns.cesta.o' "$@"

build/network-engine-dns/libnetwork-engine-dns.a: GNUmakefile build/network-engine-dns/obj/src/network_config.cesta.o build/network-engine-dns/obj/src/network_engine_frame.cesta.o build/network-engine-dns/obj/src/network_engine_packet.cesta.o build/network-engine-dns/obj/src/network_engine_neighbor.cesta.o build/network-engine-dns/obj/src/network_dns_codec.cesta.o build/network-engine-dns/obj/src/network_engine_dns.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/network-engine-dns/obj/src/network_config.cesta.o' 'build/network-engine-dns/obj/src/network_engine_frame.cesta.o' 'build/network-engine-dns/obj/src/network_engine_packet.cesta.o' 'build/network-engine-dns/obj/src/network_engine_neighbor.cesta.o' 'build/network-engine-dns/obj/src/network_dns_codec.cesta.o' 'build/network-engine-dns/obj/src/network_engine_dns.cesta.o'

build/network-engine-tcp-packet/obj/src/network_config.cesta.o: build/.objects/919a04ca0db8978f/src/network_config.cesta.o
	@mkdir -p 'build/network-engine-tcp-packet/obj/src'
	ln -f 'build/.objects/919a04ca0db8978f/src/network_config.cesta.o' "$@"

build/network-engine-tcp-packet/obj/src/network_engine_frame.cesta.o: build/.objects/024cac7ba69fe1d5/src/network_engine_frame.cesta.o
	@mkdir -p 'build/network-engine-tcp-packet/obj/src'
	ln -f 'build/.objects/024cac7ba69fe1d5/src/network_engine_frame.cesta.o' "$@"

build/network-engine-tcp-packet/obj/src/network_engine_packet.cesta.o: build/.objects/e88be3f5062b9662/src/network_engine_packet.cesta.o
	@mkdir -p 'build/network-engine-tcp-packet/obj/src'
	ln -f 'build/.objects/e88be3f5062b9662/src/network_engine_packet.cesta.o' "$@"

build/.objects/888b66ccba42b1ca/src/network_engine_tcp_packet.cesta.o: src/network_engine_tcp_packet.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_engine_tcp_packet.c' -o "$@"

build/network-engine-tcp-packet/obj/src/network_engine_tcp_packet.cesta.o: build/.objects/888b66ccba42b1ca/src/network_engine_tcp_packet.cesta.o
	@mkdir -p 'build/network-engine-tcp-packet/obj/src'
	ln -f 'build/.objects/888b66ccba42b1ca/src/network_engine_tcp_packet.cesta.o' "$@"

build/network-engine-tcp-packet/libnetwork-engine-tcp-packet.a: GNUmakefile build/network-engine-tcp-packet/obj/src/network_config.cesta.o build/network-engine-tcp-packet/obj/src/network_engine_frame.cesta.o build/network-engine-tcp-packet/obj/src/network_engine_packet.cesta.o build/network-engine-tcp-packet/obj/src/network_engine_tcp_packet.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/network-engine-tcp-packet/obj/src/network_config.cesta.o' 'build/network-engine-tcp-packet/obj/src/network_engine_frame.cesta.o' 'build/network-engine-tcp-packet/obj/src/network_engine_packet.cesta.o' 'build/network-engine-tcp-packet/obj/src/network_engine_tcp_packet.cesta.o'

build/network-tcp-client/obj/src/network_config.cesta.o: build/.objects/919a04ca0db8978f/src/network_config.cesta.o
	@mkdir -p 'build/network-tcp-client/obj/src'
	ln -f 'build/.objects/919a04ca0db8978f/src/network_config.cesta.o' "$@"

build/network-tcp-client/obj/src/network_engine_frame.cesta.o: build/.objects/024cac7ba69fe1d5/src/network_engine_frame.cesta.o
	@mkdir -p 'build/network-tcp-client/obj/src'
	ln -f 'build/.objects/024cac7ba69fe1d5/src/network_engine_frame.cesta.o' "$@"

build/network-tcp-client/obj/src/network_engine_packet.cesta.o: build/.objects/e88be3f5062b9662/src/network_engine_packet.cesta.o
	@mkdir -p 'build/network-tcp-client/obj/src'
	ln -f 'build/.objects/e88be3f5062b9662/src/network_engine_packet.cesta.o' "$@"

build/network-tcp-client/obj/src/network_engine_tcp_packet.cesta.o: build/.objects/888b66ccba42b1ca/src/network_engine_tcp_packet.cesta.o
	@mkdir -p 'build/network-tcp-client/obj/src'
	ln -f 'build/.objects/888b66ccba42b1ca/src/network_engine_tcp_packet.cesta.o' "$@"

build/.objects/822fca1c24096d10/src/network_tcp_client.cesta.o: src/network_tcp_client.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_tcp_client.c' -o "$@"

build/network-tcp-client/obj/src/network_tcp_client.cesta.o: build/.objects/822fca1c24096d10/src/network_tcp_client.cesta.o
	@mkdir -p 'build/network-tcp-client/obj/src'
	ln -f 'build/.objects/822fca1c24096d10/src/network_tcp_client.cesta.o' "$@"

build/network-tcp-client/libnetwork-tcp-client.a: GNUmakefile build/network-tcp-client/obj/src/network_config.cesta.o build/network-tcp-client/obj/src/network_engine_frame.cesta.o build/network-tcp-client/obj/src/network_engine_packet.cesta.o build/network-tcp-client/obj/src/network_engine_tcp_packet.cesta.o build/network-tcp-client/obj/src/network_tcp_client.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/network-tcp-client/obj/src/network_config.cesta.o' 'build/network-tcp-client/obj/src/network_engine_frame.cesta.o' 'build/network-tcp-client/obj/src/network_engine_packet.cesta.o' 'build/network-tcp-client/obj/src/network_engine_tcp_packet.cesta.o' 'build/network-tcp-client/obj/src/network_tcp_client.cesta.o'

build/network-engine-transport/obj/src/network_config.cesta.o: build/.objects/919a04ca0db8978f/src/network_config.cesta.o
	@mkdir -p 'build/network-engine-transport/obj/src'
	ln -f 'build/.objects/919a04ca0db8978f/src/network_config.cesta.o' "$@"

build/network-engine-transport/obj/src/network_engine_frame.cesta.o: build/.objects/024cac7ba69fe1d5/src/network_engine_frame.cesta.o
	@mkdir -p 'build/network-engine-transport/obj/src'
	ln -f 'build/.objects/024cac7ba69fe1d5/src/network_engine_frame.cesta.o' "$@"

build/network-engine-transport/obj/src/network_engine_packet.cesta.o: build/.objects/e88be3f5062b9662/src/network_engine_packet.cesta.o
	@mkdir -p 'build/network-engine-transport/obj/src'
	ln -f 'build/.objects/e88be3f5062b9662/src/network_engine_packet.cesta.o' "$@"

build/network-engine-transport/obj/src/network_engine_tcp_packet.cesta.o: build/.objects/888b66ccba42b1ca/src/network_engine_tcp_packet.cesta.o
	@mkdir -p 'build/network-engine-transport/obj/src'
	ln -f 'build/.objects/888b66ccba42b1ca/src/network_engine_tcp_packet.cesta.o' "$@"

build/network-engine-transport/obj/src/network_tcp_client.cesta.o: build/.objects/822fca1c24096d10/src/network_tcp_client.cesta.o
	@mkdir -p 'build/network-engine-transport/obj/src'
	ln -f 'build/.objects/822fca1c24096d10/src/network_tcp_client.cesta.o' "$@"

build/.objects/e75fefcd8742c9e5/src/network_tcp_listener.cesta.o: src/network_tcp_listener.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_tcp_listener.c' -o "$@"

build/network-engine-transport/obj/src/network_tcp_listener.cesta.o: build/.objects/e75fefcd8742c9e5/src/network_tcp_listener.cesta.o
	@mkdir -p 'build/network-engine-transport/obj/src'
	ln -f 'build/.objects/e75fefcd8742c9e5/src/network_tcp_listener.cesta.o' "$@"

build/network-engine-transport/obj/src/network_engine_neighbor.cesta.o: build/.objects/85f56b2ae0eb5d88/src/network_engine_neighbor.cesta.o
	@mkdir -p 'build/network-engine-transport/obj/src'
	ln -f 'build/.objects/85f56b2ae0eb5d88/src/network_engine_neighbor.cesta.o' "$@"

build/network-engine-transport/obj/src/network_dns_codec.cesta.o: build/.objects/0659b3adc088e6d5/src/network_dns_codec.cesta.o
	@mkdir -p 'build/network-engine-transport/obj/src'
	ln -f 'build/.objects/0659b3adc088e6d5/src/network_dns_codec.cesta.o' "$@"

build/network-engine-transport/obj/src/network_engine_dns.cesta.o: build/.objects/6b81973b41eac44b/src/network_engine_dns.cesta.o
	@mkdir -p 'build/network-engine-transport/obj/src'
	ln -f 'build/.objects/6b81973b41eac44b/src/network_engine_dns.cesta.o' "$@"

build/.objects/0138692a3b4b9fed/src/network_engine_transport.cesta.o: src/network_engine_transport.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_engine_transport.c' -o "$@"

build/network-engine-transport/obj/src/network_engine_transport.cesta.o: build/.objects/0138692a3b4b9fed/src/network_engine_transport.cesta.o
	@mkdir -p 'build/network-engine-transport/obj/src'
	ln -f 'build/.objects/0138692a3b4b9fed/src/network_engine_transport.cesta.o' "$@"

build/network-engine-transport/libnetwork-engine-transport.a: GNUmakefile build/network-engine-transport/obj/src/network_config.cesta.o build/network-engine-transport/obj/src/network_engine_frame.cesta.o build/network-engine-transport/obj/src/network_engine_packet.cesta.o build/network-engine-transport/obj/src/network_engine_tcp_packet.cesta.o build/network-engine-transport/obj/src/network_tcp_client.cesta.o build/network-engine-transport/obj/src/network_tcp_listener.cesta.o build/network-engine-transport/obj/src/network_engine_neighbor.cesta.o build/network-engine-transport/obj/src/network_dns_codec.cesta.o build/network-engine-transport/obj/src/network_engine_dns.cesta.o build/network-engine-transport/obj/src/network_engine_transport.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/network-engine-transport/obj/src/network_config.cesta.o' 'build/network-engine-transport/obj/src/network_engine_frame.cesta.o' 'build/network-engine-transport/obj/src/network_engine_packet.cesta.o' 'build/network-engine-transport/obj/src/network_engine_tcp_packet.cesta.o' 'build/network-engine-transport/obj/src/network_tcp_client.cesta.o' 'build/network-engine-transport/obj/src/network_tcp_listener.cesta.o' 'build/network-engine-transport/obj/src/network_engine_neighbor.cesta.o' 'build/network-engine-transport/obj/src/network_dns_codec.cesta.o' 'build/network-engine-transport/obj/src/network_engine_dns.cesta.o' 'build/network-engine-transport/obj/src/network_engine_transport.cesta.o'

build/network-job-engine/obj/src/network_job_abi.cesta.o: build/.objects/882cdb57c9f38e61/src/network_job_abi.cesta.o
	@mkdir -p 'build/network-job-engine/obj/src'
	ln -f 'build/.objects/882cdb57c9f38e61/src/network_job_abi.cesta.o' "$@"

build/network-job-engine/obj/src/network_job_wire.cesta.o: build/.objects/9a8ea406c8ee08f2/src/network_job_wire.cesta.o
	@mkdir -p 'build/network-job-engine/obj/src'
	ln -f 'build/.objects/9a8ea406c8ee08f2/src/network_job_wire.cesta.o' "$@"

build/network-job-engine/obj/src/network_job_machine.cesta.o: build/.objects/8cf505d61d8c47fc/src/network_job_machine.cesta.o
	@mkdir -p 'build/network-job-engine/obj/src'
	ln -f 'build/.objects/8cf505d61d8c47fc/src/network_job_machine.cesta.o' "$@"

build/network-job-engine/obj/src/network_config.cesta.o: build/.objects/919a04ca0db8978f/src/network_config.cesta.o
	@mkdir -p 'build/network-job-engine/obj/src'
	ln -f 'build/.objects/919a04ca0db8978f/src/network_config.cesta.o' "$@"

build/network-job-engine/obj/src/network_engine_frame.cesta.o: build/.objects/024cac7ba69fe1d5/src/network_engine_frame.cesta.o
	@mkdir -p 'build/network-job-engine/obj/src'
	ln -f 'build/.objects/024cac7ba69fe1d5/src/network_engine_frame.cesta.o' "$@"

build/network-job-engine/obj/src/network_engine_packet.cesta.o: build/.objects/e88be3f5062b9662/src/network_engine_packet.cesta.o
	@mkdir -p 'build/network-job-engine/obj/src'
	ln -f 'build/.objects/e88be3f5062b9662/src/network_engine_packet.cesta.o' "$@"

build/network-job-engine/obj/src/network_engine_tcp_packet.cesta.o: build/.objects/888b66ccba42b1ca/src/network_engine_tcp_packet.cesta.o
	@mkdir -p 'build/network-job-engine/obj/src'
	ln -f 'build/.objects/888b66ccba42b1ca/src/network_engine_tcp_packet.cesta.o' "$@"

build/network-job-engine/obj/src/network_tcp_client.cesta.o: build/.objects/822fca1c24096d10/src/network_tcp_client.cesta.o
	@mkdir -p 'build/network-job-engine/obj/src'
	ln -f 'build/.objects/822fca1c24096d10/src/network_tcp_client.cesta.o' "$@"

build/network-job-engine/obj/src/network_tcp_listener.cesta.o: build/.objects/e75fefcd8742c9e5/src/network_tcp_listener.cesta.o
	@mkdir -p 'build/network-job-engine/obj/src'
	ln -f 'build/.objects/e75fefcd8742c9e5/src/network_tcp_listener.cesta.o' "$@"

build/network-job-engine/obj/src/network_engine_neighbor.cesta.o: build/.objects/85f56b2ae0eb5d88/src/network_engine_neighbor.cesta.o
	@mkdir -p 'build/network-job-engine/obj/src'
	ln -f 'build/.objects/85f56b2ae0eb5d88/src/network_engine_neighbor.cesta.o' "$@"

build/network-job-engine/obj/src/network_dns_codec.cesta.o: build/.objects/0659b3adc088e6d5/src/network_dns_codec.cesta.o
	@mkdir -p 'build/network-job-engine/obj/src'
	ln -f 'build/.objects/0659b3adc088e6d5/src/network_dns_codec.cesta.o' "$@"

build/network-job-engine/obj/src/network_engine_dns.cesta.o: build/.objects/6b81973b41eac44b/src/network_engine_dns.cesta.o
	@mkdir -p 'build/network-job-engine/obj/src'
	ln -f 'build/.objects/6b81973b41eac44b/src/network_engine_dns.cesta.o' "$@"

build/network-job-engine/obj/src/network_engine_transport.cesta.o: build/.objects/0138692a3b4b9fed/src/network_engine_transport.cesta.o
	@mkdir -p 'build/network-job-engine/obj/src'
	ln -f 'build/.objects/0138692a3b4b9fed/src/network_engine_transport.cesta.o' "$@"

build/.objects/5c0ac8f940799344/src/network_job_engine_policy.cesta.o: src/network_job_engine_policy.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_job_engine_policy.c' -o "$@"

build/network-job-engine/obj/src/network_job_engine_policy.cesta.o: build/.objects/5c0ac8f940799344/src/network_job_engine_policy.cesta.o
	@mkdir -p 'build/network-job-engine/obj/src'
	ln -f 'build/.objects/5c0ac8f940799344/src/network_job_engine_policy.cesta.o' "$@"

build/.objects/6e95b75cc7ef0c2f/src/network_job_engine_adapter.cesta.o: src/network_job_engine_adapter.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_job_engine_adapter.c' -o "$@"

build/network-job-engine/obj/src/network_job_engine_adapter.cesta.o: build/.objects/6e95b75cc7ef0c2f/src/network_job_engine_adapter.cesta.o
	@mkdir -p 'build/network-job-engine/obj/src'
	ln -f 'build/.objects/6e95b75cc7ef0c2f/src/network_job_engine_adapter.cesta.o' "$@"

build/network-job-engine/libnetwork-job-engine.a: GNUmakefile build/network-job-engine/obj/src/network_job_abi.cesta.o build/network-job-engine/obj/src/network_job_wire.cesta.o build/network-job-engine/obj/src/network_job_machine.cesta.o build/network-job-engine/obj/src/network_config.cesta.o build/network-job-engine/obj/src/network_engine_frame.cesta.o build/network-job-engine/obj/src/network_engine_packet.cesta.o build/network-job-engine/obj/src/network_engine_tcp_packet.cesta.o build/network-job-engine/obj/src/network_tcp_client.cesta.o build/network-job-engine/obj/src/network_tcp_listener.cesta.o build/network-job-engine/obj/src/network_engine_neighbor.cesta.o build/network-job-engine/obj/src/network_dns_codec.cesta.o build/network-job-engine/obj/src/network_engine_dns.cesta.o build/network-job-engine/obj/src/network_engine_transport.cesta.o build/network-job-engine/obj/src/network_job_engine_policy.cesta.o build/network-job-engine/obj/src/network_job_engine_adapter.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/network-job-engine/obj/src/network_job_abi.cesta.o' 'build/network-job-engine/obj/src/network_job_wire.cesta.o' 'build/network-job-engine/obj/src/network_job_machine.cesta.o' 'build/network-job-engine/obj/src/network_config.cesta.o' 'build/network-job-engine/obj/src/network_engine_frame.cesta.o' 'build/network-job-engine/obj/src/network_engine_packet.cesta.o' 'build/network-job-engine/obj/src/network_engine_tcp_packet.cesta.o' 'build/network-job-engine/obj/src/network_tcp_client.cesta.o' 'build/network-job-engine/obj/src/network_tcp_listener.cesta.o' 'build/network-job-engine/obj/src/network_engine_neighbor.cesta.o' 'build/network-job-engine/obj/src/network_dns_codec.cesta.o' 'build/network-job-engine/obj/src/network_engine_dns.cesta.o' 'build/network-job-engine/obj/src/network_engine_transport.cesta.o' 'build/network-job-engine/obj/src/network_job_engine_policy.cesta.o' 'build/network-job-engine/obj/src/network_job_engine_adapter.cesta.o'

build/network-dns-job-engine/obj/src/network_job_abi.cesta.o: build/.objects/882cdb57c9f38e61/src/network_job_abi.cesta.o
	@mkdir -p 'build/network-dns-job-engine/obj/src'
	ln -f 'build/.objects/882cdb57c9f38e61/src/network_job_abi.cesta.o' "$@"

build/network-dns-job-engine/obj/src/network_job_wire.cesta.o: build/.objects/9a8ea406c8ee08f2/src/network_job_wire.cesta.o
	@mkdir -p 'build/network-dns-job-engine/obj/src'
	ln -f 'build/.objects/9a8ea406c8ee08f2/src/network_job_wire.cesta.o' "$@"

build/network-dns-job-engine/obj/src/network_job_machine.cesta.o: build/.objects/8cf505d61d8c47fc/src/network_job_machine.cesta.o
	@mkdir -p 'build/network-dns-job-engine/obj/src'
	ln -f 'build/.objects/8cf505d61d8c47fc/src/network_job_machine.cesta.o' "$@"

build/network-dns-job-engine/obj/src/network_dns_codec.cesta.o: build/.objects/0659b3adc088e6d5/src/network_dns_codec.cesta.o
	@mkdir -p 'build/network-dns-job-engine/obj/src'
	ln -f 'build/.objects/0659b3adc088e6d5/src/network_dns_codec.cesta.o' "$@"

build/network-dns-job-engine/obj/src/network_dns_job_abi.cesta.o: build/.objects/6c2f8c263229c465/src/network_dns_job_abi.cesta.o
	@mkdir -p 'build/network-dns-job-engine/obj/src'
	ln -f 'build/.objects/6c2f8c263229c465/src/network_dns_job_abi.cesta.o' "$@"

build/network-dns-job-engine/obj/src/network_dns_job_wire.cesta.o: build/.objects/3f1f9e1b3175245e/src/network_dns_job_wire.cesta.o
	@mkdir -p 'build/network-dns-job-engine/obj/src'
	ln -f 'build/.objects/3f1f9e1b3175245e/src/network_dns_job_wire.cesta.o' "$@"

build/network-dns-job-engine/obj/src/network_dns_job_machine.cesta.o: build/.objects/e195fe6a39f61bd8/src/network_dns_job_machine.cesta.o
	@mkdir -p 'build/network-dns-job-engine/obj/src'
	ln -f 'build/.objects/e195fe6a39f61bd8/src/network_dns_job_machine.cesta.o' "$@"

build/network-dns-job-engine/obj/src/network_dns_job_liveness.cesta.o: build/.objects/fe7b6b3b3451bb7e/src/network_dns_job_liveness.cesta.o
	@mkdir -p 'build/network-dns-job-engine/obj/src'
	ln -f 'build/.objects/fe7b6b3b3451bb7e/src/network_dns_job_liveness.cesta.o' "$@"

build/network-dns-job-engine/obj/src/network_config.cesta.o: build/.objects/919a04ca0db8978f/src/network_config.cesta.o
	@mkdir -p 'build/network-dns-job-engine/obj/src'
	ln -f 'build/.objects/919a04ca0db8978f/src/network_config.cesta.o' "$@"

build/network-dns-job-engine/obj/src/network_engine_frame.cesta.o: build/.objects/024cac7ba69fe1d5/src/network_engine_frame.cesta.o
	@mkdir -p 'build/network-dns-job-engine/obj/src'
	ln -f 'build/.objects/024cac7ba69fe1d5/src/network_engine_frame.cesta.o' "$@"

build/network-dns-job-engine/obj/src/network_engine_packet.cesta.o: build/.objects/e88be3f5062b9662/src/network_engine_packet.cesta.o
	@mkdir -p 'build/network-dns-job-engine/obj/src'
	ln -f 'build/.objects/e88be3f5062b9662/src/network_engine_packet.cesta.o' "$@"

build/network-dns-job-engine/obj/src/network_engine_neighbor.cesta.o: build/.objects/85f56b2ae0eb5d88/src/network_engine_neighbor.cesta.o
	@mkdir -p 'build/network-dns-job-engine/obj/src'
	ln -f 'build/.objects/85f56b2ae0eb5d88/src/network_engine_neighbor.cesta.o' "$@"

build/network-dns-job-engine/obj/src/network_engine_dns.cesta.o: build/.objects/6b81973b41eac44b/src/network_engine_dns.cesta.o
	@mkdir -p 'build/network-dns-job-engine/obj/src'
	ln -f 'build/.objects/6b81973b41eac44b/src/network_engine_dns.cesta.o' "$@"

build/network-dns-job-engine/obj/src/network_engine_transport.cesta.o: build/.objects/0138692a3b4b9fed/src/network_engine_transport.cesta.o
	@mkdir -p 'build/network-dns-job-engine/obj/src'
	ln -f 'build/.objects/0138692a3b4b9fed/src/network_engine_transport.cesta.o' "$@"

build/network-dns-job-engine/obj/src/network_engine_tcp_packet.cesta.o: build/.objects/888b66ccba42b1ca/src/network_engine_tcp_packet.cesta.o
	@mkdir -p 'build/network-dns-job-engine/obj/src'
	ln -f 'build/.objects/888b66ccba42b1ca/src/network_engine_tcp_packet.cesta.o' "$@"

build/network-dns-job-engine/obj/src/network_tcp_client.cesta.o: build/.objects/822fca1c24096d10/src/network_tcp_client.cesta.o
	@mkdir -p 'build/network-dns-job-engine/obj/src'
	ln -f 'build/.objects/822fca1c24096d10/src/network_tcp_client.cesta.o' "$@"

build/network-dns-job-engine/obj/src/network_tcp_listener.cesta.o: build/.objects/e75fefcd8742c9e5/src/network_tcp_listener.cesta.o
	@mkdir -p 'build/network-dns-job-engine/obj/src'
	ln -f 'build/.objects/e75fefcd8742c9e5/src/network_tcp_listener.cesta.o' "$@"

build/.objects/c8089ce18a168ed3/src/network_dns_job_engine_adapter.cesta.o: src/network_dns_job_engine_adapter.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_dns_job_engine_adapter.c' -o "$@"

build/network-dns-job-engine/obj/src/network_dns_job_engine_adapter.cesta.o: build/.objects/c8089ce18a168ed3/src/network_dns_job_engine_adapter.cesta.o
	@mkdir -p 'build/network-dns-job-engine/obj/src'
	ln -f 'build/.objects/c8089ce18a168ed3/src/network_dns_job_engine_adapter.cesta.o' "$@"

build/network-dns-job-engine/libnetwork-dns-job-engine.a: GNUmakefile build/network-dns-job-engine/obj/src/network_job_abi.cesta.o build/network-dns-job-engine/obj/src/network_job_wire.cesta.o build/network-dns-job-engine/obj/src/network_job_machine.cesta.o build/network-dns-job-engine/obj/src/network_dns_codec.cesta.o build/network-dns-job-engine/obj/src/network_dns_job_abi.cesta.o build/network-dns-job-engine/obj/src/network_dns_job_wire.cesta.o build/network-dns-job-engine/obj/src/network_dns_job_machine.cesta.o build/network-dns-job-engine/obj/src/network_dns_job_liveness.cesta.o build/network-dns-job-engine/obj/src/network_config.cesta.o build/network-dns-job-engine/obj/src/network_engine_frame.cesta.o build/network-dns-job-engine/obj/src/network_engine_packet.cesta.o build/network-dns-job-engine/obj/src/network_engine_neighbor.cesta.o build/network-dns-job-engine/obj/src/network_engine_dns.cesta.o build/network-dns-job-engine/obj/src/network_engine_transport.cesta.o build/network-dns-job-engine/obj/src/network_engine_tcp_packet.cesta.o build/network-dns-job-engine/obj/src/network_tcp_client.cesta.o build/network-dns-job-engine/obj/src/network_tcp_listener.cesta.o build/network-dns-job-engine/obj/src/network_dns_job_engine_adapter.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/network-dns-job-engine/obj/src/network_job_abi.cesta.o' 'build/network-dns-job-engine/obj/src/network_job_wire.cesta.o' 'build/network-dns-job-engine/obj/src/network_job_machine.cesta.o' 'build/network-dns-job-engine/obj/src/network_dns_codec.cesta.o' 'build/network-dns-job-engine/obj/src/network_dns_job_abi.cesta.o' 'build/network-dns-job-engine/obj/src/network_dns_job_wire.cesta.o' 'build/network-dns-job-engine/obj/src/network_dns_job_machine.cesta.o' 'build/network-dns-job-engine/obj/src/network_dns_job_liveness.cesta.o' 'build/network-dns-job-engine/obj/src/network_config.cesta.o' 'build/network-dns-job-engine/obj/src/network_engine_frame.cesta.o' 'build/network-dns-job-engine/obj/src/network_engine_packet.cesta.o' 'build/network-dns-job-engine/obj/src/network_engine_neighbor.cesta.o' 'build/network-dns-job-engine/obj/src/network_engine_dns.cesta.o' 'build/network-dns-job-engine/obj/src/network_engine_transport.cesta.o' 'build/network-dns-job-engine/obj/src/network_engine_tcp_packet.cesta.o' 'build/network-dns-job-engine/obj/src/network_tcp_client.cesta.o' 'build/network-dns-job-engine/obj/src/network_tcp_listener.cesta.o' 'build/network-dns-job-engine/obj/src/network_dns_job_engine_adapter.cesta.o'

build/network-tcp-listener/obj/src/network_config.cesta.o: build/.objects/919a04ca0db8978f/src/network_config.cesta.o
	@mkdir -p 'build/network-tcp-listener/obj/src'
	ln -f 'build/.objects/919a04ca0db8978f/src/network_config.cesta.o' "$@"

build/network-tcp-listener/obj/src/network_engine_frame.cesta.o: build/.objects/024cac7ba69fe1d5/src/network_engine_frame.cesta.o
	@mkdir -p 'build/network-tcp-listener/obj/src'
	ln -f 'build/.objects/024cac7ba69fe1d5/src/network_engine_frame.cesta.o' "$@"

build/network-tcp-listener/obj/src/network_engine_packet.cesta.o: build/.objects/e88be3f5062b9662/src/network_engine_packet.cesta.o
	@mkdir -p 'build/network-tcp-listener/obj/src'
	ln -f 'build/.objects/e88be3f5062b9662/src/network_engine_packet.cesta.o' "$@"

build/network-tcp-listener/obj/src/network_engine_tcp_packet.cesta.o: build/.objects/888b66ccba42b1ca/src/network_engine_tcp_packet.cesta.o
	@mkdir -p 'build/network-tcp-listener/obj/src'
	ln -f 'build/.objects/888b66ccba42b1ca/src/network_engine_tcp_packet.cesta.o' "$@"

build/network-tcp-listener/obj/src/network_tcp_listener.cesta.o: build/.objects/e75fefcd8742c9e5/src/network_tcp_listener.cesta.o
	@mkdir -p 'build/network-tcp-listener/obj/src'
	ln -f 'build/.objects/e75fefcd8742c9e5/src/network_tcp_listener.cesta.o' "$@"

build/network-tcp-listener/libnetwork-tcp-listener.a: GNUmakefile build/network-tcp-listener/obj/src/network_config.cesta.o build/network-tcp-listener/obj/src/network_engine_frame.cesta.o build/network-tcp-listener/obj/src/network_engine_packet.cesta.o build/network-tcp-listener/obj/src/network_engine_tcp_packet.cesta.o build/network-tcp-listener/obj/src/network_tcp_listener.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/network-tcp-listener/obj/src/network_config.cesta.o' 'build/network-tcp-listener/obj/src/network_engine_frame.cesta.o' 'build/network-tcp-listener/obj/src/network_engine_packet.cesta.o' 'build/network-tcp-listener/obj/src/network_engine_tcp_packet.cesta.o' 'build/network-tcp-listener/obj/src/network_tcp_listener.cesta.o'

build/network-service-core/obj/src/network_job_abi.cesta.o: build/.objects/882cdb57c9f38e61/src/network_job_abi.cesta.o
	@mkdir -p 'build/network-service-core/obj/src'
	ln -f 'build/.objects/882cdb57c9f38e61/src/network_job_abi.cesta.o' "$@"

build/network-service-core/obj/src/network_job_wire.cesta.o: build/.objects/9a8ea406c8ee08f2/src/network_job_wire.cesta.o
	@mkdir -p 'build/network-service-core/obj/src'
	ln -f 'build/.objects/9a8ea406c8ee08f2/src/network_job_wire.cesta.o' "$@"

build/network-service-core/obj/src/network_job_machine.cesta.o: build/.objects/8cf505d61d8c47fc/src/network_job_machine.cesta.o
	@mkdir -p 'build/network-service-core/obj/src'
	ln -f 'build/.objects/8cf505d61d8c47fc/src/network_job_machine.cesta.o' "$@"

build/network-service-core/obj/src/network_dns_job_abi.cesta.o: build/.objects/6c2f8c263229c465/src/network_dns_job_abi.cesta.o
	@mkdir -p 'build/network-service-core/obj/src'
	ln -f 'build/.objects/6c2f8c263229c465/src/network_dns_job_abi.cesta.o' "$@"

build/network-service-core/obj/src/network_dns_job_wire.cesta.o: build/.objects/3f1f9e1b3175245e/src/network_dns_job_wire.cesta.o
	@mkdir -p 'build/network-service-core/obj/src'
	ln -f 'build/.objects/3f1f9e1b3175245e/src/network_dns_job_wire.cesta.o' "$@"

build/network-service-core/obj/src/network_dns_job_machine.cesta.o: build/.objects/e195fe6a39f61bd8/src/network_dns_job_machine.cesta.o
	@mkdir -p 'build/network-service-core/obj/src'
	ln -f 'build/.objects/e195fe6a39f61bd8/src/network_dns_job_machine.cesta.o' "$@"

build/network-service-core/obj/src/network_dns_job_liveness.cesta.o: build/.objects/fe7b6b3b3451bb7e/src/network_dns_job_liveness.cesta.o
	@mkdir -p 'build/network-service-core/obj/src'
	ln -f 'build/.objects/fe7b6b3b3451bb7e/src/network_dns_job_liveness.cesta.o' "$@"

build/network-service-core/obj/src/network_config.cesta.o: build/.objects/919a04ca0db8978f/src/network_config.cesta.o
	@mkdir -p 'build/network-service-core/obj/src'
	ln -f 'build/.objects/919a04ca0db8978f/src/network_config.cesta.o' "$@"

build/network-service-core/obj/src/network_engine_frame.cesta.o: build/.objects/024cac7ba69fe1d5/src/network_engine_frame.cesta.o
	@mkdir -p 'build/network-service-core/obj/src'
	ln -f 'build/.objects/024cac7ba69fe1d5/src/network_engine_frame.cesta.o' "$@"

build/network-service-core/obj/src/network_engine_packet.cesta.o: build/.objects/e88be3f5062b9662/src/network_engine_packet.cesta.o
	@mkdir -p 'build/network-service-core/obj/src'
	ln -f 'build/.objects/e88be3f5062b9662/src/network_engine_packet.cesta.o' "$@"

build/network-service-core/obj/src/network_engine_tcp_packet.cesta.o: build/.objects/888b66ccba42b1ca/src/network_engine_tcp_packet.cesta.o
	@mkdir -p 'build/network-service-core/obj/src'
	ln -f 'build/.objects/888b66ccba42b1ca/src/network_engine_tcp_packet.cesta.o' "$@"

build/network-service-core/obj/src/network_tcp_client.cesta.o: build/.objects/822fca1c24096d10/src/network_tcp_client.cesta.o
	@mkdir -p 'build/network-service-core/obj/src'
	ln -f 'build/.objects/822fca1c24096d10/src/network_tcp_client.cesta.o' "$@"

build/network-service-core/obj/src/network_tcp_listener.cesta.o: build/.objects/e75fefcd8742c9e5/src/network_tcp_listener.cesta.o
	@mkdir -p 'build/network-service-core/obj/src'
	ln -f 'build/.objects/e75fefcd8742c9e5/src/network_tcp_listener.cesta.o' "$@"

build/network-service-core/obj/src/network_engine_neighbor.cesta.o: build/.objects/85f56b2ae0eb5d88/src/network_engine_neighbor.cesta.o
	@mkdir -p 'build/network-service-core/obj/src'
	ln -f 'build/.objects/85f56b2ae0eb5d88/src/network_engine_neighbor.cesta.o' "$@"

build/network-service-core/obj/src/network_dns_codec.cesta.o: build/.objects/0659b3adc088e6d5/src/network_dns_codec.cesta.o
	@mkdir -p 'build/network-service-core/obj/src'
	ln -f 'build/.objects/0659b3adc088e6d5/src/network_dns_codec.cesta.o' "$@"

build/network-service-core/obj/src/network_engine_dns.cesta.o: build/.objects/6b81973b41eac44b/src/network_engine_dns.cesta.o
	@mkdir -p 'build/network-service-core/obj/src'
	ln -f 'build/.objects/6b81973b41eac44b/src/network_engine_dns.cesta.o' "$@"

build/network-service-core/obj/src/network_engine_transport.cesta.o: build/.objects/0138692a3b4b9fed/src/network_engine_transport.cesta.o
	@mkdir -p 'build/network-service-core/obj/src'
	ln -f 'build/.objects/0138692a3b4b9fed/src/network_engine_transport.cesta.o' "$@"

build/network-service-core/obj/src/network_job_engine_policy.cesta.o: build/.objects/5c0ac8f940799344/src/network_job_engine_policy.cesta.o
	@mkdir -p 'build/network-service-core/obj/src'
	ln -f 'build/.objects/5c0ac8f940799344/src/network_job_engine_policy.cesta.o' "$@"

build/network-service-core/obj/src/network_job_engine_adapter.cesta.o: build/.objects/6e95b75cc7ef0c2f/src/network_job_engine_adapter.cesta.o
	@mkdir -p 'build/network-service-core/obj/src'
	ln -f 'build/.objects/6e95b75cc7ef0c2f/src/network_job_engine_adapter.cesta.o' "$@"

build/network-service-core/obj/src/network_dns_job_engine_adapter.cesta.o: build/.objects/c8089ce18a168ed3/src/network_dns_job_engine_adapter.cesta.o
	@mkdir -p 'build/network-service-core/obj/src'
	ln -f 'build/.objects/c8089ce18a168ed3/src/network_dns_job_engine_adapter.cesta.o' "$@"

build/.objects/555b6a8cebc42ee0/src/network_service_native.cesta.o: src/network_service_native.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_service_native.c' -o "$@"

build/network-service-core/obj/src/network_service_native.cesta.o: build/.objects/555b6a8cebc42ee0/src/network_service_native.cesta.o
	@mkdir -p 'build/network-service-core/obj/src'
	ln -f 'build/.objects/555b6a8cebc42ee0/src/network_service_native.cesta.o' "$@"

build/.objects/920258a11d0aaf74/src/network_service_core.cesta.o: src/network_service_core.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_service_core.c' -o "$@"

build/network-service-core/obj/src/network_service_core.cesta.o: build/.objects/920258a11d0aaf74/src/network_service_core.cesta.o
	@mkdir -p 'build/network-service-core/obj/src'
	ln -f 'build/.objects/920258a11d0aaf74/src/network_service_core.cesta.o' "$@"

build/network-service-core/libnetwork-service-core.a: GNUmakefile build/network-service-core/obj/src/network_job_abi.cesta.o build/network-service-core/obj/src/network_job_wire.cesta.o build/network-service-core/obj/src/network_job_machine.cesta.o build/network-service-core/obj/src/network_dns_job_abi.cesta.o build/network-service-core/obj/src/network_dns_job_wire.cesta.o build/network-service-core/obj/src/network_dns_job_machine.cesta.o build/network-service-core/obj/src/network_dns_job_liveness.cesta.o build/network-service-core/obj/src/network_config.cesta.o build/network-service-core/obj/src/network_engine_frame.cesta.o build/network-service-core/obj/src/network_engine_packet.cesta.o build/network-service-core/obj/src/network_engine_tcp_packet.cesta.o build/network-service-core/obj/src/network_tcp_client.cesta.o build/network-service-core/obj/src/network_tcp_listener.cesta.o build/network-service-core/obj/src/network_engine_neighbor.cesta.o build/network-service-core/obj/src/network_dns_codec.cesta.o build/network-service-core/obj/src/network_engine_dns.cesta.o build/network-service-core/obj/src/network_engine_transport.cesta.o build/network-service-core/obj/src/network_job_engine_policy.cesta.o build/network-service-core/obj/src/network_job_engine_adapter.cesta.o build/network-service-core/obj/src/network_dns_job_engine_adapter.cesta.o build/network-service-core/obj/src/network_service_native.cesta.o build/network-service-core/obj/src/network_service_core.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/network-service-core/obj/src/network_job_abi.cesta.o' 'build/network-service-core/obj/src/network_job_wire.cesta.o' 'build/network-service-core/obj/src/network_job_machine.cesta.o' 'build/network-service-core/obj/src/network_dns_job_abi.cesta.o' 'build/network-service-core/obj/src/network_dns_job_wire.cesta.o' 'build/network-service-core/obj/src/network_dns_job_machine.cesta.o' 'build/network-service-core/obj/src/network_dns_job_liveness.cesta.o' 'build/network-service-core/obj/src/network_config.cesta.o' 'build/network-service-core/obj/src/network_engine_frame.cesta.o' 'build/network-service-core/obj/src/network_engine_packet.cesta.o' 'build/network-service-core/obj/src/network_engine_tcp_packet.cesta.o' 'build/network-service-core/obj/src/network_tcp_client.cesta.o' 'build/network-service-core/obj/src/network_tcp_listener.cesta.o' 'build/network-service-core/obj/src/network_engine_neighbor.cesta.o' 'build/network-service-core/obj/src/network_dns_codec.cesta.o' 'build/network-service-core/obj/src/network_engine_dns.cesta.o' 'build/network-service-core/obj/src/network_engine_transport.cesta.o' 'build/network-service-core/obj/src/network_job_engine_policy.cesta.o' 'build/network-service-core/obj/src/network_job_engine_adapter.cesta.o' 'build/network-service-core/obj/src/network_dns_job_engine_adapter.cesta.o' 'build/network-service-core/obj/src/network_service_native.cesta.o' 'build/network-service-core/obj/src/network_service_core.cesta.o'

build/.objects/300853d6e2bbcec4/src/source_policy.cesta.o: src/source_policy.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/source_policy.c' -o "$@"

build/source-policy/obj/src/source_policy.cesta.o: build/.objects/300853d6e2bbcec4/src/source_policy.cesta.o
	@mkdir -p 'build/source-policy/obj/src'
	ln -f 'build/.objects/300853d6e2bbcec4/src/source_policy.cesta.o' "$@"

build/source-policy/libsource-policy.a: GNUmakefile build/source-policy/obj/src/source_policy.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/source-policy/obj/src/source_policy.cesta.o'

build/network-config/obj/src/network_config.cesta.o: build/.objects/919a04ca0db8978f/src/network_config.cesta.o
	@mkdir -p 'build/network-config/obj/src'
	ln -f 'build/.objects/919a04ca0db8978f/src/network_config.cesta.o' "$@"

build/network-config/libnetwork-config.a: GNUmakefile build/network-config/obj/src/network_config.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/network-config/obj/src/network_config.cesta.o'

build/.objects/37d4a9726e925612/src/history_recovery.cesta.o: src/history_recovery.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/history_recovery.c' -o "$@"

build/history-recovery/obj/src/history_recovery.cesta.o: build/.objects/37d4a9726e925612/src/history_recovery.cesta.o
	@mkdir -p 'build/history-recovery/obj/src'
	ln -f 'build/.objects/37d4a9726e925612/src/history_recovery.cesta.o' "$@"

build/.objects/91cc18d4e0c928cd/src/history_decode.cesta.o: src/history_decode.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/history_decode.c' -o "$@"

build/history-recovery/obj/src/history_decode.cesta.o: build/.objects/91cc18d4e0c928cd/src/history_decode.cesta.o
	@mkdir -p 'build/history-recovery/obj/src'
	ln -f 'build/.objects/91cc18d4e0c928cd/src/history_decode.cesta.o' "$@"

build/.objects/9cb7a3a73eee98c5/runtime/history_native.c.o: runtime/history_native.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/history_native.c' -o "$@"

build/history-recovery/obj/runtime/history_native.c.o: build/.objects/9cb7a3a73eee98c5/runtime/history_native.c.o
	@mkdir -p 'build/history-recovery/obj/runtime'
	ln -f 'build/.objects/9cb7a3a73eee98c5/runtime/history_native.c.o' "$@"

build/history-recovery/libhistory-recovery.a: GNUmakefile build/history-recovery/obj/src/history_recovery.cesta.o build/history-recovery/obj/src/history_decode.cesta.o build/history-recovery/obj/runtime/history_native.c.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/history-recovery/obj/src/history_recovery.cesta.o' 'build/history-recovery/obj/src/history_decode.cesta.o' 'build/history-recovery/obj/runtime/history_native.c.o'

build/.objects/8f327338554e95fd/src/pci_policy.cesta.o: src/pci_policy.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/pci_policy.c' -o "$@"

build/pci-policy/obj/src/pci_policy.cesta.o: build/.objects/8f327338554e95fd/src/pci_policy.cesta.o
	@mkdir -p 'build/pci-policy/obj/src'
	ln -f 'build/.objects/8f327338554e95fd/src/pci_policy.cesta.o' "$@"

build/pci-policy/libpci-policy.a: GNUmakefile build/pci-policy/obj/src/pci_policy.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/pci-policy/obj/src/pci_policy.cesta.o'

build/.objects/7ad7c8f21aeaab74/src/frame_lease.cesta.o: src/frame_lease.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/frame_lease.c' -o "$@"

build/frame-lease/obj/src/frame_lease.cesta.o: build/.objects/7ad7c8f21aeaab74/src/frame_lease.cesta.o
	@mkdir -p 'build/frame-lease/obj/src'
	ln -f 'build/.objects/7ad7c8f21aeaab74/src/frame_lease.cesta.o' "$@"

build/frame-lease/libframe-lease.a: GNUmakefile build/frame-lease/obj/src/frame_lease.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/frame-lease/obj/src/frame_lease.cesta.o'

build/.objects/714ae0b7f43fd359/src/program_workers.cesta.o: src/program_workers.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_workers.c' -o "$@"

build/program-workers/obj/src/program_workers.cesta.o: build/.objects/714ae0b7f43fd359/src/program_workers.cesta.o
	@mkdir -p 'build/program-workers/obj/src'
	ln -f 'build/.objects/714ae0b7f43fd359/src/program_workers.cesta.o' "$@"

build/program-workers/libprogram-workers.a: GNUmakefile build/program-workers/obj/src/program_workers.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/program-workers/obj/src/program_workers.cesta.o'

build/.objects/bd58d94e1fe2b977/src/program_records.cesta.o: src/program_records.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_records.c' -o "$@"

build/program-records/obj/src/program_records.cesta.o: build/.objects/bd58d94e1fe2b977/src/program_records.cesta.o
	@mkdir -p 'build/program-records/obj/src'
	ln -f 'build/.objects/bd58d94e1fe2b977/src/program_records.cesta.o' "$@"

build/program-records/libprogram-records.a: GNUmakefile build/program-records/obj/src/program_records.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/program-records/obj/src/program_records.cesta.o'

build/.objects/a8a44464b4c79f6f/src/program_record_memory.cesta.o: src/program_record_memory.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_record_memory.c' -o "$@"

build/program-record-memory/obj/src/program_record_memory.cesta.o: build/.objects/a8a44464b4c79f6f/src/program_record_memory.cesta.o
	@mkdir -p 'build/program-record-memory/obj/src'
	ln -f 'build/.objects/a8a44464b4c79f6f/src/program_record_memory.cesta.o' "$@"

build/program-record-memory/libprogram-record-memory.a: GNUmakefile build/program-record-memory/obj/src/program_record_memory.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/program-record-memory/obj/src/program_record_memory.cesta.o'

build/.objects/5307e9de474d2ad0/src/program_record_tasks.cesta.o: src/program_record_tasks.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_record_tasks.c' -o "$@"

build/program-record-tasks/obj/src/program_record_tasks.cesta.o: build/.objects/5307e9de474d2ad0/src/program_record_tasks.cesta.o
	@mkdir -p 'build/program-record-tasks/obj/src'
	ln -f 'build/.objects/5307e9de474d2ad0/src/program_record_tasks.cesta.o' "$@"

build/program-record-tasks/libprogram-record-tasks.a: GNUmakefile build/program-record-tasks/obj/src/program_record_tasks.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/program-record-tasks/obj/src/program_record_tasks.cesta.o'

build/.objects/5b3d5ea5fc624ea3/src/program_memory.cesta.o: src/program_memory.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_memory.c' -o "$@"

build/program-memory/obj/src/program_memory.cesta.o: build/.objects/5b3d5ea5fc624ea3/src/program_memory.cesta.o
	@mkdir -p 'build/program-memory/obj/src'
	ln -f 'build/.objects/5b3d5ea5fc624ea3/src/program_memory.cesta.o' "$@"

build/program-memory/libprogram-memory.a: GNUmakefile build/program-memory/obj/src/program_memory.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/program-memory/obj/src/program_memory.cesta.o'

build/.objects/41a91d0b62db45c8/src/epoch_journal.cesta.o: src/epoch_journal.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/epoch_journal.c' -o "$@"

build/epoch-journal/obj/src/epoch_journal.cesta.o: build/.objects/41a91d0b62db45c8/src/epoch_journal.cesta.o
	@mkdir -p 'build/epoch-journal/obj/src'
	ln -f 'build/.objects/41a91d0b62db45c8/src/epoch_journal.cesta.o' "$@"

build/epoch-journal/libepoch-journal.a: GNUmakefile build/epoch-journal/obj/src/epoch_journal.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/epoch-journal/obj/src/epoch_journal.cesta.o'

build/epoch-files/obj/src/epoch_journal.cesta.o: build/.objects/41a91d0b62db45c8/src/epoch_journal.cesta.o
	@mkdir -p 'build/epoch-files/obj/src'
	ln -f 'build/.objects/41a91d0b62db45c8/src/epoch_journal.cesta.o' "$@"

build/.objects/117192a38961a030/src/epoch_files.cesta.o: src/epoch_files.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/epoch_files.c' -o "$@"

build/epoch-files/obj/src/epoch_files.cesta.o: build/.objects/117192a38961a030/src/epoch_files.cesta.o
	@mkdir -p 'build/epoch-files/obj/src'
	ln -f 'build/.objects/117192a38961a030/src/epoch_files.cesta.o' "$@"

build/epoch-files/libepoch-files.a: GNUmakefile build/epoch-files/obj/src/epoch_journal.cesta.o build/epoch-files/obj/src/epoch_files.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/epoch-files/obj/src/epoch_journal.cesta.o' 'build/epoch-files/obj/src/epoch_files.cesta.o'

build/.objects/5e0404e689a31107/src/pager_state.cesta.o: src/pager_state.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/pager_state.c' -o "$@"

build/pager-state/obj/src/pager_state.cesta.o: build/.objects/5e0404e689a31107/src/pager_state.cesta.o
	@mkdir -p 'build/pager-state/obj/src'
	ln -f 'build/.objects/5e0404e689a31107/src/pager_state.cesta.o' "$@"

build/pager-state/libpager-state.a: GNUmakefile build/pager-state/obj/src/pager_state.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/pager-state/obj/src/pager_state.cesta.o'

build/.objects/ff6c348ceaffb34d/src/app_private.cesta.o: src/app_private.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/app_private.c' -o "$@"

build/app-private/obj/src/app_private.cesta.o: build/.objects/ff6c348ceaffb34d/src/app_private.cesta.o
	@mkdir -p 'build/app-private/obj/src'
	ln -f 'build/.objects/ff6c348ceaffb34d/src/app_private.cesta.o' "$@"

build/app-private/libapp-private.a: GNUmakefile build/app-private/obj/src/app_private.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/app-private/obj/src/app_private.cesta.o'

build/.objects/73827375c911ad0b/src/memory_regions.cesta.o: src/memory_regions.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/memory_regions.c' -o "$@"

build/memory-regions/obj/src/memory_regions.cesta.o: build/.objects/73827375c911ad0b/src/memory_regions.cesta.o
	@mkdir -p 'build/memory-regions/obj/src'
	ln -f 'build/.objects/73827375c911ad0b/src/memory_regions.cesta.o' "$@"

build/memory-regions/libmemory-regions.a: GNUmakefile build/memory-regions/obj/src/memory_regions.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/memory-regions/obj/src/memory_regions.cesta.o'

build/.objects/f5acdd013d8ef9d2/src/region_writer.cesta.o: src/region_writer.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/region_writer.c' -o "$@"

build/region-writer/obj/src/region_writer.cesta.o: build/.objects/f5acdd013d8ef9d2/src/region_writer.cesta.o
	@mkdir -p 'build/region-writer/obj/src'
	ln -f 'build/.objects/f5acdd013d8ef9d2/src/region_writer.cesta.o' "$@"

build/region-writer/libregion-writer.a: GNUmakefile build/region-writer/obj/src/region_writer.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/region-writer/obj/src/region_writer.cesta.o'

build/.objects/2fbc420df317f099/src/dma_lease.cesta.o: src/dma_lease.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/dma_lease.c' -o "$@"

build/dma-lease/obj/src/dma_lease.cesta.o: build/.objects/2fbc420df317f099/src/dma_lease.cesta.o
	@mkdir -p 'build/dma-lease/obj/src'
	ln -f 'build/.objects/2fbc420df317f099/src/dma_lease.cesta.o' "$@"

build/dma-lease/libdma-lease.a: GNUmakefile build/dma-lease/obj/src/dma_lease.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/dma-lease/obj/src/dma_lease.cesta.o'

build/.objects/b89c7c2a2c6d067a/src/device_reset.cesta.o: src/device_reset.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/device_reset.c' -o "$@"

build/device-reset/obj/src/device_reset.cesta.o: build/.objects/b89c7c2a2c6d067a/src/device_reset.cesta.o
	@mkdir -p 'build/device-reset/obj/src'
	ln -f 'build/.objects/b89c7c2a2c6d067a/src/device_reset.cesta.o' "$@"

build/device-reset/libdevice-reset.a: GNUmakefile build/device-reset/obj/src/device_reset.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/device-reset/obj/src/device_reset.cesta.o'

build/.objects/7d8f4be1d506cd52/src/nvme_msix.cesta.o: src/nvme_msix.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/nvme_msix.c' -o "$@"

build/nvme-msix/obj/src/nvme_msix.cesta.o: build/.objects/7d8f4be1d506cd52/src/nvme_msix.cesta.o
	@mkdir -p 'build/nvme-msix/obj/src'
	ln -f 'build/.objects/7d8f4be1d506cd52/src/nvme_msix.cesta.o' "$@"

build/nvme-msix/libnvme-msix.a: GNUmakefile build/nvme-msix/obj/src/nvme_msix.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/nvme-msix/obj/src/nvme_msix.cesta.o'

build/.objects/321ac8cbb497f61f/src/smp_park.cesta.o: src/smp_park.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/smp_park.c' -o "$@"

build/smp-park/obj/src/smp_park.cesta.o: build/.objects/321ac8cbb497f61f/src/smp_park.cesta.o
	@mkdir -p 'build/smp-park/obj/src'
	ln -f 'build/.objects/321ac8cbb497f61f/src/smp_park.cesta.o' "$@"

build/smp-park/libsmp-park.a: GNUmakefile build/smp-park/obj/src/smp_park.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/smp-park/obj/src/smp_park.cesta.o'

build/.objects/d50adccbf9b0827d/src/arm64_boot_policy.cesta.o: src/arm64_boot_policy.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/arm64/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/arm64_boot_policy.c' -o "$@"

build/arm64-boot-policy/obj/src/arm64_boot_policy.cesta.o: build/.objects/d50adccbf9b0827d/src/arm64_boot_policy.cesta.o
	@mkdir -p 'build/arm64-boot-policy/obj/src'
	ln -f 'build/.objects/d50adccbf9b0827d/src/arm64_boot_policy.cesta.o' "$@"

build/arm64-boot-policy/libarm64-boot-policy.a: GNUmakefile build/arm64-boot-policy/obj/src/arm64_boot_policy.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/arm64-boot-policy/obj/src/arm64_boot_policy.cesta.o'

build/.objects/49ef79b61298bd50/src/frame_lease.cesta.o: src/frame_lease.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/frame_lease.c' -o "$@"

build/sexos-kernel/obj/src/frame_lease.cesta.o: build/.objects/49ef79b61298bd50/src/frame_lease.cesta.o
	@mkdir -p 'build/sexos-kernel/obj/src'
	ln -f 'build/.objects/49ef79b61298bd50/src/frame_lease.cesta.o' "$@"

build/.objects/404e820c25149481/src/kernel_memory.cesta.o: src/kernel_memory.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_memory.c' -o "$@"

build/sexos-kernel/obj/src/kernel_memory.cesta.o: build/.objects/404e820c25149481/src/kernel_memory.cesta.o
	@mkdir -p 'build/sexos-kernel/obj/src'
	ln -f 'build/.objects/404e820c25149481/src/kernel_memory.cesta.o' "$@"

build/.objects/11abe54590f4ffaa/src/program_memory.cesta.o: src/program_memory.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_memory.c' -o "$@"

build/sexos-kernel/obj/src/program_memory.cesta.o: build/.objects/11abe54590f4ffaa/src/program_memory.cesta.o
	@mkdir -p 'build/sexos-kernel/obj/src'
	ln -f 'build/.objects/11abe54590f4ffaa/src/program_memory.cesta.o' "$@"

build/.objects/be1fbe860b9e6b10/src/program_workers.cesta.o: src/program_workers.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_workers.c' -o "$@"

build/sexos-kernel/obj/src/program_workers.cesta.o: build/.objects/be1fbe860b9e6b10/src/program_workers.cesta.o
	@mkdir -p 'build/sexos-kernel/obj/src'
	ln -f 'build/.objects/be1fbe860b9e6b10/src/program_workers.cesta.o' "$@"

build/.objects/6383d87eaff6a925/src/kernel_workers.cesta.o: src/kernel_workers.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_workers.c' -o "$@"

build/sexos-kernel/obj/src/kernel_workers.cesta.o: build/.objects/6383d87eaff6a925/src/kernel_workers.cesta.o
	@mkdir -p 'build/sexos-kernel/obj/src'
	ln -f 'build/.objects/6383d87eaff6a925/src/kernel_workers.cesta.o' "$@"

build/.objects/f77eb7ca88761d09/src/kernel_platform.cesta.o: src/kernel_platform.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_platform.c' -o "$@"

build/sexos-kernel/obj/src/kernel_platform.cesta.o: build/.objects/f77eb7ca88761d09/src/kernel_platform.cesta.o
	@mkdir -p 'build/sexos-kernel/obj/src'
	ln -f 'build/.objects/f77eb7ca88761d09/src/kernel_platform.cesta.o' "$@"

build/.objects/88c113b318615167/src/kernel_domains.cesta.o: src/kernel_domains.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_domains.c' -o "$@"

build/sexos-kernel/obj/src/kernel_domains.cesta.o: build/.objects/88c113b318615167/src/kernel_domains.cesta.o
	@mkdir -p 'build/sexos-kernel/obj/src'
	ln -f 'build/.objects/88c113b318615167/src/kernel_domains.cesta.o' "$@"

build/.objects/e764816723c52546/src/program_image.cesta.o: src/program_image.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_image.c' -o "$@"

build/sexos-kernel/obj/src/program_image.cesta.o: build/.objects/e764816723c52546/src/program_image.cesta.o
	@mkdir -p 'build/sexos-kernel/obj/src'
	ln -f 'build/.objects/e764816723c52546/src/program_image.cesta.o' "$@"

build/.objects/3d3438b6380e91e2/src/program_image_bytes.cesta.o: src/program_image_bytes.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_image_bytes.c' -o "$@"

build/sexos-kernel/obj/src/program_image_bytes.cesta.o: build/.objects/3d3438b6380e91e2/src/program_image_bytes.cesta.o
	@mkdir -p 'build/sexos-kernel/obj/src'
	ln -f 'build/.objects/3d3438b6380e91e2/src/program_image_bytes.cesta.o' "$@"

build/.objects/02fd6e45102bd17b/runtime/kernel/image_failure.c.o: runtime/kernel/image_failure.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/kernel/image_failure.c' -o "$@"

build/sexos-kernel/obj/runtime/kernel/image_failure.c.o: build/.objects/02fd6e45102bd17b/runtime/kernel/image_failure.c.o
	@mkdir -p 'build/sexos-kernel/obj/runtime/kernel'
	ln -f 'build/.objects/02fd6e45102bd17b/runtime/kernel/image_failure.c.o' "$@"

build/.objects/8400930a7ebd8dea/src/kernel_entry.cesta.o: src/kernel_entry.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_entry.c' -o "$@"

build/sexos-kernel/obj/src/kernel_entry.cesta.o: build/.objects/8400930a7ebd8dea/src/kernel_entry.cesta.o
	@mkdir -p 'build/sexos-kernel/obj/src'
	ln -f 'build/.objects/8400930a7ebd8dea/src/kernel_entry.cesta.o' "$@"

build/.objects/b763b7ea62324364/src/kernel_syscalls.cesta.o: src/kernel_syscalls.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_syscalls.c' -o "$@"

build/sexos-kernel/obj/src/kernel_syscalls.cesta.o: build/.objects/b763b7ea62324364/src/kernel_syscalls.cesta.o
	@mkdir -p 'build/sexos-kernel/obj/src'
	ln -f 'build/.objects/b763b7ea62324364/src/kernel_syscalls.cesta.o' "$@"

build/.objects/0696e24a087abec1/src/kernel_rpc.cesta.o: src/kernel_rpc.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_rpc.c' -o "$@"

build/sexos-kernel/obj/src/kernel_rpc.cesta.o: build/.objects/0696e24a087abec1/src/kernel_rpc.cesta.o
	@mkdir -p 'build/sexos-kernel/obj/src'
	ln -f 'build/.objects/0696e24a087abec1/src/kernel_rpc.cesta.o' "$@"

build/.objects/340b0a1cf643e20d/src/memory_regions.cesta.o: src/memory_regions.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/memory_regions.c' -o "$@"

build/sexos-kernel/obj/src/memory_regions.cesta.o: build/.objects/340b0a1cf643e20d/src/memory_regions.cesta.o
	@mkdir -p 'build/sexos-kernel/obj/src'
	ln -f 'build/.objects/340b0a1cf643e20d/src/memory_regions.cesta.o' "$@"

build/.objects/6c916b9add3da93e/src/region_writer.cesta.o: src/region_writer.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/region_writer.c' -o "$@"

build/sexos-kernel/obj/src/region_writer.cesta.o: build/.objects/6c916b9add3da93e/src/region_writer.cesta.o
	@mkdir -p 'build/sexos-kernel/obj/src'
	ln -f 'build/.objects/6c916b9add3da93e/src/region_writer.cesta.o' "$@"

build/.objects/4177aaf5d7ec2016/src/device_reset.cesta.o: src/device_reset.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/device_reset.c' -o "$@"

build/sexos-kernel/obj/src/device_reset.cesta.o: build/.objects/4177aaf5d7ec2016/src/device_reset.cesta.o
	@mkdir -p 'build/sexos-kernel/obj/src'
	ln -f 'build/.objects/4177aaf5d7ec2016/src/device_reset.cesta.o' "$@"

build/.objects/089192bdd6dbdfe5/src/epoch_journal.cesta.o: src/epoch_journal.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/epoch_journal.c' -o "$@"

build/sexos-kernel/obj/src/epoch_journal.cesta.o: build/.objects/089192bdd6dbdfe5/src/epoch_journal.cesta.o
	@mkdir -p 'build/sexos-kernel/obj/src'
	ln -f 'build/.objects/089192bdd6dbdfe5/src/epoch_journal.cesta.o' "$@"

build/.objects/0ecbc475992172da/src/kernel_epoch_log.cesta.o: src/kernel_epoch_log.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_epoch_log.c' -o "$@"

build/sexos-kernel/obj/src/kernel_epoch_log.cesta.o: build/.objects/0ecbc475992172da/src/kernel_epoch_log.cesta.o
	@mkdir -p 'build/sexos-kernel/obj/src'
	ln -f 'build/.objects/0ecbc475992172da/src/kernel_epoch_log.cesta.o' "$@"

build/.objects/1721c290738a8898/src/kernel_boot_memory.cesta.o: src/kernel_boot_memory.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_boot_memory.c' -o "$@"

build/sexos-kernel/obj/src/kernel_boot_memory.cesta.o: build/.objects/1721c290738a8898/src/kernel_boot_memory.cesta.o
	@mkdir -p 'build/sexos-kernel/obj/src'
	ln -f 'build/.objects/1721c290738a8898/src/kernel_boot_memory.cesta.o' "$@"

build/.objects/a7933b50241ca028/src/kernel_region_writer.cesta.o: src/kernel_region_writer.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_region_writer.c' -o "$@"

build/sexos-kernel/obj/src/kernel_region_writer.cesta.o: build/.objects/a7933b50241ca028/src/kernel_region_writer.cesta.o
	@mkdir -p 'build/sexos-kernel/obj/src'
	ln -f 'build/.objects/a7933b50241ca028/src/kernel_region_writer.cesta.o' "$@"

build/.objects/6f0f101b035f1934/src/kernel_device_interrupt.cesta.o: src/kernel_device_interrupt.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_device_interrupt.c' -o "$@"

build/sexos-kernel/obj/src/kernel_device_interrupt.cesta.o: build/.objects/6f0f101b035f1934/src/kernel_device_interrupt.cesta.o
	@mkdir -p 'build/sexos-kernel/obj/src'
	ln -f 'build/.objects/6f0f101b035f1934/src/kernel_device_interrupt.cesta.o' "$@"

build/.objects/bdade6eb55a80ab8/src/kernel_device_reset.cesta.o: src/kernel_device_reset.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_device_reset.c' -o "$@"

build/sexos-kernel/obj/src/kernel_device_reset.cesta.o: build/.objects/bdade6eb55a80ab8/src/kernel_device_reset.cesta.o
	@mkdir -p 'build/sexos-kernel/obj/src'
	ln -f 'build/.objects/bdade6eb55a80ab8/src/kernel_device_reset.cesta.o' "$@"

build/.objects/b8638b43a267347f/src/app_private.cesta.o: src/app_private.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/app_private.c' -o "$@"

build/sexos-kernel/obj/src/app_private.cesta.o: build/.objects/b8638b43a267347f/src/app_private.cesta.o
	@mkdir -p 'build/sexos-kernel/obj/src'
	ln -f 'build/.objects/b8638b43a267347f/src/app_private.cesta.o' "$@"

build/.objects/f9cf126577129dee/src/kernel_apps.cesta.o: src/kernel_apps.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_apps.c' -o "$@"

build/sexos-kernel/obj/src/kernel_apps.cesta.o: build/.objects/f9cf126577129dee/src/kernel_apps.cesta.o
	@mkdir -p 'build/sexos-kernel/obj/src'
	ln -f 'build/.objects/f9cf126577129dee/src/kernel_apps.cesta.o' "$@"

build/.objects/8b7bfc4de60c41e9/src/program_records.cesta.o: src/program_records.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_records.c' -o "$@"

build/sexos-kernel/obj/src/program_records.cesta.o: build/.objects/8b7bfc4de60c41e9/src/program_records.cesta.o
	@mkdir -p 'build/sexos-kernel/obj/src'
	ln -f 'build/.objects/8b7bfc4de60c41e9/src/program_records.cesta.o' "$@"

build/.objects/93e8a7acc874f49e/src/program_record_memory.cesta.o: src/program_record_memory.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_record_memory.c' -o "$@"

build/sexos-kernel/obj/src/program_record_memory.cesta.o: build/.objects/93e8a7acc874f49e/src/program_record_memory.cesta.o
	@mkdir -p 'build/sexos-kernel/obj/src'
	ln -f 'build/.objects/93e8a7acc874f49e/src/program_record_memory.cesta.o' "$@"

build/.objects/ee2f71846910690d/src/program_record_tasks.cesta.o: src/program_record_tasks.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_record_tasks.c' -o "$@"

build/sexos-kernel/obj/src/program_record_tasks.cesta.o: build/.objects/ee2f71846910690d/src/program_record_tasks.cesta.o
	@mkdir -p 'build/sexos-kernel/obj/src'
	ln -f 'build/.objects/ee2f71846910690d/src/program_record_tasks.cesta.o' "$@"

build/.objects/9ac6c21ec2e573dc/src/kernel_records.cesta.o: src/kernel_records.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_records.c' -o "$@"

build/sexos-kernel/obj/src/kernel_records.cesta.o: build/.objects/9ac6c21ec2e573dc/src/kernel_records.cesta.o
	@mkdir -p 'build/sexos-kernel/obj/src'
	ln -f 'build/.objects/9ac6c21ec2e573dc/src/kernel_records.cesta.o' "$@"

build/.objects/5c8a8be44e917664/src/kernel_current_boot.cesta.o: src/kernel_current_boot.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_current_boot.c' -o "$@"

build/sexos-kernel/obj/src/kernel_current_boot.cesta.o: build/.objects/5c8a8be44e917664/src/kernel_current_boot.cesta.o
	@mkdir -p 'build/sexos-kernel/obj/src'
	ln -f 'build/.objects/5c8a8be44e917664/src/kernel_current_boot.cesta.o' "$@"

build/.objects/b68a72593bd3eb02/src/kernel_paging.cesta.o: src/kernel_paging.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_paging.c' -o "$@"

build/sexos-kernel/obj/src/kernel_paging.cesta.o: build/.objects/b68a72593bd3eb02/src/kernel_paging.cesta.o
	@mkdir -p 'build/sexos-kernel/obj/src'
	ln -f 'build/.objects/b68a72593bd3eb02/src/kernel_paging.cesta.o' "$@"

build/.objects/fa43ab82c5b00d71/src/kernel_task_wait.cesta.o: src/kernel_task_wait.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_task_wait.c' -o "$@"

build/sexos-kernel/obj/src/kernel_task_wait.cesta.o: build/.objects/fa43ab82c5b00d71/src/kernel_task_wait.cesta.o
	@mkdir -p 'build/sexos-kernel/obj/src'
	ln -f 'build/.objects/fa43ab82c5b00d71/src/kernel_task_wait.cesta.o' "$@"

build/.objects/145b35cbc54eb019/src/pager_state.cesta.o: src/pager_state.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/pager_state.c' -o "$@"

build/sexos-kernel/obj/src/pager_state.cesta.o: build/.objects/145b35cbc54eb019/src/pager_state.cesta.o
	@mkdir -p 'build/sexos-kernel/obj/src'
	ln -f 'build/.objects/145b35cbc54eb019/src/pager_state.cesta.o' "$@"

build/.objects/e6e2a0f72e32a140/src/source_policy.cesta.o: src/source_policy.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/source_policy.c' -o "$@"

build/sexos-kernel/obj/src/source_policy.cesta.o: build/.objects/e6e2a0f72e32a140/src/source_policy.cesta.o
	@mkdir -p 'build/sexos-kernel/obj/src'
	ln -f 'build/.objects/e6e2a0f72e32a140/src/source_policy.cesta.o' "$@"

build/.objects/b596d9e2286030fc/src/kernel_sources.cesta.o: src/kernel_sources.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_sources.c' -o "$@"

build/sexos-kernel/obj/src/kernel_sources.cesta.o: build/.objects/b596d9e2286030fc/src/kernel_sources.cesta.o
	@mkdir -p 'build/sexos-kernel/obj/src'
	ln -f 'build/.objects/b596d9e2286030fc/src/kernel_sources.cesta.o' "$@"

build/.objects/724ad31eab3ac631/src/network_config.cesta.o: src/network_config.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_config.c' -o "$@"

build/sexos-kernel/obj/src/network_config.cesta.o: build/.objects/724ad31eab3ac631/src/network_config.cesta.o
	@mkdir -p 'build/sexos-kernel/obj/src'
	ln -f 'build/.objects/724ad31eab3ac631/src/network_config.cesta.o' "$@"

build/.objects/5d48a43b3db4080f/src/pci_policy.cesta.o: src/pci_policy.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/pci_policy.c' -o "$@"

build/sexos-kernel/obj/src/pci_policy.cesta.o: build/.objects/5d48a43b3db4080f/src/pci_policy.cesta.o
	@mkdir -p 'build/sexos-kernel/obj/src'
	ln -f 'build/.objects/5d48a43b3db4080f/src/pci_policy.cesta.o' "$@"

build/.objects/d805b10cfc8f7c66/src/kernel_pci.cesta.o: src/kernel_pci.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_pci.c' -o "$@"

build/sexos-kernel/obj/src/kernel_pci.cesta.o: build/.objects/d805b10cfc8f7c66/src/kernel_pci.cesta.o
	@mkdir -p 'build/sexos-kernel/obj/src'
	ln -f 'build/.objects/d805b10cfc8f7c66/src/kernel_pci.cesta.o' "$@"

build/.objects/a6c3d4299b7d34fe/src/history_recovery.cesta.o: src/history_recovery.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/history_recovery.c' -o "$@"

build/sexos-kernel/obj/src/history_recovery.cesta.o: build/.objects/a6c3d4299b7d34fe/src/history_recovery.cesta.o
	@mkdir -p 'build/sexos-kernel/obj/src'
	ln -f 'build/.objects/a6c3d4299b7d34fe/src/history_recovery.cesta.o' "$@"

build/.objects/33e8eeed5295d45e/src/kernel_history.cesta.o: src/kernel_history.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_history.c' -o "$@"

build/sexos-kernel/obj/src/kernel_history.cesta.o: build/.objects/33e8eeed5295d45e/src/kernel_history.cesta.o
	@mkdir -p 'build/sexos-kernel/obj/src'
	ln -f 'build/.objects/33e8eeed5295d45e/src/kernel_history.cesta.o' "$@"

build/.objects/9a2a651c28d41fe7/runtime/kernel/kernel_log.c.o: runtime/kernel/kernel_log.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/kernel/kernel_log.c' -o "$@"

build/sexos-kernel/obj/runtime/kernel/kernel_log.c.o: build/.objects/9a2a651c28d41fe7/runtime/kernel/kernel_log.c.o
	@mkdir -p 'build/sexos-kernel/obj/runtime/kernel'
	ln -f 'build/.objects/9a2a651c28d41fe7/runtime/kernel/kernel_log.c.o' "$@"

build/.objects/d5247f505bb49b6f/runtime/kernel/entry_x86_64.S.o: runtime/kernel/entry_x86_64.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/kernel/entry_x86_64.S' -o "$@"

build/sexos-kernel/obj/runtime/kernel/entry_x86_64.S.o: build/.objects/d5247f505bb49b6f/runtime/kernel/entry_x86_64.S.o
	@mkdir -p 'build/sexos-kernel/obj/runtime/kernel'
	ln -f 'build/.objects/d5247f505bb49b6f/runtime/kernel/entry_x86_64.S.o' "$@"

build/sexos-kernel/sexos-kernel: GNUmakefile build/sexos-kernel/obj/src/frame_lease.cesta.o build/sexos-kernel/obj/src/kernel_memory.cesta.o build/sexos-kernel/obj/src/program_memory.cesta.o build/sexos-kernel/obj/src/program_workers.cesta.o build/sexos-kernel/obj/src/kernel_workers.cesta.o build/sexos-kernel/obj/src/kernel_platform.cesta.o build/sexos-kernel/obj/src/kernel_domains.cesta.o build/sexos-kernel/obj/src/program_image.cesta.o build/sexos-kernel/obj/src/program_image_bytes.cesta.o build/sexos-kernel/obj/runtime/kernel/image_failure.c.o build/sexos-kernel/obj/src/kernel_entry.cesta.o build/sexos-kernel/obj/src/kernel_syscalls.cesta.o build/sexos-kernel/obj/src/kernel_rpc.cesta.o build/sexos-kernel/obj/src/memory_regions.cesta.o build/sexos-kernel/obj/src/region_writer.cesta.o build/sexos-kernel/obj/src/device_reset.cesta.o build/sexos-kernel/obj/src/epoch_journal.cesta.o build/sexos-kernel/obj/src/kernel_epoch_log.cesta.o build/sexos-kernel/obj/src/kernel_boot_memory.cesta.o build/sexos-kernel/obj/src/kernel_region_writer.cesta.o build/sexos-kernel/obj/src/kernel_device_interrupt.cesta.o build/sexos-kernel/obj/src/kernel_device_reset.cesta.o build/sexos-kernel/obj/src/app_private.cesta.o build/sexos-kernel/obj/src/kernel_apps.cesta.o build/sexos-kernel/obj/src/program_records.cesta.o build/sexos-kernel/obj/src/program_record_memory.cesta.o build/sexos-kernel/obj/src/program_record_tasks.cesta.o build/sexos-kernel/obj/src/kernel_records.cesta.o build/sexos-kernel/obj/src/kernel_current_boot.cesta.o build/sexos-kernel/obj/src/kernel_paging.cesta.o build/sexos-kernel/obj/src/kernel_task_wait.cesta.o build/sexos-kernel/obj/src/pager_state.cesta.o build/sexos-kernel/obj/src/source_policy.cesta.o build/sexos-kernel/obj/src/kernel_sources.cesta.o build/sexos-kernel/obj/src/network_config.cesta.o build/sexos-kernel/obj/src/pci_policy.cesta.o build/sexos-kernel/obj/src/kernel_pci.cesta.o build/sexos-kernel/obj/src/history_recovery.cesta.o build/sexos-kernel/obj/src/kernel_history.cesta.o build/sexos-kernel/obj/runtime/kernel/kernel_log.c.o build/sexos-kernel/obj/runtime/kernel/entry_x86_64.S.o runtime/kernel/kernel.ld
	@mkdir -p "$(@D)"
	$(CC) 'build/sexos-kernel/obj/src/frame_lease.cesta.o' 'build/sexos-kernel/obj/src/kernel_memory.cesta.o' 'build/sexos-kernel/obj/src/program_memory.cesta.o' 'build/sexos-kernel/obj/src/program_workers.cesta.o' 'build/sexos-kernel/obj/src/kernel_workers.cesta.o' 'build/sexos-kernel/obj/src/kernel_platform.cesta.o' 'build/sexos-kernel/obj/src/kernel_domains.cesta.o' 'build/sexos-kernel/obj/src/program_image.cesta.o' 'build/sexos-kernel/obj/src/program_image_bytes.cesta.o' 'build/sexos-kernel/obj/runtime/kernel/image_failure.c.o' 'build/sexos-kernel/obj/src/kernel_entry.cesta.o' 'build/sexos-kernel/obj/src/kernel_syscalls.cesta.o' 'build/sexos-kernel/obj/src/kernel_rpc.cesta.o' 'build/sexos-kernel/obj/src/memory_regions.cesta.o' 'build/sexos-kernel/obj/src/region_writer.cesta.o' 'build/sexos-kernel/obj/src/device_reset.cesta.o' 'build/sexos-kernel/obj/src/epoch_journal.cesta.o' 'build/sexos-kernel/obj/src/kernel_epoch_log.cesta.o' 'build/sexos-kernel/obj/src/kernel_boot_memory.cesta.o' 'build/sexos-kernel/obj/src/kernel_region_writer.cesta.o' 'build/sexos-kernel/obj/src/kernel_device_interrupt.cesta.o' 'build/sexos-kernel/obj/src/kernel_device_reset.cesta.o' 'build/sexos-kernel/obj/src/app_private.cesta.o' 'build/sexos-kernel/obj/src/kernel_apps.cesta.o' 'build/sexos-kernel/obj/src/program_records.cesta.o' 'build/sexos-kernel/obj/src/program_record_memory.cesta.o' 'build/sexos-kernel/obj/src/program_record_tasks.cesta.o' 'build/sexos-kernel/obj/src/kernel_records.cesta.o' 'build/sexos-kernel/obj/src/kernel_current_boot.cesta.o' 'build/sexos-kernel/obj/src/kernel_paging.cesta.o' 'build/sexos-kernel/obj/src/kernel_task_wait.cesta.o' 'build/sexos-kernel/obj/src/pager_state.cesta.o' 'build/sexos-kernel/obj/src/source_policy.cesta.o' 'build/sexos-kernel/obj/src/kernel_sources.cesta.o' 'build/sexos-kernel/obj/src/network_config.cesta.o' 'build/sexos-kernel/obj/src/pci_policy.cesta.o' 'build/sexos-kernel/obj/src/kernel_pci.cesta.o' 'build/sexos-kernel/obj/src/history_recovery.cesta.o' 'build/sexos-kernel/obj/src/kernel_history.cesta.o' 'build/sexos-kernel/obj/runtime/kernel/kernel_log.c.o' 'build/sexos-kernel/obj/runtime/kernel/entry_x86_64.S.o' '-nostdlib' '-static' '-Wl,-T,runtime/kernel/kernel.ld' '-Wl,--build-id=none' '-Wl,-z,max-page-size=0x1000' -o "$@"

build/.objects/5d66cc14d65071ca/src/silk_shell_models.cesta.o: src/silk_shell_models.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_LINEN_CORRELATED_REQUIRED=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_models.c' -o "$@"

build/silk-shell-models/obj/src/silk_shell_models.cesta.o: build/.objects/5d66cc14d65071ca/src/silk_shell_models.cesta.o
	@mkdir -p 'build/silk-shell-models/obj/src'
	ln -f 'build/.objects/5d66cc14d65071ca/src/silk_shell_models.cesta.o' "$@"

build/.objects/cc56761a9d323c4a/src/silk_shell_commands.cesta.o: src/silk_shell_commands.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_LINEN_CORRELATED_REQUIRED=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_commands.c' -o "$@"

build/silk-shell-models/obj/src/silk_shell_commands.cesta.o: build/.objects/cc56761a9d323c4a/src/silk_shell_commands.cesta.o
	@mkdir -p 'build/silk-shell-models/obj/src'
	ln -f 'build/.objects/cc56761a9d323c4a/src/silk_shell_commands.cesta.o' "$@"

build/.objects/90067874a21561db/src/silk_shell_models_root.cesta.o: src/silk_shell_models_root.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_LINEN_CORRELATED_REQUIRED=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_models_root.c' -o "$@"

build/silk-shell-models/obj/src/silk_shell_models_root.cesta.o: build/.objects/90067874a21561db/src/silk_shell_models_root.cesta.o
	@mkdir -p 'build/silk-shell-models/obj/src'
	ln -f 'build/.objects/90067874a21561db/src/silk_shell_models_root.cesta.o' "$@"

build/.objects/cc0528db9f8411d0/src/silk_shell_models_bell.cesta.o: src/silk_shell_models_bell.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_LINEN_CORRELATED_REQUIRED=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_models_bell.c' -o "$@"

build/silk-shell-models/obj/src/silk_shell_models_bell.cesta.o: build/.objects/cc0528db9f8411d0/src/silk_shell_models_bell.cesta.o
	@mkdir -p 'build/silk-shell-models/obj/src'
	ln -f 'build/.objects/cc0528db9f8411d0/src/silk_shell_models_bell.cesta.o' "$@"

build/.objects/b7c41f1aa53ab8f7/src/silk_shell_models_linen.cesta.o: src/silk_shell_models_linen.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_LINEN_CORRELATED_REQUIRED=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_models_linen.c' -o "$@"

build/silk-shell-models/obj/src/silk_shell_models_linen.cesta.o: build/.objects/b7c41f1aa53ab8f7/src/silk_shell_models_linen.cesta.o
	@mkdir -p 'build/silk-shell-models/obj/src'
	ln -f 'build/.objects/b7c41f1aa53ab8f7/src/silk_shell_models_linen.cesta.o' "$@"

build/.objects/1f4ede46e2b21a58/src/silk_shell_models_projection.cesta.o: src/silk_shell_models_projection.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_LINEN_CORRELATED_REQUIRED=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_models_projection.c' -o "$@"

build/silk-shell-models/obj/src/silk_shell_models_projection.cesta.o: build/.objects/1f4ede46e2b21a58/src/silk_shell_models_projection.cesta.o
	@mkdir -p 'build/silk-shell-models/obj/src'
	ln -f 'build/.objects/1f4ede46e2b21a58/src/silk_shell_models_projection.cesta.o' "$@"

build/.objects/999ed027716364ba/src/silk_shell_models_publish.cesta.o: src/silk_shell_models_publish.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_LINEN_CORRELATED_REQUIRED=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_models_publish.c' -o "$@"

build/silk-shell-models/obj/src/silk_shell_models_publish.cesta.o: build/.objects/999ed027716364ba/src/silk_shell_models_publish.cesta.o
	@mkdir -p 'build/silk-shell-models/obj/src'
	ln -f 'build/.objects/999ed027716364ba/src/silk_shell_models_publish.cesta.o' "$@"

build/.objects/2748179c8f891e06/src/silk_shell_models_publish_native.cesta.o: src/silk_shell_models_publish_native.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_LINEN_CORRELATED_REQUIRED=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_models_publish_native.c' -o "$@"

build/silk-shell-models/obj/src/silk_shell_models_publish_native.cesta.o: build/.objects/2748179c8f891e06/src/silk_shell_models_publish_native.cesta.o
	@mkdir -p 'build/silk-shell-models/obj/src'
	ln -f 'build/.objects/2748179c8f891e06/src/silk_shell_models_publish_native.cesta.o' "$@"

build/.objects/14bce8598562815b/src/silk_shell_models_render.cesta.o: src/silk_shell_models_render.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_LINEN_CORRELATED_REQUIRED=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_models_render.c' -o "$@"

build/silk-shell-models/obj/src/silk_shell_models_render.cesta.o: build/.objects/14bce8598562815b/src/silk_shell_models_render.cesta.o
	@mkdir -p 'build/silk-shell-models/obj/src'
	ln -f 'build/.objects/14bce8598562815b/src/silk_shell_models_render.cesta.o' "$@"

build/.objects/bf4380968b4ec6fd/src/silk_shell_models_render_native.cesta.o: src/silk_shell_models_render_native.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_LINEN_CORRELATED_REQUIRED=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_models_render_native.c' -o "$@"

build/silk-shell-models/obj/src/silk_shell_models_render_native.cesta.o: build/.objects/bf4380968b4ec6fd/src/silk_shell_models_render_native.cesta.o
	@mkdir -p 'build/silk-shell-models/obj/src'
	ln -f 'build/.objects/bf4380968b4ec6fd/src/silk_shell_models_render_native.cesta.o' "$@"

build/.objects/08e1bb93efd1a543/src/silk_shell_models_paint.cesta.o: src/silk_shell_models_paint.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_LINEN_CORRELATED_REQUIRED=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_models_paint.c' -o "$@"

build/silk-shell-models/obj/src/silk_shell_models_paint.cesta.o: build/.objects/08e1bb93efd1a543/src/silk_shell_models_paint.cesta.o
	@mkdir -p 'build/silk-shell-models/obj/src'
	ln -f 'build/.objects/08e1bb93efd1a543/src/silk_shell_models_paint.cesta.o' "$@"

build/.objects/f3db094a04826125/src/silk_shell_models_paint_native.cesta.o: src/silk_shell_models_paint_native.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_LINEN_CORRELATED_REQUIRED=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_models_paint_native.c' -o "$@"

build/silk-shell-models/obj/src/silk_shell_models_paint_native.cesta.o: build/.objects/f3db094a04826125/src/silk_shell_models_paint_native.cesta.o
	@mkdir -p 'build/silk-shell-models/obj/src'
	ln -f 'build/.objects/f3db094a04826125/src/silk_shell_models_paint_native.cesta.o' "$@"

build/.objects/64c1f7cf4424ecec/src/silk_shell_models_workspace.cesta.o: src/silk_shell_models_workspace.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_LINEN_CORRELATED_REQUIRED=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_models_workspace.c' -o "$@"

build/silk-shell-models/obj/src/silk_shell_models_workspace.cesta.o: build/.objects/64c1f7cf4424ecec/src/silk_shell_models_workspace.cesta.o
	@mkdir -p 'build/silk-shell-models/obj/src'
	ln -f 'build/.objects/64c1f7cf4424ecec/src/silk_shell_models_workspace.cesta.o' "$@"

build/.objects/a2c02c445cd002d0/src/silk_shell_models_workspace_native.cesta.o: src/silk_shell_models_workspace_native.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_LINEN_CORRELATED_REQUIRED=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_models_workspace_native.c' -o "$@"

build/silk-shell-models/obj/src/silk_shell_models_workspace_native.cesta.o: build/.objects/a2c02c445cd002d0/src/silk_shell_models_workspace_native.cesta.o
	@mkdir -p 'build/silk-shell-models/obj/src'
	ln -f 'build/.objects/a2c02c445cd002d0/src/silk_shell_models_workspace_native.cesta.o' "$@"

build/.objects/845b9b1a5383fa54/src/linen_capture.cesta.o: src/linen_capture.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_LINEN_CORRELATED_REQUIRED=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/linen_capture.c' -o "$@"

build/silk-shell-models/obj/src/linen_capture.cesta.o: build/.objects/845b9b1a5383fa54/src/linen_capture.cesta.o
	@mkdir -p 'build/silk-shell-models/obj/src'
	ln -f 'build/.objects/845b9b1a5383fa54/src/linen_capture.cesta.o' "$@"

build/.objects/aed60e62d91bb4b4/src/linen_capture_client.cesta.o: src/linen_capture_client.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_LINEN_CORRELATED_REQUIRED=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/linen_capture_client.c' -o "$@"

build/silk-shell-models/obj/src/linen_capture_client.cesta.o: build/.objects/aed60e62d91bb4b4/src/linen_capture_client.cesta.o
	@mkdir -p 'build/silk-shell-models/obj/src'
	ln -f 'build/.objects/aed60e62d91bb4b4/src/linen_capture_client.cesta.o' "$@"

build/.objects/18c0533873de667a/src/linen_capture_transport.cesta.o: src/linen_capture_transport.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_LINEN_CORRELATED_REQUIRED=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/linen_capture_transport.c' -o "$@"

build/silk-shell-models/obj/src/linen_capture_transport.cesta.o: build/.objects/18c0533873de667a/src/linen_capture_transport.cesta.o
	@mkdir -p 'build/silk-shell-models/obj/src'
	ln -f 'build/.objects/18c0533873de667a/src/linen_capture_transport.cesta.o' "$@"

build/.objects/48e68aed90876805/src/shell_linen_snapshot.cesta.o: src/shell_linen_snapshot.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_LINEN_CORRELATED_REQUIRED=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/shell_linen_snapshot.c' -o "$@"

build/silk-shell-models/obj/src/shell_linen_snapshot.cesta.o: build/.objects/48e68aed90876805/src/shell_linen_snapshot.cesta.o
	@mkdir -p 'build/silk-shell-models/obj/src'
	ln -f 'build/.objects/48e68aed90876805/src/shell_linen_snapshot.cesta.o' "$@"

build/.objects/a45bcfb07857a969/src/shell_linen_list.cesta.o: src/shell_linen_list.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_LINEN_CORRELATED_REQUIRED=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/shell_linen_list.c' -o "$@"

build/silk-shell-models/obj/src/shell_linen_list.cesta.o: build/.objects/a45bcfb07857a969/src/shell_linen_list.cesta.o
	@mkdir -p 'build/silk-shell-models/obj/src'
	ln -f 'build/.objects/a45bcfb07857a969/src/shell_linen_list.cesta.o' "$@"

build/.objects/c092e7b22817268c/src/shell_text_delivery.cesta.o: src/shell_text_delivery.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_LINEN_CORRELATED_REQUIRED=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/shell_text_delivery.c' -o "$@"

build/silk-shell-models/obj/src/shell_text_delivery.cesta.o: build/.objects/c092e7b22817268c/src/shell_text_delivery.cesta.o
	@mkdir -p 'build/silk-shell-models/obj/src'
	ln -f 'build/.objects/c092e7b22817268c/src/shell_text_delivery.cesta.o' "$@"

build/.objects/2c705a127a579430/src/shell_text_delivery_native.cesta.o: src/shell_text_delivery_native.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_LINEN_CORRELATED_REQUIRED=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/shell_text_delivery_native.c' -o "$@"

build/silk-shell-models/obj/src/shell_text_delivery_native.cesta.o: build/.objects/2c705a127a579430/src/shell_text_delivery_native.cesta.o
	@mkdir -p 'build/silk-shell-models/obj/src'
	ln -f 'build/.objects/2c705a127a579430/src/shell_text_delivery_native.cesta.o' "$@"

build/.objects/a3914c98944506d7/src/shell_text_delivery_display.cesta.o: src/shell_text_delivery_display.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_LINEN_CORRELATED_REQUIRED=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/shell_text_delivery_display.c' -o "$@"

build/silk-shell-models/obj/src/shell_text_delivery_display.cesta.o: build/.objects/a3914c98944506d7/src/shell_text_delivery_display.cesta.o
	@mkdir -p 'build/silk-shell-models/obj/src'
	ln -f 'build/.objects/a3914c98944506d7/src/shell_text_delivery_display.cesta.o' "$@"

build/.objects/5d98c798dd257e49/src/shell_linen_presentation.cesta.o: src/shell_linen_presentation.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_LINEN_CORRELATED_REQUIRED=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/shell_linen_presentation.c' -o "$@"

build/silk-shell-models/obj/src/shell_linen_presentation.cesta.o: build/.objects/5d98c798dd257e49/src/shell_linen_presentation.cesta.o
	@mkdir -p 'build/silk-shell-models/obj/src'
	ln -f 'build/.objects/5d98c798dd257e49/src/shell_linen_presentation.cesta.o' "$@"

build/.objects/c6e885b3982513eb/src/silk_shell_custody.cesta.o: src/silk_shell_custody.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_LINEN_CORRELATED_REQUIRED=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_custody.c' -o "$@"

build/silk-shell-models/obj/src/silk_shell_custody.cesta.o: build/.objects/c6e885b3982513eb/src/silk_shell_custody.cesta.o
	@mkdir -p 'build/silk-shell-models/obj/src'
	ln -f 'build/.objects/c6e885b3982513eb/src/silk_shell_custody.cesta.o' "$@"

build/silk-shell-models/libsilk-shell-models.a: GNUmakefile build/silk-shell-models/obj/src/silk_shell_models.cesta.o build/silk-shell-models/obj/src/silk_shell_commands.cesta.o build/silk-shell-models/obj/src/silk_shell_models_root.cesta.o build/silk-shell-models/obj/src/silk_shell_models_bell.cesta.o build/silk-shell-models/obj/src/silk_shell_models_linen.cesta.o build/silk-shell-models/obj/src/silk_shell_models_projection.cesta.o build/silk-shell-models/obj/src/silk_shell_models_publish.cesta.o build/silk-shell-models/obj/src/silk_shell_models_publish_native.cesta.o build/silk-shell-models/obj/src/silk_shell_models_render.cesta.o build/silk-shell-models/obj/src/silk_shell_models_render_native.cesta.o build/silk-shell-models/obj/src/silk_shell_models_paint.cesta.o build/silk-shell-models/obj/src/silk_shell_models_paint_native.cesta.o build/silk-shell-models/obj/src/silk_shell_models_workspace.cesta.o build/silk-shell-models/obj/src/silk_shell_models_workspace_native.cesta.o build/silk-shell-models/obj/src/linen_capture.cesta.o build/silk-shell-models/obj/src/linen_capture_client.cesta.o build/silk-shell-models/obj/src/linen_capture_transport.cesta.o build/silk-shell-models/obj/src/shell_linen_snapshot.cesta.o build/silk-shell-models/obj/src/shell_linen_list.cesta.o build/silk-shell-models/obj/src/shell_text_delivery.cesta.o build/silk-shell-models/obj/src/shell_text_delivery_native.cesta.o build/silk-shell-models/obj/src/shell_text_delivery_display.cesta.o build/silk-shell-models/obj/src/shell_linen_presentation.cesta.o build/silk-shell-models/obj/src/silk_shell_custody.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/silk-shell-models/obj/src/silk_shell_models.cesta.o' 'build/silk-shell-models/obj/src/silk_shell_commands.cesta.o' 'build/silk-shell-models/obj/src/silk_shell_models_root.cesta.o' 'build/silk-shell-models/obj/src/silk_shell_models_bell.cesta.o' 'build/silk-shell-models/obj/src/silk_shell_models_linen.cesta.o' 'build/silk-shell-models/obj/src/silk_shell_models_projection.cesta.o' 'build/silk-shell-models/obj/src/silk_shell_models_publish.cesta.o' 'build/silk-shell-models/obj/src/silk_shell_models_publish_native.cesta.o' 'build/silk-shell-models/obj/src/silk_shell_models_render.cesta.o' 'build/silk-shell-models/obj/src/silk_shell_models_render_native.cesta.o' 'build/silk-shell-models/obj/src/silk_shell_models_paint.cesta.o' 'build/silk-shell-models/obj/src/silk_shell_models_paint_native.cesta.o' 'build/silk-shell-models/obj/src/silk_shell_models_workspace.cesta.o' 'build/silk-shell-models/obj/src/silk_shell_models_workspace_native.cesta.o' 'build/silk-shell-models/obj/src/linen_capture.cesta.o' 'build/silk-shell-models/obj/src/linen_capture_client.cesta.o' 'build/silk-shell-models/obj/src/linen_capture_transport.cesta.o' 'build/silk-shell-models/obj/src/shell_linen_snapshot.cesta.o' 'build/silk-shell-models/obj/src/shell_linen_list.cesta.o' 'build/silk-shell-models/obj/src/shell_text_delivery.cesta.o' 'build/silk-shell-models/obj/src/shell_text_delivery_native.cesta.o' 'build/silk-shell-models/obj/src/shell_text_delivery_display.cesta.o' 'build/silk-shell-models/obj/src/shell_linen_presentation.cesta.o' 'build/silk-shell-models/obj/src/silk_shell_custody.cesta.o'

build/.objects/7938783cebaf3601/src/linen_protocol_probe.cesta.o: src/linen_protocol_probe.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=linen_protocol_probe_main' '-DSEXOS_LINEN_CORRELATED_REQUIRED=1' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/linen_protocol_probe.c' -o "$@"

build/linen-protocol-probe/obj/src/linen_protocol_probe.cesta.o: build/.objects/7938783cebaf3601/src/linen_protocol_probe.cesta.o
	@mkdir -p 'build/linen-protocol-probe/obj/src'
	ln -f 'build/.objects/7938783cebaf3601/src/linen_protocol_probe.cesta.o' "$@"

build/.objects/dbe0500c8f303b69/src/silk_shell_models_linen.cesta.o: src/silk_shell_models_linen.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=linen_protocol_probe_main' '-DSEXOS_LINEN_CORRELATED_REQUIRED=1' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_models_linen.c' -o "$@"

build/linen-protocol-probe/obj/src/silk_shell_models_linen.cesta.o: build/.objects/dbe0500c8f303b69/src/silk_shell_models_linen.cesta.o
	@mkdir -p 'build/linen-protocol-probe/obj/src'
	ln -f 'build/.objects/dbe0500c8f303b69/src/silk_shell_models_linen.cesta.o' "$@"

build/.objects/6082cf75c340150c/src/silk_shell_models_projection.cesta.o: src/silk_shell_models_projection.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=linen_protocol_probe_main' '-DSEXOS_LINEN_CORRELATED_REQUIRED=1' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_models_projection.c' -o "$@"

build/linen-protocol-probe/obj/src/silk_shell_models_projection.cesta.o: build/.objects/6082cf75c340150c/src/silk_shell_models_projection.cesta.o
	@mkdir -p 'build/linen-protocol-probe/obj/src'
	ln -f 'build/.objects/6082cf75c340150c/src/silk_shell_models_projection.cesta.o' "$@"

build/.objects/d19302beb8fe17f2/src/silk_shell_models_publish.cesta.o: src/silk_shell_models_publish.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=linen_protocol_probe_main' '-DSEXOS_LINEN_CORRELATED_REQUIRED=1' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_models_publish.c' -o "$@"

build/linen-protocol-probe/obj/src/silk_shell_models_publish.cesta.o: build/.objects/d19302beb8fe17f2/src/silk_shell_models_publish.cesta.o
	@mkdir -p 'build/linen-protocol-probe/obj/src'
	ln -f 'build/.objects/d19302beb8fe17f2/src/silk_shell_models_publish.cesta.o' "$@"

build/.objects/4d9916a5973a9ee6/src/silk_shell_models_publish_native.cesta.o: src/silk_shell_models_publish_native.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=linen_protocol_probe_main' '-DSEXOS_LINEN_CORRELATED_REQUIRED=1' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_models_publish_native.c' -o "$@"

build/linen-protocol-probe/obj/src/silk_shell_models_publish_native.cesta.o: build/.objects/4d9916a5973a9ee6/src/silk_shell_models_publish_native.cesta.o
	@mkdir -p 'build/linen-protocol-probe/obj/src'
	ln -f 'build/.objects/4d9916a5973a9ee6/src/silk_shell_models_publish_native.cesta.o' "$@"

build/.objects/58bc65bcef97d010/src/linen_capture.cesta.o: src/linen_capture.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=linen_protocol_probe_main' '-DSEXOS_LINEN_CORRELATED_REQUIRED=1' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/linen_capture.c' -o "$@"

build/linen-protocol-probe/obj/src/linen_capture.cesta.o: build/.objects/58bc65bcef97d010/src/linen_capture.cesta.o
	@mkdir -p 'build/linen-protocol-probe/obj/src'
	ln -f 'build/.objects/58bc65bcef97d010/src/linen_capture.cesta.o' "$@"

build/.objects/5c101235663d4cf0/src/linen_capture_client.cesta.o: src/linen_capture_client.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=linen_protocol_probe_main' '-DSEXOS_LINEN_CORRELATED_REQUIRED=1' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/linen_capture_client.c' -o "$@"

build/linen-protocol-probe/obj/src/linen_capture_client.cesta.o: build/.objects/5c101235663d4cf0/src/linen_capture_client.cesta.o
	@mkdir -p 'build/linen-protocol-probe/obj/src'
	ln -f 'build/.objects/5c101235663d4cf0/src/linen_capture_client.cesta.o' "$@"

build/.objects/d941e32a76a9fb32/src/linen_capture_transport.cesta.o: src/linen_capture_transport.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=linen_protocol_probe_main' '-DSEXOS_LINEN_CORRELATED_REQUIRED=1' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/linen_capture_transport.c' -o "$@"

build/linen-protocol-probe/obj/src/linen_capture_transport.cesta.o: build/.objects/d941e32a76a9fb32/src/linen_capture_transport.cesta.o
	@mkdir -p 'build/linen-protocol-probe/obj/src'
	ln -f 'build/.objects/d941e32a76a9fb32/src/linen_capture_transport.cesta.o' "$@"

build/.objects/04039a1d0dd9370b/src/shell_linen_snapshot.cesta.o: src/shell_linen_snapshot.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=linen_protocol_probe_main' '-DSEXOS_LINEN_CORRELATED_REQUIRED=1' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/shell_linen_snapshot.c' -o "$@"

build/linen-protocol-probe/obj/src/shell_linen_snapshot.cesta.o: build/.objects/04039a1d0dd9370b/src/shell_linen_snapshot.cesta.o
	@mkdir -p 'build/linen-protocol-probe/obj/src'
	ln -f 'build/.objects/04039a1d0dd9370b/src/shell_linen_snapshot.cesta.o' "$@"

build/.objects/413bb3970f653068/src/shell_text_delivery.cesta.o: src/shell_text_delivery.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=linen_protocol_probe_main' '-DSEXOS_LINEN_CORRELATED_REQUIRED=1' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/shell_text_delivery.c' -o "$@"

build/linen-protocol-probe/obj/src/shell_text_delivery.cesta.o: build/.objects/413bb3970f653068/src/shell_text_delivery.cesta.o
	@mkdir -p 'build/linen-protocol-probe/obj/src'
	ln -f 'build/.objects/413bb3970f653068/src/shell_text_delivery.cesta.o' "$@"

build/.objects/46257244f08e5c64/src/shell_text_delivery_native.cesta.o: src/shell_text_delivery_native.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=linen_protocol_probe_main' '-DSEXOS_LINEN_CORRELATED_REQUIRED=1' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/shell_text_delivery_native.c' -o "$@"

build/linen-protocol-probe/obj/src/shell_text_delivery_native.cesta.o: build/.objects/46257244f08e5c64/src/shell_text_delivery_native.cesta.o
	@mkdir -p 'build/linen-protocol-probe/obj/src'
	ln -f 'build/.objects/46257244f08e5c64/src/shell_text_delivery_native.cesta.o' "$@"

build/.objects/61799c2112d17249/src/shell_text_delivery_display.cesta.o: src/shell_text_delivery_display.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=linen_protocol_probe_main' '-DSEXOS_LINEN_CORRELATED_REQUIRED=1' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/shell_text_delivery_display.c' -o "$@"

build/linen-protocol-probe/obj/src/shell_text_delivery_display.cesta.o: build/.objects/61799c2112d17249/src/shell_text_delivery_display.cesta.o
	@mkdir -p 'build/linen-protocol-probe/obj/src'
	ln -f 'build/.objects/61799c2112d17249/src/shell_text_delivery_display.cesta.o' "$@"

build/.objects/2360bd6b23ce2f97/src/shell_linen_presentation.cesta.o: src/shell_linen_presentation.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=linen_protocol_probe_main' '-DSEXOS_LINEN_CORRELATED_REQUIRED=1' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/shell_linen_presentation.c' -o "$@"

build/linen-protocol-probe/obj/src/shell_linen_presentation.cesta.o: build/.objects/2360bd6b23ce2f97/src/shell_linen_presentation.cesta.o
	@mkdir -p 'build/linen-protocol-probe/obj/src'
	ln -f 'build/.objects/2360bd6b23ce2f97/src/shell_linen_presentation.cesta.o' "$@"

build/.objects/d058faa75db37800/runtime/client_runtime.c.o: runtime/client_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=linen_protocol_probe_main' '-DSEXOS_LINEN_CORRELATED_REQUIRED=1' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/linen-protocol-probe/obj/runtime/client_runtime.c.o: build/.objects/d058faa75db37800/runtime/client_runtime.c.o
	@mkdir -p 'build/linen-protocol-probe/obj/runtime'
	ln -f 'build/.objects/d058faa75db37800/runtime/client_runtime.c.o' "$@"

build/.objects/b7144ac63e0b9804/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=linen_protocol_probe_main' '-DSEXOS_LINEN_CORRELATED_REQUIRED=1' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/linen-protocol-probe/obj/runtime/traps_x86_64.c.o: build/.objects/b7144ac63e0b9804/runtime/traps_x86_64.c.o
	@mkdir -p 'build/linen-protocol-probe/obj/runtime'
	ln -f 'build/.objects/b7144ac63e0b9804/runtime/traps_x86_64.c.o' "$@"

build/.objects/5c0b9f1df62c4728/runtime/rpc_traps_x86_64.c.o: runtime/rpc_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=linen_protocol_probe_main' '-DSEXOS_LINEN_CORRELATED_REQUIRED=1' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/rpc_traps_x86_64.c' -o "$@"

build/linen-protocol-probe/obj/runtime/rpc_traps_x86_64.c.o: build/.objects/5c0b9f1df62c4728/runtime/rpc_traps_x86_64.c.o
	@mkdir -p 'build/linen-protocol-probe/obj/runtime'
	ln -f 'build/.objects/5c0b9f1df62c4728/runtime/rpc_traps_x86_64.c.o' "$@"

build/.objects/65b46107433a53d0/runtime/start_x86_64.S.o: runtime/start_x86_64.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=linen_protocol_probe_main' '-DSEXOS_LINEN_CORRELATED_REQUIRED=1' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/linen-protocol-probe/obj/runtime/start_x86_64.S.o: build/.objects/65b46107433a53d0/runtime/start_x86_64.S.o
	@mkdir -p 'build/linen-protocol-probe/obj/runtime'
	ln -f 'build/.objects/65b46107433a53d0/runtime/start_x86_64.S.o' "$@"

build/.objects/9a341b76481cffad/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=linen_protocol_probe_main' '-DSEXOS_LINEN_CORRELATED_REQUIRED=1' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/linen-protocol-probe/obj/generated/runtime/format_bytes.c.o: build/.objects/9a341b76481cffad/generated/runtime/format_bytes.c.o
	@mkdir -p 'build/linen-protocol-probe/obj/generated/runtime'
	ln -f 'build/.objects/9a341b76481cffad/generated/runtime/format_bytes.c.o' "$@"

build/.objects/6e255aeabc6023a0/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=linen_protocol_probe_main' '-DSEXOS_LINEN_CORRELATED_REQUIRED=1' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/linen-protocol-probe/obj/generated/runtime/packed_words.c.o: build/.objects/6e255aeabc6023a0/generated/runtime/packed_words.c.o
	@mkdir -p 'build/linen-protocol-probe/obj/generated/runtime'
	ln -f 'build/.objects/6e255aeabc6023a0/generated/runtime/packed_words.c.o' "$@"

build/linen-protocol-probe/linen-protocol-probe: GNUmakefile build/linen-protocol-probe/obj/src/linen_protocol_probe.cesta.o build/linen-protocol-probe/obj/src/silk_shell_models_linen.cesta.o build/linen-protocol-probe/obj/src/silk_shell_models_projection.cesta.o build/linen-protocol-probe/obj/src/silk_shell_models_publish.cesta.o build/linen-protocol-probe/obj/src/silk_shell_models_publish_native.cesta.o build/linen-protocol-probe/obj/src/linen_capture.cesta.o build/linen-protocol-probe/obj/src/linen_capture_client.cesta.o build/linen-protocol-probe/obj/src/linen_capture_transport.cesta.o build/linen-protocol-probe/obj/src/shell_linen_snapshot.cesta.o build/linen-protocol-probe/obj/src/shell_text_delivery.cesta.o build/linen-protocol-probe/obj/src/shell_text_delivery_native.cesta.o build/linen-protocol-probe/obj/src/shell_text_delivery_display.cesta.o build/linen-protocol-probe/obj/src/shell_linen_presentation.cesta.o build/linen-protocol-probe/obj/runtime/client_runtime.c.o build/linen-protocol-probe/obj/runtime/traps_x86_64.c.o build/linen-protocol-probe/obj/runtime/rpc_traps_x86_64.c.o build/linen-protocol-probe/obj/runtime/start_x86_64.S.o build/linen-protocol-probe/obj/generated/runtime/format_bytes.c.o build/linen-protocol-probe/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/linen-protocol-probe/obj/src/linen_protocol_probe.cesta.o' 'build/linen-protocol-probe/obj/src/silk_shell_models_linen.cesta.o' 'build/linen-protocol-probe/obj/src/silk_shell_models_projection.cesta.o' 'build/linen-protocol-probe/obj/src/silk_shell_models_publish.cesta.o' 'build/linen-protocol-probe/obj/src/silk_shell_models_publish_native.cesta.o' 'build/linen-protocol-probe/obj/src/linen_capture.cesta.o' 'build/linen-protocol-probe/obj/src/linen_capture_client.cesta.o' 'build/linen-protocol-probe/obj/src/linen_capture_transport.cesta.o' 'build/linen-protocol-probe/obj/src/shell_linen_snapshot.cesta.o' 'build/linen-protocol-probe/obj/src/shell_text_delivery.cesta.o' 'build/linen-protocol-probe/obj/src/shell_text_delivery_native.cesta.o' 'build/linen-protocol-probe/obj/src/shell_text_delivery_display.cesta.o' 'build/linen-protocol-probe/obj/src/shell_linen_presentation.cesta.o' 'build/linen-protocol-probe/obj/runtime/client_runtime.c.o' 'build/linen-protocol-probe/obj/runtime/traps_x86_64.c.o' 'build/linen-protocol-probe/obj/runtime/rpc_traps_x86_64.c.o' 'build/linen-protocol-probe/obj/runtime/start_x86_64.S.o' 'build/linen-protocol-probe/obj/generated/runtime/format_bytes.c.o' 'build/linen-protocol-probe/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/.objects/8be2c011fb3ce56f/src/linen_capture.cesta.o: src/linen_capture.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/linen_capture.c' -o "$@"

build/linen-capture/obj/src/linen_capture.cesta.o: build/.objects/8be2c011fb3ce56f/src/linen_capture.cesta.o
	@mkdir -p 'build/linen-capture/obj/src'
	ln -f 'build/.objects/8be2c011fb3ce56f/src/linen_capture.cesta.o' "$@"

build/.objects/855cf409c208294f/src/linen_capture_client.cesta.o: src/linen_capture_client.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/linen_capture_client.c' -o "$@"

build/linen-capture/obj/src/linen_capture_client.cesta.o: build/.objects/855cf409c208294f/src/linen_capture_client.cesta.o
	@mkdir -p 'build/linen-capture/obj/src'
	ln -f 'build/.objects/855cf409c208294f/src/linen_capture_client.cesta.o' "$@"

build/.objects/bf7572eb35fe3349/src/linen_capture_transport.cesta.o: src/linen_capture_transport.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/linen_capture_transport.c' -o "$@"

build/linen-capture/obj/src/linen_capture_transport.cesta.o: build/.objects/bf7572eb35fe3349/src/linen_capture_transport.cesta.o
	@mkdir -p 'build/linen-capture/obj/src'
	ln -f 'build/.objects/bf7572eb35fe3349/src/linen_capture_transport.cesta.o' "$@"

build/.objects/f9549877b4e91dd3/src/linen_capture_service.cesta.o: src/linen_capture_service.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/linen_capture_service.c' -o "$@"

build/linen-capture/obj/src/linen_capture_service.cesta.o: build/.objects/f9549877b4e91dd3/src/linen_capture_service.cesta.o
	@mkdir -p 'build/linen-capture/obj/src'
	ln -f 'build/.objects/f9549877b4e91dd3/src/linen_capture_service.cesta.o' "$@"

build/linen-capture/liblinen-capture.a: GNUmakefile build/linen-capture/obj/src/linen_capture.cesta.o build/linen-capture/obj/src/linen_capture_client.cesta.o build/linen-capture/obj/src/linen_capture_transport.cesta.o build/linen-capture/obj/src/linen_capture_service.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/linen-capture/obj/src/linen_capture.cesta.o' 'build/linen-capture/obj/src/linen_capture_client.cesta.o' 'build/linen-capture/obj/src/linen_capture_transport.cesta.o' 'build/linen-capture/obj/src/linen_capture_service.cesta.o'

build/.objects/85cdac3968fd4ac2/src/shell_linen_snapshot.cesta.o: src/shell_linen_snapshot.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/shell_linen_snapshot.c' -o "$@"

build/shell-linen-snapshot/obj/src/shell_linen_snapshot.cesta.o: build/.objects/85cdac3968fd4ac2/src/shell_linen_snapshot.cesta.o
	@mkdir -p 'build/shell-linen-snapshot/obj/src'
	ln -f 'build/.objects/85cdac3968fd4ac2/src/shell_linen_snapshot.cesta.o' "$@"

build/shell-linen-snapshot/libshell-linen-snapshot.a: GNUmakefile build/shell-linen-snapshot/obj/src/shell_linen_snapshot.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/shell-linen-snapshot/obj/src/shell_linen_snapshot.cesta.o'

build/.objects/8428c2651c3d0fb0/src/silk_shell_models_linen.cesta.o: src/silk_shell_models_linen.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_models_linen.c' -o "$@"

build/silk-shell-models-linen/obj/src/silk_shell_models_linen.cesta.o: build/.objects/8428c2651c3d0fb0/src/silk_shell_models_linen.cesta.o
	@mkdir -p 'build/silk-shell-models-linen/obj/src'
	ln -f 'build/.objects/8428c2651c3d0fb0/src/silk_shell_models_linen.cesta.o' "$@"

build/.objects/b1e8ea3303c5440b/src/silk_shell_models_projection.cesta.o: src/silk_shell_models_projection.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_models_projection.c' -o "$@"

build/silk-shell-models-linen/obj/src/silk_shell_models_projection.cesta.o: build/.objects/b1e8ea3303c5440b/src/silk_shell_models_projection.cesta.o
	@mkdir -p 'build/silk-shell-models-linen/obj/src'
	ln -f 'build/.objects/b1e8ea3303c5440b/src/silk_shell_models_projection.cesta.o' "$@"

build/.objects/1b5a2f5d57c44b09/src/silk_shell_models_publish.cesta.o: src/silk_shell_models_publish.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_models_publish.c' -o "$@"

build/silk-shell-models-linen/obj/src/silk_shell_models_publish.cesta.o: build/.objects/1b5a2f5d57c44b09/src/silk_shell_models_publish.cesta.o
	@mkdir -p 'build/silk-shell-models-linen/obj/src'
	ln -f 'build/.objects/1b5a2f5d57c44b09/src/silk_shell_models_publish.cesta.o' "$@"

build/.objects/5edf5677ecce46fd/src/silk_shell_models_publish_native.cesta.o: src/silk_shell_models_publish_native.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_models_publish_native.c' -o "$@"

build/silk-shell-models-linen/obj/src/silk_shell_models_publish_native.cesta.o: build/.objects/5edf5677ecce46fd/src/silk_shell_models_publish_native.cesta.o
	@mkdir -p 'build/silk-shell-models-linen/obj/src'
	ln -f 'build/.objects/5edf5677ecce46fd/src/silk_shell_models_publish_native.cesta.o' "$@"

build/silk-shell-models-linen/obj/src/linen_capture.cesta.o: build/.objects/8be2c011fb3ce56f/src/linen_capture.cesta.o
	@mkdir -p 'build/silk-shell-models-linen/obj/src'
	ln -f 'build/.objects/8be2c011fb3ce56f/src/linen_capture.cesta.o' "$@"

build/silk-shell-models-linen/obj/src/linen_capture_client.cesta.o: build/.objects/855cf409c208294f/src/linen_capture_client.cesta.o
	@mkdir -p 'build/silk-shell-models-linen/obj/src'
	ln -f 'build/.objects/855cf409c208294f/src/linen_capture_client.cesta.o' "$@"

build/silk-shell-models-linen/obj/src/linen_capture_transport.cesta.o: build/.objects/bf7572eb35fe3349/src/linen_capture_transport.cesta.o
	@mkdir -p 'build/silk-shell-models-linen/obj/src'
	ln -f 'build/.objects/bf7572eb35fe3349/src/linen_capture_transport.cesta.o' "$@"

build/silk-shell-models-linen/obj/src/shell_linen_snapshot.cesta.o: build/.objects/85cdac3968fd4ac2/src/shell_linen_snapshot.cesta.o
	@mkdir -p 'build/silk-shell-models-linen/obj/src'
	ln -f 'build/.objects/85cdac3968fd4ac2/src/shell_linen_snapshot.cesta.o' "$@"

build/.objects/c7012c49e102c817/src/shell_text_delivery.cesta.o: src/shell_text_delivery.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/shell_text_delivery.c' -o "$@"

build/silk-shell-models-linen/obj/src/shell_text_delivery.cesta.o: build/.objects/c7012c49e102c817/src/shell_text_delivery.cesta.o
	@mkdir -p 'build/silk-shell-models-linen/obj/src'
	ln -f 'build/.objects/c7012c49e102c817/src/shell_text_delivery.cesta.o' "$@"

build/.objects/6bbc66393ab49813/src/shell_text_delivery_native.cesta.o: src/shell_text_delivery_native.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/shell_text_delivery_native.c' -o "$@"

build/silk-shell-models-linen/obj/src/shell_text_delivery_native.cesta.o: build/.objects/6bbc66393ab49813/src/shell_text_delivery_native.cesta.o
	@mkdir -p 'build/silk-shell-models-linen/obj/src'
	ln -f 'build/.objects/6bbc66393ab49813/src/shell_text_delivery_native.cesta.o' "$@"

build/.objects/3bd3fa3dfebffe90/src/shell_text_delivery_display.cesta.o: src/shell_text_delivery_display.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/shell_text_delivery_display.c' -o "$@"

build/silk-shell-models-linen/obj/src/shell_text_delivery_display.cesta.o: build/.objects/3bd3fa3dfebffe90/src/shell_text_delivery_display.cesta.o
	@mkdir -p 'build/silk-shell-models-linen/obj/src'
	ln -f 'build/.objects/3bd3fa3dfebffe90/src/shell_text_delivery_display.cesta.o' "$@"

build/.objects/56b06fdba4219bae/src/shell_linen_presentation.cesta.o: src/shell_linen_presentation.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/shell_linen_presentation.c' -o "$@"

build/silk-shell-models-linen/obj/src/shell_linen_presentation.cesta.o: build/.objects/56b06fdba4219bae/src/shell_linen_presentation.cesta.o
	@mkdir -p 'build/silk-shell-models-linen/obj/src'
	ln -f 'build/.objects/56b06fdba4219bae/src/shell_linen_presentation.cesta.o' "$@"

build/silk-shell-models-linen/libsilk-shell-models-linen.a: GNUmakefile build/silk-shell-models-linen/obj/src/silk_shell_models_linen.cesta.o build/silk-shell-models-linen/obj/src/silk_shell_models_projection.cesta.o build/silk-shell-models-linen/obj/src/silk_shell_models_publish.cesta.o build/silk-shell-models-linen/obj/src/silk_shell_models_publish_native.cesta.o build/silk-shell-models-linen/obj/src/linen_capture.cesta.o build/silk-shell-models-linen/obj/src/linen_capture_client.cesta.o build/silk-shell-models-linen/obj/src/linen_capture_transport.cesta.o build/silk-shell-models-linen/obj/src/shell_linen_snapshot.cesta.o build/silk-shell-models-linen/obj/src/shell_text_delivery.cesta.o build/silk-shell-models-linen/obj/src/shell_text_delivery_native.cesta.o build/silk-shell-models-linen/obj/src/shell_text_delivery_display.cesta.o build/silk-shell-models-linen/obj/src/shell_linen_presentation.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/silk-shell-models-linen/obj/src/silk_shell_models_linen.cesta.o' 'build/silk-shell-models-linen/obj/src/silk_shell_models_projection.cesta.o' 'build/silk-shell-models-linen/obj/src/silk_shell_models_publish.cesta.o' 'build/silk-shell-models-linen/obj/src/silk_shell_models_publish_native.cesta.o' 'build/silk-shell-models-linen/obj/src/linen_capture.cesta.o' 'build/silk-shell-models-linen/obj/src/linen_capture_client.cesta.o' 'build/silk-shell-models-linen/obj/src/linen_capture_transport.cesta.o' 'build/silk-shell-models-linen/obj/src/shell_linen_snapshot.cesta.o' 'build/silk-shell-models-linen/obj/src/shell_text_delivery.cesta.o' 'build/silk-shell-models-linen/obj/src/shell_text_delivery_native.cesta.o' 'build/silk-shell-models-linen/obj/src/shell_text_delivery_display.cesta.o' 'build/silk-shell-models-linen/obj/src/shell_linen_presentation.cesta.o'

build/.objects/8bc92584a1bc60bc/src/silk_shell_models_render.cesta.o: src/silk_shell_models_render.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_models_render.c' -o "$@"

build/silk-shell-models-render/obj/src/silk_shell_models_render.cesta.o: build/.objects/8bc92584a1bc60bc/src/silk_shell_models_render.cesta.o
	@mkdir -p 'build/silk-shell-models-render/obj/src'
	ln -f 'build/.objects/8bc92584a1bc60bc/src/silk_shell_models_render.cesta.o' "$@"

build/.objects/9e867867d35cc70a/src/silk_shell_models_render_native.cesta.o: src/silk_shell_models_render_native.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_models_render_native.c' -o "$@"

build/silk-shell-models-render/obj/src/silk_shell_models_render_native.cesta.o: build/.objects/9e867867d35cc70a/src/silk_shell_models_render_native.cesta.o
	@mkdir -p 'build/silk-shell-models-render/obj/src'
	ln -f 'build/.objects/9e867867d35cc70a/src/silk_shell_models_render_native.cesta.o' "$@"

build/.objects/d096b29b4e65a714/src/silk_shell_models_paint.cesta.o: src/silk_shell_models_paint.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_models_paint.c' -o "$@"

build/silk-shell-models-render/obj/src/silk_shell_models_paint.cesta.o: build/.objects/d096b29b4e65a714/src/silk_shell_models_paint.cesta.o
	@mkdir -p 'build/silk-shell-models-render/obj/src'
	ln -f 'build/.objects/d096b29b4e65a714/src/silk_shell_models_paint.cesta.o' "$@"

build/.objects/4d154c6352e69c62/src/silk_shell_models_paint_native.cesta.o: src/silk_shell_models_paint_native.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_models_paint_native.c' -o "$@"

build/silk-shell-models-render/obj/src/silk_shell_models_paint_native.cesta.o: build/.objects/4d154c6352e69c62/src/silk_shell_models_paint_native.cesta.o
	@mkdir -p 'build/silk-shell-models-render/obj/src'
	ln -f 'build/.objects/4d154c6352e69c62/src/silk_shell_models_paint_native.cesta.o' "$@"

build/silk-shell-models-render/obj/src/silk_shell_models_linen.cesta.o: build/.objects/8428c2651c3d0fb0/src/silk_shell_models_linen.cesta.o
	@mkdir -p 'build/silk-shell-models-render/obj/src'
	ln -f 'build/.objects/8428c2651c3d0fb0/src/silk_shell_models_linen.cesta.o' "$@"

build/silk-shell-models-render/obj/src/silk_shell_models_projection.cesta.o: build/.objects/b1e8ea3303c5440b/src/silk_shell_models_projection.cesta.o
	@mkdir -p 'build/silk-shell-models-render/obj/src'
	ln -f 'build/.objects/b1e8ea3303c5440b/src/silk_shell_models_projection.cesta.o' "$@"

build/silk-shell-models-render/obj/src/silk_shell_models_publish.cesta.o: build/.objects/1b5a2f5d57c44b09/src/silk_shell_models_publish.cesta.o
	@mkdir -p 'build/silk-shell-models-render/obj/src'
	ln -f 'build/.objects/1b5a2f5d57c44b09/src/silk_shell_models_publish.cesta.o' "$@"

build/silk-shell-models-render/obj/src/silk_shell_models_publish_native.cesta.o: build/.objects/5edf5677ecce46fd/src/silk_shell_models_publish_native.cesta.o
	@mkdir -p 'build/silk-shell-models-render/obj/src'
	ln -f 'build/.objects/5edf5677ecce46fd/src/silk_shell_models_publish_native.cesta.o' "$@"

build/silk-shell-models-render/obj/src/linen_capture.cesta.o: build/.objects/8be2c011fb3ce56f/src/linen_capture.cesta.o
	@mkdir -p 'build/silk-shell-models-render/obj/src'
	ln -f 'build/.objects/8be2c011fb3ce56f/src/linen_capture.cesta.o' "$@"

build/silk-shell-models-render/obj/src/linen_capture_client.cesta.o: build/.objects/855cf409c208294f/src/linen_capture_client.cesta.o
	@mkdir -p 'build/silk-shell-models-render/obj/src'
	ln -f 'build/.objects/855cf409c208294f/src/linen_capture_client.cesta.o' "$@"

build/silk-shell-models-render/obj/src/linen_capture_transport.cesta.o: build/.objects/bf7572eb35fe3349/src/linen_capture_transport.cesta.o
	@mkdir -p 'build/silk-shell-models-render/obj/src'
	ln -f 'build/.objects/bf7572eb35fe3349/src/linen_capture_transport.cesta.o' "$@"

build/silk-shell-models-render/obj/src/shell_linen_snapshot.cesta.o: build/.objects/85cdac3968fd4ac2/src/shell_linen_snapshot.cesta.o
	@mkdir -p 'build/silk-shell-models-render/obj/src'
	ln -f 'build/.objects/85cdac3968fd4ac2/src/shell_linen_snapshot.cesta.o' "$@"

build/.objects/abfbd4f34ae3d0ce/src/shell_linen_list.cesta.o: src/shell_linen_list.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/shell_linen_list.c' -o "$@"

build/silk-shell-models-render/obj/src/shell_linen_list.cesta.o: build/.objects/abfbd4f34ae3d0ce/src/shell_linen_list.cesta.o
	@mkdir -p 'build/silk-shell-models-render/obj/src'
	ln -f 'build/.objects/abfbd4f34ae3d0ce/src/shell_linen_list.cesta.o' "$@"

build/silk-shell-models-render/obj/src/shell_text_delivery.cesta.o: build/.objects/c7012c49e102c817/src/shell_text_delivery.cesta.o
	@mkdir -p 'build/silk-shell-models-render/obj/src'
	ln -f 'build/.objects/c7012c49e102c817/src/shell_text_delivery.cesta.o' "$@"

build/silk-shell-models-render/obj/src/shell_text_delivery_native.cesta.o: build/.objects/6bbc66393ab49813/src/shell_text_delivery_native.cesta.o
	@mkdir -p 'build/silk-shell-models-render/obj/src'
	ln -f 'build/.objects/6bbc66393ab49813/src/shell_text_delivery_native.cesta.o' "$@"

build/silk-shell-models-render/obj/src/shell_linen_presentation.cesta.o: build/.objects/56b06fdba4219bae/src/shell_linen_presentation.cesta.o
	@mkdir -p 'build/silk-shell-models-render/obj/src'
	ln -f 'build/.objects/56b06fdba4219bae/src/shell_linen_presentation.cesta.o' "$@"

build/silk-shell-models-render/libsilk-shell-models-render.a: GNUmakefile build/silk-shell-models-render/obj/src/silk_shell_models_render.cesta.o build/silk-shell-models-render/obj/src/silk_shell_models_render_native.cesta.o build/silk-shell-models-render/obj/src/silk_shell_models_paint.cesta.o build/silk-shell-models-render/obj/src/silk_shell_models_paint_native.cesta.o build/silk-shell-models-render/obj/src/silk_shell_models_linen.cesta.o build/silk-shell-models-render/obj/src/silk_shell_models_projection.cesta.o build/silk-shell-models-render/obj/src/silk_shell_models_publish.cesta.o build/silk-shell-models-render/obj/src/silk_shell_models_publish_native.cesta.o build/silk-shell-models-render/obj/src/linen_capture.cesta.o build/silk-shell-models-render/obj/src/linen_capture_client.cesta.o build/silk-shell-models-render/obj/src/linen_capture_transport.cesta.o build/silk-shell-models-render/obj/src/shell_linen_snapshot.cesta.o build/silk-shell-models-render/obj/src/shell_linen_list.cesta.o build/silk-shell-models-render/obj/src/shell_text_delivery.cesta.o build/silk-shell-models-render/obj/src/shell_text_delivery_native.cesta.o build/silk-shell-models-render/obj/src/shell_linen_presentation.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/silk-shell-models-render/obj/src/silk_shell_models_render.cesta.o' 'build/silk-shell-models-render/obj/src/silk_shell_models_render_native.cesta.o' 'build/silk-shell-models-render/obj/src/silk_shell_models_paint.cesta.o' 'build/silk-shell-models-render/obj/src/silk_shell_models_paint_native.cesta.o' 'build/silk-shell-models-render/obj/src/silk_shell_models_linen.cesta.o' 'build/silk-shell-models-render/obj/src/silk_shell_models_projection.cesta.o' 'build/silk-shell-models-render/obj/src/silk_shell_models_publish.cesta.o' 'build/silk-shell-models-render/obj/src/silk_shell_models_publish_native.cesta.o' 'build/silk-shell-models-render/obj/src/linen_capture.cesta.o' 'build/silk-shell-models-render/obj/src/linen_capture_client.cesta.o' 'build/silk-shell-models-render/obj/src/linen_capture_transport.cesta.o' 'build/silk-shell-models-render/obj/src/shell_linen_snapshot.cesta.o' 'build/silk-shell-models-render/obj/src/shell_linen_list.cesta.o' 'build/silk-shell-models-render/obj/src/shell_text_delivery.cesta.o' 'build/silk-shell-models-render/obj/src/shell_text_delivery_native.cesta.o' 'build/silk-shell-models-render/obj/src/shell_linen_presentation.cesta.o'

build/.objects/586b63906ab18df9/src/silk_shell_diagnostics_a.cesta.o: src/silk_shell_diagnostics_a.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_diagnostics_a.c' -o "$@"

build/silk-shell-diagnostics-a/obj/src/silk_shell_diagnostics_a.cesta.o: build/.objects/586b63906ab18df9/src/silk_shell_diagnostics_a.cesta.o
	@mkdir -p 'build/silk-shell-diagnostics-a/obj/src'
	ln -f 'build/.objects/586b63906ab18df9/src/silk_shell_diagnostics_a.cesta.o' "$@"

build/.objects/1e8ca0cf268ab0ed/src/silk_shell_diagnostics_a_native.cesta.o: src/silk_shell_diagnostics_a_native.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_diagnostics_a_native.c' -o "$@"

build/silk-shell-diagnostics-a/obj/src/silk_shell_diagnostics_a_native.cesta.o: build/.objects/1e8ca0cf268ab0ed/src/silk_shell_diagnostics_a_native.cesta.o
	@mkdir -p 'build/silk-shell-diagnostics-a/obj/src'
	ln -f 'build/.objects/1e8ca0cf268ab0ed/src/silk_shell_diagnostics_a_native.cesta.o' "$@"

build/silk-shell-diagnostics-a/libsilk-shell-diagnostics-a.a: GNUmakefile build/silk-shell-diagnostics-a/obj/src/silk_shell_diagnostics_a.cesta.o build/silk-shell-diagnostics-a/obj/src/silk_shell_diagnostics_a_native.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/silk-shell-diagnostics-a/obj/src/silk_shell_diagnostics_a.cesta.o' 'build/silk-shell-diagnostics-a/obj/src/silk_shell_diagnostics_a_native.cesta.o'

build/.objects/fd90c076105550fe/src/silk_shell_proofs_a.cesta.o: src/silk_shell_proofs_a.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_proofs_a.c' -o "$@"

build/silk-shell-proofs-a/obj/src/silk_shell_proofs_a.cesta.o: build/.objects/fd90c076105550fe/src/silk_shell_proofs_a.cesta.o
	@mkdir -p 'build/silk-shell-proofs-a/obj/src'
	ln -f 'build/.objects/fd90c076105550fe/src/silk_shell_proofs_a.cesta.o' "$@"

build/.objects/43d7f060ae34d5d1/src/silk_shell_proofs_a_root.cesta.o: src/silk_shell_proofs_a_root.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_proofs_a_root.c' -o "$@"

build/silk-shell-proofs-a/obj/src/silk_shell_proofs_a_root.cesta.o: build/.objects/43d7f060ae34d5d1/src/silk_shell_proofs_a_root.cesta.o
	@mkdir -p 'build/silk-shell-proofs-a/obj/src'
	ln -f 'build/.objects/43d7f060ae34d5d1/src/silk_shell_proofs_a_root.cesta.o' "$@"

build/.objects/542e98710af5dcaf/src/silk_shell_proofs_a_routes.cesta.o: src/silk_shell_proofs_a_routes.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_proofs_a_routes.c' -o "$@"

build/silk-shell-proofs-a/obj/src/silk_shell_proofs_a_routes.cesta.o: build/.objects/542e98710af5dcaf/src/silk_shell_proofs_a_routes.cesta.o
	@mkdir -p 'build/silk-shell-proofs-a/obj/src'
	ln -f 'build/.objects/542e98710af5dcaf/src/silk_shell_proofs_a_routes.cesta.o' "$@"

build/.objects/7a2dffcbbff43b2b/src/silk_shell_proofs_a_routes_native.cesta.o: src/silk_shell_proofs_a_routes_native.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_proofs_a_routes_native.c' -o "$@"

build/silk-shell-proofs-a/obj/src/silk_shell_proofs_a_routes_native.cesta.o: build/.objects/7a2dffcbbff43b2b/src/silk_shell_proofs_a_routes_native.cesta.o
	@mkdir -p 'build/silk-shell-proofs-a/obj/src'
	ln -f 'build/.objects/7a2dffcbbff43b2b/src/silk_shell_proofs_a_routes_native.cesta.o' "$@"

build/.objects/22f296b27cc61ba6/src/shell_proof_lifecycle_fixture.cesta.o: src/shell_proof_lifecycle_fixture.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/shell_proof_lifecycle_fixture.c' -o "$@"

build/silk-shell-proofs-a/obj/src/shell_proof_lifecycle_fixture.cesta.o: build/.objects/22f296b27cc61ba6/src/shell_proof_lifecycle_fixture.cesta.o
	@mkdir -p 'build/silk-shell-proofs-a/obj/src'
	ln -f 'build/.objects/22f296b27cc61ba6/src/shell_proof_lifecycle_fixture.cesta.o' "$@"

build/.objects/69291b03f5c2d657/src/silk_shell_proofs_a_lifecycle.cesta.o: src/silk_shell_proofs_a_lifecycle.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_proofs_a_lifecycle.c' -o "$@"

build/silk-shell-proofs-a/obj/src/silk_shell_proofs_a_lifecycle.cesta.o: build/.objects/69291b03f5c2d657/src/silk_shell_proofs_a_lifecycle.cesta.o
	@mkdir -p 'build/silk-shell-proofs-a/obj/src'
	ln -f 'build/.objects/69291b03f5c2d657/src/silk_shell_proofs_a_lifecycle.cesta.o' "$@"

build/.objects/b3225b4ae7fee353/src/silk_shell_proofs_a_lifecycle_native.cesta.o: src/silk_shell_proofs_a_lifecycle_native.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_proofs_a_lifecycle_native.c' -o "$@"

build/silk-shell-proofs-a/obj/src/silk_shell_proofs_a_lifecycle_native.cesta.o: build/.objects/b3225b4ae7fee353/src/silk_shell_proofs_a_lifecycle_native.cesta.o
	@mkdir -p 'build/silk-shell-proofs-a/obj/src'
	ln -f 'build/.objects/b3225b4ae7fee353/src/silk_shell_proofs_a_lifecycle_native.cesta.o' "$@"

build/silk-shell-proofs-a/obj/src/silk_shell_diagnostics_a.cesta.o: build/.objects/586b63906ab18df9/src/silk_shell_diagnostics_a.cesta.o
	@mkdir -p 'build/silk-shell-proofs-a/obj/src'
	ln -f 'build/.objects/586b63906ab18df9/src/silk_shell_diagnostics_a.cesta.o' "$@"

build/silk-shell-proofs-a/obj/src/silk_shell_diagnostics_a_native.cesta.o: build/.objects/1e8ca0cf268ab0ed/src/silk_shell_diagnostics_a_native.cesta.o
	@mkdir -p 'build/silk-shell-proofs-a/obj/src'
	ln -f 'build/.objects/1e8ca0cf268ab0ed/src/silk_shell_diagnostics_a_native.cesta.o' "$@"

build/silk-shell-proofs-a/libsilk-shell-proofs-a.a: GNUmakefile build/silk-shell-proofs-a/obj/src/silk_shell_proofs_a.cesta.o build/silk-shell-proofs-a/obj/src/silk_shell_proofs_a_root.cesta.o build/silk-shell-proofs-a/obj/src/silk_shell_proofs_a_routes.cesta.o build/silk-shell-proofs-a/obj/src/silk_shell_proofs_a_routes_native.cesta.o build/silk-shell-proofs-a/obj/src/shell_proof_lifecycle_fixture.cesta.o build/silk-shell-proofs-a/obj/src/silk_shell_proofs_a_lifecycle.cesta.o build/silk-shell-proofs-a/obj/src/silk_shell_proofs_a_lifecycle_native.cesta.o build/silk-shell-proofs-a/obj/src/silk_shell_diagnostics_a.cesta.o build/silk-shell-proofs-a/obj/src/silk_shell_diagnostics_a_native.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/silk-shell-proofs-a/obj/src/silk_shell_proofs_a.cesta.o' 'build/silk-shell-proofs-a/obj/src/silk_shell_proofs_a_root.cesta.o' 'build/silk-shell-proofs-a/obj/src/silk_shell_proofs_a_routes.cesta.o' 'build/silk-shell-proofs-a/obj/src/silk_shell_proofs_a_routes_native.cesta.o' 'build/silk-shell-proofs-a/obj/src/shell_proof_lifecycle_fixture.cesta.o' 'build/silk-shell-proofs-a/obj/src/silk_shell_proofs_a_lifecycle.cesta.o' 'build/silk-shell-proofs-a/obj/src/silk_shell_proofs_a_lifecycle_native.cesta.o' 'build/silk-shell-proofs-a/obj/src/silk_shell_diagnostics_a.cesta.o' 'build/silk-shell-proofs-a/obj/src/silk_shell_diagnostics_a_native.cesta.o'

build/.objects/097f472246023bf9/src/silk_shell_proofs_b_palette.cesta.o: src/silk_shell_proofs_b_palette.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_proofs_b_palette.c' -o "$@"

build/silk-shell-proofs-b-palette/obj/src/silk_shell_proofs_b_palette.cesta.o: build/.objects/097f472246023bf9/src/silk_shell_proofs_b_palette.cesta.o
	@mkdir -p 'build/silk-shell-proofs-b-palette/obj/src'
	ln -f 'build/.objects/097f472246023bf9/src/silk_shell_proofs_b_palette.cesta.o' "$@"

build/.objects/5bf20b0b5d4b8aed/src/silk_shell_proofs_b_palette_native.cesta.o: src/silk_shell_proofs_b_palette_native.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_proofs_b_palette_native.c' -o "$@"

build/silk-shell-proofs-b-palette/obj/src/silk_shell_proofs_b_palette_native.cesta.o: build/.objects/5bf20b0b5d4b8aed/src/silk_shell_proofs_b_palette_native.cesta.o
	@mkdir -p 'build/silk-shell-proofs-b-palette/obj/src'
	ln -f 'build/.objects/5bf20b0b5d4b8aed/src/silk_shell_proofs_b_palette_native.cesta.o' "$@"

build/silk-shell-proofs-b-palette/libsilk-shell-proofs-b-palette.a: GNUmakefile build/silk-shell-proofs-b-palette/obj/src/silk_shell_proofs_b_palette.cesta.o build/silk-shell-proofs-b-palette/obj/src/silk_shell_proofs_b_palette_native.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/silk-shell-proofs-b-palette/obj/src/silk_shell_proofs_b_palette.cesta.o' 'build/silk-shell-proofs-b-palette/obj/src/silk_shell_proofs_b_palette_native.cesta.o'

build/.objects/4cd6003c48d7cc0d/src/silk_shell_proofs_b_atlas.cesta.o: src/silk_shell_proofs_b_atlas.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_proofs_b_atlas.c' -o "$@"

build/silk-shell-proofs-b-atlas/obj/src/silk_shell_proofs_b_atlas.cesta.o: build/.objects/4cd6003c48d7cc0d/src/silk_shell_proofs_b_atlas.cesta.o
	@mkdir -p 'build/silk-shell-proofs-b-atlas/obj/src'
	ln -f 'build/.objects/4cd6003c48d7cc0d/src/silk_shell_proofs_b_atlas.cesta.o' "$@"

build/.objects/229e6db3cfb10931/src/silk_shell_proofs_b_atlas_native.cesta.o: src/silk_shell_proofs_b_atlas_native.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_proofs_b_atlas_native.c' -o "$@"

build/silk-shell-proofs-b-atlas/obj/src/silk_shell_proofs_b_atlas_native.cesta.o: build/.objects/229e6db3cfb10931/src/silk_shell_proofs_b_atlas_native.cesta.o
	@mkdir -p 'build/silk-shell-proofs-b-atlas/obj/src'
	ln -f 'build/.objects/229e6db3cfb10931/src/silk_shell_proofs_b_atlas_native.cesta.o' "$@"

build/silk-shell-proofs-b-atlas/libsilk-shell-proofs-b-atlas.a: GNUmakefile build/silk-shell-proofs-b-atlas/obj/src/silk_shell_proofs_b_atlas.cesta.o build/silk-shell-proofs-b-atlas/obj/src/silk_shell_proofs_b_atlas_native.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/silk-shell-proofs-b-atlas/obj/src/silk_shell_proofs_b_atlas.cesta.o' 'build/silk-shell-proofs-b-atlas/obj/src/silk_shell_proofs_b_atlas_native.cesta.o'

build/.objects/f56b0da36cb87098/src/silk_shell_proofs_b_input.cesta.o: src/silk_shell_proofs_b_input.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_proofs_b_input.c' -o "$@"

build/silk-shell-proofs-b-input/obj/src/silk_shell_proofs_b_input.cesta.o: build/.objects/f56b0da36cb87098/src/silk_shell_proofs_b_input.cesta.o
	@mkdir -p 'build/silk-shell-proofs-b-input/obj/src'
	ln -f 'build/.objects/f56b0da36cb87098/src/silk_shell_proofs_b_input.cesta.o' "$@"

build/.objects/078d4efafbaa08d6/src/silk_shell_proofs_b_input_native.cesta.o: src/silk_shell_proofs_b_input_native.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_proofs_b_input_native.c' -o "$@"

build/silk-shell-proofs-b-input/obj/src/silk_shell_proofs_b_input_native.cesta.o: build/.objects/078d4efafbaa08d6/src/silk_shell_proofs_b_input_native.cesta.o
	@mkdir -p 'build/silk-shell-proofs-b-input/obj/src'
	ln -f 'build/.objects/078d4efafbaa08d6/src/silk_shell_proofs_b_input_native.cesta.o' "$@"

build/silk-shell-proofs-b-input/libsilk-shell-proofs-b-input.a: GNUmakefile build/silk-shell-proofs-b-input/obj/src/silk_shell_proofs_b_input.cesta.o build/silk-shell-proofs-b-input/obj/src/silk_shell_proofs_b_input_native.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/silk-shell-proofs-b-input/obj/src/silk_shell_proofs_b_input.cesta.o' 'build/silk-shell-proofs-b-input/obj/src/silk_shell_proofs_b_input_native.cesta.o'

build/silk-shell-proofs-b/obj/src/silk_shell_proofs_b_palette.cesta.o: build/.objects/097f472246023bf9/src/silk_shell_proofs_b_palette.cesta.o
	@mkdir -p 'build/silk-shell-proofs-b/obj/src'
	ln -f 'build/.objects/097f472246023bf9/src/silk_shell_proofs_b_palette.cesta.o' "$@"

build/silk-shell-proofs-b/obj/src/silk_shell_proofs_b_palette_native.cesta.o: build/.objects/5bf20b0b5d4b8aed/src/silk_shell_proofs_b_palette_native.cesta.o
	@mkdir -p 'build/silk-shell-proofs-b/obj/src'
	ln -f 'build/.objects/5bf20b0b5d4b8aed/src/silk_shell_proofs_b_palette_native.cesta.o' "$@"

build/silk-shell-proofs-b/obj/src/silk_shell_proofs_b_atlas.cesta.o: build/.objects/4cd6003c48d7cc0d/src/silk_shell_proofs_b_atlas.cesta.o
	@mkdir -p 'build/silk-shell-proofs-b/obj/src'
	ln -f 'build/.objects/4cd6003c48d7cc0d/src/silk_shell_proofs_b_atlas.cesta.o' "$@"

build/silk-shell-proofs-b/obj/src/silk_shell_proofs_b_atlas_native.cesta.o: build/.objects/229e6db3cfb10931/src/silk_shell_proofs_b_atlas_native.cesta.o
	@mkdir -p 'build/silk-shell-proofs-b/obj/src'
	ln -f 'build/.objects/229e6db3cfb10931/src/silk_shell_proofs_b_atlas_native.cesta.o' "$@"

build/silk-shell-proofs-b/obj/src/silk_shell_proofs_b_input.cesta.o: build/.objects/f56b0da36cb87098/src/silk_shell_proofs_b_input.cesta.o
	@mkdir -p 'build/silk-shell-proofs-b/obj/src'
	ln -f 'build/.objects/f56b0da36cb87098/src/silk_shell_proofs_b_input.cesta.o' "$@"

build/silk-shell-proofs-b/obj/src/silk_shell_proofs_b_input_native.cesta.o: build/.objects/078d4efafbaa08d6/src/silk_shell_proofs_b_input_native.cesta.o
	@mkdir -p 'build/silk-shell-proofs-b/obj/src'
	ln -f 'build/.objects/078d4efafbaa08d6/src/silk_shell_proofs_b_input_native.cesta.o' "$@"

build/.objects/a03373d4dd99b420/src/silk_shell_proofs_b_root.cesta.o: src/silk_shell_proofs_b_root.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_proofs_b_root.c' -o "$@"

build/silk-shell-proofs-b/obj/src/silk_shell_proofs_b_root.cesta.o: build/.objects/a03373d4dd99b420/src/silk_shell_proofs_b_root.cesta.o
	@mkdir -p 'build/silk-shell-proofs-b/obj/src'
	ln -f 'build/.objects/a03373d4dd99b420/src/silk_shell_proofs_b_root.cesta.o' "$@"

build/silk-shell-proofs-b/libsilk-shell-proofs-b.a: GNUmakefile build/silk-shell-proofs-b/obj/src/silk_shell_proofs_b_palette.cesta.o build/silk-shell-proofs-b/obj/src/silk_shell_proofs_b_palette_native.cesta.o build/silk-shell-proofs-b/obj/src/silk_shell_proofs_b_atlas.cesta.o build/silk-shell-proofs-b/obj/src/silk_shell_proofs_b_atlas_native.cesta.o build/silk-shell-proofs-b/obj/src/silk_shell_proofs_b_input.cesta.o build/silk-shell-proofs-b/obj/src/silk_shell_proofs_b_input_native.cesta.o build/silk-shell-proofs-b/obj/src/silk_shell_proofs_b_root.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/silk-shell-proofs-b/obj/src/silk_shell_proofs_b_palette.cesta.o' 'build/silk-shell-proofs-b/obj/src/silk_shell_proofs_b_palette_native.cesta.o' 'build/silk-shell-proofs-b/obj/src/silk_shell_proofs_b_atlas.cesta.o' 'build/silk-shell-proofs-b/obj/src/silk_shell_proofs_b_atlas_native.cesta.o' 'build/silk-shell-proofs-b/obj/src/silk_shell_proofs_b_input.cesta.o' 'build/silk-shell-proofs-b/obj/src/silk_shell_proofs_b_input_native.cesta.o' 'build/silk-shell-proofs-b/obj/src/silk_shell_proofs_b_root.cesta.o'

build/.objects/03a810070a584965/src/silk_shell_core_root.cesta.o: src/silk_shell_core_root.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_core_root.c' -o "$@"

build/silk-shell-core/obj/src/silk_shell_core_root.cesta.o: build/.objects/03a810070a584965/src/silk_shell_core_root.cesta.o
	@mkdir -p 'build/silk-shell-core/obj/src'
	ln -f 'build/.objects/03a810070a584965/src/silk_shell_core_root.cesta.o' "$@"

build/.objects/bcb131a54bbae66f/src/silk_shell_core_dispatch.cesta.o: src/silk_shell_core_dispatch.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_core_dispatch.c' -o "$@"

build/silk-shell-core/obj/src/silk_shell_core_dispatch.cesta.o: build/.objects/bcb131a54bbae66f/src/silk_shell_core_dispatch.cesta.o
	@mkdir -p 'build/silk-shell-core/obj/src'
	ln -f 'build/.objects/bcb131a54bbae66f/src/silk_shell_core_dispatch.cesta.o' "$@"

build/.objects/a3066bb833682a36/src/silk_shell_startup.cesta.o: src/silk_shell_startup.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_startup.c' -o "$@"

build/silk-shell-core/obj/src/silk_shell_startup.cesta.o: build/.objects/a3066bb833682a36/src/silk_shell_startup.cesta.o
	@mkdir -p 'build/silk-shell-core/obj/src'
	ln -f 'build/.objects/a3066bb833682a36/src/silk_shell_startup.cesta.o' "$@"

build/.objects/7cfc24c6a2cae60a/src/silk_shell_core_main.cesta.o: src/silk_shell_core_main.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_core_main.c' -o "$@"

build/silk-shell-core/obj/src/silk_shell_core_main.cesta.o: build/.objects/7cfc24c6a2cae60a/src/silk_shell_core_main.cesta.o
	@mkdir -p 'build/silk-shell-core/obj/src'
	ln -f 'build/.objects/7cfc24c6a2cae60a/src/silk_shell_core_main.cesta.o' "$@"

build/.objects/e8a08c226cf54157/src/silk_shell_join.cesta.o: src/silk_shell_join.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_join.c' -o "$@"

build/silk-shell-core/obj/src/silk_shell_join.cesta.o: build/.objects/e8a08c226cf54157/src/silk_shell_join.cesta.o
	@mkdir -p 'build/silk-shell-core/obj/src'
	ln -f 'build/.objects/e8a08c226cf54157/src/silk_shell_join.cesta.o' "$@"

build/silk-shell-core/obj/src/silk_shell_proofs_b_palette.cesta.o: build/.objects/097f472246023bf9/src/silk_shell_proofs_b_palette.cesta.o
	@mkdir -p 'build/silk-shell-core/obj/src'
	ln -f 'build/.objects/097f472246023bf9/src/silk_shell_proofs_b_palette.cesta.o' "$@"

build/silk-shell-core/obj/src/silk_shell_proofs_b_palette_native.cesta.o: build/.objects/5bf20b0b5d4b8aed/src/silk_shell_proofs_b_palette_native.cesta.o
	@mkdir -p 'build/silk-shell-core/obj/src'
	ln -f 'build/.objects/5bf20b0b5d4b8aed/src/silk_shell_proofs_b_palette_native.cesta.o' "$@"

build/silk-shell-core/obj/src/silk_shell_proofs_b_atlas.cesta.o: build/.objects/4cd6003c48d7cc0d/src/silk_shell_proofs_b_atlas.cesta.o
	@mkdir -p 'build/silk-shell-core/obj/src'
	ln -f 'build/.objects/4cd6003c48d7cc0d/src/silk_shell_proofs_b_atlas.cesta.o' "$@"

build/silk-shell-core/obj/src/silk_shell_proofs_b_atlas_native.cesta.o: build/.objects/229e6db3cfb10931/src/silk_shell_proofs_b_atlas_native.cesta.o
	@mkdir -p 'build/silk-shell-core/obj/src'
	ln -f 'build/.objects/229e6db3cfb10931/src/silk_shell_proofs_b_atlas_native.cesta.o' "$@"

build/silk-shell-core/obj/src/silk_shell_proofs_b_input.cesta.o: build/.objects/f56b0da36cb87098/src/silk_shell_proofs_b_input.cesta.o
	@mkdir -p 'build/silk-shell-core/obj/src'
	ln -f 'build/.objects/f56b0da36cb87098/src/silk_shell_proofs_b_input.cesta.o' "$@"

build/silk-shell-core/obj/src/silk_shell_proofs_b_input_native.cesta.o: build/.objects/078d4efafbaa08d6/src/silk_shell_proofs_b_input_native.cesta.o
	@mkdir -p 'build/silk-shell-core/obj/src'
	ln -f 'build/.objects/078d4efafbaa08d6/src/silk_shell_proofs_b_input_native.cesta.o' "$@"

build/silk-shell-core/obj/src/silk_shell_proofs_b_root.cesta.o: build/.objects/a03373d4dd99b420/src/silk_shell_proofs_b_root.cesta.o
	@mkdir -p 'build/silk-shell-core/obj/src'
	ln -f 'build/.objects/a03373d4dd99b420/src/silk_shell_proofs_b_root.cesta.o' "$@"

build/silk-shell-core/libsilk-shell-core.a: GNUmakefile build/silk-shell-core/obj/src/silk_shell_core_root.cesta.o build/silk-shell-core/obj/src/silk_shell_core_dispatch.cesta.o build/silk-shell-core/obj/src/silk_shell_startup.cesta.o build/silk-shell-core/obj/src/silk_shell_core_main.cesta.o build/silk-shell-core/obj/src/silk_shell_join.cesta.o build/silk-shell-core/obj/src/silk_shell_proofs_b_palette.cesta.o build/silk-shell-core/obj/src/silk_shell_proofs_b_palette_native.cesta.o build/silk-shell-core/obj/src/silk_shell_proofs_b_atlas.cesta.o build/silk-shell-core/obj/src/silk_shell_proofs_b_atlas_native.cesta.o build/silk-shell-core/obj/src/silk_shell_proofs_b_input.cesta.o build/silk-shell-core/obj/src/silk_shell_proofs_b_input_native.cesta.o build/silk-shell-core/obj/src/silk_shell_proofs_b_root.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/silk-shell-core/obj/src/silk_shell_core_root.cesta.o' 'build/silk-shell-core/obj/src/silk_shell_core_dispatch.cesta.o' 'build/silk-shell-core/obj/src/silk_shell_startup.cesta.o' 'build/silk-shell-core/obj/src/silk_shell_core_main.cesta.o' 'build/silk-shell-core/obj/src/silk_shell_join.cesta.o' 'build/silk-shell-core/obj/src/silk_shell_proofs_b_palette.cesta.o' 'build/silk-shell-core/obj/src/silk_shell_proofs_b_palette_native.cesta.o' 'build/silk-shell-core/obj/src/silk_shell_proofs_b_atlas.cesta.o' 'build/silk-shell-core/obj/src/silk_shell_proofs_b_atlas_native.cesta.o' 'build/silk-shell-core/obj/src/silk_shell_proofs_b_input.cesta.o' 'build/silk-shell-core/obj/src/silk_shell_proofs_b_input_native.cesta.o' 'build/silk-shell-core/obj/src/silk_shell_proofs_b_root.cesta.o'

build/shell-proof-lifecycle-fixture/obj/src/shell_proof_lifecycle_fixture.cesta.o: build/.objects/22f296b27cc61ba6/src/shell_proof_lifecycle_fixture.cesta.o
	@mkdir -p 'build/shell-proof-lifecycle-fixture/obj/src'
	ln -f 'build/.objects/22f296b27cc61ba6/src/shell_proof_lifecycle_fixture.cesta.o' "$@"

build/shell-proof-lifecycle-fixture/libshell-proof-lifecycle-fixture.a: GNUmakefile build/shell-proof-lifecycle-fixture/obj/src/shell_proof_lifecycle_fixture.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/shell-proof-lifecycle-fixture/obj/src/shell_proof_lifecycle_fixture.cesta.o'

build/.objects/71b563a6f4e34a01/src/silk_shell_input.cesta.o: src/silk_shell_input.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_input.c' -o "$@"

build/silk-shell-input/obj/src/silk_shell_input.cesta.o: build/.objects/71b563a6f4e34a01/src/silk_shell_input.cesta.o
	@mkdir -p 'build/silk-shell-input/obj/src'
	ln -f 'build/.objects/71b563a6f4e34a01/src/silk_shell_input.cesta.o' "$@"

build/.objects/da1f6cad4ad6f709/src/silk_shell_input_adapter.cesta.o: src/silk_shell_input_adapter.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_input_adapter.c' -o "$@"

build/silk-shell-input/obj/src/silk_shell_input_adapter.cesta.o: build/.objects/da1f6cad4ad6f709/src/silk_shell_input_adapter.cesta.o
	@mkdir -p 'build/silk-shell-input/obj/src'
	ln -f 'build/.objects/da1f6cad4ad6f709/src/silk_shell_input_adapter.cesta.o' "$@"

build/.objects/25a1a6c5e9145498/src/silk_shell_input_diagnostics.cesta.o: src/silk_shell_input_diagnostics.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_input_diagnostics.c' -o "$@"

build/silk-shell-input/obj/src/silk_shell_input_diagnostics.cesta.o: build/.objects/25a1a6c5e9145498/src/silk_shell_input_diagnostics.cesta.o
	@mkdir -p 'build/silk-shell-input/obj/src'
	ln -f 'build/.objects/25a1a6c5e9145498/src/silk_shell_input_diagnostics.cesta.o' "$@"

build/.objects/82deb16b8491ce24/src/silk_shell_input_apps.cesta.o: src/silk_shell_input_apps.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_input_apps.c' -o "$@"

build/silk-shell-input/obj/src/silk_shell_input_apps.cesta.o: build/.objects/82deb16b8491ce24/src/silk_shell_input_apps.cesta.o
	@mkdir -p 'build/silk-shell-input/obj/src'
	ln -f 'build/.objects/82deb16b8491ce24/src/silk_shell_input_apps.cesta.o' "$@"

build/silk-shell-input/obj/src/silk_shell_diagnostics_a.cesta.o: build/.objects/586b63906ab18df9/src/silk_shell_diagnostics_a.cesta.o
	@mkdir -p 'build/silk-shell-input/obj/src'
	ln -f 'build/.objects/586b63906ab18df9/src/silk_shell_diagnostics_a.cesta.o' "$@"

build/silk-shell-input/obj/src/silk_shell_diagnostics_a_native.cesta.o: build/.objects/1e8ca0cf268ab0ed/src/silk_shell_diagnostics_a_native.cesta.o
	@mkdir -p 'build/silk-shell-input/obj/src'
	ln -f 'build/.objects/1e8ca0cf268ab0ed/src/silk_shell_diagnostics_a_native.cesta.o' "$@"

build/.objects/d3de41d482095a03/src/silk_shell_chrome_atomic.cesta.o: src/silk_shell_chrome_atomic.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_chrome_atomic.c' -o "$@"

build/silk-shell-input/obj/src/silk_shell_chrome_atomic.cesta.o: build/.objects/d3de41d482095a03/src/silk_shell_chrome_atomic.cesta.o
	@mkdir -p 'build/silk-shell-input/obj/src'
	ln -f 'build/.objects/d3de41d482095a03/src/silk_shell_chrome_atomic.cesta.o' "$@"

build/.objects/318917eac90e568c/src/silk_shell_custody.cesta.o: src/silk_shell_custody.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_custody.c' -o "$@"

build/silk-shell-input/obj/src/silk_shell_custody.cesta.o: build/.objects/318917eac90e568c/src/silk_shell_custody.cesta.o
	@mkdir -p 'build/silk-shell-input/obj/src'
	ln -f 'build/.objects/318917eac90e568c/src/silk_shell_custody.cesta.o' "$@"

build/silk-shell-input/libsilk-shell-input.a: GNUmakefile build/silk-shell-input/obj/src/silk_shell_input.cesta.o build/silk-shell-input/obj/src/silk_shell_input_adapter.cesta.o build/silk-shell-input/obj/src/silk_shell_input_diagnostics.cesta.o build/silk-shell-input/obj/src/silk_shell_input_apps.cesta.o build/silk-shell-input/obj/src/silk_shell_diagnostics_a.cesta.o build/silk-shell-input/obj/src/silk_shell_diagnostics_a_native.cesta.o build/silk-shell-input/obj/src/silk_shell_chrome_atomic.cesta.o build/silk-shell-input/obj/src/silk_shell_custody.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/silk-shell-input/obj/src/silk_shell_input.cesta.o' 'build/silk-shell-input/obj/src/silk_shell_input_adapter.cesta.o' 'build/silk-shell-input/obj/src/silk_shell_input_diagnostics.cesta.o' 'build/silk-shell-input/obj/src/silk_shell_input_apps.cesta.o' 'build/silk-shell-input/obj/src/silk_shell_diagnostics_a.cesta.o' 'build/silk-shell-input/obj/src/silk_shell_diagnostics_a_native.cesta.o' 'build/silk-shell-input/obj/src/silk_shell_chrome_atomic.cesta.o' 'build/silk-shell-input/obj/src/silk_shell_custody.cesta.o'

build/.objects/9a34f0c200d44680/src/silk_shell_atlas.cesta.o: src/silk_shell_atlas.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_atlas.c' -o "$@"

build/silk-shell-atlas/obj/src/silk_shell_atlas.cesta.o: build/.objects/9a34f0c200d44680/src/silk_shell_atlas.cesta.o
	@mkdir -p 'build/silk-shell-atlas/obj/src'
	ln -f 'build/.objects/9a34f0c200d44680/src/silk_shell_atlas.cesta.o' "$@"

build/.objects/6578356dc544545c/src/silk_shell_atlas_adapter.cesta.o: src/silk_shell_atlas_adapter.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_atlas_adapter.c' -o "$@"

build/silk-shell-atlas/obj/src/silk_shell_atlas_adapter.cesta.o: build/.objects/6578356dc544545c/src/silk_shell_atlas_adapter.cesta.o
	@mkdir -p 'build/silk-shell-atlas/obj/src'
	ln -f 'build/.objects/6578356dc544545c/src/silk_shell_atlas_adapter.cesta.o' "$@"

build/silk-shell-atlas/libsilk-shell-atlas.a: GNUmakefile build/silk-shell-atlas/obj/src/silk_shell_atlas.cesta.o build/silk-shell-atlas/obj/src/silk_shell_atlas_adapter.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/silk-shell-atlas/obj/src/silk_shell_atlas.cesta.o' 'build/silk-shell-atlas/obj/src/silk_shell_atlas_adapter.cesta.o'

build/silk-shell-custody/obj/src/silk_shell_custody.cesta.o: build/.objects/318917eac90e568c/src/silk_shell_custody.cesta.o
	@mkdir -p 'build/silk-shell-custody/obj/src'
	ln -f 'build/.objects/318917eac90e568c/src/silk_shell_custody.cesta.o' "$@"

build/silk-shell-custody/libsilk-shell-custody.a: GNUmakefile build/silk-shell-custody/obj/src/silk_shell_custody.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/silk-shell-custody/obj/src/silk_shell_custody.cesta.o'

build/.objects/8680e8dba55106dd/src/silk_shell_chrome.cesta.o: src/silk_shell_chrome.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_chrome.c' -o "$@"

build/silk-shell-chrome/obj/src/silk_shell_chrome.cesta.o: build/.objects/8680e8dba55106dd/src/silk_shell_chrome.cesta.o
	@mkdir -p 'build/silk-shell-chrome/obj/src'
	ln -f 'build/.objects/8680e8dba55106dd/src/silk_shell_chrome.cesta.o' "$@"

build/silk-shell-chrome/obj/src/silk_shell_chrome_atomic.cesta.o: build/.objects/d3de41d482095a03/src/silk_shell_chrome_atomic.cesta.o
	@mkdir -p 'build/silk-shell-chrome/obj/src'
	ln -f 'build/.objects/d3de41d482095a03/src/silk_shell_chrome_atomic.cesta.o' "$@"

build/silk-shell-chrome/obj/src/silk_shell_custody.cesta.o: build/.objects/318917eac90e568c/src/silk_shell_custody.cesta.o
	@mkdir -p 'build/silk-shell-chrome/obj/src'
	ln -f 'build/.objects/318917eac90e568c/src/silk_shell_custody.cesta.o' "$@"

build/.objects/4ca431533f2a9801/src/silk_shell_viewport.cesta.o: src/silk_shell_viewport.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_viewport.c' -o "$@"

build/silk-shell-chrome/obj/src/silk_shell_viewport.cesta.o: build/.objects/4ca431533f2a9801/src/silk_shell_viewport.cesta.o
	@mkdir -p 'build/silk-shell-chrome/obj/src'
	ln -f 'build/.objects/4ca431533f2a9801/src/silk_shell_viewport.cesta.o' "$@"

build/.objects/b273779147a5b9ad/src/silkbar_geometry.cesta.o: src/silkbar_geometry.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silkbar_geometry.c' -o "$@"

build/silk-shell-chrome/obj/src/silkbar_geometry.cesta.o: build/.objects/b273779147a5b9ad/src/silkbar_geometry.cesta.o
	@mkdir -p 'build/silk-shell-chrome/obj/src'
	ln -f 'build/.objects/b273779147a5b9ad/src/silkbar_geometry.cesta.o' "$@"

build/.objects/75703275afdd8d3c/src/silkbar_model.cesta.o: src/silkbar_model.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silkbar_model.c' -o "$@"

build/silk-shell-chrome/obj/src/silkbar_model.cesta.o: build/.objects/75703275afdd8d3c/src/silkbar_model.cesta.o
	@mkdir -p 'build/silk-shell-chrome/obj/src'
	ln -f 'build/.objects/75703275afdd8d3c/src/silkbar_model.cesta.o' "$@"

build/silk-shell-chrome/libsilk-shell-chrome.a: GNUmakefile build/silk-shell-chrome/obj/src/silk_shell_chrome.cesta.o build/silk-shell-chrome/obj/src/silk_shell_chrome_atomic.cesta.o build/silk-shell-chrome/obj/src/silk_shell_custody.cesta.o build/silk-shell-chrome/obj/src/silk_shell_viewport.cesta.o build/silk-shell-chrome/obj/src/silkbar_geometry.cesta.o build/silk-shell-chrome/obj/src/silkbar_model.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/silk-shell-chrome/obj/src/silk_shell_chrome.cesta.o' 'build/silk-shell-chrome/obj/src/silk_shell_chrome_atomic.cesta.o' 'build/silk-shell-chrome/obj/src/silk_shell_custody.cesta.o' 'build/silk-shell-chrome/obj/src/silk_shell_viewport.cesta.o' 'build/silk-shell-chrome/obj/src/silkbar_geometry.cesta.o' 'build/silk-shell-chrome/obj/src/silkbar_model.cesta.o'

build/silk-shell-viewport/obj/src/silk_shell_viewport.cesta.o: build/.objects/4ca431533f2a9801/src/silk_shell_viewport.cesta.o
	@mkdir -p 'build/silk-shell-viewport/obj/src'
	ln -f 'build/.objects/4ca431533f2a9801/src/silk_shell_viewport.cesta.o' "$@"

build/silk-shell-viewport/obj/src/silk_shell_custody.cesta.o: build/.objects/318917eac90e568c/src/silk_shell_custody.cesta.o
	@mkdir -p 'build/silk-shell-viewport/obj/src'
	ln -f 'build/.objects/318917eac90e568c/src/silk_shell_custody.cesta.o' "$@"

build/silk-shell-viewport/libsilk-shell-viewport.a: GNUmakefile build/silk-shell-viewport/obj/src/silk_shell_viewport.cesta.o build/silk-shell-viewport/obj/src/silk_shell_custody.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/silk-shell-viewport/obj/src/silk_shell_viewport.cesta.o' 'build/silk-shell-viewport/obj/src/silk_shell_custody.cesta.o'

build/shell-text-delivery/obj/src/shell_text_delivery.cesta.o: build/.objects/c7012c49e102c817/src/shell_text_delivery.cesta.o
	@mkdir -p 'build/shell-text-delivery/obj/src'
	ln -f 'build/.objects/c7012c49e102c817/src/shell_text_delivery.cesta.o' "$@"

build/shell-text-delivery/obj/src/shell_text_delivery_native.cesta.o: build/.objects/6bbc66393ab49813/src/shell_text_delivery_native.cesta.o
	@mkdir -p 'build/shell-text-delivery/obj/src'
	ln -f 'build/.objects/6bbc66393ab49813/src/shell_text_delivery_native.cesta.o' "$@"

build/shell-text-delivery/obj/src/shell_text_delivery_display.cesta.o: build/.objects/3bd3fa3dfebffe90/src/shell_text_delivery_display.cesta.o
	@mkdir -p 'build/shell-text-delivery/obj/src'
	ln -f 'build/.objects/3bd3fa3dfebffe90/src/shell_text_delivery_display.cesta.o' "$@"

build/shell-text-delivery/libshell-text-delivery.a: GNUmakefile build/shell-text-delivery/obj/src/shell_text_delivery.cesta.o build/shell-text-delivery/obj/src/shell_text_delivery_native.cesta.o build/shell-text-delivery/obj/src/shell_text_delivery_display.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/shell-text-delivery/obj/src/shell_text_delivery.cesta.o' 'build/shell-text-delivery/obj/src/shell_text_delivery_native.cesta.o' 'build/shell-text-delivery/obj/src/shell_text_delivery_display.cesta.o'

build/.objects/0da0f4564068a4db/src/silk_shell_surfaces.cesta.o: src/silk_shell_surfaces.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_surfaces.c' -o "$@"

build/silk-shell-surfaces/obj/src/silk_shell_surfaces.cesta.o: build/.objects/0da0f4564068a4db/src/silk_shell_surfaces.cesta.o
	@mkdir -p 'build/silk-shell-surfaces/obj/src'
	ln -f 'build/.objects/0da0f4564068a4db/src/silk_shell_surfaces.cesta.o' "$@"

build/.objects/df5b33e129550817/src/silk_shell_surfaces_native.cesta.o: src/silk_shell_surfaces_native.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_surfaces_native.c' -o "$@"

build/silk-shell-surfaces/obj/src/silk_shell_surfaces_native.cesta.o: build/.objects/df5b33e129550817/src/silk_shell_surfaces_native.cesta.o
	@mkdir -p 'build/silk-shell-surfaces/obj/src'
	ln -f 'build/.objects/df5b33e129550817/src/silk_shell_surfaces_native.cesta.o' "$@"

build/.objects/0fade820838c2599/src/silk_shell_commands.cesta.o: src/silk_shell_commands.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_commands.c' -o "$@"

build/silk-shell-surfaces/obj/src/silk_shell_commands.cesta.o: build/.objects/0fade820838c2599/src/silk_shell_commands.cesta.o
	@mkdir -p 'build/silk-shell-surfaces/obj/src'
	ln -f 'build/.objects/0fade820838c2599/src/silk_shell_commands.cesta.o' "$@"

build/.objects/269ace6ffec6523e/src/silk_shell_command_count.cesta.o: src/silk_shell_command_count.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_command_count.c' -o "$@"

build/silk-shell-surfaces/obj/src/silk_shell_command_count.cesta.o: build/.objects/269ace6ffec6523e/src/silk_shell_command_count.cesta.o
	@mkdir -p 'build/silk-shell-surfaces/obj/src'
	ln -f 'build/.objects/269ace6ffec6523e/src/silk_shell_command_count.cesta.o' "$@"

build/silk-shell-surfaces/obj/src/shell_text_delivery.cesta.o: build/.objects/c7012c49e102c817/src/shell_text_delivery.cesta.o
	@mkdir -p 'build/silk-shell-surfaces/obj/src'
	ln -f 'build/.objects/c7012c49e102c817/src/shell_text_delivery.cesta.o' "$@"

build/silk-shell-surfaces/obj/src/shell_text_delivery_native.cesta.o: build/.objects/6bbc66393ab49813/src/shell_text_delivery_native.cesta.o
	@mkdir -p 'build/silk-shell-surfaces/obj/src'
	ln -f 'build/.objects/6bbc66393ab49813/src/shell_text_delivery_native.cesta.o' "$@"

build/silk-shell-surfaces/obj/src/shell_text_delivery_display.cesta.o: build/.objects/3bd3fa3dfebffe90/src/shell_text_delivery_display.cesta.o
	@mkdir -p 'build/silk-shell-surfaces/obj/src'
	ln -f 'build/.objects/3bd3fa3dfebffe90/src/shell_text_delivery_display.cesta.o' "$@"

build/silk-shell-surfaces/obj/src/silk_shell_chrome.cesta.o: build/.objects/8680e8dba55106dd/src/silk_shell_chrome.cesta.o
	@mkdir -p 'build/silk-shell-surfaces/obj/src'
	ln -f 'build/.objects/8680e8dba55106dd/src/silk_shell_chrome.cesta.o' "$@"

build/silk-shell-surfaces/obj/src/silk_shell_chrome_atomic.cesta.o: build/.objects/d3de41d482095a03/src/silk_shell_chrome_atomic.cesta.o
	@mkdir -p 'build/silk-shell-surfaces/obj/src'
	ln -f 'build/.objects/d3de41d482095a03/src/silk_shell_chrome_atomic.cesta.o' "$@"

build/silk-shell-surfaces/obj/src/silk_shell_custody.cesta.o: build/.objects/318917eac90e568c/src/silk_shell_custody.cesta.o
	@mkdir -p 'build/silk-shell-surfaces/obj/src'
	ln -f 'build/.objects/318917eac90e568c/src/silk_shell_custody.cesta.o' "$@"

build/silk-shell-surfaces/obj/src/silk_shell_viewport.cesta.o: build/.objects/4ca431533f2a9801/src/silk_shell_viewport.cesta.o
	@mkdir -p 'build/silk-shell-surfaces/obj/src'
	ln -f 'build/.objects/4ca431533f2a9801/src/silk_shell_viewport.cesta.o' "$@"

build/silk-shell-surfaces/obj/src/silkbar_geometry.cesta.o: build/.objects/b273779147a5b9ad/src/silkbar_geometry.cesta.o
	@mkdir -p 'build/silk-shell-surfaces/obj/src'
	ln -f 'build/.objects/b273779147a5b9ad/src/silkbar_geometry.cesta.o' "$@"

build/silk-shell-surfaces/obj/src/silkbar_model.cesta.o: build/.objects/75703275afdd8d3c/src/silkbar_model.cesta.o
	@mkdir -p 'build/silk-shell-surfaces/obj/src'
	ln -f 'build/.objects/75703275afdd8d3c/src/silkbar_model.cesta.o' "$@"

build/silk-shell-surfaces/libsilk-shell-surfaces.a: GNUmakefile build/silk-shell-surfaces/obj/src/silk_shell_surfaces.cesta.o build/silk-shell-surfaces/obj/src/silk_shell_surfaces_native.cesta.o build/silk-shell-surfaces/obj/src/silk_shell_commands.cesta.o build/silk-shell-surfaces/obj/src/silk_shell_command_count.cesta.o build/silk-shell-surfaces/obj/src/shell_text_delivery.cesta.o build/silk-shell-surfaces/obj/src/shell_text_delivery_native.cesta.o build/silk-shell-surfaces/obj/src/shell_text_delivery_display.cesta.o build/silk-shell-surfaces/obj/src/silk_shell_chrome.cesta.o build/silk-shell-surfaces/obj/src/silk_shell_chrome_atomic.cesta.o build/silk-shell-surfaces/obj/src/silk_shell_custody.cesta.o build/silk-shell-surfaces/obj/src/silk_shell_viewport.cesta.o build/silk-shell-surfaces/obj/src/silkbar_geometry.cesta.o build/silk-shell-surfaces/obj/src/silkbar_model.cesta.o
	@mkdir -p "$(@D)"
	rm -f "$@" && $(AR) rcs "$@" 'build/silk-shell-surfaces/obj/src/silk_shell_surfaces.cesta.o' 'build/silk-shell-surfaces/obj/src/silk_shell_surfaces_native.cesta.o' 'build/silk-shell-surfaces/obj/src/silk_shell_commands.cesta.o' 'build/silk-shell-surfaces/obj/src/silk_shell_command_count.cesta.o' 'build/silk-shell-surfaces/obj/src/shell_text_delivery.cesta.o' 'build/silk-shell-surfaces/obj/src/shell_text_delivery_native.cesta.o' 'build/silk-shell-surfaces/obj/src/shell_text_delivery_display.cesta.o' 'build/silk-shell-surfaces/obj/src/silk_shell_chrome.cesta.o' 'build/silk-shell-surfaces/obj/src/silk_shell_chrome_atomic.cesta.o' 'build/silk-shell-surfaces/obj/src/silk_shell_custody.cesta.o' 'build/silk-shell-surfaces/obj/src/silk_shell_viewport.cesta.o' 'build/silk-shell-surfaces/obj/src/silkbar_geometry.cesta.o' 'build/silk-shell-surfaces/obj/src/silkbar_model.cesta.o'

build/.objects/ad36d7b1050a569c/src/silk_shell_entry.cesta.o: src/silk_shell_entry.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=silk_shell_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silk_shell_entry.c' -o "$@"

build/silk-shell/obj/src/silk_shell_entry.cesta.o: build/.objects/ad36d7b1050a569c/src/silk_shell_entry.cesta.o
	@mkdir -p 'build/silk-shell/obj/src'
	ln -f 'build/.objects/ad36d7b1050a569c/src/silk_shell_entry.cesta.o' "$@"

build/.objects/c15ee7431716832a/runtime/client_runtime.c.o: runtime/client_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=silk_shell_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/silk-shell/obj/runtime/client_runtime.c.o: build/.objects/c15ee7431716832a/runtime/client_runtime.c.o
	@mkdir -p 'build/silk-shell/obj/runtime'
	ln -f 'build/.objects/c15ee7431716832a/runtime/client_runtime.c.o' "$@"

build/.objects/5c3e1ccf60d99346/runtime/server_runtime.c.o: runtime/server_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=silk_shell_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_runtime.c' -o "$@"

build/silk-shell/obj/runtime/server_runtime.c.o: build/.objects/5c3e1ccf60d99346/runtime/server_runtime.c.o
	@mkdir -p 'build/silk-shell/obj/runtime'
	ln -f 'build/.objects/5c3e1ccf60d99346/runtime/server_runtime.c.o' "$@"

build/.objects/bd1440bf39fc845e/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=silk_shell_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/silk-shell/obj/runtime/traps_x86_64.c.o: build/.objects/bd1440bf39fc845e/runtime/traps_x86_64.c.o
	@mkdir -p 'build/silk-shell/obj/runtime'
	ln -f 'build/.objects/bd1440bf39fc845e/runtime/traps_x86_64.c.o' "$@"

build/.objects/d480912068b8dcea/runtime/server_traps_x86_64.c.o: runtime/server_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=silk_shell_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_traps_x86_64.c' -o "$@"

build/silk-shell/obj/runtime/server_traps_x86_64.c.o: build/.objects/d480912068b8dcea/runtime/server_traps_x86_64.c.o
	@mkdir -p 'build/silk-shell/obj/runtime'
	ln -f 'build/.objects/d480912068b8dcea/runtime/server_traps_x86_64.c.o' "$@"

build/.objects/79ffa47f373abe9a/runtime/start_x86_64.S.o: runtime/start_x86_64.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=silk_shell_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/silk-shell/obj/runtime/start_x86_64.S.o: build/.objects/79ffa47f373abe9a/runtime/start_x86_64.S.o
	@mkdir -p 'build/silk-shell/obj/runtime'
	ln -f 'build/.objects/79ffa47f373abe9a/runtime/start_x86_64.S.o' "$@"

build/.objects/3bf24639e5833dd2/runtime/rpc_traps_x86_64.c.o: runtime/rpc_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=silk_shell_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/rpc_traps_x86_64.c' -o "$@"

build/silk-shell/obj/runtime/rpc_traps_x86_64.c.o: build/.objects/3bf24639e5833dd2/runtime/rpc_traps_x86_64.c.o
	@mkdir -p 'build/silk-shell/obj/runtime'
	ln -f 'build/.objects/3bf24639e5833dd2/runtime/rpc_traps_x86_64.c.o' "$@"

build/.objects/ccfc3357f2199433/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=silk_shell_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/silk-shell/obj/generated/runtime/format_bytes.c.o: build/.objects/ccfc3357f2199433/generated/runtime/format_bytes.c.o
	@mkdir -p 'build/silk-shell/obj/generated/runtime'
	ln -f 'build/.objects/ccfc3357f2199433/generated/runtime/format_bytes.c.o' "$@"

build/.objects/36b5fd63c24ac30a/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=silk_shell_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/silk-shell/obj/generated/runtime/packed_words.c.o: build/.objects/36b5fd63c24ac30a/generated/runtime/packed_words.c.o
	@mkdir -p 'build/silk-shell/obj/generated/runtime'
	ln -f 'build/.objects/36b5fd63c24ac30a/generated/runtime/packed_words.c.o' "$@"

build/silk-shell/silk-shell: GNUmakefile build/silk-shell/obj/src/silk_shell_entry.cesta.o build/silk-shell/obj/runtime/client_runtime.c.o build/silk-shell/obj/runtime/server_runtime.c.o build/silk-shell/obj/runtime/traps_x86_64.c.o build/silk-shell/obj/runtime/server_traps_x86_64.c.o build/silk-shell/obj/runtime/start_x86_64.S.o build/silk-shell/obj/runtime/rpc_traps_x86_64.c.o build/silk-shell/obj/generated/runtime/format_bytes.c.o build/silk-shell/obj/generated/runtime/packed_words.c.o build/silk-shell-core/libsilk-shell-core.a build/silk-shell-models/libsilk-shell-models.a build/silk-shell-input/libsilk-shell-input.a build/silk-shell-atlas/libsilk-shell-atlas.a build/silk-shell-surfaces/libsilk-shell-surfaces.a build/silk-shell-chrome/libsilk-shell-chrome.a build/silk-shell-diagnostics-a/libsilk-shell-diagnostics-a.a build/silk-shell-custody/libsilk-shell-custody.a build/silk-shell-viewport/libsilk-shell-viewport.a build/shell-text-delivery/libshell-text-delivery.a
	@mkdir -p "$(@D)"
	$(CC) 'build/silk-shell/obj/src/silk_shell_entry.cesta.o' 'build/silk-shell/obj/runtime/client_runtime.c.o' 'build/silk-shell/obj/runtime/server_runtime.c.o' 'build/silk-shell/obj/runtime/traps_x86_64.c.o' 'build/silk-shell/obj/runtime/server_traps_x86_64.c.o' 'build/silk-shell/obj/runtime/start_x86_64.S.o' 'build/silk-shell/obj/runtime/rpc_traps_x86_64.c.o' 'build/silk-shell/obj/generated/runtime/format_bytes.c.o' 'build/silk-shell/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' '-Wl,--start-group' 'build/silk-shell-core/libsilk-shell-core.a' 'build/silk-shell-models/libsilk-shell-models.a' 'build/silk-shell-input/libsilk-shell-input.a' 'build/silk-shell-atlas/libsilk-shell-atlas.a' 'build/silk-shell-surfaces/libsilk-shell-surfaces.a' 'build/silk-shell-chrome/libsilk-shell-chrome.a' 'build/silk-shell-diagnostics-a/libsilk-shell-diagnostics-a.a' 'build/silk-shell-custody/libsilk-shell-custody.a' 'build/silk-shell-viewport/libsilk-shell-viewport.a' 'build/shell-text-delivery/libshell-text-delivery.a' '-Wl,--end-group' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/.objects/a62e5922dd8c3c8b/tests/pager_boot/client.cesta.o: tests/pager_boot/client.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=pager_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'tests/pager_boot/client.c' -o "$@"

build/pager-probe/obj/tests/pager_boot/client.cesta.o: build/.objects/a62e5922dd8c3c8b/tests/pager_boot/client.cesta.o
	@mkdir -p 'build/pager-probe/obj/tests/pager_boot'
	ln -f 'build/.objects/a62e5922dd8c3c8b/tests/pager_boot/client.cesta.o' "$@"

build/.objects/dda77889cda65e0f/runtime/client_runtime.c.o: runtime/client_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=pager_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/pager-probe/obj/runtime/client_runtime.c.o: build/.objects/dda77889cda65e0f/runtime/client_runtime.c.o
	@mkdir -p 'build/pager-probe/obj/runtime'
	ln -f 'build/.objects/dda77889cda65e0f/runtime/client_runtime.c.o' "$@"

build/.objects/d358e8a82b99adb3/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=pager_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/pager-probe/obj/runtime/traps_x86_64.c.o: build/.objects/d358e8a82b99adb3/runtime/traps_x86_64.c.o
	@mkdir -p 'build/pager-probe/obj/runtime'
	ln -f 'build/.objects/d358e8a82b99adb3/runtime/traps_x86_64.c.o' "$@"

build/.objects/15474164bcb3d2ff/runtime/start_x86_64.S.o: runtime/start_x86_64.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=pager_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/pager-probe/obj/runtime/start_x86_64.S.o: build/.objects/15474164bcb3d2ff/runtime/start_x86_64.S.o
	@mkdir -p 'build/pager-probe/obj/runtime'
	ln -f 'build/.objects/15474164bcb3d2ff/runtime/start_x86_64.S.o' "$@"

build/.objects/d72a42f7281f5d80/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=pager_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/pager-probe/obj/generated/runtime/format_bytes.c.o: build/.objects/d72a42f7281f5d80/generated/runtime/format_bytes.c.o
	@mkdir -p 'build/pager-probe/obj/generated/runtime'
	ln -f 'build/.objects/d72a42f7281f5d80/generated/runtime/format_bytes.c.o' "$@"

build/.objects/9a1dfed1a02bf0ef/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=pager_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/pager-probe/obj/generated/runtime/packed_words.c.o: build/.objects/9a1dfed1a02bf0ef/generated/runtime/packed_words.c.o
	@mkdir -p 'build/pager-probe/obj/generated/runtime'
	ln -f 'build/.objects/9a1dfed1a02bf0ef/generated/runtime/packed_words.c.o' "$@"

build/pager-probe/pager-probe: GNUmakefile build/pager-probe/obj/tests/pager_boot/client.cesta.o build/pager-probe/obj/runtime/client_runtime.c.o build/pager-probe/obj/runtime/traps_x86_64.c.o build/pager-probe/obj/runtime/start_x86_64.S.o build/pager-probe/obj/generated/runtime/format_bytes.c.o build/pager-probe/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/pager-probe/obj/tests/pager_boot/client.cesta.o' 'build/pager-probe/obj/runtime/client_runtime.c.o' 'build/pager-probe/obj/runtime/traps_x86_64.c.o' 'build/pager-probe/obj/runtime/start_x86_64.S.o' 'build/pager-probe/obj/generated/runtime/format_bytes.c.o' 'build/pager-probe/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/.objects/919adea1445a6082/tests/kernel_sources/probe.cesta.o: tests/kernel_sources/probe.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=source_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'tests/kernel_sources/probe.c' -o "$@"

build/source-probe/obj/tests/kernel_sources/probe.cesta.o: build/.objects/919adea1445a6082/tests/kernel_sources/probe.cesta.o
	@mkdir -p 'build/source-probe/obj/tests/kernel_sources'
	ln -f 'build/.objects/919adea1445a6082/tests/kernel_sources/probe.cesta.o' "$@"

build/.objects/3023b88ae6bb5507/runtime/client_runtime.c.o: runtime/client_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=source_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/source-probe/obj/runtime/client_runtime.c.o: build/.objects/3023b88ae6bb5507/runtime/client_runtime.c.o
	@mkdir -p 'build/source-probe/obj/runtime'
	ln -f 'build/.objects/3023b88ae6bb5507/runtime/client_runtime.c.o' "$@"

build/.objects/b17bb058d1baeb2b/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=source_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/source-probe/obj/runtime/traps_x86_64.c.o: build/.objects/b17bb058d1baeb2b/runtime/traps_x86_64.c.o
	@mkdir -p 'build/source-probe/obj/runtime'
	ln -f 'build/.objects/b17bb058d1baeb2b/runtime/traps_x86_64.c.o' "$@"

build/.objects/3ba865a8a62be037/runtime/start_x86_64.S.o: runtime/start_x86_64.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=source_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/source-probe/obj/runtime/start_x86_64.S.o: build/.objects/3ba865a8a62be037/runtime/start_x86_64.S.o
	@mkdir -p 'build/source-probe/obj/runtime'
	ln -f 'build/.objects/3ba865a8a62be037/runtime/start_x86_64.S.o' "$@"

build/.objects/c8494a256898d9da/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=source_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/source-probe/obj/generated/runtime/format_bytes.c.o: build/.objects/c8494a256898d9da/generated/runtime/format_bytes.c.o
	@mkdir -p 'build/source-probe/obj/generated/runtime'
	ln -f 'build/.objects/c8494a256898d9da/generated/runtime/format_bytes.c.o' "$@"

build/.objects/cb6fe1a102bba967/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=source_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/source-probe/obj/generated/runtime/packed_words.c.o: build/.objects/cb6fe1a102bba967/generated/runtime/packed_words.c.o
	@mkdir -p 'build/source-probe/obj/generated/runtime'
	ln -f 'build/.objects/cb6fe1a102bba967/generated/runtime/packed_words.c.o' "$@"

build/source-probe/source-probe: GNUmakefile build/source-probe/obj/tests/kernel_sources/probe.cesta.o build/source-probe/obj/runtime/client_runtime.c.o build/source-probe/obj/runtime/traps_x86_64.c.o build/source-probe/obj/runtime/start_x86_64.S.o build/source-probe/obj/generated/runtime/format_bytes.c.o build/source-probe/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/source-probe/obj/tests/kernel_sources/probe.cesta.o' 'build/source-probe/obj/runtime/client_runtime.c.o' 'build/source-probe/obj/runtime/traps_x86_64.c.o' 'build/source-probe/obj/runtime/start_x86_64.S.o' 'build/source-probe/obj/generated/runtime/format_bytes.c.o' 'build/source-probe/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/.objects/6220d230814ccdd1/tests/kernel_pci/probe.cesta.o: tests/kernel_pci/probe.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=pci_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'tests/kernel_pci/probe.c' -o "$@"

build/pci-probe/obj/tests/kernel_pci/probe.cesta.o: build/.objects/6220d230814ccdd1/tests/kernel_pci/probe.cesta.o
	@mkdir -p 'build/pci-probe/obj/tests/kernel_pci'
	ln -f 'build/.objects/6220d230814ccdd1/tests/kernel_pci/probe.cesta.o' "$@"

build/.objects/ae07bfb64a68deb4/runtime/client_runtime.c.o: runtime/client_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=pci_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/pci-probe/obj/runtime/client_runtime.c.o: build/.objects/ae07bfb64a68deb4/runtime/client_runtime.c.o
	@mkdir -p 'build/pci-probe/obj/runtime'
	ln -f 'build/.objects/ae07bfb64a68deb4/runtime/client_runtime.c.o' "$@"

build/.objects/3168376a1459dc68/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=pci_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/pci-probe/obj/runtime/traps_x86_64.c.o: build/.objects/3168376a1459dc68/runtime/traps_x86_64.c.o
	@mkdir -p 'build/pci-probe/obj/runtime'
	ln -f 'build/.objects/3168376a1459dc68/runtime/traps_x86_64.c.o' "$@"

build/.objects/11744d0c50cf8a04/runtime/start_x86_64.S.o: runtime/start_x86_64.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=pci_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/pci-probe/obj/runtime/start_x86_64.S.o: build/.objects/11744d0c50cf8a04/runtime/start_x86_64.S.o
	@mkdir -p 'build/pci-probe/obj/runtime'
	ln -f 'build/.objects/11744d0c50cf8a04/runtime/start_x86_64.S.o' "$@"

build/.objects/16d95ffca3b78843/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=pci_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/pci-probe/obj/generated/runtime/format_bytes.c.o: build/.objects/16d95ffca3b78843/generated/runtime/format_bytes.c.o
	@mkdir -p 'build/pci-probe/obj/generated/runtime'
	ln -f 'build/.objects/16d95ffca3b78843/generated/runtime/format_bytes.c.o' "$@"

build/.objects/be9016d95693fe14/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=pci_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/pci-probe/obj/generated/runtime/packed_words.c.o: build/.objects/be9016d95693fe14/generated/runtime/packed_words.c.o
	@mkdir -p 'build/pci-probe/obj/generated/runtime'
	ln -f 'build/.objects/be9016d95693fe14/generated/runtime/packed_words.c.o' "$@"

build/pci-probe/pci-probe: GNUmakefile build/pci-probe/obj/tests/kernel_pci/probe.cesta.o build/pci-probe/obj/runtime/client_runtime.c.o build/pci-probe/obj/runtime/traps_x86_64.c.o build/pci-probe/obj/runtime/start_x86_64.S.o build/pci-probe/obj/generated/runtime/format_bytes.c.o build/pci-probe/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/pci-probe/obj/tests/kernel_pci/probe.cesta.o' 'build/pci-probe/obj/runtime/client_runtime.c.o' 'build/pci-probe/obj/runtime/traps_x86_64.c.o' 'build/pci-probe/obj/runtime/start_x86_64.S.o' 'build/pci-probe/obj/generated/runtime/format_bytes.c.o' 'build/pci-probe/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/.objects/5edd7c83164cfcd1/tests/region_writer/probe.cesta.o: tests/region_writer/probe.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=region_writer_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'tests/region_writer/probe.c' -o "$@"

build/region-writer-probe/obj/tests/region_writer/probe.cesta.o: build/.objects/5edd7c83164cfcd1/tests/region_writer/probe.cesta.o
	@mkdir -p 'build/region-writer-probe/obj/tests/region_writer'
	ln -f 'build/.objects/5edd7c83164cfcd1/tests/region_writer/probe.cesta.o' "$@"

build/.objects/f23c2820b4ff15e6/runtime/client_runtime.c.o: runtime/client_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=region_writer_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/region-writer-probe/obj/runtime/client_runtime.c.o: build/.objects/f23c2820b4ff15e6/runtime/client_runtime.c.o
	@mkdir -p 'build/region-writer-probe/obj/runtime'
	ln -f 'build/.objects/f23c2820b4ff15e6/runtime/client_runtime.c.o' "$@"

build/.objects/3c1224cb5ab9ebf2/runtime/server_runtime.c.o: runtime/server_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=region_writer_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_runtime.c' -o "$@"

build/region-writer-probe/obj/runtime/server_runtime.c.o: build/.objects/3c1224cb5ab9ebf2/runtime/server_runtime.c.o
	@mkdir -p 'build/region-writer-probe/obj/runtime'
	ln -f 'build/.objects/3c1224cb5ab9ebf2/runtime/server_runtime.c.o' "$@"

build/.objects/ce497601b0b0924a/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=region_writer_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/region-writer-probe/obj/runtime/traps_x86_64.c.o: build/.objects/ce497601b0b0924a/runtime/traps_x86_64.c.o
	@mkdir -p 'build/region-writer-probe/obj/runtime'
	ln -f 'build/.objects/ce497601b0b0924a/runtime/traps_x86_64.c.o' "$@"

build/.objects/0448ac03a5f16426/runtime/server_traps_x86_64.c.o: runtime/server_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=region_writer_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_traps_x86_64.c' -o "$@"

build/region-writer-probe/obj/runtime/server_traps_x86_64.c.o: build/.objects/0448ac03a5f16426/runtime/server_traps_x86_64.c.o
	@mkdir -p 'build/region-writer-probe/obj/runtime'
	ln -f 'build/.objects/0448ac03a5f16426/runtime/server_traps_x86_64.c.o' "$@"

build/.objects/13b2f4fc5ba8ae7b/runtime/region_writer_traps_x86_64.c.o: runtime/region_writer_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=region_writer_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/region_writer_traps_x86_64.c' -o "$@"

build/region-writer-probe/obj/runtime/region_writer_traps_x86_64.c.o: build/.objects/13b2f4fc5ba8ae7b/runtime/region_writer_traps_x86_64.c.o
	@mkdir -p 'build/region-writer-probe/obj/runtime'
	ln -f 'build/.objects/13b2f4fc5ba8ae7b/runtime/region_writer_traps_x86_64.c.o' "$@"

build/.objects/74dbdb8697d0f456/runtime/start_x86_64.S.o: runtime/start_x86_64.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=region_writer_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/region-writer-probe/obj/runtime/start_x86_64.S.o: build/.objects/74dbdb8697d0f456/runtime/start_x86_64.S.o
	@mkdir -p 'build/region-writer-probe/obj/runtime'
	ln -f 'build/.objects/74dbdb8697d0f456/runtime/start_x86_64.S.o' "$@"

build/.objects/624f143532dc80b9/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=region_writer_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/region-writer-probe/obj/generated/runtime/format_bytes.c.o: build/.objects/624f143532dc80b9/generated/runtime/format_bytes.c.o
	@mkdir -p 'build/region-writer-probe/obj/generated/runtime'
	ln -f 'build/.objects/624f143532dc80b9/generated/runtime/format_bytes.c.o' "$@"

build/.objects/6e5c2f8f0e823d86/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=region_writer_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/region-writer-probe/obj/generated/runtime/packed_words.c.o: build/.objects/6e5c2f8f0e823d86/generated/runtime/packed_words.c.o
	@mkdir -p 'build/region-writer-probe/obj/generated/runtime'
	ln -f 'build/.objects/6e5c2f8f0e823d86/generated/runtime/packed_words.c.o' "$@"

build/region-writer-probe/region-writer-probe: GNUmakefile build/region-writer-probe/obj/tests/region_writer/probe.cesta.o build/region-writer-probe/obj/runtime/client_runtime.c.o build/region-writer-probe/obj/runtime/server_runtime.c.o build/region-writer-probe/obj/runtime/traps_x86_64.c.o build/region-writer-probe/obj/runtime/server_traps_x86_64.c.o build/region-writer-probe/obj/runtime/region_writer_traps_x86_64.c.o build/region-writer-probe/obj/runtime/start_x86_64.S.o build/region-writer-probe/obj/generated/runtime/format_bytes.c.o build/region-writer-probe/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/region-writer-probe/obj/tests/region_writer/probe.cesta.o' 'build/region-writer-probe/obj/runtime/client_runtime.c.o' 'build/region-writer-probe/obj/runtime/server_runtime.c.o' 'build/region-writer-probe/obj/runtime/traps_x86_64.c.o' 'build/region-writer-probe/obj/runtime/server_traps_x86_64.c.o' 'build/region-writer-probe/obj/runtime/region_writer_traps_x86_64.c.o' 'build/region-writer-probe/obj/runtime/start_x86_64.S.o' 'build/region-writer-probe/obj/generated/runtime/format_bytes.c.o' 'build/region-writer-probe/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/.objects/1f42e0be5d979d2b/tests/fpu_context/probe.cesta.o: tests/fpu_context/probe.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-msse2' '-mfpmath=sse' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=fpu_probe_main' '-DFPU_PROBE_ID=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'tests/fpu_context/probe.c' -o "$@"

build/fpu-probe-a/obj/tests/fpu_context/probe.cesta.o: build/.objects/1f42e0be5d979d2b/tests/fpu_context/probe.cesta.o
	@mkdir -p 'build/fpu-probe-a/obj/tests/fpu_context'
	ln -f 'build/.objects/1f42e0be5d979d2b/tests/fpu_context/probe.cesta.o' "$@"

build/.objects/3084e5f95c3eefed/runtime/client_runtime.c.o: runtime/client_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-msse2' '-mfpmath=sse' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=fpu_probe_main' '-DFPU_PROBE_ID=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/fpu-probe-a/obj/runtime/client_runtime.c.o: build/.objects/3084e5f95c3eefed/runtime/client_runtime.c.o
	@mkdir -p 'build/fpu-probe-a/obj/runtime'
	ln -f 'build/.objects/3084e5f95c3eefed/runtime/client_runtime.c.o' "$@"

build/.objects/6d483866b2e2b771/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-msse2' '-mfpmath=sse' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=fpu_probe_main' '-DFPU_PROBE_ID=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/fpu-probe-a/obj/runtime/traps_x86_64.c.o: build/.objects/6d483866b2e2b771/runtime/traps_x86_64.c.o
	@mkdir -p 'build/fpu-probe-a/obj/runtime'
	ln -f 'build/.objects/6d483866b2e2b771/runtime/traps_x86_64.c.o' "$@"

build/.objects/562fb9929acb631d/runtime/start_x86_64.S.o: runtime/start_x86_64.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-msse2' '-mfpmath=sse' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=fpu_probe_main' '-DFPU_PROBE_ID=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/fpu-probe-a/obj/runtime/start_x86_64.S.o: build/.objects/562fb9929acb631d/runtime/start_x86_64.S.o
	@mkdir -p 'build/fpu-probe-a/obj/runtime'
	ln -f 'build/.objects/562fb9929acb631d/runtime/start_x86_64.S.o' "$@"

build/.objects/e6d07e2f1d3e4f7c/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-msse2' '-mfpmath=sse' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=fpu_probe_main' '-DFPU_PROBE_ID=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/fpu-probe-a/obj/generated/runtime/format_bytes.c.o: build/.objects/e6d07e2f1d3e4f7c/generated/runtime/format_bytes.c.o
	@mkdir -p 'build/fpu-probe-a/obj/generated/runtime'
	ln -f 'build/.objects/e6d07e2f1d3e4f7c/generated/runtime/format_bytes.c.o' "$@"

build/.objects/9a40f61525e9b40d/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-msse2' '-mfpmath=sse' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=fpu_probe_main' '-DFPU_PROBE_ID=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/fpu-probe-a/obj/generated/runtime/packed_words.c.o: build/.objects/9a40f61525e9b40d/generated/runtime/packed_words.c.o
	@mkdir -p 'build/fpu-probe-a/obj/generated/runtime'
	ln -f 'build/.objects/9a40f61525e9b40d/generated/runtime/packed_words.c.o' "$@"

build/fpu-probe-a/fpu-probe-a: GNUmakefile build/fpu-probe-a/obj/tests/fpu_context/probe.cesta.o build/fpu-probe-a/obj/runtime/client_runtime.c.o build/fpu-probe-a/obj/runtime/traps_x86_64.c.o build/fpu-probe-a/obj/runtime/start_x86_64.S.o build/fpu-probe-a/obj/generated/runtime/format_bytes.c.o build/fpu-probe-a/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/fpu-probe-a/obj/tests/fpu_context/probe.cesta.o' 'build/fpu-probe-a/obj/runtime/client_runtime.c.o' 'build/fpu-probe-a/obj/runtime/traps_x86_64.c.o' 'build/fpu-probe-a/obj/runtime/start_x86_64.S.o' 'build/fpu-probe-a/obj/generated/runtime/format_bytes.c.o' 'build/fpu-probe-a/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/.objects/1f3ee0be5d93b454/tests/fpu_context/probe.cesta.o: tests/fpu_context/probe.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-msse2' '-mfpmath=sse' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=fpu_probe_main' '-DFPU_PROBE_ID=2' -MMD -MP -MF "$@.d" -MQ "$@" -c 'tests/fpu_context/probe.c' -o "$@"

build/fpu-probe-b/obj/tests/fpu_context/probe.cesta.o: build/.objects/1f3ee0be5d93b454/tests/fpu_context/probe.cesta.o
	@mkdir -p 'build/fpu-probe-b/obj/tests/fpu_context'
	ln -f 'build/.objects/1f3ee0be5d93b454/tests/fpu_context/probe.cesta.o' "$@"

build/.objects/308ee5f95c474472/runtime/client_runtime.c.o: runtime/client_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-msse2' '-mfpmath=sse' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=fpu_probe_main' '-DFPU_PROBE_ID=2' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/fpu-probe-b/obj/runtime/client_runtime.c.o: build/.objects/308ee5f95c474472/runtime/client_runtime.c.o
	@mkdir -p 'build/fpu-probe-b/obj/runtime'
	ln -f 'build/.objects/308ee5f95c474472/runtime/client_runtime.c.o' "$@"

build/.objects/6d523866b2eb0bf6/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-msse2' '-mfpmath=sse' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=fpu_probe_main' '-DFPU_PROBE_ID=2' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/fpu-probe-b/obj/runtime/traps_x86_64.c.o: build/.objects/6d523866b2eb0bf6/runtime/traps_x86_64.c.o
	@mkdir -p 'build/fpu-probe-b/obj/runtime'
	ln -f 'build/.objects/6d523866b2eb0bf6/runtime/traps_x86_64.c.o' "$@"

build/.objects/5639b9929ad3b7a2/runtime/start_x86_64.S.o: runtime/start_x86_64.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-msse2' '-mfpmath=sse' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=fpu_probe_main' '-DFPU_PROBE_ID=2' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/fpu-probe-b/obj/runtime/start_x86_64.S.o: build/.objects/5639b9929ad3b7a2/runtime/start_x86_64.S.o
	@mkdir -p 'build/fpu-probe-b/obj/runtime'
	ln -f 'build/.objects/5639b9929ad3b7a2/runtime/start_x86_64.S.o' "$@"

build/.objects/e6d47e2f1d423853/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-msse2' '-mfpmath=sse' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=fpu_probe_main' '-DFPU_PROBE_ID=2' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/fpu-probe-b/obj/generated/runtime/format_bytes.c.o: build/.objects/e6d47e2f1d423853/generated/runtime/format_bytes.c.o
	@mkdir -p 'build/fpu-probe-b/obj/generated/runtime'
	ln -f 'build/.objects/e6d47e2f1d423853/generated/runtime/format_bytes.c.o' "$@"

build/.objects/9a4af61525f20892/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-msse2' '-mfpmath=sse' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=fpu_probe_main' '-DFPU_PROBE_ID=2' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/fpu-probe-b/obj/generated/runtime/packed_words.c.o: build/.objects/9a4af61525f20892/generated/runtime/packed_words.c.o
	@mkdir -p 'build/fpu-probe-b/obj/generated/runtime'
	ln -f 'build/.objects/9a4af61525f20892/generated/runtime/packed_words.c.o' "$@"

build/fpu-probe-b/fpu-probe-b: GNUmakefile build/fpu-probe-b/obj/tests/fpu_context/probe.cesta.o build/fpu-probe-b/obj/runtime/client_runtime.c.o build/fpu-probe-b/obj/runtime/traps_x86_64.c.o build/fpu-probe-b/obj/runtime/start_x86_64.S.o build/fpu-probe-b/obj/generated/runtime/format_bytes.c.o build/fpu-probe-b/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/fpu-probe-b/obj/tests/fpu_context/probe.cesta.o' 'build/fpu-probe-b/obj/runtime/client_runtime.c.o' 'build/fpu-probe-b/obj/runtime/traps_x86_64.c.o' 'build/fpu-probe-b/obj/runtime/start_x86_64.S.o' 'build/fpu-probe-b/obj/generated/runtime/format_bytes.c.o' 'build/fpu-probe-b/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/.objects/a4cbe9e6c889cdb1/tests/history_recovery/probe.cesta.o: tests/history_recovery/probe.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=history_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'tests/history_recovery/probe.c' -o "$@"

build/history-probe/obj/tests/history_recovery/probe.cesta.o: build/.objects/a4cbe9e6c889cdb1/tests/history_recovery/probe.cesta.o
	@mkdir -p 'build/history-probe/obj/tests/history_recovery'
	ln -f 'build/.objects/a4cbe9e6c889cdb1/tests/history_recovery/probe.cesta.o' "$@"

build/.objects/3bb970543f06ba05/src/history_recovery.cesta.o: src/history_recovery.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=history_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/history_recovery.c' -o "$@"

build/history-probe/obj/src/history_recovery.cesta.o: build/.objects/3bb970543f06ba05/src/history_recovery.cesta.o
	@mkdir -p 'build/history-probe/obj/src'
	ln -f 'build/.objects/3bb970543f06ba05/src/history_recovery.cesta.o' "$@"

build/.objects/34042c3a5c92b3e6/src/history_decode.cesta.o: src/history_decode.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=history_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/history_decode.c' -o "$@"

build/history-probe/obj/src/history_decode.cesta.o: build/.objects/34042c3a5c92b3e6/src/history_decode.cesta.o
	@mkdir -p 'build/history-probe/obj/src'
	ln -f 'build/.objects/34042c3a5c92b3e6/src/history_decode.cesta.o' "$@"

build/.objects/6f36173c8d4f799e/runtime/history_native.c.o: runtime/history_native.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=history_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/history_native.c' -o "$@"

build/history-probe/obj/runtime/history_native.c.o: build/.objects/6f36173c8d4f799e/runtime/history_native.c.o
	@mkdir -p 'build/history-probe/obj/runtime'
	ln -f 'build/.objects/6f36173c8d4f799e/runtime/history_native.c.o' "$@"

build/.objects/e623acb4b02b133c/runtime/client_runtime.c.o: runtime/client_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=history_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/history-probe/obj/runtime/client_runtime.c.o: build/.objects/e623acb4b02b133c/runtime/client_runtime.c.o
	@mkdir -p 'build/history-probe/obj/runtime'
	ln -f 'build/.objects/e623acb4b02b133c/runtime/client_runtime.c.o' "$@"

build/.objects/850610202a0b2480/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=history_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/history-probe/obj/runtime/traps_x86_64.c.o: build/.objects/850610202a0b2480/runtime/traps_x86_64.c.o
	@mkdir -p 'build/history-probe/obj/runtime'
	ln -f 'build/.objects/850610202a0b2480/runtime/traps_x86_64.c.o' "$@"

build/.objects/0e0592afba2a432c/runtime/start_x86_64.S.o: runtime/start_x86_64.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=history_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/history-probe/obj/runtime/start_x86_64.S.o: build/.objects/0e0592afba2a432c/runtime/start_x86_64.S.o
	@mkdir -p 'build/history-probe/obj/runtime'
	ln -f 'build/.objects/0e0592afba2a432c/runtime/start_x86_64.S.o' "$@"

build/.objects/fd1ffafa56df85b3/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=history_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/history-probe/obj/generated/runtime/format_bytes.c.o: build/.objects/fd1ffafa56df85b3/generated/runtime/format_bytes.c.o
	@mkdir -p 'build/history-probe/obj/generated/runtime'
	ln -f 'build/.objects/fd1ffafa56df85b3/generated/runtime/format_bytes.c.o' "$@"

build/.objects/7cc80f39caddb61c/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=history_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/history-probe/obj/generated/runtime/packed_words.c.o: build/.objects/7cc80f39caddb61c/generated/runtime/packed_words.c.o
	@mkdir -p 'build/history-probe/obj/generated/runtime'
	ln -f 'build/.objects/7cc80f39caddb61c/generated/runtime/packed_words.c.o' "$@"

build/history-probe/history-probe: GNUmakefile build/history-probe/obj/tests/history_recovery/probe.cesta.o build/history-probe/obj/src/history_recovery.cesta.o build/history-probe/obj/src/history_decode.cesta.o build/history-probe/obj/runtime/history_native.c.o build/history-probe/obj/runtime/client_runtime.c.o build/history-probe/obj/runtime/traps_x86_64.c.o build/history-probe/obj/runtime/start_x86_64.S.o build/history-probe/obj/generated/runtime/format_bytes.c.o build/history-probe/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/history-probe/obj/tests/history_recovery/probe.cesta.o' 'build/history-probe/obj/src/history_recovery.cesta.o' 'build/history-probe/obj/src/history_decode.cesta.o' 'build/history-probe/obj/runtime/history_native.c.o' 'build/history-probe/obj/runtime/client_runtime.c.o' 'build/history-probe/obj/runtime/traps_x86_64.c.o' 'build/history-probe/obj/runtime/start_x86_64.S.o' 'build/history-probe/obj/generated/runtime/format_bytes.c.o' 'build/history-probe/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/.objects/aae765b14158881f/tests/kernel_retire/probe.cesta.o: tests/kernel_retire/probe.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=retire_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'tests/kernel_retire/probe.c' -o "$@"

build/retire-probe/obj/tests/kernel_retire/probe.cesta.o: build/.objects/aae765b14158881f/tests/kernel_retire/probe.cesta.o
	@mkdir -p 'build/retire-probe/obj/tests/kernel_retire'
	ln -f 'build/.objects/aae765b14158881f/tests/kernel_retire/probe.cesta.o' "$@"

build/.objects/fe07ae6525b26c81/runtime/client_runtime.c.o: runtime/client_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=retire_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/retire-probe/obj/runtime/client_runtime.c.o: build/.objects/fe07ae6525b26c81/runtime/client_runtime.c.o
	@mkdir -p 'build/retire-probe/obj/runtime'
	ln -f 'build/.objects/fe07ae6525b26c81/runtime/client_runtime.c.o' "$@"

build/.objects/7d2d780e24066335/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=retire_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/retire-probe/obj/runtime/traps_x86_64.c.o: build/.objects/7d2d780e24066335/runtime/traps_x86_64.c.o
	@mkdir -p 'build/retire-probe/obj/runtime'
	ln -f 'build/.objects/7d2d780e24066335/runtime/traps_x86_64.c.o' "$@"

build/.objects/4a36d0f14981fef1/runtime/start_x86_64.S.o: runtime/start_x86_64.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=retire_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/retire-probe/obj/runtime/start_x86_64.S.o: build/.objects/4a36d0f14981fef1/runtime/start_x86_64.S.o
	@mkdir -p 'build/retire-probe/obj/runtime'
	ln -f 'build/.objects/4a36d0f14981fef1/runtime/start_x86_64.S.o' "$@"

build/.objects/a0173ed5a409d760/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=retire_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/retire-probe/obj/generated/runtime/format_bytes.c.o: build/.objects/a0173ed5a409d760/generated/runtime/format_bytes.c.o
	@mkdir -p 'build/retire-probe/obj/generated/runtime'
	ln -f 'build/.objects/a0173ed5a409d760/generated/runtime/format_bytes.c.o' "$@"

build/.objects/3cc21fc0369cd1e1/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=retire_probe_main' '-DSEXOS_PDX_SENDER_TASK=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/retire-probe/obj/generated/runtime/packed_words.c.o: build/.objects/3cc21fc0369cd1e1/generated/runtime/packed_words.c.o
	@mkdir -p 'build/retire-probe/obj/generated/runtime'
	ln -f 'build/.objects/3cc21fc0369cd1e1/generated/runtime/packed_words.c.o' "$@"

build/retire-probe/retire-probe: GNUmakefile build/retire-probe/obj/tests/kernel_retire/probe.cesta.o build/retire-probe/obj/runtime/client_runtime.c.o build/retire-probe/obj/runtime/traps_x86_64.c.o build/retire-probe/obj/runtime/start_x86_64.S.o build/retire-probe/obj/generated/runtime/format_bytes.c.o build/retire-probe/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/retire-probe/obj/tests/kernel_retire/probe.cesta.o' 'build/retire-probe/obj/runtime/client_runtime.c.o' 'build/retire-probe/obj/runtime/traps_x86_64.c.o' 'build/retire-probe/obj/runtime/start_x86_64.S.o' 'build/retire-probe/obj/generated/runtime/format_bytes.c.o' 'build/retire-probe/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/.objects/84966b113f482211/src/sext.cesta.o: src/sext.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sext_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DSEXOS_SEXT_PARTITIONED_ALLOCATOR=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/sext.c' -o "$@"

build/sext/obj/src/sext.cesta.o: build/.objects/84966b113f482211/src/sext.cesta.o
	@mkdir -p 'build/sext/obj/src'
	ln -f 'build/.objects/84966b113f482211/src/sext.cesta.o' "$@"

build/.objects/e1779b41157bd3a9/src/sext_service.cesta.o: src/sext_service.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sext_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DSEXOS_SEXT_PARTITIONED_ALLOCATOR=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/sext_service.c' -o "$@"

build/sext/obj/src/sext_service.cesta.o: build/.objects/e1779b41157bd3a9/src/sext_service.cesta.o
	@mkdir -p 'build/sext/obj/src'
	ln -f 'build/.objects/e1779b41157bd3a9/src/sext_service.cesta.o' "$@"

build/.objects/69587def0e8eda0f/src/sext_workspace.cesta.o: src/sext_workspace.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sext_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DSEXOS_SEXT_PARTITIONED_ALLOCATOR=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/sext_workspace.c' -o "$@"

build/sext/obj/src/sext_workspace.cesta.o: build/.objects/69587def0e8eda0f/src/sext_workspace.cesta.o
	@mkdir -p 'build/sext/obj/src'
	ln -f 'build/.objects/69587def0e8eda0f/src/sext_workspace.cesta.o' "$@"

build/.objects/c164688ded225ee8/runtime/sext_native.c.o: runtime/sext_native.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sext_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DSEXOS_SEXT_PARTITIONED_ALLOCATOR=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/sext_native.c' -o "$@"

build/sext/obj/runtime/sext_native.c.o: build/.objects/c164688ded225ee8/runtime/sext_native.c.o
	@mkdir -p 'build/sext/obj/runtime'
	ln -f 'build/.objects/c164688ded225ee8/runtime/sext_native.c.o' "$@"

build/.objects/8788a4d079d41430/runtime/sext_worker_probe.c.o: runtime/sext_worker_probe.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sext_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DSEXOS_SEXT_PARTITIONED_ALLOCATOR=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/sext_worker_probe.c' -o "$@"

build/sext/obj/runtime/sext_worker_probe.c.o: build/.objects/8788a4d079d41430/runtime/sext_worker_probe.c.o
	@mkdir -p 'build/sext/obj/runtime'
	ln -f 'build/.objects/8788a4d079d41430/runtime/sext_worker_probe.c.o' "$@"

build/.objects/23a039077581fb1a/runtime/client_runtime.c.o: runtime/client_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sext_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DSEXOS_SEXT_PARTITIONED_ALLOCATOR=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/sext/obj/runtime/client_runtime.c.o: build/.objects/23a039077581fb1a/runtime/client_runtime.c.o
	@mkdir -p 'build/sext/obj/runtime'
	ln -f 'build/.objects/23a039077581fb1a/runtime/client_runtime.c.o' "$@"

build/.objects/9cc8ce7abfe2323e/runtime/server_runtime.c.o: runtime/server_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sext_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DSEXOS_SEXT_PARTITIONED_ALLOCATOR=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_runtime.c' -o "$@"

build/sext/obj/runtime/server_runtime.c.o: build/.objects/9cc8ce7abfe2323e/runtime/server_runtime.c.o
	@mkdir -p 'build/sext/obj/runtime'
	ln -f 'build/.objects/9cc8ce7abfe2323e/runtime/server_runtime.c.o' "$@"

build/.objects/57f79c97e94bd526/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sext_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DSEXOS_SEXT_PARTITIONED_ALLOCATOR=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/sext/obj/runtime/traps_x86_64.c.o: build/.objects/57f79c97e94bd526/runtime/traps_x86_64.c.o
	@mkdir -p 'build/sext/obj/runtime'
	ln -f 'build/.objects/57f79c97e94bd526/runtime/traps_x86_64.c.o' "$@"

build/.objects/34ffe0f95f151cda/runtime/server_traps_x86_64.c.o: runtime/server_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sext_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DSEXOS_SEXT_PARTITIONED_ALLOCATOR=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_traps_x86_64.c' -o "$@"

build/sext/obj/runtime/server_traps_x86_64.c.o: build/.objects/34ffe0f95f151cda/runtime/server_traps_x86_64.c.o
	@mkdir -p 'build/sext/obj/runtime'
	ln -f 'build/.objects/34ffe0f95f151cda/runtime/server_traps_x86_64.c.o' "$@"

build/.objects/23742f284ace3f4a/runtime/start_x86_64.S.o: runtime/start_x86_64.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sext_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DSEXOS_SEXT_PARTITIONED_ALLOCATOR=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/sext/obj/runtime/start_x86_64.S.o: build/.objects/23742f284ace3f4a/runtime/start_x86_64.S.o
	@mkdir -p 'build/sext/obj/runtime'
	ln -f 'build/.objects/23742f284ace3f4a/runtime/start_x86_64.S.o' "$@"

build/.objects/5a4dcd12385aefed/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sext_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DSEXOS_SEXT_PARTITIONED_ALLOCATOR=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/sext/obj/generated/runtime/format_bytes.c.o: build/.objects/5a4dcd12385aefed/generated/runtime/format_bytes.c.o
	@mkdir -p 'build/sext/obj/generated/runtime'
	ln -f 'build/.objects/5a4dcd12385aefed/generated/runtime/format_bytes.c.o' "$@"

build/.objects/5874f19e28db9b7a/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sext_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DSEXOS_SEXT_PARTITIONED_ALLOCATOR=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/sext/obj/generated/runtime/packed_words.c.o: build/.objects/5874f19e28db9b7a/generated/runtime/packed_words.c.o
	@mkdir -p 'build/sext/obj/generated/runtime'
	ln -f 'build/.objects/5874f19e28db9b7a/generated/runtime/packed_words.c.o' "$@"

build/sext/sext: GNUmakefile build/sext/obj/src/sext.cesta.o build/sext/obj/src/sext_service.cesta.o build/sext/obj/src/sext_workspace.cesta.o build/sext/obj/runtime/sext_native.c.o build/sext/obj/runtime/sext_worker_probe.c.o build/sext/obj/runtime/client_runtime.c.o build/sext/obj/runtime/server_runtime.c.o build/sext/obj/runtime/traps_x86_64.c.o build/sext/obj/runtime/server_traps_x86_64.c.o build/sext/obj/runtime/start_x86_64.S.o build/sext/obj/generated/runtime/format_bytes.c.o build/sext/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/sext/obj/src/sext.cesta.o' 'build/sext/obj/src/sext_service.cesta.o' 'build/sext/obj/src/sext_workspace.cesta.o' 'build/sext/obj/runtime/sext_native.c.o' 'build/sext/obj/runtime/sext_worker_probe.c.o' 'build/sext/obj/runtime/client_runtime.c.o' 'build/sext/obj/runtime/server_runtime.c.o' 'build/sext/obj/runtime/traps_x86_64.c.o' 'build/sext/obj/runtime/server_traps_x86_64.c.o' 'build/sext/obj/runtime/start_x86_64.S.o' 'build/sext/obj/generated/runtime/format_bytes.c.o' 'build/sext/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/.objects/50c6330903177953/src/sext.cesta.o: src/sext.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sext_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DSEXOS_SEXT_PARTITIONED_ALLOCATOR=1' '-DSEXOS_WORKER_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/sext.c' -o "$@"

build/sext-worker-probe/obj/src/sext.cesta.o: build/.objects/50c6330903177953/src/sext.cesta.o
	@mkdir -p 'build/sext-worker-probe/obj/src'
	ln -f 'build/.objects/50c6330903177953/src/sext.cesta.o' "$@"

build/.objects/62c86b32fc45c89b/src/sext_service.cesta.o: src/sext_service.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sext_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DSEXOS_SEXT_PARTITIONED_ALLOCATOR=1' '-DSEXOS_WORKER_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/sext_service.c' -o "$@"

build/sext-worker-probe/obj/src/sext_service.cesta.o: build/.objects/62c86b32fc45c89b/src/sext_service.cesta.o
	@mkdir -p 'build/sext-worker-probe/obj/src'
	ln -f 'build/.objects/62c86b32fc45c89b/src/sext_service.cesta.o' "$@"

build/.objects/3721b9a6cf3260ad/src/sext_workspace.cesta.o: src/sext_workspace.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sext_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DSEXOS_SEXT_PARTITIONED_ALLOCATOR=1' '-DSEXOS_WORKER_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/sext_workspace.c' -o "$@"

build/sext-worker-probe/obj/src/sext_workspace.cesta.o: build/.objects/3721b9a6cf3260ad/src/sext_workspace.cesta.o
	@mkdir -p 'build/sext-worker-probe/obj/src'
	ln -f 'build/.objects/3721b9a6cf3260ad/src/sext_workspace.cesta.o' "$@"

build/.objects/0940b94d2f086948/runtime/sext_native.c.o: runtime/sext_native.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sext_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DSEXOS_SEXT_PARTITIONED_ALLOCATOR=1' '-DSEXOS_WORKER_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/sext_native.c' -o "$@"

build/sext-worker-probe/obj/runtime/sext_native.c.o: build/.objects/0940b94d2f086948/runtime/sext_native.c.o
	@mkdir -p 'build/sext-worker-probe/obj/runtime'
	ln -f 'build/.objects/0940b94d2f086948/runtime/sext_native.c.o' "$@"

build/.objects/43432250fb269bc0/runtime/sext_worker_probe.c.o: runtime/sext_worker_probe.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sext_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DSEXOS_SEXT_PARTITIONED_ALLOCATOR=1' '-DSEXOS_WORKER_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/sext_worker_probe.c' -o "$@"

build/sext-worker-probe/obj/runtime/sext_worker_probe.c.o: build/.objects/43432250fb269bc0/runtime/sext_worker_probe.c.o
	@mkdir -p 'build/sext-worker-probe/obj/runtime'
	ln -f 'build/.objects/43432250fb269bc0/runtime/sext_worker_probe.c.o' "$@"

build/.objects/f37259d15ccd5cbe/runtime/client_runtime.c.o: runtime/client_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sext_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DSEXOS_SEXT_PARTITIONED_ALLOCATOR=1' '-DSEXOS_WORKER_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/sext-worker-probe/obj/runtime/client_runtime.c.o: build/.objects/f37259d15ccd5cbe/runtime/client_runtime.c.o
	@mkdir -p 'build/sext-worker-probe/obj/runtime'
	ln -f 'build/.objects/f37259d15ccd5cbe/runtime/client_runtime.c.o' "$@"

build/.objects/f353014a79ea2fa2/runtime/server_runtime.c.o: runtime/server_runtime.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sext_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DSEXOS_SEXT_PARTITIONED_ALLOCATOR=1' '-DSEXOS_WORKER_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_runtime.c' -o "$@"

build/sext-worker-probe/obj/runtime/server_runtime.c.o: build/.objects/f353014a79ea2fa2/runtime/server_runtime.c.o
	@mkdir -p 'build/sext-worker-probe/obj/runtime'
	ln -f 'build/.objects/f353014a79ea2fa2/runtime/server_runtime.c.o' "$@"

build/.objects/82c773c5ff98e25a/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sext_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DSEXOS_SEXT_PARTITIONED_ALLOCATOR=1' '-DSEXOS_WORKER_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/sext-worker-probe/obj/runtime/traps_x86_64.c.o: build/.objects/82c773c5ff98e25a/runtime/traps_x86_64.c.o
	@mkdir -p 'build/sext-worker-probe/obj/runtime'
	ln -f 'build/.objects/82c773c5ff98e25a/runtime/traps_x86_64.c.o' "$@"

build/.objects/062ea218e25a86fe/runtime/server_traps_x86_64.c.o: runtime/server_traps_x86_64.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sext_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DSEXOS_SEXT_PARTITIONED_ALLOCATOR=1' '-DSEXOS_WORKER_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_traps_x86_64.c' -o "$@"

build/sext-worker-probe/obj/runtime/server_traps_x86_64.c.o: build/.objects/062ea218e25a86fe/runtime/server_traps_x86_64.c.o
	@mkdir -p 'build/sext-worker-probe/obj/runtime'
	ln -f 'build/.objects/062ea218e25a86fe/runtime/server_traps_x86_64.c.o' "$@"

build/.objects/620dee108e34fe6e/runtime/start_x86_64.S.o: runtime/start_x86_64.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sext_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DSEXOS_SEXT_PARTITIONED_ALLOCATOR=1' '-DSEXOS_WORKER_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/sext-worker-probe/obj/runtime/start_x86_64.S.o: build/.objects/620dee108e34fe6e/runtime/start_x86_64.S.o
	@mkdir -p 'build/sext-worker-probe/obj/runtime'
	ln -f 'build/.objects/620dee108e34fe6e/runtime/start_x86_64.S.o' "$@"

build/.objects/60b53b011ddbd56f/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sext_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DSEXOS_SEXT_PARTITIONED_ALLOCATOR=1' '-DSEXOS_WORKER_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/sext-worker-probe/obj/generated/runtime/format_bytes.c.o: build/.objects/60b53b011ddbd56f/generated/runtime/format_bytes.c.o
	@mkdir -p 'build/sext-worker-probe/obj/generated/runtime'
	ln -f 'build/.objects/60b53b011ddbd56f/generated/runtime/format_bytes.c.o' "$@"

build/.objects/53bde9131eae7c5e/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sext_main' '-DSEXOS_PDX_SENDER_TASK=1' '-DSEXOS_SEXT_PARTITIONED_ALLOCATOR=1' '-DSEXOS_WORKER_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/sext-worker-probe/obj/generated/runtime/packed_words.c.o: build/.objects/53bde9131eae7c5e/generated/runtime/packed_words.c.o
	@mkdir -p 'build/sext-worker-probe/obj/generated/runtime'
	ln -f 'build/.objects/53bde9131eae7c5e/generated/runtime/packed_words.c.o' "$@"

build/sext-worker-probe/sext-worker-probe: GNUmakefile build/sext-worker-probe/obj/src/sext.cesta.o build/sext-worker-probe/obj/src/sext_service.cesta.o build/sext-worker-probe/obj/src/sext_workspace.cesta.o build/sext-worker-probe/obj/runtime/sext_native.c.o build/sext-worker-probe/obj/runtime/sext_worker_probe.c.o build/sext-worker-probe/obj/runtime/client_runtime.c.o build/sext-worker-probe/obj/runtime/server_runtime.c.o build/sext-worker-probe/obj/runtime/traps_x86_64.c.o build/sext-worker-probe/obj/runtime/server_traps_x86_64.c.o build/sext-worker-probe/obj/runtime/start_x86_64.S.o build/sext-worker-probe/obj/generated/runtime/format_bytes.c.o build/sext-worker-probe/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/sext-worker-probe/obj/src/sext.cesta.o' 'build/sext-worker-probe/obj/src/sext_service.cesta.o' 'build/sext-worker-probe/obj/src/sext_workspace.cesta.o' 'build/sext-worker-probe/obj/runtime/sext_native.c.o' 'build/sext-worker-probe/obj/runtime/sext_worker_probe.c.o' 'build/sext-worker-probe/obj/runtime/client_runtime.c.o' 'build/sext-worker-probe/obj/runtime/server_runtime.c.o' 'build/sext-worker-probe/obj/runtime/traps_x86_64.c.o' 'build/sext-worker-probe/obj/runtime/server_traps_x86_64.c.o' 'build/sext-worker-probe/obj/runtime/start_x86_64.S.o' 'build/sext-worker-probe/obj/generated/runtime/format_bytes.c.o' 'build/sext-worker-probe/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/.objects/b5a2a01165d1af0d/src/frame_lease.cesta.o: src/frame_lease.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/frame_lease.c' -o "$@"

build/sexos-kernel-current/obj/src/frame_lease.cesta.o: build/.objects/b5a2a01165d1af0d/src/frame_lease.cesta.o
	@mkdir -p 'build/sexos-kernel-current/obj/src'
	ln -f 'build/.objects/b5a2a01165d1af0d/src/frame_lease.cesta.o' "$@"

build/.objects/323075e01144cc38/src/kernel_memory.cesta.o: src/kernel_memory.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_memory.c' -o "$@"

build/sexos-kernel-current/obj/src/kernel_memory.cesta.o: build/.objects/323075e01144cc38/src/kernel_memory.cesta.o
	@mkdir -p 'build/sexos-kernel-current/obj/src'
	ln -f 'build/.objects/323075e01144cc38/src/kernel_memory.cesta.o' "$@"

build/.objects/e29678b3862941ef/src/program_memory.cesta.o: src/program_memory.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_memory.c' -o "$@"

build/sexos-kernel-current/obj/src/program_memory.cesta.o: build/.objects/e29678b3862941ef/src/program_memory.cesta.o
	@mkdir -p 'build/sexos-kernel-current/obj/src'
	ln -f 'build/.objects/e29678b3862941ef/src/program_memory.cesta.o' "$@"

build/.objects/968d70ab50d14dcd/src/program_workers.cesta.o: src/program_workers.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_workers.c' -o "$@"

build/sexos-kernel-current/obj/src/program_workers.cesta.o: build/.objects/968d70ab50d14dcd/src/program_workers.cesta.o
	@mkdir -p 'build/sexos-kernel-current/obj/src'
	ln -f 'build/.objects/968d70ab50d14dcd/src/program_workers.cesta.o' "$@"

build/.objects/3554b602c7bab30c/src/kernel_workers.cesta.o: src/kernel_workers.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_workers.c' -o "$@"

build/sexos-kernel-current/obj/src/kernel_workers.cesta.o: build/.objects/3554b602c7bab30c/src/kernel_workers.cesta.o
	@mkdir -p 'build/sexos-kernel-current/obj/src'
	ln -f 'build/.objects/3554b602c7bab30c/src/kernel_workers.cesta.o' "$@"

build/.objects/3555ba7b062329e0/src/kernel_platform.cesta.o: src/kernel_platform.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_platform.c' -o "$@"

build/sexos-kernel-current/obj/src/kernel_platform.cesta.o: build/.objects/3555ba7b062329e0/src/kernel_platform.cesta.o
	@mkdir -p 'build/sexos-kernel-current/obj/src'
	ln -f 'build/.objects/3555ba7b062329e0/src/kernel_platform.cesta.o' "$@"

build/.objects/98b4c43de8088ca6/src/kernel_domains.cesta.o: src/kernel_domains.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_domains.c' -o "$@"

build/sexos-kernel-current/obj/src/kernel_domains.cesta.o: build/.objects/98b4c43de8088ca6/src/kernel_domains.cesta.o
	@mkdir -p 'build/sexos-kernel-current/obj/src'
	ln -f 'build/.objects/98b4c43de8088ca6/src/kernel_domains.cesta.o' "$@"

build/.objects/5c706a61bb81a41b/src/program_image.cesta.o: src/program_image.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_image.c' -o "$@"

build/sexos-kernel-current/obj/src/program_image.cesta.o: build/.objects/5c706a61bb81a41b/src/program_image.cesta.o
	@mkdir -p 'build/sexos-kernel-current/obj/src'
	ln -f 'build/.objects/5c706a61bb81a41b/src/program_image.cesta.o' "$@"

build/.objects/8430152c97e435c7/src/program_image_bytes.cesta.o: src/program_image_bytes.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_image_bytes.c' -o "$@"

build/sexos-kernel-current/obj/src/program_image_bytes.cesta.o: build/.objects/8430152c97e435c7/src/program_image_bytes.cesta.o
	@mkdir -p 'build/sexos-kernel-current/obj/src'
	ln -f 'build/.objects/8430152c97e435c7/src/program_image_bytes.cesta.o' "$@"

build/.objects/3c0480fc5b8db57a/runtime/kernel/image_failure.c.o: runtime/kernel/image_failure.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/kernel/image_failure.c' -o "$@"

build/sexos-kernel-current/obj/runtime/kernel/image_failure.c.o: build/.objects/3c0480fc5b8db57a/runtime/kernel/image_failure.c.o
	@mkdir -p 'build/sexos-kernel-current/obj/runtime/kernel'
	ln -f 'build/.objects/3c0480fc5b8db57a/runtime/kernel/image_failure.c.o' "$@"

build/.objects/5f97dc379026952f/src/kernel_entry.cesta.o: src/kernel_entry.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_entry.c' -o "$@"

build/sexos-kernel-current/obj/src/kernel_entry.cesta.o: build/.objects/5f97dc379026952f/src/kernel_entry.cesta.o
	@mkdir -p 'build/sexos-kernel-current/obj/src'
	ln -f 'build/.objects/5f97dc379026952f/src/kernel_entry.cesta.o' "$@"

build/.objects/d06874f74a39a7f1/src/kernel_syscalls.cesta.o: src/kernel_syscalls.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_syscalls.c' -o "$@"

build/sexos-kernel-current/obj/src/kernel_syscalls.cesta.o: build/.objects/d06874f74a39a7f1/src/kernel_syscalls.cesta.o
	@mkdir -p 'build/sexos-kernel-current/obj/src'
	ln -f 'build/.objects/d06874f74a39a7f1/src/kernel_syscalls.cesta.o' "$@"

build/.objects/eb934cd0a0c48c78/src/kernel_rpc.cesta.o: src/kernel_rpc.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_rpc.c' -o "$@"

build/sexos-kernel-current/obj/src/kernel_rpc.cesta.o: build/.objects/eb934cd0a0c48c78/src/kernel_rpc.cesta.o
	@mkdir -p 'build/sexos-kernel-current/obj/src'
	ln -f 'build/.objects/eb934cd0a0c48c78/src/kernel_rpc.cesta.o' "$@"

build/.objects/a05184d10afe5934/src/memory_regions.cesta.o: src/memory_regions.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/memory_regions.c' -o "$@"

build/sexos-kernel-current/obj/src/memory_regions.cesta.o: build/.objects/a05184d10afe5934/src/memory_regions.cesta.o
	@mkdir -p 'build/sexos-kernel-current/obj/src'
	ln -f 'build/.objects/a05184d10afe5934/src/memory_regions.cesta.o' "$@"

build/.objects/07215957f1ea61b3/src/region_writer.cesta.o: src/region_writer.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/region_writer.c' -o "$@"

build/sexos-kernel-current/obj/src/region_writer.cesta.o: build/.objects/07215957f1ea61b3/src/region_writer.cesta.o
	@mkdir -p 'build/sexos-kernel-current/obj/src'
	ln -f 'build/.objects/07215957f1ea61b3/src/region_writer.cesta.o' "$@"

build/.objects/2ed075df6a116f4b/src/device_reset.cesta.o: src/device_reset.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/device_reset.c' -o "$@"

build/sexos-kernel-current/obj/src/device_reset.cesta.o: build/.objects/2ed075df6a116f4b/src/device_reset.cesta.o
	@mkdir -p 'build/sexos-kernel-current/obj/src'
	ln -f 'build/.objects/2ed075df6a116f4b/src/device_reset.cesta.o' "$@"

build/.objects/90892a5849d262cc/src/epoch_journal.cesta.o: src/epoch_journal.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/epoch_journal.c' -o "$@"

build/sexos-kernel-current/obj/src/epoch_journal.cesta.o: build/.objects/90892a5849d262cc/src/epoch_journal.cesta.o
	@mkdir -p 'build/sexos-kernel-current/obj/src'
	ln -f 'build/.objects/90892a5849d262cc/src/epoch_journal.cesta.o' "$@"

build/.objects/dfbc03653ac1b4bf/src/kernel_epoch_log.cesta.o: src/kernel_epoch_log.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_epoch_log.c' -o "$@"

build/sexos-kernel-current/obj/src/kernel_epoch_log.cesta.o: build/.objects/dfbc03653ac1b4bf/src/kernel_epoch_log.cesta.o
	@mkdir -p 'build/sexos-kernel-current/obj/src'
	ln -f 'build/.objects/dfbc03653ac1b4bf/src/kernel_epoch_log.cesta.o' "$@"

build/.objects/c64f3c4a33313f55/src/kernel_boot_memory.cesta.o: src/kernel_boot_memory.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_boot_memory.c' -o "$@"

build/sexos-kernel-current/obj/src/kernel_boot_memory.cesta.o: build/.objects/c64f3c4a33313f55/src/kernel_boot_memory.cesta.o
	@mkdir -p 'build/sexos-kernel-current/obj/src'
	ln -f 'build/.objects/c64f3c4a33313f55/src/kernel_boot_memory.cesta.o' "$@"

build/.objects/02ced040bc276485/src/kernel_region_writer.cesta.o: src/kernel_region_writer.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_region_writer.c' -o "$@"

build/sexos-kernel-current/obj/src/kernel_region_writer.cesta.o: build/.objects/02ced040bc276485/src/kernel_region_writer.cesta.o
	@mkdir -p 'build/sexos-kernel-current/obj/src'
	ln -f 'build/.objects/02ced040bc276485/src/kernel_region_writer.cesta.o' "$@"

build/.objects/b3127bab89e25ca1/src/kernel_device_interrupt.cesta.o: src/kernel_device_interrupt.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_device_interrupt.c' -o "$@"

build/sexos-kernel-current/obj/src/kernel_device_interrupt.cesta.o: build/.objects/b3127bab89e25ca1/src/kernel_device_interrupt.cesta.o
	@mkdir -p 'build/sexos-kernel-current/obj/src'
	ln -f 'build/.objects/b3127bab89e25ca1/src/kernel_device_interrupt.cesta.o' "$@"

build/.objects/516323a7112e9af5/src/kernel_device_reset.cesta.o: src/kernel_device_reset.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_device_reset.c' -o "$@"

build/sexos-kernel-current/obj/src/kernel_device_reset.cesta.o: build/.objects/516323a7112e9af5/src/kernel_device_reset.cesta.o
	@mkdir -p 'build/sexos-kernel-current/obj/src'
	ln -f 'build/.objects/516323a7112e9af5/src/kernel_device_reset.cesta.o' "$@"

build/.objects/7fd566ff9708915e/src/app_private.cesta.o: src/app_private.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/app_private.c' -o "$@"

build/sexos-kernel-current/obj/src/app_private.cesta.o: build/.objects/7fd566ff9708915e/src/app_private.cesta.o
	@mkdir -p 'build/sexos-kernel-current/obj/src'
	ln -f 'build/.objects/7fd566ff9708915e/src/app_private.cesta.o' "$@"

build/.objects/e1164d1ee98146c3/src/kernel_apps.cesta.o: src/kernel_apps.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_apps.c' -o "$@"

build/sexos-kernel-current/obj/src/kernel_apps.cesta.o: build/.objects/e1164d1ee98146c3/src/kernel_apps.cesta.o
	@mkdir -p 'build/sexos-kernel-current/obj/src'
	ln -f 'build/.objects/e1164d1ee98146c3/src/kernel_apps.cesta.o' "$@"

build/.objects/f2ddc432636e28c0/src/program_records.cesta.o: src/program_records.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_records.c' -o "$@"

build/sexos-kernel-current/obj/src/program_records.cesta.o: build/.objects/f2ddc432636e28c0/src/program_records.cesta.o
	@mkdir -p 'build/sexos-kernel-current/obj/src'
	ln -f 'build/.objects/f2ddc432636e28c0/src/program_records.cesta.o' "$@"

build/.objects/02106af68353e893/src/program_record_memory.cesta.o: src/program_record_memory.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_record_memory.c' -o "$@"

build/sexos-kernel-current/obj/src/program_record_memory.cesta.o: build/.objects/02106af68353e893/src/program_record_memory.cesta.o
	@mkdir -p 'build/sexos-kernel-current/obj/src'
	ln -f 'build/.objects/02106af68353e893/src/program_record_memory.cesta.o' "$@"

build/.objects/60e5ae78a6348834/src/program_record_tasks.cesta.o: src/program_record_tasks.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_record_tasks.c' -o "$@"

build/sexos-kernel-current/obj/src/program_record_tasks.cesta.o: build/.objects/60e5ae78a6348834/src/program_record_tasks.cesta.o
	@mkdir -p 'build/sexos-kernel-current/obj/src'
	ln -f 'build/.objects/60e5ae78a6348834/src/program_record_tasks.cesta.o' "$@"

build/.objects/9a0ad0f2fc854b89/src/kernel_records.cesta.o: src/kernel_records.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_records.c' -o "$@"

build/sexos-kernel-current/obj/src/kernel_records.cesta.o: build/.objects/9a0ad0f2fc854b89/src/kernel_records.cesta.o
	@mkdir -p 'build/sexos-kernel-current/obj/src'
	ln -f 'build/.objects/9a0ad0f2fc854b89/src/kernel_records.cesta.o' "$@"

build/.objects/a2ae5e0658e6a2f1/src/kernel_current_boot.cesta.o: src/kernel_current_boot.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_current_boot.c' -o "$@"

build/sexos-kernel-current/obj/src/kernel_current_boot.cesta.o: build/.objects/a2ae5e0658e6a2f1/src/kernel_current_boot.cesta.o
	@mkdir -p 'build/sexos-kernel-current/obj/src'
	ln -f 'build/.objects/a2ae5e0658e6a2f1/src/kernel_current_boot.cesta.o' "$@"

build/.objects/e86b4df0404e87e7/src/kernel_paging.cesta.o: src/kernel_paging.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_paging.c' -o "$@"

build/sexos-kernel-current/obj/src/kernel_paging.cesta.o: build/.objects/e86b4df0404e87e7/src/kernel_paging.cesta.o
	@mkdir -p 'build/sexos-kernel-current/obj/src'
	ln -f 'build/.objects/e86b4df0404e87e7/src/kernel_paging.cesta.o' "$@"

build/.objects/4b48ee7099daa748/src/kernel_task_wait.cesta.o: src/kernel_task_wait.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_task_wait.c' -o "$@"

build/sexos-kernel-current/obj/src/kernel_task_wait.cesta.o: build/.objects/4b48ee7099daa748/src/kernel_task_wait.cesta.o
	@mkdir -p 'build/sexos-kernel-current/obj/src'
	ln -f 'build/.objects/4b48ee7099daa748/src/kernel_task_wait.cesta.o' "$@"

build/.objects/a48364eb87797250/src/pager_state.cesta.o: src/pager_state.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/pager_state.c' -o "$@"

build/sexos-kernel-current/obj/src/pager_state.cesta.o: build/.objects/a48364eb87797250/src/pager_state.cesta.o
	@mkdir -p 'build/sexos-kernel-current/obj/src'
	ln -f 'build/.objects/a48364eb87797250/src/pager_state.cesta.o' "$@"

build/.objects/c288925a3d40a21d/src/source_policy.cesta.o: src/source_policy.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/source_policy.c' -o "$@"

build/sexos-kernel-current/obj/src/source_policy.cesta.o: build/.objects/c288925a3d40a21d/src/source_policy.cesta.o
	@mkdir -p 'build/sexos-kernel-current/obj/src'
	ln -f 'build/.objects/c288925a3d40a21d/src/source_policy.cesta.o' "$@"

build/.objects/adad61e9dae54da9/src/kernel_sources.cesta.o: src/kernel_sources.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_sources.c' -o "$@"

build/sexos-kernel-current/obj/src/kernel_sources.cesta.o: build/.objects/adad61e9dae54da9/src/kernel_sources.cesta.o
	@mkdir -p 'build/sexos-kernel-current/obj/src'
	ln -f 'build/.objects/adad61e9dae54da9/src/kernel_sources.cesta.o' "$@"

build/.objects/31642976f28c3308/src/network_config.cesta.o: src/network_config.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_config.c' -o "$@"

build/sexos-kernel-current/obj/src/network_config.cesta.o: build/.objects/31642976f28c3308/src/network_config.cesta.o
	@mkdir -p 'build/sexos-kernel-current/obj/src'
	ln -f 'build/.objects/31642976f28c3308/src/network_config.cesta.o' "$@"

build/.objects/126bf10281c7e18e/src/pci_policy.cesta.o: src/pci_policy.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/pci_policy.c' -o "$@"

build/sexos-kernel-current/obj/src/pci_policy.cesta.o: build/.objects/126bf10281c7e18e/src/pci_policy.cesta.o
	@mkdir -p 'build/sexos-kernel-current/obj/src'
	ln -f 'build/.objects/126bf10281c7e18e/src/pci_policy.cesta.o' "$@"

build/.objects/416345b45e45d93b/src/kernel_pci.cesta.o: src/kernel_pci.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_pci.c' -o "$@"

build/sexos-kernel-current/obj/src/kernel_pci.cesta.o: build/.objects/416345b45e45d93b/src/kernel_pci.cesta.o
	@mkdir -p 'build/sexos-kernel-current/obj/src'
	ln -f 'build/.objects/416345b45e45d93b/src/kernel_pci.cesta.o' "$@"

build/.objects/d8c9bf243033b073/src/history_recovery.cesta.o: src/history_recovery.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/history_recovery.c' -o "$@"

build/sexos-kernel-current/obj/src/history_recovery.cesta.o: build/.objects/d8c9bf243033b073/src/history_recovery.cesta.o
	@mkdir -p 'build/sexos-kernel-current/obj/src'
	ln -f 'build/.objects/d8c9bf243033b073/src/history_recovery.cesta.o' "$@"

build/.objects/d5add77b51d4fc53/src/kernel_history.cesta.o: src/kernel_history.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_history.c' -o "$@"

build/sexos-kernel-current/obj/src/kernel_history.cesta.o: build/.objects/d5add77b51d4fc53/src/kernel_history.cesta.o
	@mkdir -p 'build/sexos-kernel-current/obj/src'
	ln -f 'build/.objects/d5add77b51d4fc53/src/kernel_history.cesta.o' "$@"

build/.objects/1cb94bddb465d4d8/src/smp_park.cesta.o: src/smp_park.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/smp_park.c' -o "$@"

build/sexos-kernel-current/obj/src/smp_park.cesta.o: build/.objects/1cb94bddb465d4d8/src/smp_park.cesta.o
	@mkdir -p 'build/sexos-kernel-current/obj/src'
	ln -f 'build/.objects/1cb94bddb465d4d8/src/smp_park.cesta.o' "$@"

build/.objects/a49f1bc859b99f2f/src/kernel_smp.cesta.o: src/kernel_smp.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_smp.c' -o "$@"

build/sexos-kernel-current/obj/src/kernel_smp.cesta.o: build/.objects/a49f1bc859b99f2f/src/kernel_smp.cesta.o
	@mkdir -p 'build/sexos-kernel-current/obj/src'
	ln -f 'build/.objects/a49f1bc859b99f2f/src/kernel_smp.cesta.o' "$@"

build/.objects/d941a5248c413233/src/nvme_msix.cesta.o: src/nvme_msix.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/nvme_msix.c' -o "$@"

build/sexos-kernel-current/obj/src/nvme_msix.cesta.o: build/.objects/d941a5248c413233/src/nvme_msix.cesta.o
	@mkdir -p 'build/sexos-kernel-current/obj/src'
	ln -f 'build/.objects/d941a5248c413233/src/nvme_msix.cesta.o' "$@"

build/.objects/1c26d17757c16526/runtime/kernel/kernel_log.c.o: runtime/kernel/kernel_log.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/kernel/kernel_log.c' -o "$@"

build/sexos-kernel-current/obj/runtime/kernel/kernel_log.c.o: build/.objects/1c26d17757c16526/runtime/kernel/kernel_log.c.o
	@mkdir -p 'build/sexos-kernel-current/obj/runtime/kernel'
	ln -f 'build/.objects/1c26d17757c16526/runtime/kernel/kernel_log.c.o' "$@"

build/.objects/27390210d6aa74ee/runtime/kernel/entry_x86_64.S.o: runtime/kernel/entry_x86_64.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/kernel/entry_x86_64.S' -o "$@"

build/sexos-kernel-current/obj/runtime/kernel/entry_x86_64.S.o: build/.objects/27390210d6aa74ee/runtime/kernel/entry_x86_64.S.o
	@mkdir -p 'build/sexos-kernel-current/obj/runtime/kernel'
	ln -f 'build/.objects/27390210d6aa74ee/runtime/kernel/entry_x86_64.S.o' "$@"

build/sexos-kernel-current/sexos-kernel-current: GNUmakefile build/sexos-kernel-current/obj/src/frame_lease.cesta.o build/sexos-kernel-current/obj/src/kernel_memory.cesta.o build/sexos-kernel-current/obj/src/program_memory.cesta.o build/sexos-kernel-current/obj/src/program_workers.cesta.o build/sexos-kernel-current/obj/src/kernel_workers.cesta.o build/sexos-kernel-current/obj/src/kernel_platform.cesta.o build/sexos-kernel-current/obj/src/kernel_domains.cesta.o build/sexos-kernel-current/obj/src/program_image.cesta.o build/sexos-kernel-current/obj/src/program_image_bytes.cesta.o build/sexos-kernel-current/obj/runtime/kernel/image_failure.c.o build/sexos-kernel-current/obj/src/kernel_entry.cesta.o build/sexos-kernel-current/obj/src/kernel_syscalls.cesta.o build/sexos-kernel-current/obj/src/kernel_rpc.cesta.o build/sexos-kernel-current/obj/src/memory_regions.cesta.o build/sexos-kernel-current/obj/src/region_writer.cesta.o build/sexos-kernel-current/obj/src/device_reset.cesta.o build/sexos-kernel-current/obj/src/epoch_journal.cesta.o build/sexos-kernel-current/obj/src/kernel_epoch_log.cesta.o build/sexos-kernel-current/obj/src/kernel_boot_memory.cesta.o build/sexos-kernel-current/obj/src/kernel_region_writer.cesta.o build/sexos-kernel-current/obj/src/kernel_device_interrupt.cesta.o build/sexos-kernel-current/obj/src/kernel_device_reset.cesta.o build/sexos-kernel-current/obj/src/app_private.cesta.o build/sexos-kernel-current/obj/src/kernel_apps.cesta.o build/sexos-kernel-current/obj/src/program_records.cesta.o build/sexos-kernel-current/obj/src/program_record_memory.cesta.o build/sexos-kernel-current/obj/src/program_record_tasks.cesta.o build/sexos-kernel-current/obj/src/kernel_records.cesta.o build/sexos-kernel-current/obj/src/kernel_current_boot.cesta.o build/sexos-kernel-current/obj/src/kernel_paging.cesta.o build/sexos-kernel-current/obj/src/kernel_task_wait.cesta.o build/sexos-kernel-current/obj/src/pager_state.cesta.o build/sexos-kernel-current/obj/src/source_policy.cesta.o build/sexos-kernel-current/obj/src/kernel_sources.cesta.o build/sexos-kernel-current/obj/src/network_config.cesta.o build/sexos-kernel-current/obj/src/pci_policy.cesta.o build/sexos-kernel-current/obj/src/kernel_pci.cesta.o build/sexos-kernel-current/obj/src/history_recovery.cesta.o build/sexos-kernel-current/obj/src/kernel_history.cesta.o build/sexos-kernel-current/obj/src/smp_park.cesta.o build/sexos-kernel-current/obj/src/kernel_smp.cesta.o build/sexos-kernel-current/obj/src/nvme_msix.cesta.o build/sexos-kernel-current/obj/runtime/kernel/kernel_log.c.o build/sexos-kernel-current/obj/runtime/kernel/entry_x86_64.S.o runtime/kernel/kernel.ld
	@mkdir -p "$(@D)"
	$(CC) 'build/sexos-kernel-current/obj/src/frame_lease.cesta.o' 'build/sexos-kernel-current/obj/src/kernel_memory.cesta.o' 'build/sexos-kernel-current/obj/src/program_memory.cesta.o' 'build/sexos-kernel-current/obj/src/program_workers.cesta.o' 'build/sexos-kernel-current/obj/src/kernel_workers.cesta.o' 'build/sexos-kernel-current/obj/src/kernel_platform.cesta.o' 'build/sexos-kernel-current/obj/src/kernel_domains.cesta.o' 'build/sexos-kernel-current/obj/src/program_image.cesta.o' 'build/sexos-kernel-current/obj/src/program_image_bytes.cesta.o' 'build/sexos-kernel-current/obj/runtime/kernel/image_failure.c.o' 'build/sexos-kernel-current/obj/src/kernel_entry.cesta.o' 'build/sexos-kernel-current/obj/src/kernel_syscalls.cesta.o' 'build/sexos-kernel-current/obj/src/kernel_rpc.cesta.o' 'build/sexos-kernel-current/obj/src/memory_regions.cesta.o' 'build/sexos-kernel-current/obj/src/region_writer.cesta.o' 'build/sexos-kernel-current/obj/src/device_reset.cesta.o' 'build/sexos-kernel-current/obj/src/epoch_journal.cesta.o' 'build/sexos-kernel-current/obj/src/kernel_epoch_log.cesta.o' 'build/sexos-kernel-current/obj/src/kernel_boot_memory.cesta.o' 'build/sexos-kernel-current/obj/src/kernel_region_writer.cesta.o' 'build/sexos-kernel-current/obj/src/kernel_device_interrupt.cesta.o' 'build/sexos-kernel-current/obj/src/kernel_device_reset.cesta.o' 'build/sexos-kernel-current/obj/src/app_private.cesta.o' 'build/sexos-kernel-current/obj/src/kernel_apps.cesta.o' 'build/sexos-kernel-current/obj/src/program_records.cesta.o' 'build/sexos-kernel-current/obj/src/program_record_memory.cesta.o' 'build/sexos-kernel-current/obj/src/program_record_tasks.cesta.o' 'build/sexos-kernel-current/obj/src/kernel_records.cesta.o' 'build/sexos-kernel-current/obj/src/kernel_current_boot.cesta.o' 'build/sexos-kernel-current/obj/src/kernel_paging.cesta.o' 'build/sexos-kernel-current/obj/src/kernel_task_wait.cesta.o' 'build/sexos-kernel-current/obj/src/pager_state.cesta.o' 'build/sexos-kernel-current/obj/src/source_policy.cesta.o' 'build/sexos-kernel-current/obj/src/kernel_sources.cesta.o' 'build/sexos-kernel-current/obj/src/network_config.cesta.o' 'build/sexos-kernel-current/obj/src/pci_policy.cesta.o' 'build/sexos-kernel-current/obj/src/kernel_pci.cesta.o' 'build/sexos-kernel-current/obj/src/history_recovery.cesta.o' 'build/sexos-kernel-current/obj/src/kernel_history.cesta.o' 'build/sexos-kernel-current/obj/src/smp_park.cesta.o' 'build/sexos-kernel-current/obj/src/kernel_smp.cesta.o' 'build/sexos-kernel-current/obj/src/nvme_msix.cesta.o' 'build/sexos-kernel-current/obj/runtime/kernel/kernel_log.c.o' 'build/sexos-kernel-current/obj/runtime/kernel/entry_x86_64.S.o' '-nostdlib' '-static' '-Wl,-T,runtime/kernel/kernel.ld' '-Wl,--build-id=none' '-Wl,-z,max-page-size=0x1000' -o "$@"

build/.objects/012832936d1b3152/src/frame_lease.cesta.o: src/frame_lease.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_FPU_CONTEXT_CORRUPT_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/frame_lease.c' -o "$@"

build/sexos-kernel-fpu-corrupt-probe/obj/src/frame_lease.cesta.o: build/.objects/012832936d1b3152/src/frame_lease.cesta.o
	@mkdir -p 'build/sexos-kernel-fpu-corrupt-probe/obj/src'
	ln -f 'build/.objects/012832936d1b3152/src/frame_lease.cesta.o' "$@"

build/.objects/4b6c3dd6e367682b/src/kernel_memory.cesta.o: src/kernel_memory.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_FPU_CONTEXT_CORRUPT_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_memory.c' -o "$@"

build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_memory.cesta.o: build/.objects/4b6c3dd6e367682b/src/kernel_memory.cesta.o
	@mkdir -p 'build/sexos-kernel-fpu-corrupt-probe/obj/src'
	ln -f 'build/.objects/4b6c3dd6e367682b/src/kernel_memory.cesta.o' "$@"

build/.objects/ad0222d9c5f63fa0/src/program_memory.cesta.o: src/program_memory.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_FPU_CONTEXT_CORRUPT_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_memory.c' -o "$@"

build/sexos-kernel-fpu-corrupt-probe/obj/src/program_memory.cesta.o: build/.objects/ad0222d9c5f63fa0/src/program_memory.cesta.o
	@mkdir -p 'build/sexos-kernel-fpu-corrupt-probe/obj/src'
	ln -f 'build/.objects/ad0222d9c5f63fa0/src/program_memory.cesta.o' "$@"

build/.objects/e725e2cdb88c7b92/src/program_workers.cesta.o: src/program_workers.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_FPU_CONTEXT_CORRUPT_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_workers.c' -o "$@"

build/sexos-kernel-fpu-corrupt-probe/obj/src/program_workers.cesta.o: build/.objects/e725e2cdb88c7b92/src/program_workers.cesta.o
	@mkdir -p 'build/sexos-kernel-fpu-corrupt-probe/obj/src'
	ln -f 'build/.objects/e725e2cdb88c7b92/src/program_workers.cesta.o' "$@"

build/.objects/d9dca9af6f1587ef/src/kernel_workers.cesta.o: src/kernel_workers.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_FPU_CONTEXT_CORRUPT_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_workers.c' -o "$@"

build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_workers.cesta.o: build/.objects/d9dca9af6f1587ef/src/kernel_workers.cesta.o
	@mkdir -p 'build/sexos-kernel-fpu-corrupt-probe/obj/src'
	ln -f 'build/.objects/d9dca9af6f1587ef/src/kernel_workers.cesta.o' "$@"

build/.objects/bc3100f9009a5ed3/src/kernel_platform.cesta.o: src/kernel_platform.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_FPU_CONTEXT_CORRUPT_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_platform.c' -o "$@"

build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_platform.cesta.o: build/.objects/bc3100f9009a5ed3/src/kernel_platform.cesta.o
	@mkdir -p 'build/sexos-kernel-fpu-corrupt-probe/obj/src'
	ln -f 'build/.objects/bc3100f9009a5ed3/src/kernel_platform.cesta.o' "$@"

build/.objects/3e9e9d561cbc0b1d/src/kernel_domains.cesta.o: src/kernel_domains.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_FPU_CONTEXT_CORRUPT_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_domains.c' -o "$@"

build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_domains.cesta.o: build/.objects/3e9e9d561cbc0b1d/src/kernel_domains.cesta.o
	@mkdir -p 'build/sexos-kernel-fpu-corrupt-probe/obj/src'
	ln -f 'build/.objects/3e9e9d561cbc0b1d/src/kernel_domains.cesta.o' "$@"

build/.objects/8b3c452215fd79ac/src/program_image.cesta.o: src/program_image.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_FPU_CONTEXT_CORRUPT_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_image.c' -o "$@"

build/sexos-kernel-fpu-corrupt-probe/obj/src/program_image.cesta.o: build/.objects/8b3c452215fd79ac/src/program_image.cesta.o
	@mkdir -p 'build/sexos-kernel-fpu-corrupt-probe/obj/src'
	ln -f 'build/.objects/8b3c452215fd79ac/src/program_image.cesta.o' "$@"

build/.objects/bf585c79346b0598/src/program_image_bytes.cesta.o: src/program_image_bytes.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_FPU_CONTEXT_CORRUPT_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_image_bytes.c' -o "$@"

build/sexos-kernel-fpu-corrupt-probe/obj/src/program_image_bytes.cesta.o: build/.objects/bf585c79346b0598/src/program_image_bytes.cesta.o
	@mkdir -p 'build/sexos-kernel-fpu-corrupt-probe/obj/src'
	ln -f 'build/.objects/bf585c79346b0598/src/program_image_bytes.cesta.o' "$@"

build/.objects/9958c14727943b11/runtime/kernel/image_failure.c.o: runtime/kernel/image_failure.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_FPU_CONTEXT_CORRUPT_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/kernel/image_failure.c' -o "$@"

build/sexos-kernel-fpu-corrupt-probe/obj/runtime/kernel/image_failure.c.o: build/.objects/9958c14727943b11/runtime/kernel/image_failure.c.o
	@mkdir -p 'build/sexos-kernel-fpu-corrupt-probe/obj/runtime/kernel'
	ln -f 'build/.objects/9958c14727943b11/runtime/kernel/image_failure.c.o' "$@"

build/.objects/c100c487da11cd60/src/kernel_entry.cesta.o: src/kernel_entry.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_FPU_CONTEXT_CORRUPT_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_entry.c' -o "$@"

build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_entry.cesta.o: build/.objects/c100c487da11cd60/src/kernel_entry.cesta.o
	@mkdir -p 'build/sexos-kernel-fpu-corrupt-probe/obj/src'
	ln -f 'build/.objects/c100c487da11cd60/src/kernel_entry.cesta.o' "$@"

build/.objects/de53b3ed53df6666/src/kernel_syscalls.cesta.o: src/kernel_syscalls.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_FPU_CONTEXT_CORRUPT_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_syscalls.c' -o "$@"

build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_syscalls.cesta.o: build/.objects/de53b3ed53df6666/src/kernel_syscalls.cesta.o
	@mkdir -p 'build/sexos-kernel-fpu-corrupt-probe/obj/src'
	ln -f 'build/.objects/de53b3ed53df6666/src/kernel_syscalls.cesta.o' "$@"

build/.objects/ddd836cda75041eb/src/kernel_rpc.cesta.o: src/kernel_rpc.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_FPU_CONTEXT_CORRUPT_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_rpc.c' -o "$@"

build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_rpc.cesta.o: build/.objects/ddd836cda75041eb/src/kernel_rpc.cesta.o
	@mkdir -p 'build/sexos-kernel-fpu-corrupt-probe/obj/src'
	ln -f 'build/.objects/ddd836cda75041eb/src/kernel_rpc.cesta.o' "$@"

build/.objects/f7ae317a9a1eac17/src/memory_regions.cesta.o: src/memory_regions.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_FPU_CONTEXT_CORRUPT_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/memory_regions.c' -o "$@"

build/sexos-kernel-fpu-corrupt-probe/obj/src/memory_regions.cesta.o: build/.objects/f7ae317a9a1eac17/src/memory_regions.cesta.o
	@mkdir -p 'build/sexos-kernel-fpu-corrupt-probe/obj/src'
	ln -f 'build/.objects/f7ae317a9a1eac17/src/memory_regions.cesta.o' "$@"

build/.objects/862901d20385f304/src/region_writer.cesta.o: src/region_writer.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_FPU_CONTEXT_CORRUPT_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/region_writer.c' -o "$@"

build/sexos-kernel-fpu-corrupt-probe/obj/src/region_writer.cesta.o: build/.objects/862901d20385f304/src/region_writer.cesta.o
	@mkdir -p 'build/sexos-kernel-fpu-corrupt-probe/obj/src'
	ln -f 'build/.objects/862901d20385f304/src/region_writer.cesta.o' "$@"

build/.objects/08e60b291673ee1c/src/device_reset.cesta.o: src/device_reset.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_FPU_CONTEXT_CORRUPT_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/device_reset.c' -o "$@"

build/sexos-kernel-fpu-corrupt-probe/obj/src/device_reset.cesta.o: build/.objects/08e60b291673ee1c/src/device_reset.cesta.o
	@mkdir -p 'build/sexos-kernel-fpu-corrupt-probe/obj/src'
	ln -f 'build/.objects/08e60b291673ee1c/src/device_reset.cesta.o' "$@"

build/.objects/57e3ed9140ef5e2f/src/epoch_journal.cesta.o: src/epoch_journal.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_FPU_CONTEXT_CORRUPT_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/epoch_journal.c' -o "$@"

build/sexos-kernel-fpu-corrupt-probe/obj/src/epoch_journal.cesta.o: build/.objects/57e3ed9140ef5e2f/src/epoch_journal.cesta.o
	@mkdir -p 'build/sexos-kernel-fpu-corrupt-probe/obj/src'
	ln -f 'build/.objects/57e3ed9140ef5e2f/src/epoch_journal.cesta.o' "$@"

build/.objects/5c100ec5912487f0/src/kernel_epoch_log.cesta.o: src/kernel_epoch_log.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_FPU_CONTEXT_CORRUPT_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_epoch_log.c' -o "$@"

build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_epoch_log.cesta.o: build/.objects/5c100ec5912487f0/src/kernel_epoch_log.cesta.o
	@mkdir -p 'build/sexos-kernel-fpu-corrupt-probe/obj/src'
	ln -f 'build/.objects/5c100ec5912487f0/src/kernel_epoch_log.cesta.o' "$@"

build/.objects/8ef0e9941653aeda/src/kernel_boot_memory.cesta.o: src/kernel_boot_memory.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_FPU_CONTEXT_CORRUPT_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_boot_memory.c' -o "$@"

build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_boot_memory.cesta.o: build/.objects/8ef0e9941653aeda/src/kernel_boot_memory.cesta.o
	@mkdir -p 'build/sexos-kernel-fpu-corrupt-probe/obj/src'
	ln -f 'build/.objects/8ef0e9941653aeda/src/kernel_boot_memory.cesta.o' "$@"

build/.objects/62e64bfb4d1ce38a/src/kernel_region_writer.cesta.o: src/kernel_region_writer.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_FPU_CONTEXT_CORRUPT_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_region_writer.c' -o "$@"

build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_region_writer.cesta.o: build/.objects/62e64bfb4d1ce38a/src/kernel_region_writer.cesta.o
	@mkdir -p 'build/sexos-kernel-fpu-corrupt-probe/obj/src'
	ln -f 'build/.objects/62e64bfb4d1ce38a/src/kernel_region_writer.cesta.o' "$@"

build/.objects/26be9bc4ffab7b96/src/kernel_device_interrupt.cesta.o: src/kernel_device_interrupt.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_FPU_CONTEXT_CORRUPT_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_device_interrupt.c' -o "$@"

build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_device_interrupt.cesta.o: build/.objects/26be9bc4ffab7b96/src/kernel_device_interrupt.cesta.o
	@mkdir -p 'build/sexos-kernel-fpu-corrupt-probe/obj/src'
	ln -f 'build/.objects/26be9bc4ffab7b96/src/kernel_device_interrupt.cesta.o' "$@"

build/.objects/aba6e6598ec6237a/src/kernel_device_reset.cesta.o: src/kernel_device_reset.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_FPU_CONTEXT_CORRUPT_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_device_reset.c' -o "$@"

build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_device_reset.cesta.o: build/.objects/aba6e6598ec6237a/src/kernel_device_reset.cesta.o
	@mkdir -p 'build/sexos-kernel-fpu-corrupt-probe/obj/src'
	ln -f 'build/.objects/aba6e6598ec6237a/src/kernel_device_reset.cesta.o' "$@"

build/.objects/3605d45eff7bb3d5/src/app_private.cesta.o: src/app_private.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_FPU_CONTEXT_CORRUPT_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/app_private.c' -o "$@"

build/sexos-kernel-fpu-corrupt-probe/obj/src/app_private.cesta.o: build/.objects/3605d45eff7bb3d5/src/app_private.cesta.o
	@mkdir -p 'build/sexos-kernel-fpu-corrupt-probe/obj/src'
	ln -f 'build/.objects/3605d45eff7bb3d5/src/app_private.cesta.o' "$@"

build/.objects/bf36f48a017e61d4/src/kernel_apps.cesta.o: src/kernel_apps.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_FPU_CONTEXT_CORRUPT_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_apps.c' -o "$@"

build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_apps.cesta.o: build/.objects/bf36f48a017e61d4/src/kernel_apps.cesta.o
	@mkdir -p 'build/sexos-kernel-fpu-corrupt-probe/obj/src'
	ln -f 'build/.objects/bf36f48a017e61d4/src/kernel_apps.cesta.o' "$@"

build/.objects/fa05c5da99790b33/src/program_records.cesta.o: src/program_records.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_FPU_CONTEXT_CORRUPT_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_records.c' -o "$@"

build/sexos-kernel-fpu-corrupt-probe/obj/src/program_records.cesta.o: build/.objects/fa05c5da99790b33/src/program_records.cesta.o
	@mkdir -p 'build/sexos-kernel-fpu-corrupt-probe/obj/src'
	ln -f 'build/.objects/fa05c5da99790b33/src/program_records.cesta.o' "$@"

build/.objects/5e255764dc41d4e4/src/program_record_memory.cesta.o: src/program_record_memory.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_FPU_CONTEXT_CORRUPT_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_record_memory.c' -o "$@"

build/sexos-kernel-fpu-corrupt-probe/obj/src/program_record_memory.cesta.o: build/.objects/5e255764dc41d4e4/src/program_record_memory.cesta.o
	@mkdir -p 'build/sexos-kernel-fpu-corrupt-probe/obj/src'
	ln -f 'build/.objects/5e255764dc41d4e4/src/program_record_memory.cesta.o' "$@"

build/.objects/796dc29a9562cb17/src/program_record_tasks.cesta.o: src/program_record_tasks.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_FPU_CONTEXT_CORRUPT_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_record_tasks.c' -o "$@"

build/sexos-kernel-fpu-corrupt-probe/obj/src/program_record_tasks.cesta.o: build/.objects/796dc29a9562cb17/src/program_record_tasks.cesta.o
	@mkdir -p 'build/sexos-kernel-fpu-corrupt-probe/obj/src'
	ln -f 'build/.objects/796dc29a9562cb17/src/program_record_tasks.cesta.o' "$@"

build/.objects/a05c3142e3b20b1e/src/kernel_records.cesta.o: src/kernel_records.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_FPU_CONTEXT_CORRUPT_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_records.c' -o "$@"

build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_records.cesta.o: build/.objects/a05c3142e3b20b1e/src/kernel_records.cesta.o
	@mkdir -p 'build/sexos-kernel-fpu-corrupt-probe/obj/src'
	ln -f 'build/.objects/a05c3142e3b20b1e/src/kernel_records.cesta.o' "$@"

build/.objects/ae0b5cac27031166/src/kernel_current_boot.cesta.o: src/kernel_current_boot.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_FPU_CONTEXT_CORRUPT_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_current_boot.c' -o "$@"

build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_current_boot.cesta.o: build/.objects/ae0b5cac27031166/src/kernel_current_boot.cesta.o
	@mkdir -p 'build/sexos-kernel-fpu-corrupt-probe/obj/src'
	ln -f 'build/.objects/ae0b5cac27031166/src/kernel_current_boot.cesta.o' "$@"

build/.objects/c6fd9f4b38a9fb38/src/kernel_paging.cesta.o: src/kernel_paging.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_FPU_CONTEXT_CORRUPT_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_paging.c' -o "$@"

build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_paging.cesta.o: build/.objects/c6fd9f4b38a9fb38/src/kernel_paging.cesta.o
	@mkdir -p 'build/sexos-kernel-fpu-corrupt-probe/obj/src'
	ln -f 'build/.objects/c6fd9f4b38a9fb38/src/kernel_paging.cesta.o' "$@"

build/.objects/48f8c07a314691bb/src/kernel_task_wait.cesta.o: src/kernel_task_wait.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_FPU_CONTEXT_CORRUPT_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_task_wait.c' -o "$@"

build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_task_wait.cesta.o: build/.objects/48f8c07a314691bb/src/kernel_task_wait.cesta.o
	@mkdir -p 'build/sexos-kernel-fpu-corrupt-probe/obj/src'
	ln -f 'build/.objects/48f8c07a314691bb/src/kernel_task_wait.cesta.o' "$@"

build/.objects/2d2a74b24c5f5503/src/pager_state.cesta.o: src/pager_state.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_FPU_CONTEXT_CORRUPT_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/pager_state.c' -o "$@"

build/sexos-kernel-fpu-corrupt-probe/obj/src/pager_state.cesta.o: build/.objects/2d2a74b24c5f5503/src/pager_state.cesta.o
	@mkdir -p 'build/sexos-kernel-fpu-corrupt-probe/obj/src'
	ln -f 'build/.objects/2d2a74b24c5f5503/src/pager_state.cesta.o' "$@"

build/.objects/d45907a0080d8762/src/source_policy.cesta.o: src/source_policy.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_FPU_CONTEXT_CORRUPT_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/source_policy.c' -o "$@"

build/sexos-kernel-fpu-corrupt-probe/obj/src/source_policy.cesta.o: build/.objects/d45907a0080d8762/src/source_policy.cesta.o
	@mkdir -p 'build/sexos-kernel-fpu-corrupt-probe/obj/src'
	ln -f 'build/.objects/d45907a0080d8762/src/source_policy.cesta.o' "$@"

build/.objects/fa1d06156fd1d33e/src/kernel_sources.cesta.o: src/kernel_sources.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_FPU_CONTEXT_CORRUPT_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_sources.c' -o "$@"

build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_sources.cesta.o: build/.objects/fa1d06156fd1d33e/src/kernel_sources.cesta.o
	@mkdir -p 'build/sexos-kernel-fpu-corrupt-probe/obj/src'
	ln -f 'build/.objects/fa1d06156fd1d33e/src/kernel_sources.cesta.o' "$@"

build/.objects/bc7ab41fa23da6fb/src/network_config.cesta.o: src/network_config.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_FPU_CONTEXT_CORRUPT_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_config.c' -o "$@"

build/sexos-kernel-fpu-corrupt-probe/obj/src/network_config.cesta.o: build/.objects/bc7ab41fa23da6fb/src/network_config.cesta.o
	@mkdir -p 'build/sexos-kernel-fpu-corrupt-probe/obj/src'
	ln -f 'build/.objects/bc7ab41fa23da6fb/src/network_config.cesta.o' "$@"

build/.objects/68b22ba9b3144e85/src/pci_policy.cesta.o: src/pci_policy.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_FPU_CONTEXT_CORRUPT_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/pci_policy.c' -o "$@"

build/sexos-kernel-fpu-corrupt-probe/obj/src/pci_policy.cesta.o: build/.objects/68b22ba9b3144e85/src/pci_policy.cesta.o
	@mkdir -p 'build/sexos-kernel-fpu-corrupt-probe/obj/src'
	ln -f 'build/.objects/68b22ba9b3144e85/src/pci_policy.cesta.o' "$@"

build/.objects/346b5252e0ddc8cc/src/kernel_pci.cesta.o: src/kernel_pci.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_FPU_CONTEXT_CORRUPT_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_pci.c' -o "$@"

build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_pci.cesta.o: build/.objects/346b5252e0ddc8cc/src/kernel_pci.cesta.o
	@mkdir -p 'build/sexos-kernel-fpu-corrupt-probe/obj/src'
	ln -f 'build/.objects/346b5252e0ddc8cc/src/kernel_pci.cesta.o' "$@"

build/.objects/2a550027ef5b5244/src/history_recovery.cesta.o: src/history_recovery.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_FPU_CONTEXT_CORRUPT_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/history_recovery.c' -o "$@"

build/sexos-kernel-fpu-corrupt-probe/obj/src/history_recovery.cesta.o: build/.objects/2a550027ef5b5244/src/history_recovery.cesta.o
	@mkdir -p 'build/sexos-kernel-fpu-corrupt-probe/obj/src'
	ln -f 'build/.objects/2a550027ef5b5244/src/history_recovery.cesta.o' "$@"

build/.objects/8dd4452add544124/src/kernel_history.cesta.o: src/kernel_history.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_FPU_CONTEXT_CORRUPT_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_history.c' -o "$@"

build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_history.cesta.o: build/.objects/8dd4452add544124/src/kernel_history.cesta.o
	@mkdir -p 'build/sexos-kernel-fpu-corrupt-probe/obj/src'
	ln -f 'build/.objects/8dd4452add544124/src/kernel_history.cesta.o' "$@"

build/.objects/5a0f16e06a8b15cb/src/smp_park.cesta.o: src/smp_park.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_FPU_CONTEXT_CORRUPT_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/smp_park.c' -o "$@"

build/sexos-kernel-fpu-corrupt-probe/obj/src/smp_park.cesta.o: build/.objects/5a0f16e06a8b15cb/src/smp_park.cesta.o
	@mkdir -p 'build/sexos-kernel-fpu-corrupt-probe/obj/src'
	ln -f 'build/.objects/5a0f16e06a8b15cb/src/smp_park.cesta.o' "$@"

build/.objects/a77a0c5b6a877760/src/kernel_smp.cesta.o: src/kernel_smp.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_FPU_CONTEXT_CORRUPT_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_smp.c' -o "$@"

build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_smp.cesta.o: build/.objects/a77a0c5b6a877760/src/kernel_smp.cesta.o
	@mkdir -p 'build/sexos-kernel-fpu-corrupt-probe/obj/src'
	ln -f 'build/.objects/a77a0c5b6a877760/src/kernel_smp.cesta.o' "$@"

build/.objects/ab7c1623d07a9884/src/nvme_msix.cesta.o: src/nvme_msix.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_FPU_CONTEXT_CORRUPT_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/nvme_msix.c' -o "$@"

build/sexos-kernel-fpu-corrupt-probe/obj/src/nvme_msix.cesta.o: build/.objects/ab7c1623d07a9884/src/nvme_msix.cesta.o
	@mkdir -p 'build/sexos-kernel-fpu-corrupt-probe/obj/src'
	ln -f 'build/.objects/ab7c1623d07a9884/src/nvme_msix.cesta.o' "$@"

build/.objects/601cb46df15b389d/runtime/kernel/kernel_log.c.o: runtime/kernel/kernel_log.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_FPU_CONTEXT_CORRUPT_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/kernel/kernel_log.c' -o "$@"

build/sexos-kernel-fpu-corrupt-probe/obj/runtime/kernel/kernel_log.c.o: build/.objects/601cb46df15b389d/runtime/kernel/kernel_log.c.o
	@mkdir -p 'build/sexos-kernel-fpu-corrupt-probe/obj/runtime/kernel'
	ln -f 'build/.objects/601cb46df15b389d/runtime/kernel/kernel_log.c.o' "$@"

build/.objects/89a93f157b5d71e5/runtime/kernel/entry_x86_64.S.o: runtime/kernel/entry_x86_64.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' '-DSEXOS_FPU_CONTEXT_CORRUPT_PROBE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/kernel/entry_x86_64.S' -o "$@"

build/sexos-kernel-fpu-corrupt-probe/obj/runtime/kernel/entry_x86_64.S.o: build/.objects/89a93f157b5d71e5/runtime/kernel/entry_x86_64.S.o
	@mkdir -p 'build/sexos-kernel-fpu-corrupt-probe/obj/runtime/kernel'
	ln -f 'build/.objects/89a93f157b5d71e5/runtime/kernel/entry_x86_64.S.o' "$@"

build/sexos-kernel-fpu-corrupt-probe/sexos-kernel-fpu-corrupt-probe: GNUmakefile build/sexos-kernel-fpu-corrupt-probe/obj/src/frame_lease.cesta.o build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_memory.cesta.o build/sexos-kernel-fpu-corrupt-probe/obj/src/program_memory.cesta.o build/sexos-kernel-fpu-corrupt-probe/obj/src/program_workers.cesta.o build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_workers.cesta.o build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_platform.cesta.o build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_domains.cesta.o build/sexos-kernel-fpu-corrupt-probe/obj/src/program_image.cesta.o build/sexos-kernel-fpu-corrupt-probe/obj/src/program_image_bytes.cesta.o build/sexos-kernel-fpu-corrupt-probe/obj/runtime/kernel/image_failure.c.o build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_entry.cesta.o build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_syscalls.cesta.o build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_rpc.cesta.o build/sexos-kernel-fpu-corrupt-probe/obj/src/memory_regions.cesta.o build/sexos-kernel-fpu-corrupt-probe/obj/src/region_writer.cesta.o build/sexos-kernel-fpu-corrupt-probe/obj/src/device_reset.cesta.o build/sexos-kernel-fpu-corrupt-probe/obj/src/epoch_journal.cesta.o build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_epoch_log.cesta.o build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_boot_memory.cesta.o build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_region_writer.cesta.o build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_device_interrupt.cesta.o build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_device_reset.cesta.o build/sexos-kernel-fpu-corrupt-probe/obj/src/app_private.cesta.o build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_apps.cesta.o build/sexos-kernel-fpu-corrupt-probe/obj/src/program_records.cesta.o build/sexos-kernel-fpu-corrupt-probe/obj/src/program_record_memory.cesta.o build/sexos-kernel-fpu-corrupt-probe/obj/src/program_record_tasks.cesta.o build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_records.cesta.o build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_current_boot.cesta.o build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_paging.cesta.o build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_task_wait.cesta.o build/sexos-kernel-fpu-corrupt-probe/obj/src/pager_state.cesta.o build/sexos-kernel-fpu-corrupt-probe/obj/src/source_policy.cesta.o build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_sources.cesta.o build/sexos-kernel-fpu-corrupt-probe/obj/src/network_config.cesta.o build/sexos-kernel-fpu-corrupt-probe/obj/src/pci_policy.cesta.o build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_pci.cesta.o build/sexos-kernel-fpu-corrupt-probe/obj/src/history_recovery.cesta.o build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_history.cesta.o build/sexos-kernel-fpu-corrupt-probe/obj/src/smp_park.cesta.o build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_smp.cesta.o build/sexos-kernel-fpu-corrupt-probe/obj/src/nvme_msix.cesta.o build/sexos-kernel-fpu-corrupt-probe/obj/runtime/kernel/kernel_log.c.o build/sexos-kernel-fpu-corrupt-probe/obj/runtime/kernel/entry_x86_64.S.o runtime/kernel/kernel.ld
	@mkdir -p "$(@D)"
	$(CC) 'build/sexos-kernel-fpu-corrupt-probe/obj/src/frame_lease.cesta.o' 'build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_memory.cesta.o' 'build/sexos-kernel-fpu-corrupt-probe/obj/src/program_memory.cesta.o' 'build/sexos-kernel-fpu-corrupt-probe/obj/src/program_workers.cesta.o' 'build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_workers.cesta.o' 'build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_platform.cesta.o' 'build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_domains.cesta.o' 'build/sexos-kernel-fpu-corrupt-probe/obj/src/program_image.cesta.o' 'build/sexos-kernel-fpu-corrupt-probe/obj/src/program_image_bytes.cesta.o' 'build/sexos-kernel-fpu-corrupt-probe/obj/runtime/kernel/image_failure.c.o' 'build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_entry.cesta.o' 'build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_syscalls.cesta.o' 'build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_rpc.cesta.o' 'build/sexos-kernel-fpu-corrupt-probe/obj/src/memory_regions.cesta.o' 'build/sexos-kernel-fpu-corrupt-probe/obj/src/region_writer.cesta.o' 'build/sexos-kernel-fpu-corrupt-probe/obj/src/device_reset.cesta.o' 'build/sexos-kernel-fpu-corrupt-probe/obj/src/epoch_journal.cesta.o' 'build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_epoch_log.cesta.o' 'build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_boot_memory.cesta.o' 'build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_region_writer.cesta.o' 'build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_device_interrupt.cesta.o' 'build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_device_reset.cesta.o' 'build/sexos-kernel-fpu-corrupt-probe/obj/src/app_private.cesta.o' 'build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_apps.cesta.o' 'build/sexos-kernel-fpu-corrupt-probe/obj/src/program_records.cesta.o' 'build/sexos-kernel-fpu-corrupt-probe/obj/src/program_record_memory.cesta.o' 'build/sexos-kernel-fpu-corrupt-probe/obj/src/program_record_tasks.cesta.o' 'build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_records.cesta.o' 'build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_current_boot.cesta.o' 'build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_paging.cesta.o' 'build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_task_wait.cesta.o' 'build/sexos-kernel-fpu-corrupt-probe/obj/src/pager_state.cesta.o' 'build/sexos-kernel-fpu-corrupt-probe/obj/src/source_policy.cesta.o' 'build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_sources.cesta.o' 'build/sexos-kernel-fpu-corrupt-probe/obj/src/network_config.cesta.o' 'build/sexos-kernel-fpu-corrupt-probe/obj/src/pci_policy.cesta.o' 'build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_pci.cesta.o' 'build/sexos-kernel-fpu-corrupt-probe/obj/src/history_recovery.cesta.o' 'build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_history.cesta.o' 'build/sexos-kernel-fpu-corrupt-probe/obj/src/smp_park.cesta.o' 'build/sexos-kernel-fpu-corrupt-probe/obj/src/kernel_smp.cesta.o' 'build/sexos-kernel-fpu-corrupt-probe/obj/src/nvme_msix.cesta.o' 'build/sexos-kernel-fpu-corrupt-probe/obj/runtime/kernel/kernel_log.c.o' 'build/sexos-kernel-fpu-corrupt-probe/obj/runtime/kernel/entry_x86_64.S.o' '-nostdlib' '-static' '-Wl,-T,runtime/kernel/kernel.ld' '-Wl,--build-id=none' '-Wl,-z,max-page-size=0x1000' -o "$@"

build/.objects/86d16ae28840bb54/src/frame_lease.cesta.o: src/frame_lease.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_KERNEL_BOOT_AUDIT=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/frame_lease.c' -o "$@"

build/sexos-kernel-boot-audit/obj/src/frame_lease.cesta.o: build/.objects/86d16ae28840bb54/src/frame_lease.cesta.o
	@mkdir -p 'build/sexos-kernel-boot-audit/obj/src'
	ln -f 'build/.objects/86d16ae28840bb54/src/frame_lease.cesta.o' "$@"

build/.objects/a9ea9e0c1184fadd/src/kernel_memory.cesta.o: src/kernel_memory.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_KERNEL_BOOT_AUDIT=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_memory.c' -o "$@"

build/sexos-kernel-boot-audit/obj/src/kernel_memory.cesta.o: build/.objects/a9ea9e0c1184fadd/src/kernel_memory.cesta.o
	@mkdir -p 'build/sexos-kernel-boot-audit/obj/src'
	ln -f 'build/.objects/a9ea9e0c1184fadd/src/kernel_memory.cesta.o' "$@"

build/.objects/059e9a1516998dfa/src/program_memory.cesta.o: src/program_memory.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_KERNEL_BOOT_AUDIT=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_memory.c' -o "$@"

build/sexos-kernel-boot-audit/obj/src/program_memory.cesta.o: build/.objects/059e9a1516998dfa/src/program_memory.cesta.o
	@mkdir -p 'build/sexos-kernel-boot-audit/obj/src'
	ln -f 'build/.objects/059e9a1516998dfa/src/program_memory.cesta.o' "$@"

build/.objects/6d8ce2d4ce1ac894/src/program_workers.cesta.o: src/program_workers.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_KERNEL_BOOT_AUDIT=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_workers.c' -o "$@"

build/sexos-kernel-boot-audit/obj/src/program_workers.cesta.o: build/.objects/6d8ce2d4ce1ac894/src/program_workers.cesta.o
	@mkdir -p 'build/sexos-kernel-boot-audit/obj/src'
	ln -f 'build/.objects/6d8ce2d4ce1ac894/src/program_workers.cesta.o' "$@"

build/.objects/caa69b2a3b1c0c29/src/kernel_workers.cesta.o: src/kernel_workers.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_KERNEL_BOOT_AUDIT=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_workers.c' -o "$@"

build/sexos-kernel-boot-audit/obj/src/kernel_workers.cesta.o: build/.objects/caa69b2a3b1c0c29/src/kernel_workers.cesta.o
	@mkdir -p 'build/sexos-kernel-boot-audit/obj/src'
	ln -f 'build/.objects/caa69b2a3b1c0c29/src/kernel_workers.cesta.o' "$@"

build/.objects/1c85c728624201e5/src/kernel_platform.cesta.o: src/kernel_platform.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_KERNEL_BOOT_AUDIT=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_platform.c' -o "$@"

build/sexos-kernel-boot-audit/obj/src/kernel_platform.cesta.o: build/.objects/1c85c728624201e5/src/kernel_platform.cesta.o
	@mkdir -p 'build/sexos-kernel-boot-audit/obj/src'
	ln -f 'build/.objects/1c85c728624201e5/src/kernel_platform.cesta.o' "$@"

build/.objects/00b5d34311986a6f/src/kernel_domains.cesta.o: src/kernel_domains.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_KERNEL_BOOT_AUDIT=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_domains.c' -o "$@"

build/sexos-kernel-boot-audit/obj/src/kernel_domains.cesta.o: build/.objects/00b5d34311986a6f/src/kernel_domains.cesta.o
	@mkdir -p 'build/sexos-kernel-boot-audit/obj/src'
	ln -f 'build/.objects/00b5d34311986a6f/src/kernel_domains.cesta.o' "$@"

build/.objects/eb64be6bed76eede/src/program_image.cesta.o: src/program_image.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_KERNEL_BOOT_AUDIT=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_image.c' -o "$@"

build/sexos-kernel-boot-audit/obj/src/program_image.cesta.o: build/.objects/eb64be6bed76eede/src/program_image.cesta.o
	@mkdir -p 'build/sexos-kernel-boot-audit/obj/src'
	ln -f 'build/.objects/eb64be6bed76eede/src/program_image.cesta.o' "$@"

build/.objects/399b51fbe2e82a32/src/program_image_bytes.cesta.o: src/program_image_bytes.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_KERNEL_BOOT_AUDIT=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_image_bytes.c' -o "$@"

build/sexos-kernel-boot-audit/obj/src/program_image_bytes.cesta.o: build/.objects/399b51fbe2e82a32/src/program_image_bytes.cesta.o
	@mkdir -p 'build/sexos-kernel-boot-audit/obj/src'
	ln -f 'build/.objects/399b51fbe2e82a32/src/program_image_bytes.cesta.o' "$@"

build/.objects/d707ffc0ba0f054b/runtime/kernel/image_failure.c.o: runtime/kernel/image_failure.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_KERNEL_BOOT_AUDIT=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/kernel/image_failure.c' -o "$@"

build/sexos-kernel-boot-audit/obj/runtime/kernel/image_failure.c.o: build/.objects/d707ffc0ba0f054b/runtime/kernel/image_failure.c.o
	@mkdir -p 'build/sexos-kernel-boot-audit/obj/runtime/kernel'
	ln -f 'build/.objects/d707ffc0ba0f054b/runtime/kernel/image_failure.c.o' "$@"

build/.objects/fc7b39e3d73198ba/src/kernel_entry.cesta.o: src/kernel_entry.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_KERNEL_BOOT_AUDIT=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_entry.c' -o "$@"

build/sexos-kernel-boot-audit/obj/src/kernel_entry.cesta.o: build/.objects/fc7b39e3d73198ba/src/kernel_entry.cesta.o
	@mkdir -p 'build/sexos-kernel-boot-audit/obj/src'
	ln -f 'build/.objects/fc7b39e3d73198ba/src/kernel_entry.cesta.o' "$@"

build/.objects/5cc478917fbf8c70/src/kernel_syscalls.cesta.o: src/kernel_syscalls.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_KERNEL_BOOT_AUDIT=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_syscalls.c' -o "$@"

build/sexos-kernel-boot-audit/obj/src/kernel_syscalls.cesta.o: build/.objects/5cc478917fbf8c70/src/kernel_syscalls.cesta.o
	@mkdir -p 'build/sexos-kernel-boot-audit/obj/src'
	ln -f 'build/.objects/5cc478917fbf8c70/src/kernel_syscalls.cesta.o' "$@"

build/.objects/3c08714bbd59bb9d/src/kernel_rpc.cesta.o: src/kernel_rpc.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_KERNEL_BOOT_AUDIT=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_rpc.c' -o "$@"

build/sexos-kernel-boot-audit/obj/src/kernel_rpc.cesta.o: build/.objects/3c08714bbd59bb9d/src/kernel_rpc.cesta.o
	@mkdir -p 'build/sexos-kernel-boot-audit/obj/src'
	ln -f 'build/.objects/3c08714bbd59bb9d/src/kernel_rpc.cesta.o' "$@"

build/.objects/617e58147dd5fbd1/src/memory_regions.cesta.o: src/memory_regions.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_KERNEL_BOOT_AUDIT=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/memory_regions.c' -o "$@"

build/sexos-kernel-boot-audit/obj/src/memory_regions.cesta.o: build/.objects/617e58147dd5fbd1/src/memory_regions.cesta.o
	@mkdir -p 'build/sexos-kernel-boot-audit/obj/src'
	ln -f 'build/.objects/617e58147dd5fbd1/src/memory_regions.cesta.o' "$@"

build/.objects/0c88752e1d853976/src/region_writer.cesta.o: src/region_writer.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_KERNEL_BOOT_AUDIT=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/region_writer.c' -o "$@"

build/sexos-kernel-boot-audit/obj/src/region_writer.cesta.o: build/.objects/0c88752e1d853976/src/region_writer.cesta.o
	@mkdir -p 'build/sexos-kernel-boot-audit/obj/src'
	ln -f 'build/.objects/0c88752e1d853976/src/region_writer.cesta.o' "$@"

build/.objects/e7d5c679263f0bee/src/device_reset.cesta.o: src/device_reset.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_KERNEL_BOOT_AUDIT=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/device_reset.c' -o "$@"

build/sexos-kernel-boot-audit/obj/src/device_reset.cesta.o: build/.objects/e7d5c679263f0bee/src/device_reset.cesta.o
	@mkdir -p 'build/sexos-kernel-boot-audit/obj/src'
	ln -f 'build/.objects/e7d5c679263f0bee/src/device_reset.cesta.o' "$@"

build/.objects/c21160de7f584f69/src/epoch_journal.cesta.o: src/epoch_journal.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_KERNEL_BOOT_AUDIT=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/epoch_journal.c' -o "$@"

build/sexos-kernel-boot-audit/obj/src/epoch_journal.cesta.o: build/.objects/c21160de7f584f69/src/epoch_journal.cesta.o
	@mkdir -p 'build/sexos-kernel-boot-audit/obj/src'
	ln -f 'build/.objects/c21160de7f584f69/src/epoch_journal.cesta.o' "$@"

build/.objects/07c9d0c04699ba2a/src/kernel_epoch_log.cesta.o: src/kernel_epoch_log.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_KERNEL_BOOT_AUDIT=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_epoch_log.c' -o "$@"

build/sexos-kernel-boot-audit/obj/src/kernel_epoch_log.cesta.o: build/.objects/07c9d0c04699ba2a/src/kernel_epoch_log.cesta.o
	@mkdir -p 'build/sexos-kernel-boot-audit/obj/src'
	ln -f 'build/.objects/07c9d0c04699ba2a/src/kernel_epoch_log.cesta.o' "$@"

build/.objects/cee9d22d6268961c/src/kernel_boot_memory.cesta.o: src/kernel_boot_memory.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_KERNEL_BOOT_AUDIT=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_boot_memory.c' -o "$@"

build/sexos-kernel-boot-audit/obj/src/kernel_boot_memory.cesta.o: build/.objects/cee9d22d6268961c/src/kernel_boot_memory.cesta.o
	@mkdir -p 'build/sexos-kernel-boot-audit/obj/src'
	ln -f 'build/.objects/cee9d22d6268961c/src/kernel_boot_memory.cesta.o' "$@"

build/.objects/00accb0eeaa92bac/src/kernel_region_writer.cesta.o: src/kernel_region_writer.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_KERNEL_BOOT_AUDIT=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_region_writer.c' -o "$@"

build/sexos-kernel-boot-audit/obj/src/kernel_region_writer.cesta.o: build/.objects/00accb0eeaa92bac/src/kernel_region_writer.cesta.o
	@mkdir -p 'build/sexos-kernel-boot-audit/obj/src'
	ln -f 'build/.objects/00accb0eeaa92bac/src/kernel_region_writer.cesta.o' "$@"

build/.objects/246e587d304df6c0/src/kernel_device_interrupt.cesta.o: src/kernel_device_interrupt.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_KERNEL_BOOT_AUDIT=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_device_interrupt.c' -o "$@"

build/sexos-kernel-boot-audit/obj/src/kernel_device_interrupt.cesta.o: build/.objects/246e587d304df6c0/src/kernel_device_interrupt.cesta.o
	@mkdir -p 'build/sexos-kernel-boot-audit/obj/src'
	ln -f 'build/.objects/246e587d304df6c0/src/kernel_device_interrupt.cesta.o' "$@"

build/.objects/aa150810d8e97bbc/src/kernel_device_reset.cesta.o: src/kernel_device_reset.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_KERNEL_BOOT_AUDIT=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_device_reset.c' -o "$@"

build/sexos-kernel-boot-audit/obj/src/kernel_device_reset.cesta.o: build/.objects/aa150810d8e97bbc/src/kernel_device_reset.cesta.o
	@mkdir -p 'build/sexos-kernel-boot-audit/obj/src'
	ln -f 'build/.objects/aa150810d8e97bbc/src/kernel_device_reset.cesta.o' "$@"

build/.objects/3a50a5701de808a7/src/app_private.cesta.o: src/app_private.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_KERNEL_BOOT_AUDIT=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/app_private.c' -o "$@"

build/sexos-kernel-boot-audit/obj/src/app_private.cesta.o: build/.objects/3a50a5701de808a7/src/app_private.cesta.o
	@mkdir -p 'build/sexos-kernel-boot-audit/obj/src'
	ln -f 'build/.objects/3a50a5701de808a7/src/app_private.cesta.o' "$@"

build/.objects/20ff552677f8fb26/src/kernel_apps.cesta.o: src/kernel_apps.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_KERNEL_BOOT_AUDIT=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_apps.c' -o "$@"

build/sexos-kernel-boot-audit/obj/src/kernel_apps.cesta.o: build/.objects/20ff552677f8fb26/src/kernel_apps.cesta.o
	@mkdir -p 'build/sexos-kernel-boot-audit/obj/src'
	ln -f 'build/.objects/20ff552677f8fb26/src/kernel_apps.cesta.o' "$@"

build/.objects/6dafcf3f1cdd2cc5/src/program_records.cesta.o: src/program_records.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_KERNEL_BOOT_AUDIT=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_records.c' -o "$@"

build/sexos-kernel-boot-audit/obj/src/program_records.cesta.o: build/.objects/6dafcf3f1cdd2cc5/src/program_records.cesta.o
	@mkdir -p 'build/sexos-kernel-boot-audit/obj/src'
	ln -f 'build/.objects/6dafcf3f1cdd2cc5/src/program_records.cesta.o' "$@"

build/.objects/a235394a4c6a1c56/src/program_record_memory.cesta.o: src/program_record_memory.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_KERNEL_BOOT_AUDIT=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_record_memory.c' -o "$@"

build/sexos-kernel-boot-audit/obj/src/program_record_memory.cesta.o: build/.objects/a235394a4c6a1c56/src/program_record_memory.cesta.o
	@mkdir -p 'build/sexos-kernel-boot-audit/obj/src'
	ln -f 'build/.objects/a235394a4c6a1c56/src/program_record_memory.cesta.o' "$@"

build/.objects/755d8548f11c3ad1/src/program_record_tasks.cesta.o: src/program_record_tasks.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_KERNEL_BOOT_AUDIT=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_record_tasks.c' -o "$@"

build/sexos-kernel-boot-audit/obj/src/program_record_tasks.cesta.o: build/.objects/755d8548f11c3ad1/src/program_record_tasks.cesta.o
	@mkdir -p 'build/sexos-kernel-boot-audit/obj/src'
	ln -f 'build/.objects/755d8548f11c3ad1/src/program_record_tasks.cesta.o' "$@"

build/.objects/85f24a6d61afb988/src/kernel_records.cesta.o: src/kernel_records.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_KERNEL_BOOT_AUDIT=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_records.c' -o "$@"

build/sexos-kernel-boot-audit/obj/src/kernel_records.cesta.o: build/.objects/85f24a6d61afb988/src/kernel_records.cesta.o
	@mkdir -p 'build/sexos-kernel-boot-audit/obj/src'
	ln -f 'build/.objects/85f24a6d61afb988/src/kernel_records.cesta.o' "$@"

build/.objects/9bd8bf6ba2abd770/src/kernel_current_boot.cesta.o: src/kernel_current_boot.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_KERNEL_BOOT_AUDIT=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_current_boot.c' -o "$@"

build/sexos-kernel-boot-audit/obj/src/kernel_current_boot.cesta.o: build/.objects/9bd8bf6ba2abd770/src/kernel_current_boot.cesta.o
	@mkdir -p 'build/sexos-kernel-boot-audit/obj/src'
	ln -f 'build/.objects/9bd8bf6ba2abd770/src/kernel_current_boot.cesta.o' "$@"

build/.objects/909942c67ce05ad2/src/kernel_paging.cesta.o: src/kernel_paging.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_KERNEL_BOOT_AUDIT=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_paging.c' -o "$@"

build/sexos-kernel-boot-audit/obj/src/kernel_paging.cesta.o: build/.objects/909942c67ce05ad2/src/kernel_paging.cesta.o
	@mkdir -p 'build/sexos-kernel-boot-audit/obj/src'
	ln -f 'build/.objects/909942c67ce05ad2/src/kernel_paging.cesta.o' "$@"

build/.objects/f436aadde869abcd/src/kernel_task_wait.cesta.o: src/kernel_task_wait.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_KERNEL_BOOT_AUDIT=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_task_wait.c' -o "$@"

build/sexos-kernel-boot-audit/obj/src/kernel_task_wait.cesta.o: build/.objects/f436aadde869abcd/src/kernel_task_wait.cesta.o
	@mkdir -p 'build/sexos-kernel-boot-audit/obj/src'
	ln -f 'build/.objects/f436aadde869abcd/src/kernel_task_wait.cesta.o' "$@"

build/.objects/f48c71ca1af8ed35/src/pager_state.cesta.o: src/pager_state.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_KERNEL_BOOT_AUDIT=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/pager_state.c' -o "$@"

build/sexos-kernel-boot-audit/obj/src/pager_state.cesta.o: build/.objects/f48c71ca1af8ed35/src/pager_state.cesta.o
	@mkdir -p 'build/sexos-kernel-boot-audit/obj/src'
	ln -f 'build/.objects/f48c71ca1af8ed35/src/pager_state.cesta.o' "$@"

build/.objects/76b499397f04f784/src/source_policy.cesta.o: src/source_policy.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_KERNEL_BOOT_AUDIT=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/source_policy.c' -o "$@"

build/sexos-kernel-boot-audit/obj/src/source_policy.cesta.o: build/.objects/76b499397f04f784/src/source_policy.cesta.o
	@mkdir -p 'build/sexos-kernel-boot-audit/obj/src'
	ln -f 'build/.objects/76b499397f04f784/src/source_policy.cesta.o' "$@"

build/.objects/a29e7ba0f4e001a8/src/kernel_sources.cesta.o: src/kernel_sources.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_KERNEL_BOOT_AUDIT=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_sources.c' -o "$@"

build/sexos-kernel-boot-audit/obj/src/kernel_sources.cesta.o: build/.objects/a29e7ba0f4e001a8/src/kernel_sources.cesta.o
	@mkdir -p 'build/sexos-kernel-boot-audit/obj/src'
	ln -f 'build/.objects/a29e7ba0f4e001a8/src/kernel_sources.cesta.o' "$@"

build/.objects/0893de9f728ff60d/src/network_config.cesta.o: src/network_config.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_KERNEL_BOOT_AUDIT=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_config.c' -o "$@"

build/sexos-kernel-boot-audit/obj/src/network_config.cesta.o: build/.objects/0893de9f728ff60d/src/network_config.cesta.o
	@mkdir -p 'build/sexos-kernel-boot-audit/obj/src'
	ln -f 'build/.objects/0893de9f728ff60d/src/network_config.cesta.o' "$@"

build/.objects/af47148609338177/src/pci_policy.cesta.o: src/pci_policy.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_KERNEL_BOOT_AUDIT=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/pci_policy.c' -o "$@"

build/sexos-kernel-boot-audit/obj/src/pci_policy.cesta.o: build/.objects/af47148609338177/src/pci_policy.cesta.o
	@mkdir -p 'build/sexos-kernel-boot-audit/obj/src'
	ln -f 'build/.objects/af47148609338177/src/pci_policy.cesta.o' "$@"

build/.objects/dfe1da6baa274c7e/src/kernel_pci.cesta.o: src/kernel_pci.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_KERNEL_BOOT_AUDIT=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_pci.c' -o "$@"

build/sexos-kernel-boot-audit/obj/src/kernel_pci.cesta.o: build/.objects/dfe1da6baa274c7e/src/kernel_pci.cesta.o
	@mkdir -p 'build/sexos-kernel-boot-audit/obj/src'
	ln -f 'build/.objects/dfe1da6baa274c7e/src/kernel_pci.cesta.o' "$@"

build/.objects/415d7776e3cfabb6/src/history_recovery.cesta.o: src/history_recovery.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_KERNEL_BOOT_AUDIT=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/history_recovery.c' -o "$@"

build/sexos-kernel-boot-audit/obj/src/history_recovery.cesta.o: build/.objects/415d7776e3cfabb6/src/history_recovery.cesta.o
	@mkdir -p 'build/sexos-kernel-boot-audit/obj/src'
	ln -f 'build/.objects/415d7776e3cfabb6/src/history_recovery.cesta.o' "$@"

build/.objects/b000d70e2e5f0f96/src/kernel_history.cesta.o: src/kernel_history.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_KERNEL_BOOT_AUDIT=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_history.c' -o "$@"

build/sexos-kernel-boot-audit/obj/src/kernel_history.cesta.o: build/.objects/b000d70e2e5f0f96/src/kernel_history.cesta.o
	@mkdir -p 'build/sexos-kernel-boot-audit/obj/src'
	ln -f 'build/.objects/b000d70e2e5f0f96/src/kernel_history.cesta.o' "$@"

build/.objects/bde128e4130da5fd/src/smp_park.cesta.o: src/smp_park.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_KERNEL_BOOT_AUDIT=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/smp_park.c' -o "$@"

build/sexos-kernel-boot-audit/obj/src/smp_park.cesta.o: build/.objects/bde128e4130da5fd/src/smp_park.cesta.o
	@mkdir -p 'build/sexos-kernel-boot-audit/obj/src'
	ln -f 'build/.objects/bde128e4130da5fd/src/smp_park.cesta.o' "$@"

build/.objects/1e3a5985427602ba/src/kernel_smp.cesta.o: src/kernel_smp.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_KERNEL_BOOT_AUDIT=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_smp.c' -o "$@"

build/sexos-kernel-boot-audit/obj/src/kernel_smp.cesta.o: build/.objects/1e3a5985427602ba/src/kernel_smp.cesta.o
	@mkdir -p 'build/sexos-kernel-boot-audit/obj/src'
	ln -f 'build/.objects/1e3a5985427602ba/src/kernel_smp.cesta.o' "$@"

build/.objects/69796f8406fe4af6/src/nvme_msix.cesta.o: src/nvme_msix.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_KERNEL_BOOT_AUDIT=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/nvme_msix.c' -o "$@"

build/sexos-kernel-boot-audit/obj/src/nvme_msix.cesta.o: build/.objects/69796f8406fe4af6/src/nvme_msix.cesta.o
	@mkdir -p 'build/sexos-kernel-boot-audit/obj/src'
	ln -f 'build/.objects/69796f8406fe4af6/src/nvme_msix.cesta.o' "$@"

build/.objects/c3e5f87c0dca49ef/runtime/kernel/kernel_log.c.o: runtime/kernel/kernel_log.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_KERNEL_BOOT_AUDIT=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/kernel/kernel_log.c' -o "$@"

build/sexos-kernel-boot-audit/obj/runtime/kernel/kernel_log.c.o: build/.objects/c3e5f87c0dca49ef/runtime/kernel/kernel_log.c.o
	@mkdir -p 'build/sexos-kernel-boot-audit/obj/runtime/kernel'
	ln -f 'build/.objects/c3e5f87c0dca49ef/runtime/kernel/kernel_log.c.o' "$@"

build/.objects/16a2cf044ddac257/runtime/kernel/entry_x86_64.S.o: runtime/kernel/entry_x86_64.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_KERNEL_BOOT_AUDIT=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/kernel/entry_x86_64.S' -o "$@"

build/sexos-kernel-boot-audit/obj/runtime/kernel/entry_x86_64.S.o: build/.objects/16a2cf044ddac257/runtime/kernel/entry_x86_64.S.o
	@mkdir -p 'build/sexos-kernel-boot-audit/obj/runtime/kernel'
	ln -f 'build/.objects/16a2cf044ddac257/runtime/kernel/entry_x86_64.S.o' "$@"

build/sexos-kernel-boot-audit/sexos-kernel-boot-audit: GNUmakefile build/sexos-kernel-boot-audit/obj/src/frame_lease.cesta.o build/sexos-kernel-boot-audit/obj/src/kernel_memory.cesta.o build/sexos-kernel-boot-audit/obj/src/program_memory.cesta.o build/sexos-kernel-boot-audit/obj/src/program_workers.cesta.o build/sexos-kernel-boot-audit/obj/src/kernel_workers.cesta.o build/sexos-kernel-boot-audit/obj/src/kernel_platform.cesta.o build/sexos-kernel-boot-audit/obj/src/kernel_domains.cesta.o build/sexos-kernel-boot-audit/obj/src/program_image.cesta.o build/sexos-kernel-boot-audit/obj/src/program_image_bytes.cesta.o build/sexos-kernel-boot-audit/obj/runtime/kernel/image_failure.c.o build/sexos-kernel-boot-audit/obj/src/kernel_entry.cesta.o build/sexos-kernel-boot-audit/obj/src/kernel_syscalls.cesta.o build/sexos-kernel-boot-audit/obj/src/kernel_rpc.cesta.o build/sexos-kernel-boot-audit/obj/src/memory_regions.cesta.o build/sexos-kernel-boot-audit/obj/src/region_writer.cesta.o build/sexos-kernel-boot-audit/obj/src/device_reset.cesta.o build/sexos-kernel-boot-audit/obj/src/epoch_journal.cesta.o build/sexos-kernel-boot-audit/obj/src/kernel_epoch_log.cesta.o build/sexos-kernel-boot-audit/obj/src/kernel_boot_memory.cesta.o build/sexos-kernel-boot-audit/obj/src/kernel_region_writer.cesta.o build/sexos-kernel-boot-audit/obj/src/kernel_device_interrupt.cesta.o build/sexos-kernel-boot-audit/obj/src/kernel_device_reset.cesta.o build/sexos-kernel-boot-audit/obj/src/app_private.cesta.o build/sexos-kernel-boot-audit/obj/src/kernel_apps.cesta.o build/sexos-kernel-boot-audit/obj/src/program_records.cesta.o build/sexos-kernel-boot-audit/obj/src/program_record_memory.cesta.o build/sexos-kernel-boot-audit/obj/src/program_record_tasks.cesta.o build/sexos-kernel-boot-audit/obj/src/kernel_records.cesta.o build/sexos-kernel-boot-audit/obj/src/kernel_current_boot.cesta.o build/sexos-kernel-boot-audit/obj/src/kernel_paging.cesta.o build/sexos-kernel-boot-audit/obj/src/kernel_task_wait.cesta.o build/sexos-kernel-boot-audit/obj/src/pager_state.cesta.o build/sexos-kernel-boot-audit/obj/src/source_policy.cesta.o build/sexos-kernel-boot-audit/obj/src/kernel_sources.cesta.o build/sexos-kernel-boot-audit/obj/src/network_config.cesta.o build/sexos-kernel-boot-audit/obj/src/pci_policy.cesta.o build/sexos-kernel-boot-audit/obj/src/kernel_pci.cesta.o build/sexos-kernel-boot-audit/obj/src/history_recovery.cesta.o build/sexos-kernel-boot-audit/obj/src/kernel_history.cesta.o build/sexos-kernel-boot-audit/obj/src/smp_park.cesta.o build/sexos-kernel-boot-audit/obj/src/kernel_smp.cesta.o build/sexos-kernel-boot-audit/obj/src/nvme_msix.cesta.o build/sexos-kernel-boot-audit/obj/runtime/kernel/kernel_log.c.o build/sexos-kernel-boot-audit/obj/runtime/kernel/entry_x86_64.S.o runtime/kernel/kernel.ld
	@mkdir -p "$(@D)"
	$(CC) 'build/sexos-kernel-boot-audit/obj/src/frame_lease.cesta.o' 'build/sexos-kernel-boot-audit/obj/src/kernel_memory.cesta.o' 'build/sexos-kernel-boot-audit/obj/src/program_memory.cesta.o' 'build/sexos-kernel-boot-audit/obj/src/program_workers.cesta.o' 'build/sexos-kernel-boot-audit/obj/src/kernel_workers.cesta.o' 'build/sexos-kernel-boot-audit/obj/src/kernel_platform.cesta.o' 'build/sexos-kernel-boot-audit/obj/src/kernel_domains.cesta.o' 'build/sexos-kernel-boot-audit/obj/src/program_image.cesta.o' 'build/sexos-kernel-boot-audit/obj/src/program_image_bytes.cesta.o' 'build/sexos-kernel-boot-audit/obj/runtime/kernel/image_failure.c.o' 'build/sexos-kernel-boot-audit/obj/src/kernel_entry.cesta.o' 'build/sexos-kernel-boot-audit/obj/src/kernel_syscalls.cesta.o' 'build/sexos-kernel-boot-audit/obj/src/kernel_rpc.cesta.o' 'build/sexos-kernel-boot-audit/obj/src/memory_regions.cesta.o' 'build/sexos-kernel-boot-audit/obj/src/region_writer.cesta.o' 'build/sexos-kernel-boot-audit/obj/src/device_reset.cesta.o' 'build/sexos-kernel-boot-audit/obj/src/epoch_journal.cesta.o' 'build/sexos-kernel-boot-audit/obj/src/kernel_epoch_log.cesta.o' 'build/sexos-kernel-boot-audit/obj/src/kernel_boot_memory.cesta.o' 'build/sexos-kernel-boot-audit/obj/src/kernel_region_writer.cesta.o' 'build/sexos-kernel-boot-audit/obj/src/kernel_device_interrupt.cesta.o' 'build/sexos-kernel-boot-audit/obj/src/kernel_device_reset.cesta.o' 'build/sexos-kernel-boot-audit/obj/src/app_private.cesta.o' 'build/sexos-kernel-boot-audit/obj/src/kernel_apps.cesta.o' 'build/sexos-kernel-boot-audit/obj/src/program_records.cesta.o' 'build/sexos-kernel-boot-audit/obj/src/program_record_memory.cesta.o' 'build/sexos-kernel-boot-audit/obj/src/program_record_tasks.cesta.o' 'build/sexos-kernel-boot-audit/obj/src/kernel_records.cesta.o' 'build/sexos-kernel-boot-audit/obj/src/kernel_current_boot.cesta.o' 'build/sexos-kernel-boot-audit/obj/src/kernel_paging.cesta.o' 'build/sexos-kernel-boot-audit/obj/src/kernel_task_wait.cesta.o' 'build/sexos-kernel-boot-audit/obj/src/pager_state.cesta.o' 'build/sexos-kernel-boot-audit/obj/src/source_policy.cesta.o' 'build/sexos-kernel-boot-audit/obj/src/kernel_sources.cesta.o' 'build/sexos-kernel-boot-audit/obj/src/network_config.cesta.o' 'build/sexos-kernel-boot-audit/obj/src/pci_policy.cesta.o' 'build/sexos-kernel-boot-audit/obj/src/kernel_pci.cesta.o' 'build/sexos-kernel-boot-audit/obj/src/history_recovery.cesta.o' 'build/sexos-kernel-boot-audit/obj/src/kernel_history.cesta.o' 'build/sexos-kernel-boot-audit/obj/src/smp_park.cesta.o' 'build/sexos-kernel-boot-audit/obj/src/kernel_smp.cesta.o' 'build/sexos-kernel-boot-audit/obj/src/nvme_msix.cesta.o' 'build/sexos-kernel-boot-audit/obj/runtime/kernel/kernel_log.c.o' 'build/sexos-kernel-boot-audit/obj/runtime/kernel/entry_x86_64.S.o' '-nostdlib' '-static' '-Wl,-T,runtime/kernel/kernel.ld' '-Wl,--build-id=none' '-Wl,-z,max-page-size=0x1000' -o "$@"

build/.objects/7cffa22ad4439fe7/src/frame_lease.cesta.o: src/frame_lease.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_SMP_EPOCH_CORRUPT_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/frame_lease.c' -o "$@"

build/sexos-kernel-smp-corrupt-probe/obj/src/frame_lease.cesta.o: build/.objects/7cffa22ad4439fe7/src/frame_lease.cesta.o
	@mkdir -p 'build/sexos-kernel-smp-corrupt-probe/obj/src'
	ln -f 'build/.objects/7cffa22ad4439fe7/src/frame_lease.cesta.o' "$@"

build/.objects/b87c12279a819c32/src/kernel_memory.cesta.o: src/kernel_memory.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_SMP_EPOCH_CORRUPT_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_memory.c' -o "$@"

build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_memory.cesta.o: build/.objects/b87c12279a819c32/src/kernel_memory.cesta.o
	@mkdir -p 'build/sexos-kernel-smp-corrupt-probe/obj/src'
	ln -f 'build/.objects/b87c12279a819c32/src/kernel_memory.cesta.o' "$@"

build/.objects/1528d478616ada09/src/program_memory.cesta.o: src/program_memory.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_SMP_EPOCH_CORRUPT_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_memory.c' -o "$@"

build/sexos-kernel-smp-corrupt-probe/obj/src/program_memory.cesta.o: build/.objects/1528d478616ada09/src/program_memory.cesta.o
	@mkdir -p 'build/sexos-kernel-smp-corrupt-probe/obj/src'
	ln -f 'build/.objects/1528d478616ada09/src/program_memory.cesta.o' "$@"

build/.objects/9905b13a93f1ad27/src/program_workers.cesta.o: src/program_workers.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_SMP_EPOCH_CORRUPT_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_workers.c' -o "$@"

build/sexos-kernel-smp-corrupt-probe/obj/src/program_workers.cesta.o: build/.objects/9905b13a93f1ad27/src/program_workers.cesta.o
	@mkdir -p 'build/sexos-kernel-smp-corrupt-probe/obj/src'
	ln -f 'build/.objects/9905b13a93f1ad27/src/program_workers.cesta.o' "$@"

build/.objects/22cd2c870dde1fd6/src/kernel_workers.cesta.o: src/kernel_workers.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_SMP_EPOCH_CORRUPT_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_workers.c' -o "$@"

build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_workers.cesta.o: build/.objects/22cd2c870dde1fd6/src/kernel_workers.cesta.o
	@mkdir -p 'build/sexos-kernel-smp-corrupt-probe/obj/src'
	ln -f 'build/.objects/22cd2c870dde1fd6/src/kernel_workers.cesta.o' "$@"

build/.objects/13ced179a4c6a33a/src/kernel_platform.cesta.o: src/kernel_platform.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_SMP_EPOCH_CORRUPT_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_platform.c' -o "$@"

build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_platform.cesta.o: build/.objects/13ced179a4c6a33a/src/kernel_platform.cesta.o
	@mkdir -p 'build/sexos-kernel-smp-corrupt-probe/obj/src'
	ln -f 'build/.objects/13ced179a4c6a33a/src/kernel_platform.cesta.o' "$@"

build/.objects/78c9e19f23ef4dc0/src/kernel_domains.cesta.o: src/kernel_domains.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_SMP_EPOCH_CORRUPT_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_domains.c' -o "$@"

build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_domains.cesta.o: build/.objects/78c9e19f23ef4dc0/src/kernel_domains.cesta.o
	@mkdir -p 'build/sexos-kernel-smp-corrupt-probe/obj/src'
	ln -f 'build/.objects/78c9e19f23ef4dc0/src/kernel_domains.cesta.o' "$@"

build/.objects/9651cfe8987a58e5/src/program_image.cesta.o: src/program_image.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_SMP_EPOCH_CORRUPT_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_image.c' -o "$@"

build/sexos-kernel-smp-corrupt-probe/obj/src/program_image.cesta.o: build/.objects/9651cfe8987a58e5/src/program_image.cesta.o
	@mkdir -p 'build/sexos-kernel-smp-corrupt-probe/obj/src'
	ln -f 'build/.objects/9651cfe8987a58e5/src/program_image.cesta.o' "$@"

build/.objects/31cbdabc276fde21/src/program_image_bytes.cesta.o: src/program_image_bytes.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_SMP_EPOCH_CORRUPT_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_image_bytes.c' -o "$@"

build/sexos-kernel-smp-corrupt-probe/obj/src/program_image_bytes.cesta.o: build/.objects/31cbdabc276fde21/src/program_image_bytes.cesta.o
	@mkdir -p 'build/sexos-kernel-smp-corrupt-probe/obj/src'
	ln -f 'build/.objects/31cbdabc276fde21/src/program_image_bytes.cesta.o' "$@"

build/.objects/72ad8d50e8079804/runtime/kernel/image_failure.c.o: runtime/kernel/image_failure.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_SMP_EPOCH_CORRUPT_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/kernel/image_failure.c' -o "$@"

build/sexos-kernel-smp-corrupt-probe/obj/runtime/kernel/image_failure.c.o: build/.objects/72ad8d50e8079804/runtime/kernel/image_failure.c.o
	@mkdir -p 'build/sexos-kernel-smp-corrupt-probe/obj/runtime/kernel'
	ln -f 'build/.objects/72ad8d50e8079804/runtime/kernel/image_failure.c.o' "$@"

build/.objects/31c53a389be871c9/src/kernel_entry.cesta.o: src/kernel_entry.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_SMP_EPOCH_CORRUPT_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_entry.c' -o "$@"

build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_entry.cesta.o: build/.objects/31c53a389be871c9/src/kernel_entry.cesta.o
	@mkdir -p 'build/sexos-kernel-smp-corrupt-probe/obj/src'
	ln -f 'build/.objects/31c53a389be871c9/src/kernel_entry.cesta.o' "$@"

build/.objects/c26f35445322d63b/src/kernel_syscalls.cesta.o: src/kernel_syscalls.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_SMP_EPOCH_CORRUPT_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_syscalls.c' -o "$@"

build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_syscalls.cesta.o: build/.objects/c26f35445322d63b/src/kernel_syscalls.cesta.o
	@mkdir -p 'build/sexos-kernel-smp-corrupt-probe/obj/src'
	ln -f 'build/.objects/c26f35445322d63b/src/kernel_syscalls.cesta.o' "$@"

build/.objects/1e178d6f5ccbf0f2/src/kernel_rpc.cesta.o: src/kernel_rpc.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_SMP_EPOCH_CORRUPT_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_rpc.c' -o "$@"

build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_rpc.cesta.o: build/.objects/1e178d6f5ccbf0f2/src/kernel_rpc.cesta.o
	@mkdir -p 'build/sexos-kernel-smp-corrupt-probe/obj/src'
	ln -f 'build/.objects/1e178d6f5ccbf0f2/src/kernel_rpc.cesta.o' "$@"

build/.objects/4c3edfad1775291e/src/memory_regions.cesta.o: src/memory_regions.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_SMP_EPOCH_CORRUPT_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/memory_regions.c' -o "$@"

build/sexos-kernel-smp-corrupt-probe/obj/src/memory_regions.cesta.o: build/.objects/4c3edfad1775291e/src/memory_regions.cesta.o
	@mkdir -p 'build/sexos-kernel-smp-corrupt-probe/obj/src'
	ln -f 'build/.objects/4c3edfad1775291e/src/memory_regions.cesta.o' "$@"

build/.objects/26c7289f162ad31d/src/region_writer.cesta.o: src/region_writer.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_SMP_EPOCH_CORRUPT_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/region_writer.c' -o "$@"

build/sexos-kernel-smp-corrupt-probe/obj/src/region_writer.cesta.o: build/.objects/26c7289f162ad31d/src/region_writer.cesta.o
	@mkdir -p 'build/sexos-kernel-smp-corrupt-probe/obj/src'
	ln -f 'build/.objects/26c7289f162ad31d/src/region_writer.cesta.o' "$@"

build/.objects/9a0a9c31cc535e95/src/device_reset.cesta.o: src/device_reset.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_SMP_EPOCH_CORRUPT_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/device_reset.c' -o "$@"

build/sexos-kernel-smp-corrupt-probe/obj/src/device_reset.cesta.o: build/.objects/9a0a9c31cc535e95/src/device_reset.cesta.o
	@mkdir -p 'build/sexos-kernel-smp-corrupt-probe/obj/src'
	ln -f 'build/.objects/9a0a9c31cc535e95/src/device_reset.cesta.o' "$@"

build/.objects/c283c0ab74868b16/src/epoch_journal.cesta.o: src/epoch_journal.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_SMP_EPOCH_CORRUPT_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/epoch_journal.c' -o "$@"

build/sexos-kernel-smp-corrupt-probe/obj/src/epoch_journal.cesta.o: build/.objects/c283c0ab74868b16/src/epoch_journal.cesta.o
	@mkdir -p 'build/sexos-kernel-smp-corrupt-probe/obj/src'
	ln -f 'build/.objects/c283c0ab74868b16/src/epoch_journal.cesta.o' "$@"

build/.objects/8c5b83d45ff03419/src/kernel_epoch_log.cesta.o: src/kernel_epoch_log.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_SMP_EPOCH_CORRUPT_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_epoch_log.c' -o "$@"

build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_epoch_log.cesta.o: build/.objects/8c5b83d45ff03419/src/kernel_epoch_log.cesta.o
	@mkdir -p 'build/sexos-kernel-smp-corrupt-probe/obj/src'
	ln -f 'build/.objects/8c5b83d45ff03419/src/kernel_epoch_log.cesta.o' "$@"

build/.objects/82c9b0925b0853ef/src/kernel_boot_memory.cesta.o: src/kernel_boot_memory.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_SMP_EPOCH_CORRUPT_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_boot_memory.c' -o "$@"

build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_boot_memory.cesta.o: build/.objects/82c9b0925b0853ef/src/kernel_boot_memory.cesta.o
	@mkdir -p 'build/sexos-kernel-smp-corrupt-probe/obj/src'
	ln -f 'build/.objects/82c9b0925b0853ef/src/kernel_boot_memory.cesta.o' "$@"

build/.objects/6d4704673e3f879f/src/kernel_region_writer.cesta.o: src/kernel_region_writer.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_SMP_EPOCH_CORRUPT_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_region_writer.c' -o "$@"

build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_region_writer.cesta.o: build/.objects/6d4704673e3f879f/src/kernel_region_writer.cesta.o
	@mkdir -p 'build/sexos-kernel-smp-corrupt-probe/obj/src'
	ln -f 'build/.objects/6d4704673e3f879f/src/kernel_region_writer.cesta.o' "$@"

build/.objects/af034ccbf6ef5d6b/src/kernel_device_interrupt.cesta.o: src/kernel_device_interrupt.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_SMP_EPOCH_CORRUPT_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_device_interrupt.c' -o "$@"

build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_device_interrupt.cesta.o: build/.objects/af034ccbf6ef5d6b/src/kernel_device_interrupt.cesta.o
	@mkdir -p 'build/sexos-kernel-smp-corrupt-probe/obj/src'
	ln -f 'build/.objects/af034ccbf6ef5d6b/src/kernel_device_interrupt.cesta.o' "$@"

build/.objects/ac87056cc54faa0f/src/kernel_device_reset.cesta.o: src/kernel_device_reset.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_SMP_EPOCH_CORRUPT_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_device_reset.c' -o "$@"

build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_device_reset.cesta.o: build/.objects/ac87056cc54faa0f/src/kernel_device_reset.cesta.o
	@mkdir -p 'build/sexos-kernel-smp-corrupt-probe/obj/src'
	ln -f 'build/.objects/ac87056cc54faa0f/src/kernel_device_reset.cesta.o' "$@"

build/.objects/4d9da9c2a8fd6918/src/app_private.cesta.o: src/app_private.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_SMP_EPOCH_CORRUPT_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/app_private.c' -o "$@"

build/sexos-kernel-smp-corrupt-probe/obj/src/app_private.cesta.o: build/.objects/4d9da9c2a8fd6918/src/app_private.cesta.o
	@mkdir -p 'build/sexos-kernel-smp-corrupt-probe/obj/src'
	ln -f 'build/.objects/4d9da9c2a8fd6918/src/app_private.cesta.o' "$@"

build/.objects/9efd4a6bed35bfed/src/kernel_apps.cesta.o: src/kernel_apps.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_SMP_EPOCH_CORRUPT_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_apps.c' -o "$@"

build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_apps.cesta.o: build/.objects/9efd4a6bed35bfed/src/kernel_apps.cesta.o
	@mkdir -p 'build/sexos-kernel-smp-corrupt-probe/obj/src'
	ln -f 'build/.objects/9efd4a6bed35bfed/src/kernel_apps.cesta.o' "$@"

build/.objects/83096829fd10411a/src/program_records.cesta.o: src/program_records.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_SMP_EPOCH_CORRUPT_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_records.c' -o "$@"

build/sexos-kernel-smp-corrupt-probe/obj/src/program_records.cesta.o: build/.objects/83096829fd10411a/src/program_records.cesta.o
	@mkdir -p 'build/sexos-kernel-smp-corrupt-probe/obj/src'
	ln -f 'build/.objects/83096829fd10411a/src/program_records.cesta.o' "$@"

build/.objects/b91577d00068697d/src/program_record_memory.cesta.o: src/program_record_memory.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_SMP_EPOCH_CORRUPT_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_record_memory.c' -o "$@"

build/sexos-kernel-smp-corrupt-probe/obj/src/program_record_memory.cesta.o: build/.objects/b91577d00068697d/src/program_record_memory.cesta.o
	@mkdir -p 'build/sexos-kernel-smp-corrupt-probe/obj/src'
	ln -f 'build/.objects/b91577d00068697d/src/program_record_memory.cesta.o' "$@"

build/.objects/51535cf765d3301e/src/program_record_tasks.cesta.o: src/program_record_tasks.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_SMP_EPOCH_CORRUPT_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_record_tasks.c' -o "$@"

build/sexos-kernel-smp-corrupt-probe/obj/src/program_record_tasks.cesta.o: build/.objects/51535cf765d3301e/src/program_record_tasks.cesta.o
	@mkdir -p 'build/sexos-kernel-smp-corrupt-probe/obj/src'
	ln -f 'build/.objects/51535cf765d3301e/src/program_record_tasks.cesta.o' "$@"

build/.objects/9ec458d8afd46333/src/kernel_records.cesta.o: src/kernel_records.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_SMP_EPOCH_CORRUPT_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_records.c' -o "$@"

build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_records.cesta.o: build/.objects/9ec458d8afd46333/src/kernel_records.cesta.o
	@mkdir -p 'build/sexos-kernel-smp-corrupt-probe/obj/src'
	ln -f 'build/.objects/9ec458d8afd46333/src/kernel_records.cesta.o' "$@"

build/.objects/33e1faba76c1893b/src/kernel_current_boot.cesta.o: src/kernel_current_boot.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_SMP_EPOCH_CORRUPT_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_current_boot.c' -o "$@"

build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_current_boot.cesta.o: build/.objects/33e1faba76c1893b/src/kernel_current_boot.cesta.o
	@mkdir -p 'build/sexos-kernel-smp-corrupt-probe/obj/src'
	ln -f 'build/.objects/33e1faba76c1893b/src/kernel_current_boot.cesta.o' "$@"

build/.objects/f3c6320fb3fe4241/src/kernel_paging.cesta.o: src/kernel_paging.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_SMP_EPOCH_CORRUPT_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_paging.c' -o "$@"

build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_paging.cesta.o: build/.objects/f3c6320fb3fe4241/src/kernel_paging.cesta.o
	@mkdir -p 'build/sexos-kernel-smp-corrupt-probe/obj/src'
	ln -f 'build/.objects/f3c6320fb3fe4241/src/kernel_paging.cesta.o' "$@"

build/.objects/0823a4733e976cc2/src/kernel_task_wait.cesta.o: src/kernel_task_wait.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_SMP_EPOCH_CORRUPT_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_task_wait.c' -o "$@"

build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_task_wait.cesta.o: build/.objects/0823a4733e976cc2/src/kernel_task_wait.cesta.o
	@mkdir -p 'build/sexos-kernel-smp-corrupt-probe/obj/src'
	ln -f 'build/.objects/0823a4733e976cc2/src/kernel_task_wait.cesta.o' "$@"

build/.objects/0cc2af90ae05a12a/src/pager_state.cesta.o: src/pager_state.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_SMP_EPOCH_CORRUPT_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/pager_state.c' -o "$@"

build/sexos-kernel-smp-corrupt-probe/obj/src/pager_state.cesta.o: build/.objects/0cc2af90ae05a12a/src/pager_state.cesta.o
	@mkdir -p 'build/sexos-kernel-smp-corrupt-probe/obj/src'
	ln -f 'build/.objects/0cc2af90ae05a12a/src/pager_state.cesta.o' "$@"

build/.objects/70e3b93c1a8d4d37/src/source_policy.cesta.o: src/source_policy.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_SMP_EPOCH_CORRUPT_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/source_policy.c' -o "$@"

build/sexos-kernel-smp-corrupt-probe/obj/src/source_policy.cesta.o: build/.objects/70e3b93c1a8d4d37/src/source_policy.cesta.o
	@mkdir -p 'build/sexos-kernel-smp-corrupt-probe/obj/src'
	ln -f 'build/.objects/70e3b93c1a8d4d37/src/source_policy.cesta.o' "$@"

build/.objects/59ba92bd68c736d3/src/kernel_sources.cesta.o: src/kernel_sources.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_SMP_EPOCH_CORRUPT_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_sources.c' -o "$@"

build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_sources.cesta.o: build/.objects/59ba92bd68c736d3/src/kernel_sources.cesta.o
	@mkdir -p 'build/sexos-kernel-smp-corrupt-probe/obj/src'
	ln -f 'build/.objects/59ba92bd68c736d3/src/kernel_sources.cesta.o' "$@"

build/.objects/54d255915fbd6702/src/network_config.cesta.o: src/network_config.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_SMP_EPOCH_CORRUPT_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_config.c' -o "$@"

build/sexos-kernel-smp-corrupt-probe/obj/src/network_config.cesta.o: build/.objects/54d255915fbd6702/src/network_config.cesta.o
	@mkdir -p 'build/sexos-kernel-smp-corrupt-probe/obj/src'
	ln -f 'build/.objects/54d255915fbd6702/src/network_config.cesta.o' "$@"

build/.objects/0bce08f80da1db08/src/pci_policy.cesta.o: src/pci_policy.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_SMP_EPOCH_CORRUPT_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/pci_policy.c' -o "$@"

build/sexos-kernel-smp-corrupt-probe/obj/src/pci_policy.cesta.o: build/.objects/0bce08f80da1db08/src/pci_policy.cesta.o
	@mkdir -p 'build/sexos-kernel-smp-corrupt-probe/obj/src'
	ln -f 'build/.objects/0bce08f80da1db08/src/pci_policy.cesta.o' "$@"

build/.objects/0b053bc99aab1705/src/kernel_pci.cesta.o: src/kernel_pci.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_SMP_EPOCH_CORRUPT_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_pci.c' -o "$@"

build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_pci.cesta.o: build/.objects/0b053bc99aab1705/src/kernel_pci.cesta.o
	@mkdir -p 'build/sexos-kernel-smp-corrupt-probe/obj/src'
	ln -f 'build/.objects/0b053bc99aab1705/src/kernel_pci.cesta.o' "$@"

build/.objects/6d2c76fc1e0e205d/src/history_recovery.cesta.o: src/history_recovery.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_SMP_EPOCH_CORRUPT_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/history_recovery.c' -o "$@"

build/sexos-kernel-smp-corrupt-probe/obj/src/history_recovery.cesta.o: build/.objects/6d2c76fc1e0e205d/src/history_recovery.cesta.o
	@mkdir -p 'build/sexos-kernel-smp-corrupt-probe/obj/src'
	ln -f 'build/.objects/6d2c76fc1e0e205d/src/history_recovery.cesta.o' "$@"

build/.objects/98cdd20a4b7831bd/src/kernel_history.cesta.o: src/kernel_history.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_SMP_EPOCH_CORRUPT_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_history.c' -o "$@"

build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_history.cesta.o: build/.objects/98cdd20a4b7831bd/src/kernel_history.cesta.o
	@mkdir -p 'build/sexos-kernel-smp-corrupt-probe/obj/src'
	ln -f 'build/.objects/98cdd20a4b7831bd/src/kernel_history.cesta.o' "$@"

build/.objects/30671ec126408852/src/smp_park.cesta.o: src/smp_park.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_SMP_EPOCH_CORRUPT_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/smp_park.c' -o "$@"

build/sexos-kernel-smp-corrupt-probe/obj/src/smp_park.cesta.o: build/.objects/30671ec126408852/src/smp_park.cesta.o
	@mkdir -p 'build/sexos-kernel-smp-corrupt-probe/obj/src'
	ln -f 'build/.objects/30671ec126408852/src/smp_park.cesta.o' "$@"

build/.objects/87d323aa32200bc9/src/kernel_smp.cesta.o: src/kernel_smp.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_SMP_EPOCH_CORRUPT_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_smp.c' -o "$@"

build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_smp.cesta.o: build/.objects/87d323aa32200bc9/src/kernel_smp.cesta.o
	@mkdir -p 'build/sexos-kernel-smp-corrupt-probe/obj/src'
	ln -f 'build/.objects/87d323aa32200bc9/src/kernel_smp.cesta.o' "$@"

build/.objects/cf2aa7fb645d1e9d/src/nvme_msix.cesta.o: src/nvme_msix.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_SMP_EPOCH_CORRUPT_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/nvme_msix.c' -o "$@"

build/sexos-kernel-smp-corrupt-probe/obj/src/nvme_msix.cesta.o: build/.objects/cf2aa7fb645d1e9d/src/nvme_msix.cesta.o
	@mkdir -p 'build/sexos-kernel-smp-corrupt-probe/obj/src'
	ln -f 'build/.objects/cf2aa7fb645d1e9d/src/nvme_msix.cesta.o' "$@"

build/.objects/e3ab2da04bbc9d40/runtime/kernel/kernel_log.c.o: runtime/kernel/kernel_log.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_SMP_EPOCH_CORRUPT_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/kernel/kernel_log.c' -o "$@"

build/sexos-kernel-smp-corrupt-probe/obj/runtime/kernel/kernel_log.c.o: build/.objects/e3ab2da04bbc9d40/runtime/kernel/kernel_log.c.o
	@mkdir -p 'build/sexos-kernel-smp-corrupt-probe/obj/runtime/kernel'
	ln -f 'build/.objects/e3ab2da04bbc9d40/runtime/kernel/kernel_log.c.o' "$@"

build/.objects/423f48a237a09d68/runtime/kernel/entry_x86_64.S.o: runtime/kernel/entry_x86_64.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_KERNEL_CURRENT_PDX=1' '-DSEXOS_PAGER_PARTITIONED_ALLOCATOR=1' '-DSEXOS_KERNEL_CURRENT_SMP=1' '-DSEXOS_SMP_EPOCH_CORRUPT_PROBE=1' '-DSEXOS_NVME_INTERRUPT_MODE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/kernel/entry_x86_64.S' -o "$@"

build/sexos-kernel-smp-corrupt-probe/obj/runtime/kernel/entry_x86_64.S.o: build/.objects/423f48a237a09d68/runtime/kernel/entry_x86_64.S.o
	@mkdir -p 'build/sexos-kernel-smp-corrupt-probe/obj/runtime/kernel'
	ln -f 'build/.objects/423f48a237a09d68/runtime/kernel/entry_x86_64.S.o' "$@"

build/sexos-kernel-smp-corrupt-probe/sexos-kernel-smp-corrupt-probe: GNUmakefile build/sexos-kernel-smp-corrupt-probe/obj/src/frame_lease.cesta.o build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_memory.cesta.o build/sexos-kernel-smp-corrupt-probe/obj/src/program_memory.cesta.o build/sexos-kernel-smp-corrupt-probe/obj/src/program_workers.cesta.o build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_workers.cesta.o build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_platform.cesta.o build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_domains.cesta.o build/sexos-kernel-smp-corrupt-probe/obj/src/program_image.cesta.o build/sexos-kernel-smp-corrupt-probe/obj/src/program_image_bytes.cesta.o build/sexos-kernel-smp-corrupt-probe/obj/runtime/kernel/image_failure.c.o build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_entry.cesta.o build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_syscalls.cesta.o build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_rpc.cesta.o build/sexos-kernel-smp-corrupt-probe/obj/src/memory_regions.cesta.o build/sexos-kernel-smp-corrupt-probe/obj/src/region_writer.cesta.o build/sexos-kernel-smp-corrupt-probe/obj/src/device_reset.cesta.o build/sexos-kernel-smp-corrupt-probe/obj/src/epoch_journal.cesta.o build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_epoch_log.cesta.o build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_boot_memory.cesta.o build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_region_writer.cesta.o build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_device_interrupt.cesta.o build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_device_reset.cesta.o build/sexos-kernel-smp-corrupt-probe/obj/src/app_private.cesta.o build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_apps.cesta.o build/sexos-kernel-smp-corrupt-probe/obj/src/program_records.cesta.o build/sexos-kernel-smp-corrupt-probe/obj/src/program_record_memory.cesta.o build/sexos-kernel-smp-corrupt-probe/obj/src/program_record_tasks.cesta.o build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_records.cesta.o build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_current_boot.cesta.o build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_paging.cesta.o build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_task_wait.cesta.o build/sexos-kernel-smp-corrupt-probe/obj/src/pager_state.cesta.o build/sexos-kernel-smp-corrupt-probe/obj/src/source_policy.cesta.o build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_sources.cesta.o build/sexos-kernel-smp-corrupt-probe/obj/src/network_config.cesta.o build/sexos-kernel-smp-corrupt-probe/obj/src/pci_policy.cesta.o build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_pci.cesta.o build/sexos-kernel-smp-corrupt-probe/obj/src/history_recovery.cesta.o build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_history.cesta.o build/sexos-kernel-smp-corrupt-probe/obj/src/smp_park.cesta.o build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_smp.cesta.o build/sexos-kernel-smp-corrupt-probe/obj/src/nvme_msix.cesta.o build/sexos-kernel-smp-corrupt-probe/obj/runtime/kernel/kernel_log.c.o build/sexos-kernel-smp-corrupt-probe/obj/runtime/kernel/entry_x86_64.S.o runtime/kernel/kernel.ld
	@mkdir -p "$(@D)"
	$(CC) 'build/sexos-kernel-smp-corrupt-probe/obj/src/frame_lease.cesta.o' 'build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_memory.cesta.o' 'build/sexos-kernel-smp-corrupt-probe/obj/src/program_memory.cesta.o' 'build/sexos-kernel-smp-corrupt-probe/obj/src/program_workers.cesta.o' 'build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_workers.cesta.o' 'build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_platform.cesta.o' 'build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_domains.cesta.o' 'build/sexos-kernel-smp-corrupt-probe/obj/src/program_image.cesta.o' 'build/sexos-kernel-smp-corrupt-probe/obj/src/program_image_bytes.cesta.o' 'build/sexos-kernel-smp-corrupt-probe/obj/runtime/kernel/image_failure.c.o' 'build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_entry.cesta.o' 'build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_syscalls.cesta.o' 'build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_rpc.cesta.o' 'build/sexos-kernel-smp-corrupt-probe/obj/src/memory_regions.cesta.o' 'build/sexos-kernel-smp-corrupt-probe/obj/src/region_writer.cesta.o' 'build/sexos-kernel-smp-corrupt-probe/obj/src/device_reset.cesta.o' 'build/sexos-kernel-smp-corrupt-probe/obj/src/epoch_journal.cesta.o' 'build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_epoch_log.cesta.o' 'build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_boot_memory.cesta.o' 'build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_region_writer.cesta.o' 'build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_device_interrupt.cesta.o' 'build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_device_reset.cesta.o' 'build/sexos-kernel-smp-corrupt-probe/obj/src/app_private.cesta.o' 'build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_apps.cesta.o' 'build/sexos-kernel-smp-corrupt-probe/obj/src/program_records.cesta.o' 'build/sexos-kernel-smp-corrupt-probe/obj/src/program_record_memory.cesta.o' 'build/sexos-kernel-smp-corrupt-probe/obj/src/program_record_tasks.cesta.o' 'build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_records.cesta.o' 'build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_current_boot.cesta.o' 'build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_paging.cesta.o' 'build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_task_wait.cesta.o' 'build/sexos-kernel-smp-corrupt-probe/obj/src/pager_state.cesta.o' 'build/sexos-kernel-smp-corrupt-probe/obj/src/source_policy.cesta.o' 'build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_sources.cesta.o' 'build/sexos-kernel-smp-corrupt-probe/obj/src/network_config.cesta.o' 'build/sexos-kernel-smp-corrupt-probe/obj/src/pci_policy.cesta.o' 'build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_pci.cesta.o' 'build/sexos-kernel-smp-corrupt-probe/obj/src/history_recovery.cesta.o' 'build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_history.cesta.o' 'build/sexos-kernel-smp-corrupt-probe/obj/src/smp_park.cesta.o' 'build/sexos-kernel-smp-corrupt-probe/obj/src/kernel_smp.cesta.o' 'build/sexos-kernel-smp-corrupt-probe/obj/src/nvme_msix.cesta.o' 'build/sexos-kernel-smp-corrupt-probe/obj/runtime/kernel/kernel_log.c.o' 'build/sexos-kernel-smp-corrupt-probe/obj/runtime/kernel/entry_x86_64.S.o' '-nostdlib' '-static' '-Wl,-T,runtime/kernel/kernel.ld' '-Wl,--build-id=none' '-Wl,-z,max-page-size=0x1000' -o "$@"

build/.objects/09bfeb12b2497b81/src/frame_lease.cesta.o: src/frame_lease.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/frame_lease.c' -o "$@"

build/sexos-kernel-network/obj/src/frame_lease.cesta.o: build/.objects/09bfeb12b2497b81/src/frame_lease.cesta.o
	@mkdir -p 'build/sexos-kernel-network/obj/src'
	ln -f 'build/.objects/09bfeb12b2497b81/src/frame_lease.cesta.o' "$@"

build/.objects/3410d1e3cbe41d10/src/kernel_memory.cesta.o: src/kernel_memory.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_memory.c' -o "$@"

build/sexos-kernel-network/obj/src/kernel_memory.cesta.o: build/.objects/3410d1e3cbe41d10/src/kernel_memory.cesta.o
	@mkdir -p 'build/sexos-kernel-network/obj/src'
	ln -f 'build/.objects/3410d1e3cbe41d10/src/kernel_memory.cesta.o' "$@"

build/.objects/67b28dd741c3381b/src/program_memory.cesta.o: src/program_memory.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_memory.c' -o "$@"

build/sexos-kernel-network/obj/src/program_memory.cesta.o: build/.objects/67b28dd741c3381b/src/program_memory.cesta.o
	@mkdir -p 'build/sexos-kernel-network/obj/src'
	ln -f 'build/.objects/67b28dd741c3381b/src/program_memory.cesta.o' "$@"

build/.objects/b3ea2afde0bfc7c1/src/program_workers.cesta.o: src/program_workers.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_workers.c' -o "$@"

build/sexos-kernel-network/obj/src/program_workers.cesta.o: build/.objects/b3ea2afde0bfc7c1/src/program_workers.cesta.o
	@mkdir -p 'build/sexos-kernel-network/obj/src'
	ln -f 'build/.objects/b3ea2afde0bfc7c1/src/program_workers.cesta.o' "$@"

build/.objects/a87fb56601f28c6c/src/kernel_workers.cesta.o: src/kernel_workers.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_workers.c' -o "$@"

build/sexos-kernel-network/obj/src/kernel_workers.cesta.o: build/.objects/a87fb56601f28c6c/src/kernel_workers.cesta.o
	@mkdir -p 'build/sexos-kernel-network/obj/src'
	ln -f 'build/.objects/a87fb56601f28c6c/src/kernel_workers.cesta.o' "$@"

build/.objects/a3b1ddb8b33faa88/src/kernel_platform.cesta.o: src/kernel_platform.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_platform.c' -o "$@"

build/sexos-kernel-network/obj/src/kernel_platform.cesta.o: build/.objects/a3b1ddb8b33faa88/src/kernel_platform.cesta.o
	@mkdir -p 'build/sexos-kernel-network/obj/src'
	ln -f 'build/.objects/a3b1ddb8b33faa88/src/kernel_platform.cesta.o' "$@"

build/.objects/641855720cc8cdb6/src/kernel_domains.cesta.o: src/kernel_domains.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_domains.c' -o "$@"

build/sexos-kernel-network/obj/src/kernel_domains.cesta.o: build/.objects/641855720cc8cdb6/src/kernel_domains.cesta.o
	@mkdir -p 'build/sexos-kernel-network/obj/src'
	ln -f 'build/.objects/641855720cc8cdb6/src/kernel_domains.cesta.o' "$@"

build/.objects/1178520fac410cef/src/program_image.cesta.o: src/program_image.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_image.c' -o "$@"

build/sexos-kernel-network/obj/src/program_image.cesta.o: build/.objects/1178520fac410cef/src/program_image.cesta.o
	@mkdir -p 'build/sexos-kernel-network/obj/src'
	ln -f 'build/.objects/1178520fac410cef/src/program_image.cesta.o' "$@"

build/.objects/2535ea340f396583/src/program_image_bytes.cesta.o: src/program_image_bytes.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_image_bytes.c' -o "$@"

build/sexos-kernel-network/obj/src/program_image_bytes.cesta.o: build/.objects/2535ea340f396583/src/program_image_bytes.cesta.o
	@mkdir -p 'build/sexos-kernel-network/obj/src'
	ln -f 'build/.objects/2535ea340f396583/src/program_image_bytes.cesta.o' "$@"

build/.objects/8baa5a08aeb9b3b2/runtime/kernel/image_failure.c.o: runtime/kernel/image_failure.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/kernel/image_failure.c' -o "$@"

build/sexos-kernel-network/obj/runtime/kernel/image_failure.c.o: build/.objects/8baa5a08aeb9b3b2/runtime/kernel/image_failure.c.o
	@mkdir -p 'build/sexos-kernel-network/obj/runtime/kernel'
	ln -f 'build/.objects/8baa5a08aeb9b3b2/runtime/kernel/image_failure.c.o' "$@"

build/.objects/5a4eeb8b1bd46adb/src/kernel_entry.cesta.o: src/kernel_entry.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_entry.c' -o "$@"

build/sexos-kernel-network/obj/src/kernel_entry.cesta.o: build/.objects/5a4eeb8b1bd46adb/src/kernel_entry.cesta.o
	@mkdir -p 'build/sexos-kernel-network/obj/src'
	ln -f 'build/.objects/5a4eeb8b1bd46adb/src/kernel_entry.cesta.o' "$@"

build/.objects/0f3596d06cfd5efd/src/kernel_syscalls.cesta.o: src/kernel_syscalls.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_syscalls.c' -o "$@"

build/sexos-kernel-network/obj/src/kernel_syscalls.cesta.o: build/.objects/0f3596d06cfd5efd/src/kernel_syscalls.cesta.o
	@mkdir -p 'build/sexos-kernel-network/obj/src'
	ln -f 'build/.objects/0f3596d06cfd5efd/src/kernel_syscalls.cesta.o' "$@"

build/.objects/e2fcc53f015a02d0/src/kernel_rpc.cesta.o: src/kernel_rpc.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_rpc.c' -o "$@"

build/sexos-kernel-network/obj/src/kernel_rpc.cesta.o: build/.objects/e2fcc53f015a02d0/src/kernel_rpc.cesta.o
	@mkdir -p 'build/sexos-kernel-network/obj/src'
	ln -f 'build/.objects/e2fcc53f015a02d0/src/kernel_rpc.cesta.o' "$@"

build/.objects/a09a3114a6028c84/src/memory_regions.cesta.o: src/memory_regions.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/memory_regions.c' -o "$@"

build/sexos-kernel-network/obj/src/memory_regions.cesta.o: build/.objects/a09a3114a6028c84/src/memory_regions.cesta.o
	@mkdir -p 'build/sexos-kernel-network/obj/src'
	ln -f 'build/.objects/a09a3114a6028c84/src/memory_regions.cesta.o' "$@"

build/.objects/a844c7b166db5cd7/src/region_writer.cesta.o: src/region_writer.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/region_writer.c' -o "$@"

build/sexos-kernel-network/obj/src/region_writer.cesta.o: build/.objects/a844c7b166db5cd7/src/region_writer.cesta.o
	@mkdir -p 'build/sexos-kernel-network/obj/src'
	ln -f 'build/.objects/a844c7b166db5cd7/src/region_writer.cesta.o' "$@"

build/.objects/90604c9ebe9afd5f/src/device_reset.cesta.o: src/device_reset.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/device_reset.c' -o "$@"

build/sexos-kernel-network/obj/src/device_reset.cesta.o: build/.objects/90604c9ebe9afd5f/src/device_reset.cesta.o
	@mkdir -p 'build/sexos-kernel-network/obj/src'
	ln -f 'build/.objects/90604c9ebe9afd5f/src/device_reset.cesta.o' "$@"

build/.objects/b2071afe6e07f6ac/src/epoch_journal.cesta.o: src/epoch_journal.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/epoch_journal.c' -o "$@"

build/sexos-kernel-network/obj/src/epoch_journal.cesta.o: build/.objects/b2071afe6e07f6ac/src/epoch_journal.cesta.o
	@mkdir -p 'build/sexos-kernel-network/obj/src'
	ln -f 'build/.objects/b2071afe6e07f6ac/src/epoch_journal.cesta.o' "$@"

build/.objects/df0678e9bcd1fb6b/src/kernel_epoch_log.cesta.o: src/kernel_epoch_log.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_epoch_log.c' -o "$@"

build/sexos-kernel-network/obj/src/kernel_epoch_log.cesta.o: build/.objects/df0678e9bcd1fb6b/src/kernel_epoch_log.cesta.o
	@mkdir -p 'build/sexos-kernel-network/obj/src'
	ln -f 'build/.objects/df0678e9bcd1fb6b/src/kernel_epoch_log.cesta.o' "$@"

build/.objects/430f35f76588f199/src/kernel_boot_memory.cesta.o: src/kernel_boot_memory.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_boot_memory.c' -o "$@"

build/sexos-kernel-network/obj/src/kernel_boot_memory.cesta.o: build/.objects/430f35f76588f199/src/kernel_boot_memory.cesta.o
	@mkdir -p 'build/sexos-kernel-network/obj/src'
	ln -f 'build/.objects/430f35f76588f199/src/kernel_boot_memory.cesta.o' "$@"

build/.objects/a161c12c02ecd8c9/src/kernel_region_writer.cesta.o: src/kernel_region_writer.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_region_writer.c' -o "$@"

build/sexos-kernel-network/obj/src/kernel_region_writer.cesta.o: build/.objects/a161c12c02ecd8c9/src/kernel_region_writer.cesta.o
	@mkdir -p 'build/sexos-kernel-network/obj/src'
	ln -f 'build/.objects/a161c12c02ecd8c9/src/kernel_region_writer.cesta.o' "$@"

build/.objects/8143c69a2d57132d/src/kernel_device_interrupt.cesta.o: src/kernel_device_interrupt.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_device_interrupt.c' -o "$@"

build/sexos-kernel-network/obj/src/kernel_device_interrupt.cesta.o: build/.objects/8143c69a2d57132d/src/kernel_device_interrupt.cesta.o
	@mkdir -p 'build/sexos-kernel-network/obj/src'
	ln -f 'build/.objects/8143c69a2d57132d/src/kernel_device_interrupt.cesta.o' "$@"

build/.objects/aae207fa2c8d9eb9/src/kernel_device_reset.cesta.o: src/kernel_device_reset.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_device_reset.c' -o "$@"

build/sexos-kernel-network/obj/src/kernel_device_reset.cesta.o: build/.objects/aae207fa2c8d9eb9/src/kernel_device_reset.cesta.o
	@mkdir -p 'build/sexos-kernel-network/obj/src'
	ln -f 'build/.objects/aae207fa2c8d9eb9/src/kernel_device_reset.cesta.o' "$@"

build/.objects/4a47e88da6675f3e/src/app_private.cesta.o: src/app_private.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/app_private.c' -o "$@"

build/sexos-kernel-network/obj/src/app_private.cesta.o: build/.objects/4a47e88da6675f3e/src/app_private.cesta.o
	@mkdir -p 'build/sexos-kernel-network/obj/src'
	ln -f 'build/.objects/4a47e88da6675f3e/src/app_private.cesta.o' "$@"

build/.objects/50eaa5984c76f427/src/kernel_apps.cesta.o: src/kernel_apps.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_apps.c' -o "$@"

build/sexos-kernel-network/obj/src/kernel_apps.cesta.o: build/.objects/50eaa5984c76f427/src/kernel_apps.cesta.o
	@mkdir -p 'build/sexos-kernel-network/obj/src'
	ln -f 'build/.objects/50eaa5984c76f427/src/kernel_apps.cesta.o' "$@"

build/.objects/95a85035e44298e8/src/program_records.cesta.o: src/program_records.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_records.c' -o "$@"

build/sexos-kernel-network/obj/src/program_records.cesta.o: build/.objects/95a85035e44298e8/src/program_records.cesta.o
	@mkdir -p 'build/sexos-kernel-network/obj/src'
	ln -f 'build/.objects/95a85035e44298e8/src/program_records.cesta.o' "$@"

build/.objects/2c67389215c30237/src/program_record_memory.cesta.o: src/program_record_memory.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_record_memory.c' -o "$@"

build/sexos-kernel-network/obj/src/program_record_memory.cesta.o: build/.objects/2c67389215c30237/src/program_record_memory.cesta.o
	@mkdir -p 'build/sexos-kernel-network/obj/src'
	ln -f 'build/.objects/2c67389215c30237/src/program_record_memory.cesta.o' "$@"

build/.objects/e7f31f25c34acb84/src/program_record_tasks.cesta.o: src/program_record_tasks.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/program_record_tasks.c' -o "$@"

build/sexos-kernel-network/obj/src/program_record_tasks.cesta.o: build/.objects/e7f31f25c34acb84/src/program_record_tasks.cesta.o
	@mkdir -p 'build/sexos-kernel-network/obj/src'
	ln -f 'build/.objects/e7f31f25c34acb84/src/program_record_tasks.cesta.o' "$@"

build/.objects/0b8c2670e8e5bbe5/src/kernel_records.cesta.o: src/kernel_records.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_records.c' -o "$@"

build/sexos-kernel-network/obj/src/kernel_records.cesta.o: build/.objects/0b8c2670e8e5bbe5/src/kernel_records.cesta.o
	@mkdir -p 'build/sexos-kernel-network/obj/src'
	ln -f 'build/.objects/0b8c2670e8e5bbe5/src/kernel_records.cesta.o' "$@"

build/.objects/8eceb612dfd3a9fd/src/kernel_current_boot.cesta.o: src/kernel_current_boot.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_current_boot.c' -o "$@"

build/sexos-kernel-network/obj/src/kernel_current_boot.cesta.o: build/.objects/8eceb612dfd3a9fd/src/kernel_current_boot.cesta.o
	@mkdir -p 'build/sexos-kernel-network/obj/src'
	ln -f 'build/.objects/8eceb612dfd3a9fd/src/kernel_current_boot.cesta.o' "$@"

build/.objects/802a1dd6d6e5d923/src/kernel_paging.cesta.o: src/kernel_paging.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_paging.c' -o "$@"

build/sexos-kernel-network/obj/src/kernel_paging.cesta.o: build/.objects/802a1dd6d6e5d923/src/kernel_paging.cesta.o
	@mkdir -p 'build/sexos-kernel-network/obj/src'
	ln -f 'build/.objects/802a1dd6d6e5d923/src/kernel_paging.cesta.o' "$@"

build/.objects/e1bd7873d5ff9f60/src/kernel_task_wait.cesta.o: src/kernel_task_wait.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_task_wait.c' -o "$@"

build/sexos-kernel-network/obj/src/kernel_task_wait.cesta.o: build/.objects/e1bd7873d5ff9f60/src/kernel_task_wait.cesta.o
	@mkdir -p 'build/sexos-kernel-network/obj/src'
	ln -f 'build/.objects/e1bd7873d5ff9f60/src/kernel_task_wait.cesta.o' "$@"

build/.objects/8a2ec29333cc89f8/src/pager_state.cesta.o: src/pager_state.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/pager_state.c' -o "$@"

build/sexos-kernel-network/obj/src/pager_state.cesta.o: build/.objects/8a2ec29333cc89f8/src/pager_state.cesta.o
	@mkdir -p 'build/sexos-kernel-network/obj/src'
	ln -f 'build/.objects/8a2ec29333cc89f8/src/pager_state.cesta.o' "$@"

build/.objects/3df8e33e6f898c51/src/source_policy.cesta.o: src/source_policy.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/source_policy.c' -o "$@"

build/sexos-kernel-network/obj/src/source_policy.cesta.o: build/.objects/3df8e33e6f898c51/src/source_policy.cesta.o
	@mkdir -p 'build/sexos-kernel-network/obj/src'
	ln -f 'build/.objects/3df8e33e6f898c51/src/source_policy.cesta.o' "$@"

build/.objects/f42779a309c20985/src/kernel_sources.cesta.o: src/kernel_sources.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_sources.c' -o "$@"

build/sexos-kernel-network/obj/src/kernel_sources.cesta.o: build/.objects/f42779a309c20985/src/kernel_sources.cesta.o
	@mkdir -p 'build/sexos-kernel-network/obj/src'
	ln -f 'build/.objects/f42779a309c20985/src/kernel_sources.cesta.o' "$@"

build/.objects/f6217fa807c7d1a0/src/network_config.cesta.o: src/network_config.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_config.c' -o "$@"

build/sexos-kernel-network/obj/src/network_config.cesta.o: build/.objects/f6217fa807c7d1a0/src/network_config.cesta.o
	@mkdir -p 'build/sexos-kernel-network/obj/src'
	ln -f 'build/.objects/f6217fa807c7d1a0/src/network_config.cesta.o' "$@"

build/.objects/f3644a368e270dae/src/pci_policy.cesta.o: src/pci_policy.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/pci_policy.c' -o "$@"

build/sexos-kernel-network/obj/src/pci_policy.cesta.o: build/.objects/f3644a368e270dae/src/pci_policy.cesta.o
	@mkdir -p 'build/sexos-kernel-network/obj/src'
	ln -f 'build/.objects/f3644a368e270dae/src/pci_policy.cesta.o' "$@"

build/.objects/a0c5117fff6dd70f/src/kernel_pci.cesta.o: src/kernel_pci.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_pci.c' -o "$@"

build/sexos-kernel-network/obj/src/kernel_pci.cesta.o: build/.objects/a0c5117fff6dd70f/src/kernel_pci.cesta.o
	@mkdir -p 'build/sexos-kernel-network/obj/src'
	ln -f 'build/.objects/a0c5117fff6dd70f/src/kernel_pci.cesta.o' "$@"

build/.objects/eb02612c40b82e17/src/history_recovery.cesta.o: src/history_recovery.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/history_recovery.c' -o "$@"

build/sexos-kernel-network/obj/src/history_recovery.cesta.o: build/.objects/eb02612c40b82e17/src/history_recovery.cesta.o
	@mkdir -p 'build/sexos-kernel-network/obj/src'
	ln -f 'build/.objects/eb02612c40b82e17/src/history_recovery.cesta.o' "$@"

build/.objects/3fa443e197af8677/src/kernel_history.cesta.o: src/kernel_history.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_history.c' -o "$@"

build/sexos-kernel-network/obj/src/kernel_history.cesta.o: build/.objects/3fa443e197af8677/src/kernel_history.cesta.o
	@mkdir -p 'build/sexos-kernel-network/obj/src'
	ln -f 'build/.objects/3fa443e197af8677/src/kernel_history.cesta.o' "$@"

build/.objects/e1985e6411290336/runtime/kernel/kernel_log.c.o: runtime/kernel/kernel_log.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/kernel/kernel_log.c' -o "$@"

build/sexos-kernel-network/obj/runtime/kernel/kernel_log.c.o: build/.objects/e1985e6411290336/runtime/kernel/kernel_log.c.o
	@mkdir -p 'build/sexos-kernel-network/obj/runtime/kernel'
	ln -f 'build/.objects/e1985e6411290336/runtime/kernel/kernel_log.c.o' "$@"

build/.objects/0a335b9e8f88498e/runtime/kernel/entry_x86_64.S.o: runtime/kernel/entry_x86_64.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/kernel/entry_x86_64.S' -o "$@"

build/sexos-kernel-network/obj/runtime/kernel/entry_x86_64.S.o: build/.objects/0a335b9e8f88498e/runtime/kernel/entry_x86_64.S.o
	@mkdir -p 'build/sexos-kernel-network/obj/runtime/kernel'
	ln -f 'build/.objects/0a335b9e8f88498e/runtime/kernel/entry_x86_64.S.o' "$@"

build/sexos-kernel-network/sexos-kernel-network: GNUmakefile build/sexos-kernel-network/obj/src/frame_lease.cesta.o build/sexos-kernel-network/obj/src/kernel_memory.cesta.o build/sexos-kernel-network/obj/src/program_memory.cesta.o build/sexos-kernel-network/obj/src/program_workers.cesta.o build/sexos-kernel-network/obj/src/kernel_workers.cesta.o build/sexos-kernel-network/obj/src/kernel_platform.cesta.o build/sexos-kernel-network/obj/src/kernel_domains.cesta.o build/sexos-kernel-network/obj/src/program_image.cesta.o build/sexos-kernel-network/obj/src/program_image_bytes.cesta.o build/sexos-kernel-network/obj/runtime/kernel/image_failure.c.o build/sexos-kernel-network/obj/src/kernel_entry.cesta.o build/sexos-kernel-network/obj/src/kernel_syscalls.cesta.o build/sexos-kernel-network/obj/src/kernel_rpc.cesta.o build/sexos-kernel-network/obj/src/memory_regions.cesta.o build/sexos-kernel-network/obj/src/region_writer.cesta.o build/sexos-kernel-network/obj/src/device_reset.cesta.o build/sexos-kernel-network/obj/src/epoch_journal.cesta.o build/sexos-kernel-network/obj/src/kernel_epoch_log.cesta.o build/sexos-kernel-network/obj/src/kernel_boot_memory.cesta.o build/sexos-kernel-network/obj/src/kernel_region_writer.cesta.o build/sexos-kernel-network/obj/src/kernel_device_interrupt.cesta.o build/sexos-kernel-network/obj/src/kernel_device_reset.cesta.o build/sexos-kernel-network/obj/src/app_private.cesta.o build/sexos-kernel-network/obj/src/kernel_apps.cesta.o build/sexos-kernel-network/obj/src/program_records.cesta.o build/sexos-kernel-network/obj/src/program_record_memory.cesta.o build/sexos-kernel-network/obj/src/program_record_tasks.cesta.o build/sexos-kernel-network/obj/src/kernel_records.cesta.o build/sexos-kernel-network/obj/src/kernel_current_boot.cesta.o build/sexos-kernel-network/obj/src/kernel_paging.cesta.o build/sexos-kernel-network/obj/src/kernel_task_wait.cesta.o build/sexos-kernel-network/obj/src/pager_state.cesta.o build/sexos-kernel-network/obj/src/source_policy.cesta.o build/sexos-kernel-network/obj/src/kernel_sources.cesta.o build/sexos-kernel-network/obj/src/network_config.cesta.o build/sexos-kernel-network/obj/src/pci_policy.cesta.o build/sexos-kernel-network/obj/src/kernel_pci.cesta.o build/sexos-kernel-network/obj/src/history_recovery.cesta.o build/sexos-kernel-network/obj/src/kernel_history.cesta.o build/sexos-kernel-network/obj/runtime/kernel/kernel_log.c.o build/sexos-kernel-network/obj/runtime/kernel/entry_x86_64.S.o runtime/kernel/kernel.ld
	@mkdir -p "$(@D)"
	$(CC) 'build/sexos-kernel-network/obj/src/frame_lease.cesta.o' 'build/sexos-kernel-network/obj/src/kernel_memory.cesta.o' 'build/sexos-kernel-network/obj/src/program_memory.cesta.o' 'build/sexos-kernel-network/obj/src/program_workers.cesta.o' 'build/sexos-kernel-network/obj/src/kernel_workers.cesta.o' 'build/sexos-kernel-network/obj/src/kernel_platform.cesta.o' 'build/sexos-kernel-network/obj/src/kernel_domains.cesta.o' 'build/sexos-kernel-network/obj/src/program_image.cesta.o' 'build/sexos-kernel-network/obj/src/program_image_bytes.cesta.o' 'build/sexos-kernel-network/obj/runtime/kernel/image_failure.c.o' 'build/sexos-kernel-network/obj/src/kernel_entry.cesta.o' 'build/sexos-kernel-network/obj/src/kernel_syscalls.cesta.o' 'build/sexos-kernel-network/obj/src/kernel_rpc.cesta.o' 'build/sexos-kernel-network/obj/src/memory_regions.cesta.o' 'build/sexos-kernel-network/obj/src/region_writer.cesta.o' 'build/sexos-kernel-network/obj/src/device_reset.cesta.o' 'build/sexos-kernel-network/obj/src/epoch_journal.cesta.o' 'build/sexos-kernel-network/obj/src/kernel_epoch_log.cesta.o' 'build/sexos-kernel-network/obj/src/kernel_boot_memory.cesta.o' 'build/sexos-kernel-network/obj/src/kernel_region_writer.cesta.o' 'build/sexos-kernel-network/obj/src/kernel_device_interrupt.cesta.o' 'build/sexos-kernel-network/obj/src/kernel_device_reset.cesta.o' 'build/sexos-kernel-network/obj/src/app_private.cesta.o' 'build/sexos-kernel-network/obj/src/kernel_apps.cesta.o' 'build/sexos-kernel-network/obj/src/program_records.cesta.o' 'build/sexos-kernel-network/obj/src/program_record_memory.cesta.o' 'build/sexos-kernel-network/obj/src/program_record_tasks.cesta.o' 'build/sexos-kernel-network/obj/src/kernel_records.cesta.o' 'build/sexos-kernel-network/obj/src/kernel_current_boot.cesta.o' 'build/sexos-kernel-network/obj/src/kernel_paging.cesta.o' 'build/sexos-kernel-network/obj/src/kernel_task_wait.cesta.o' 'build/sexos-kernel-network/obj/src/pager_state.cesta.o' 'build/sexos-kernel-network/obj/src/source_policy.cesta.o' 'build/sexos-kernel-network/obj/src/kernel_sources.cesta.o' 'build/sexos-kernel-network/obj/src/network_config.cesta.o' 'build/sexos-kernel-network/obj/src/pci_policy.cesta.o' 'build/sexos-kernel-network/obj/src/kernel_pci.cesta.o' 'build/sexos-kernel-network/obj/src/history_recovery.cesta.o' 'build/sexos-kernel-network/obj/src/kernel_history.cesta.o' 'build/sexos-kernel-network/obj/runtime/kernel/kernel_log.c.o' 'build/sexos-kernel-network/obj/runtime/kernel/entry_x86_64.S.o' '-nostdlib' '-static' '-Wl,-T,runtime/kernel/kernel.ld' '-Wl,--build-id=none' '-Wl,-z,max-page-size=0x1000' -o "$@"

-include build/.objects/3056f357363e2d28/src/kaleidoscope.cesta.o.d build/.objects/1972bc0eb8330e3b/src/kaleidoscope_body.cesta.o.d build/.objects/1c25e7c040b59cf7/src/kaleidoscope_web_page.cesta.o.d build/.objects/a107cbf27579b4e8/src/kaleidoscope_decimal.cesta.o.d build/.objects/2f3c865ef8a0b1cd/src/kaleidoscope_script_lexer.cesta.o.d build/.objects/ca12216c321cdcc8/src/kaleidoscope_regex.cesta.o.d build/.objects/a4810fb4ef4fcafa/src/kaleidoscope_regex_match.cesta.o.d build/.objects/5a1b94b120a66954/src/kaleidoscope_script_core.cesta.o.d build/.objects/1d1d75e5b87d331e/src/kaleidoscope_html.cesta.o.d build/.objects/800aaa8ad5cc7e60/runtime/client_runtime.c.o.d build/.objects/bc299f33d79e4eb4/runtime/traps_x86_64.c.o.d build/.objects/eb0ca2bfcbe92530/runtime/start_x86_64.S.o.d build/.objects/e20b5db439e268d1/generated/runtime/format_bytes.c.o.d build/.objects/ccefbd874e02dbc0/generated/runtime/packed_words.c.o.d build/.objects/3a47b50c3706c99c/src/kaleidoscope_navigation.cesta.o.d build/.objects/1fad969ee0e0b087/src/kaleidoscope_regex.cesta.o.d build/.objects/612754aef6f9a07d/src/kaleidoscope_regex_match.cesta.o.d build/.objects/70e762eb5967f38a/src/sexdisplay_font.cesta.o.d build/.objects/c6120ba0e3abde30/src/kaleidoscope_decimal.cesta.o.d build/.objects/4c04085f5f58523f/src/kaleidoscope_script_lexer.cesta.o.d build/.objects/6f1bd58338b211d0/src/kaleidoscope_regex.cesta.o.d build/.objects/07b8b4c281f4ed82/src/kaleidoscope_regex_match.cesta.o.d build/.objects/a4d68bbd3dc3d604/src/kaleidoscope_script_core.cesta.o.d build/.objects/224eff0969acfd2e/src/kaleidoscope_html.cesta.o.d build/.objects/be688e4577b7ad7f/src/kaleidoscope_qoi.cesta.o.d build/.objects/bc54e5dcedfd5c53/src/kaleidoscope_bitmap.cesta.o.d build/.objects/29052c55faa1738e/src/kaleidoscope_presentation.cesta.o.d build/.objects/527608ff2658c286/src/kaleidoscope_asset_view.cesta.o.d build/.objects/1349797934031919/src/kaleidoscope_asset_view_render.cesta.o.d build/.objects/4fe491afb201ac77/src/kaleidoscope_png.cesta.o.d build/.objects/7f51cb38bd8f22fb/src/kaleidoscope_asset_pipeline.cesta.o.d build/.objects/809a7111a7845347/src/silkbar.cesta.o.d build/.objects/644594c8a824e27d/src/silkbar_model.cesta.o.d build/.objects/c869434c2f4d31e2/runtime/client_runtime.c.o.d build/.objects/c75551cc96199ab6/runtime/server_runtime.c.o.d build/.objects/01af3a7d099c537e/runtime/traps_x86_64.c.o.d build/.objects/79bc4f70b09c47a2/runtime/server_traps_x86_64.c.o.d build/.objects/96f32a455f237b12/runtime/start_x86_64.S.o.d build/.objects/b7384930acaa6d45/generated/runtime/format_bytes.c.o.d build/.objects/0fd89aeff9cdcc82/generated/runtime/packed_words.c.o.d build/.objects/a4707781d0fe720b/src/sexinput.cesta.o.d build/.objects/8e204911891a6d92/runtime/client_runtime.c.o.d build/.objects/367aded7cdd027c6/runtime/server_runtime.c.o.d build/.objects/b1bc55baa915f5fe/runtime/traps_x86_64.c.o.d build/.objects/733636f7ad779852/runtime/server_traps_x86_64.c.o.d build/.objects/92b82526b742e042/runtime/start_x86_64.S.o.d build/.objects/f464c6d4274f99cb/generated/runtime/format_bytes.c.o.d build/.objects/f6f1dfeeae4fb732/generated/runtime/packed_words.c.o.d build/.objects/3da5541b3486e217/src/sexstore.cesta.o.d build/.objects/29a1bf533b092787/src/sexstore_durable.cesta.o.d build/.objects/dfa365f626df1cec/src/sexstore_pages.cesta.o.d build/.objects/616a435a57c97fd7/runtime/client_runtime.c.o.d build/.objects/f3bb2267bde9d17b/runtime/server_runtime.c.o.d build/.objects/a6447b1b0ca15303/runtime/traps_x86_64.c.o.d build/.objects/eaeb859be8f1a397/runtime/server_traps_x86_64.c.o.d build/.objects/9388a08a4ba779c7/runtime/start_x86_64.S.o.d build/.objects/2256bede4842d5b6/generated/runtime/format_bytes.c.o.d build/.objects/6ea72e92e9c9c277/generated/runtime/packed_words.c.o.d build/.objects/5a0ca417f3760ac3/src/sexdisplay.cesta.o.d build/.objects/d163714ba77f2519/src/sexdisplay_bitmap.cesta.o.d build/.objects/51a0a620f55af961/src/silkbar_model.cesta.o.d build/.objects/d4e2dae5387473b8/runtime/client_runtime.c.o.d build/.objects/f36b8bc462ecfa64/runtime/server_runtime.c.o.d build/.objects/735c91cfb0ba6bec/runtime/traps_x86_64.c.o.d build/.objects/90661e54f0ff0e78/runtime/server_traps_x86_64.c.o.d build/.objects/3b76cd050b006be8/runtime/start_x86_64.S.o.d build/.objects/8fdc6fb557af24e9/generated/runtime/format_bytes.c.o.d build/.objects/29a4f676195a7f58/generated/runtime/packed_words.c.o.d build/.objects/cbfa40cf4adae574/src/sexdisplay_bitmap.cesta.o.d build/.objects/c32e109efccfdf91/src/sexbell.cesta.o.d build/.objects/e590269888e2aed5/runtime/client_runtime.c.o.d build/.objects/3c786e56085eb211/runtime/server_runtime.c.o.d build/.objects/19d1d0dc6c4f3179/runtime/traps_x86_64.c.o.d build/.objects/b75bfcf4e4ee0495/runtime/server_traps_x86_64.c.o.d build/.objects/30e9ad6134f44bc5/runtime/start_x86_64.S.o.d build/.objects/e04ea3d5602080ba/generated/runtime/format_bytes.c.o.d build/.objects/dfbceadf90464d75/generated/runtime/packed_words.c.o.d build/.objects/b54de591e04b8483/src/linen.cesta.o.d build/.objects/f2deeab1d24f3148/src/linen_capture.cesta.o.d build/.objects/35aeca1326bc696c/src/linen_capture_service.cesta.o.d build/.objects/5f82a4e6a0f85973/src/rpc_runtime.cesta.o.d build/.objects/861cde55e9a62468/src/rpc_wait.cesta.o.d build/.objects/9e77e25e23e6f0d8/runtime/client_runtime.c.o.d build/.objects/bf205fd1161e0994/runtime/server_runtime.c.o.d build/.objects/64ea149318e130cc/runtime/traps_x86_64.c.o.d build/.objects/e2ef32fe3aa90c98/runtime/server_traps_x86_64.c.o.d build/.objects/ceaa54f05ca06720/runtime/rpc_traps_x86_64.c.o.d build/.objects/a7e6e981437c9708/runtime/start_x86_64.S.o.d build/.objects/2e907a8b132a7513/generated/runtime/format_bytes.c.o.d build/.objects/8db73e788b389278/generated/runtime/packed_words.c.o.d build/.objects/e6e743d2e51d98c1/src/quil.cesta.o.d build/.objects/967b854a771372e4/src/quil_autosave.cesta.o.d build/.objects/4af8542b27172520/src/document_protocol.cesta.o.d build/.objects/f6736279b51b0377/src/document_client.cesta.o.d build/.objects/7d9a3f61c646846d/src/document_client_transport.cesta.o.d build/.objects/a3633f27b5a48530/src/document_snapshot.cesta.o.d build/.objects/a67c74ccb841b23e/src/rpc_runtime.cesta.o.d build/.objects/ed8587918fa4593f/src/rpc_wait.cesta.o.d build/.objects/567cdb011585538f/runtime/client_runtime.c.o.d build/.objects/c372a10d7926abc3/runtime/server_runtime.c.o.d build/.objects/b2b435410352384b/runtime/traps_x86_64.c.o.d build/.objects/919b8a939ffe92cf/runtime/server_traps_x86_64.c.o.d build/.objects/df7b10a9764042a7/runtime/rpc_traps_x86_64.c.o.d build/.objects/872afc0b3e76c564/runtime/region_writer_traps_x86_64.c.o.d build/.objects/9846a42140dda85f/runtime/start_x86_64.S.o.d build/.objects/57549f378afff75e/generated/runtime/format_bytes.c.o.d build/.objects/ca90890dd233acaf/generated/runtime/packed_words.c.o.d build/.objects/8d61c75b319d78ae/src/quil_autosave.cesta.o.d build/.objects/7fcbc0ca2406e722/src/document_protocol.cesta.o.d build/.objects/69833e4346244ca7/src/document_client.cesta.o.d build/.objects/6e57d784b623fd71/src/document_client_transport.cesta.o.d build/.objects/c8bd7e292e644bfb/src/document_service.cesta.o.d build/.objects/933a7f8e0b3d2b47/src/document_journal.cesta.o.d build/.objects/93eaae1ca9c1c847/src/document_growth.cesta.o.d build/.objects/c5c998aa5253694d/src/document_service_transport.cesta.o.d build/.objects/e2ae61b39ea3c2f2/src/document_snapshot.cesta.o.d build/.objects/1da44c0c23c00b90/src/document_protocol_probe.cesta.o.d build/.objects/4941a6e194a218c3/src/document_protocol.cesta.o.d build/.objects/6c2b96870a90f612/src/document_client.cesta.o.d build/.objects/fb5a930cc96e8a28/src/document_client_transport.cesta.o.d build/.objects/b733e1665e827bb3/src/document_snapshot.cesta.o.d build/.objects/7c41d7cfa87d853a/src/rpc_wait.cesta.o.d build/.objects/d1d0f9eb1b544bea/runtime/client_runtime.c.o.d build/.objects/40a447f3259e3f56/runtime/traps_x86_64.c.o.d build/.objects/08e77be7fa965362/runtime/rpc_traps_x86_64.c.o.d build/.objects/2635fe4b77574e67/runtime/region_writer_traps_x86_64.c.o.d build/.objects/c834544658f5d3da/runtime/start_x86_64.S.o.d build/.objects/63cf55c2f3f07271/generated/runtime/format_bytes.c.o.d build/.objects/6d833c8edacc9e8a/generated/runtime/packed_words.c.o.d build/.objects/d785b029746545d1/src/document_protocol_probe.cesta.o.d build/.objects/7dec28beb62b6864/src/document_protocol.cesta.o.d build/.objects/e398c10d6c59ec97/src/document_client.cesta.o.d build/.objects/1be682a7a1074799/src/document_client_transport.cesta.o.d build/.objects/11139b4a9cbb0bd4/src/document_snapshot.cesta.o.d build/.objects/f65ba791e6bb22af/src/rpc_wait.cesta.o.d build/.objects/d63925bdebff395f/runtime/client_runtime.c.o.d build/.objects/df96042043fed73b/runtime/traps_x86_64.c.o.d build/.objects/c580a0a18a826d67/runtime/rpc_traps_x86_64.c.o.d build/.objects/97802513469cb058/runtime/region_writer_traps_x86_64.c.o.d build/.objects/b1bb41c86efb000f/runtime/start_x86_64.S.o.d build/.objects/998a0cfbe8acaf56/generated/runtime/format_bytes.c.o.d build/.objects/be25464ea86ec63f/generated/runtime/packed_words.c.o.d build/.objects/d78fb029746d9a56/src/document_protocol_probe.cesta.o.d build/.objects/7df028beb62f513b/src/document_protocol.cesta.o.d build/.objects/e394c10d6c5603c0/src/document_client.cesta.o.d build/.objects/1bf082a7a10f9c1e/src/document_client_transport.cesta.o.d build/.objects/11159b4a9cbb8eab/src/document_snapshot.cesta.o.d build/.objects/f659a791e6ba9fd8/src/rpc_wait.cesta.o.d build/.objects/d63525bdebfb5088/runtime/client_runtime.c.o.d build/.objects/df92042043faee64/runtime/traps_x86_64.c.o.d build/.objects/c57ca0a18a7e8490/runtime/rpc_traps_x86_64.c.o.d build/.objects/9784251346a0992f/runtime/region_writer_traps_x86_64.c.o.d build/.objects/b1b941c86efa7d38/runtime/start_x86_64.S.o.d build/.objects/99800cfbe8a45ad1/generated/runtime/format_bytes.c.o.d build/.objects/be23464ea86e4368/generated/runtime/packed_words.c.o.d build/.objects/8e2e67d20378e132/src/record_hum.cesta.o.d build/.objects/757081c64bd6cb57/runtime/client_runtime.c.o.d build/.objects/e563e048b79b0ebb/runtime/traps_x86_64.c.o.d build/.objects/0cacde62a2c593c7/runtime/start_x86_64.S.o.d build/.objects/ab2be8f41e7f3fde/generated/runtime/format_bytes.c.o.d build/.objects/66f8f98cff1ead37/generated/runtime/packed_words.c.o.d build/.objects/52fc098079fcb95c/src/spindle.cesta.o.d build/.objects/75db67f3e9848c96/src/spindle_calc.cesta.o.d build/.objects/d7be94a0a029098c/src/spindle_complete.cesta.o.d build/.objects/23cce5806f327186/src/spindle_sextime.cesta.o.d build/.objects/dddc42035b51937b/src/sex_graphics.cesta.o.d build/.objects/27be57ed3e65269a/runtime/client_runtime.c.o.d build/.objects/2c22230e9be0fc46/runtime/server_runtime.c.o.d build/.objects/18850ed1142857de/runtime/traps_x86_64.c.o.d build/.objects/5e509e36fc953c5a/runtime/server_traps_x86_64.c.o.d build/.objects/4e134fb9c40170aa/runtime/start_x86_64.S.o.d build/.objects/5d6bafa70b42dddf/generated/runtime/format_bytes.c.o.d build/.objects/327bb8f3cd58df7a/generated/runtime/packed_words.c.o.d build/.objects/e8d2d6b86865479b/src/spindle.cesta.o.d build/.objects/5595892e3f8886e5/src/spindle_calc.cesta.o.d build/.objects/4e8ccaa66276deeb/src/spindle_complete.cesta.o.d build/.objects/183770f0daf9e2b5/src/spindle_sextime.cesta.o.d build/.objects/f4801b387e8f4576/src/sex_graphics.cesta.o.d build/.objects/668f501f0f732f79/runtime/client_runtime.c.o.d build/.objects/dacd0e8e92b959f5/runtime/server_runtime.c.o.d build/.objects/56587e500d19c58d/runtime/traps_x86_64.c.o.d build/.objects/b2a6ca10e76c75b9/runtime/server_traps_x86_64.c.o.d build/.objects/a48a511cb9bab169/runtime/start_x86_64.S.o.d build/.objects/daa01083cfab945a/generated/runtime/format_bytes.c.o.d build/.objects/0363f0644b24ca99/generated/runtime/packed_words.c.o.d build/.objects/d86b5d7c92ac583b/src/purple_scanout.cesta.o.d build/.objects/198bfc2a6887f5d4/runtime/client_runtime.c.o.d build/.objects/721d3e11079126b8/runtime/server_runtime.c.o.d build/.objects/579d8bde38143d80/runtime/traps_x86_64.c.o.d build/.objects/e2fd53136869b094/runtime/server_traps_x86_64.c.o.d build/.objects/ababdd2e7be5ac24/runtime/start_x86_64.S.o.d build/.objects/5b78eb1fd76d46e9/generated/runtime/format_bytes.c.o.d build/.objects/e073844c54607b34/generated/runtime/packed_words.c.o.d build/.objects/a71dcfa714c2611a/src/sexfiles.cesta.o.d build/.objects/67e6322d79d7d8d5/src/sexfiles_program.cesta.o.d build/.objects/652b5208ddec474f/src/sexfiles_diskfs.cesta.o.d build/.objects/f2dee059fde3f3db/src/sexfiles_proof.cesta.o.d build/.objects/61aac9c69a721938/src/epoch_journal.cesta.o.d build/.objects/d8612c2ee33724f0/src/epoch_files.cesta.o.d build/.objects/3f4c10f006f3c47b/src/document_protocol.cesta.o.d build/.objects/e6b803bb93331c00/src/document_service.cesta.o.d build/.objects/b05d122bd97a0eca/src/document_service_transport.cesta.o.d build/.objects/3c37a074f28fce1c/src/document_journal.cesta.o.d build/.objects/34261f61d9037d1c/src/document_growth.cesta.o.d build/.objects/f9f4117f3919a0cb/src/document_snapshot.cesta.o.d build/.objects/27568d615786cf8d/src/rpc_runtime.cesta.o.d build/.objects/68b44764937f6e24/src/rpc_wait.cesta.o.d build/.objects/d903d6cff3919f34/runtime/client_runtime.c.o.d build/.objects/5ef762284da990b0/runtime/server_runtime.c.o.d build/.objects/9690092e4e97d9f8/runtime/traps_x86_64.c.o.d build/.objects/13b8f6b879b84a74/runtime/server_traps_x86_64.c.o.d build/.objects/2ba6e3868998506c/runtime/rpc_traps_x86_64.c.o.d build/.objects/20dd3d9935959ae7/runtime/region_writer_traps_x86_64.c.o.d build/.objects/8ee070b022ad5d02/runtime/epoch_journal_traps_x86_64.c.o.d build/.objects/d38c6856f3db1c44/runtime/start_x86_64.S.o.d build/.objects/d112c55955f5cfad/generated/runtime/format_bytes.c.o.d build/.objects/0497411883005dd4/generated/runtime/packed_words.c.o.d build/.objects/ded14289bd6ffb6a/src/sexdrive.cesta.o.d build/.objects/f10ea48f21e5f97d/src/dma_lease.cesta.o.d build/.objects/b2ab4820a183cf6f/runtime/client_runtime.c.o.d build/.objects/273a41d80e61617b/runtime/server_runtime.c.o.d build/.objects/dd141473aba81703/runtime/traps_x86_64.c.o.d build/.objects/6d5a207547ad60af/runtime/server_traps_x86_64.c.o.d build/.objects/e3594cccb735b440/runtime/mmio_traps_x86_64.c.o.d build/.objects/6335af194ac06a10/runtime/region_writer_traps_x86_64.c.o.d build/.objects/ffd66f93799ff460/runtime/device_reset_traps_x86_64.c.o.d build/.objects/47af5b783d9f5d5f/runtime/start_x86_64.S.o.d build/.objects/ec7e430756ff7eea/generated/runtime/format_bytes.c.o.d build/.objects/19026752e82df00f/generated/runtime/packed_words.c.o.d build/.objects/c58c3ef33fbf0241/src/sexdrive.cesta.o.d build/.objects/45534eaf6053e878/src/dma_lease.cesta.o.d build/.objects/00ed6375ad789e52/runtime/client_runtime.c.o.d build/.objects/febfd909519f4546/runtime/server_runtime.c.o.d build/.objects/3f46cb5331d3980e/runtime/traps_x86_64.c.o.d build/.objects/f9e9e5a0c33a6912/runtime/server_traps_x86_64.c.o.d build/.objects/8a07b2959889e697/runtime/mmio_traps_x86_64.c.o.d build/.objects/53fc252d5a204da7/runtime/region_writer_traps_x86_64.c.o.d build/.objects/d009dc578e808a77/runtime/device_reset_traps_x86_64.c.o.d build/.objects/8aa8337660d78a42/runtime/start_x86_64.S.o.d build/.objects/23464f8895a6b5c1/generated/runtime/format_bytes.c.o.d build/.objects/fc900b04fb7fc832/generated/runtime/packed_words.c.o.d build/.objects/f6a25d0e84faf1c7/src/sexusb.cesta.o.d build/.objects/ec5fcc705520ae04/runtime/client_runtime.c.o.d build/.objects/4567609367161408/runtime/server_runtime.c.o.d build/.objects/c2c4c2609090b560/runtime/traps_x86_64.c.o.d build/.objects/ae45c1d7f694b9c4/runtime/server_traps_x86_64.c.o.d build/.objects/3ea50ea4a0c6ed63/runtime/mmio_traps_x86_64.c.o.d build/.objects/f3bdc7dbddccc9d4/runtime/start_x86_64.S.o.d build/.objects/e1b1fc4fbeebe9e9/generated/runtime/format_bytes.c.o.d build/.objects/403db740e8ab71a4/generated/runtime/packed_words.c.o.d build/.objects/8e915d7bafb6cc07/src/sexnet.cesta.o.d build/.objects/3eceb5b4af6d5333/src/network_job_abi.cesta.o.d build/.objects/9f5695410c728086/src/network_job_wire.cesta.o.d build/.objects/92248878f05028ac/src/network_job_machine.cesta.o.d build/.objects/ef555c840897231f/src/network_dns_job_abi.cesta.o.d build/.objects/2f1cad95d02ec6ba/src/network_dns_job_wire.cesta.o.d build/.objects/68690a23e82ab490/src/network_dns_job_machine.cesta.o.d build/.objects/627cca4e7a94f945/src/network_config.cesta.o.d build/.objects/180d67fa00ac4d6f/src/network_engine_frame.cesta.o.d build/.objects/0e577519b3af1656/src/network_engine_packet.cesta.o.d build/.objects/bfb9b98f70f2e79e/src/network_engine_tcp_packet.cesta.o.d build/.objects/bf0af8f1cf595e28/src/network_tcp_client.cesta.o.d build/.objects/90030264f91ff89f/src/network_tcp_listener.cesta.o.d build/.objects/7d46a94af557ea20/src/network_engine_neighbor.cesta.o.d build/.objects/007a6cdb4c056c6f/src/network_dns_codec.cesta.o.d build/.objects/3d1826956711f7c9/src/network_engine_dns.cesta.o.d build/.objects/251617bc120f4e67/src/network_engine_transport.cesta.o.d build/.objects/b826d5b0cd4a9c54/src/network_job_engine_policy.cesta.o.d build/.objects/8d741e2cb603a265/src/network_job_engine_adapter.cesta.o.d build/.objects/343a1fa44876c3d1/src/network_dns_job_engine_adapter.cesta.o.d build/.objects/5b6b39e572778898/src/network_service_native.cesta.o.d build/.objects/4844fe243adc1f24/src/network_service_core.cesta.o.d build/.objects/9b96c825ed2f0ada/src/network_dns_job_liveness.cesta.o.d build/.objects/334680c75bbc97b5/runtime/client_runtime.c.o.d build/.objects/51b5e008a03562e9/runtime/server_runtime.c.o.d build/.objects/4db7939580c31531/runtime/traps_x86_64.c.o.d build/.objects/9f4aebdd267a9f75/runtime/server_traps_x86_64.c.o.d build/.objects/318879ba7803ad1a/runtime/mmio_traps_x86_64.c.o.d build/.objects/31690fe2979d5065/runtime/start_x86_64.S.o.d build/.objects/e2878123870062f4/generated/runtime/format_bytes.c.o.d build/.objects/87246b97ef475b55/generated/runtime/packed_words.c.o.d build/.objects/fb5d138511f21649/src/network_browser.cesta.o.d build/.objects/d1f208141784c2b3/src/network_observation.cesta.o.d build/.objects/6f996727ff1d4d6e/src/rpc_runtime.cesta.o.d build/.objects/35ba9b4754952b21/src/rpc_wait.cesta.o.d build/.objects/0622bc66be197e91/runtime/client_runtime.c.o.d build/.objects/9f80bccf9067b7fd/runtime/traps_x86_64.c.o.d build/.objects/5f0134b7a12a8351/runtime/server_traps_x86_64.c.o.d build/.objects/6b8e8352ea7353a9/runtime/rpc_traps_x86_64.c.o.d build/.objects/a1ff3500248a8601/runtime/start_x86_64.S.o.d build/.objects/9d7649c9c3c893ce/generated/runtime/format_bytes.c.o.d build/.objects/df75ded7d79ff471/generated/runtime/packed_words.c.o.d build/.objects/e41d83cef6424d0f/src/network_protocol_probe.cesta.o.d build/.objects/757f1163dd766626/src/network_job_abi.cesta.o.d build/.objects/2065401b6e4a3754/runtime/client_runtime.c.o.d build/.objects/fb850bf0b3040b20/runtime/traps_x86_64.c.o.d build/.objects/62b68b96fb4136cc/runtime/rpc_traps_x86_64.c.o.d build/.objects/5c5874eb51449524/runtime/start_x86_64.S.o.d build/.objects/ab68de999b00d795/generated/runtime/format_bytes.c.o.d build/.objects/7df85f02daf82174/generated/runtime/packed_words.c.o.d build/.objects/a07cec5d3fb9aabb/src/network_dns_protocol_probe.cesta.o.d build/.objects/1ba90a6b9ea3a2ca/src/network_job_abi.cesta.o.d build/.objects/47acb57384d55ede/src/network_dns_job_abi.cesta.o.d build/.objects/be689d27fca8f7cc/runtime/client_runtime.c.o.d build/.objects/ad95784d31e23250/runtime/traps_x86_64.c.o.d build/.objects/6b01f4ff1fc05e84/runtime/rpc_traps_x86_64.c.o.d build/.objects/c95c4058e3224e9c/runtime/start_x86_64.S.o.d build/.objects/a25f538c8426ed69/generated/runtime/format_bytes.c.o.d build/.objects/4cf545cfffc6ad2c/generated/runtime/packed_words.c.o.d build/.objects/df60facc18c94c03/src/network_web_protocol_probe.cesta.o.d build/.objects/8c984f9b72b9fac7/runtime/client_runtime.c.o.d build/.objects/faedc500b2bd3d4b/runtime/traps_x86_64.c.o.d build/.objects/91c9bae0bd9b384f/runtime/rpc_traps_x86_64.c.o.d build/.objects/5dd3e89018e476c8/runtime/region_writer_traps_x86_64.c.o.d build/.objects/481a512481549417/runtime/start_x86_64.S.o.d build/.objects/8214953be79f5b9a/generated/runtime/format_bytes.c.o.d build/.objects/f37ae42ff52bd3a7/generated/runtime/packed_words.c.o.d build/.objects/2565dd9b70a0b331/src/network_service.cesta.o.d build/.objects/f628801e19b283de/src/network_job_abi.cesta.o.d build/.objects/54199538d8dd3d59/src/network_job_wire.cesta.o.d build/.objects/69535348fb8c5667/src/network_job_machine.cesta.o.d build/.objects/0fc56426d0f730f2/src/network_dns_job_abi.cesta.o.d build/.objects/0b3273d33499b445/src/network_dns_job_wire.cesta.o.d build/.objects/699f78e15ecc21e3/src/network_dns_job_machine.cesta.o.d build/.objects/332737ee2fe018d8/src/network_config.cesta.o.d build/.objects/14dbd5787041ff42/src/network_engine_frame.cesta.o.d build/.objects/142af6a4f489b2e9/src/network_engine_packet.cesta.o.d build/.objects/dea3277b5387a731/src/network_engine_tcp_packet.cesta.o.d build/.objects/50e34292d452047b/src/network_tcp_client.cesta.o.d build/.objects/dbeb7d6943ff3572/src/network_tcp_listener.cesta.o.d build/.objects/e2472e0d064a9df3/src/network_engine_neighbor.cesta.o.d build/.objects/0130c0d83dd4ec42/src/network_dns_codec.cesta.o.d build/.objects/e036755d896258f4/src/network_engine_dns.cesta.o.d build/.objects/0c3ebca6f73775fa/src/network_engine_transport.cesta.o.d build/.objects/e409a38a58f96baf/src/network_job_engine_policy.cesta.o.d build/.objects/ca6caa9541a5b878/src/network_job_engine_adapter.cesta.o.d build/.objects/9ed3f9531faad2dc/src/network_dns_job_engine_adapter.cesta.o.d build/.objects/8ded30649fa5c82b/src/network_service_native.cesta.o.d build/.objects/4b3ea6c898a05d7f/src/network_service_core.cesta.o.d build/.objects/2e7297d1bfc0eb86/src/network_observation.cesta.o.d build/.objects/2eca027f28e860b4/src/network_device.cesta.o.d build/.objects/c0525384d4ebc8a8/src/network_send.cesta.o.d build/.objects/2c5ae818c12d805d/src/network_tcp.cesta.o.d build/.objects/d595698e080c773b/src/network_service_current.cesta.o.d build/.objects/013604783d72683a/src/network_device_current.cesta.o.d build/.objects/28f04fbb785ec5e5/src/network_dns_job_liveness.cesta.o.d build/.objects/61c64007a77b2c02/src/network_open.cesta.o.d build/.objects/3ac8744a3128946a/src/network_web.cesta.o.d build/.objects/ea2e92f3eb2369b7/src/network_web_open.cesta.o.d build/.objects/df1c96f29e62b5b6/src/network_http.cesta.o.d build/.objects/b2b5feb99edd962f/src/rpc_runtime.cesta.o.d build/.objects/4117e732fa7ec958/src/rpc_wait.cesta.o.d build/.objects/520d834280a17088/runtime/client_runtime.c.o.d build/.objects/8d95a521a7d69314/runtime/server_runtime.c.o.d build/.objects/bfbd66c63d674ebc/runtime/traps_x86_64.c.o.d build/.objects/747b0553b0799748/runtime/server_traps_x86_64.c.o.d build/.objects/db9c855e5f7eaf60/runtime/rpc_traps_x86_64.c.o.d build/.objects/f155396b47b68e25/runtime/mmio_traps_x86_64.c.o.d build/.objects/90d48cf0db1adc75/runtime/region_writer_traps_x86_64.c.o.d build/.objects/b07f5bf2d462e278/runtime/start_x86_64.S.o.d build/.objects/e550ec13e0bae2cf/generated/runtime/format_bytes.c.o.d build/.objects/9aa710a4d2226628/generated/runtime/packed_words.c.o.d build/.objects/882cdb57c9f38e61/src/network_job_abi.cesta.o.d build/.objects/9a8ea406c8ee08f2/src/network_job_wire.cesta.o.d build/.objects/8cf505d61d8c47fc/src/network_job_machine.cesta.o.d build/.objects/0659b3adc088e6d5/src/network_dns_codec.cesta.o.d build/.objects/6c2f8c263229c465/src/network_dns_job_abi.cesta.o.d build/.objects/3f1f9e1b3175245e/src/network_dns_job_wire.cesta.o.d build/.objects/e195fe6a39f61bd8/src/network_dns_job_machine.cesta.o.d build/.objects/fe7b6b3b3451bb7e/src/network_dns_job_liveness.cesta.o.d build/.objects/919a04ca0db8978f/src/network_config.cesta.o.d build/.objects/024cac7ba69fe1d5/src/network_engine_frame.cesta.o.d build/.objects/e88be3f5062b9662/src/network_engine_packet.cesta.o.d build/.objects/85f56b2ae0eb5d88/src/network_engine_neighbor.cesta.o.d build/.objects/6b81973b41eac44b/src/network_engine_dns.cesta.o.d build/.objects/888b66ccba42b1ca/src/network_engine_tcp_packet.cesta.o.d build/.objects/822fca1c24096d10/src/network_tcp_client.cesta.o.d build/.objects/e75fefcd8742c9e5/src/network_tcp_listener.cesta.o.d build/.objects/0138692a3b4b9fed/src/network_engine_transport.cesta.o.d build/.objects/5c0ac8f940799344/src/network_job_engine_policy.cesta.o.d build/.objects/6e95b75cc7ef0c2f/src/network_job_engine_adapter.cesta.o.d build/.objects/c8089ce18a168ed3/src/network_dns_job_engine_adapter.cesta.o.d build/.objects/555b6a8cebc42ee0/src/network_service_native.cesta.o.d build/.objects/920258a11d0aaf74/src/network_service_core.cesta.o.d build/.objects/300853d6e2bbcec4/src/source_policy.cesta.o.d build/.objects/37d4a9726e925612/src/history_recovery.cesta.o.d build/.objects/91cc18d4e0c928cd/src/history_decode.cesta.o.d build/.objects/9cb7a3a73eee98c5/runtime/history_native.c.o.d build/.objects/8f327338554e95fd/src/pci_policy.cesta.o.d build/.objects/7ad7c8f21aeaab74/src/frame_lease.cesta.o.d build/.objects/714ae0b7f43fd359/src/program_workers.cesta.o.d build/.objects/bd58d94e1fe2b977/src/program_records.cesta.o.d build/.objects/a8a44464b4c79f6f/src/program_record_memory.cesta.o.d build/.objects/5307e9de474d2ad0/src/program_record_tasks.cesta.o.d build/.objects/5b3d5ea5fc624ea3/src/program_memory.cesta.o.d build/.objects/41a91d0b62db45c8/src/epoch_journal.cesta.o.d build/.objects/117192a38961a030/src/epoch_files.cesta.o.d build/.objects/5e0404e689a31107/src/pager_state.cesta.o.d build/.objects/ff6c348ceaffb34d/src/app_private.cesta.o.d build/.objects/73827375c911ad0b/src/memory_regions.cesta.o.d build/.objects/f5acdd013d8ef9d2/src/region_writer.cesta.o.d build/.objects/2fbc420df317f099/src/dma_lease.cesta.o.d build/.objects/b89c7c2a2c6d067a/src/device_reset.cesta.o.d build/.objects/7d8f4be1d506cd52/src/nvme_msix.cesta.o.d build/.objects/321ac8cbb497f61f/src/smp_park.cesta.o.d build/.objects/d50adccbf9b0827d/src/arm64_boot_policy.cesta.o.d build/.objects/49ef79b61298bd50/src/frame_lease.cesta.o.d build/.objects/404e820c25149481/src/kernel_memory.cesta.o.d build/.objects/11abe54590f4ffaa/src/program_memory.cesta.o.d build/.objects/be1fbe860b9e6b10/src/program_workers.cesta.o.d build/.objects/6383d87eaff6a925/src/kernel_workers.cesta.o.d build/.objects/f77eb7ca88761d09/src/kernel_platform.cesta.o.d build/.objects/88c113b318615167/src/kernel_domains.cesta.o.d build/.objects/e764816723c52546/src/program_image.cesta.o.d build/.objects/3d3438b6380e91e2/src/program_image_bytes.cesta.o.d build/.objects/02fd6e45102bd17b/runtime/kernel/image_failure.c.o.d build/.objects/8400930a7ebd8dea/src/kernel_entry.cesta.o.d build/.objects/b763b7ea62324364/src/kernel_syscalls.cesta.o.d build/.objects/0696e24a087abec1/src/kernel_rpc.cesta.o.d build/.objects/340b0a1cf643e20d/src/memory_regions.cesta.o.d build/.objects/6c916b9add3da93e/src/region_writer.cesta.o.d build/.objects/4177aaf5d7ec2016/src/device_reset.cesta.o.d build/.objects/089192bdd6dbdfe5/src/epoch_journal.cesta.o.d build/.objects/0ecbc475992172da/src/kernel_epoch_log.cesta.o.d build/.objects/1721c290738a8898/src/kernel_boot_memory.cesta.o.d build/.objects/a7933b50241ca028/src/kernel_region_writer.cesta.o.d build/.objects/6f0f101b035f1934/src/kernel_device_interrupt.cesta.o.d build/.objects/bdade6eb55a80ab8/src/kernel_device_reset.cesta.o.d build/.objects/b8638b43a267347f/src/app_private.cesta.o.d build/.objects/f9cf126577129dee/src/kernel_apps.cesta.o.d build/.objects/8b7bfc4de60c41e9/src/program_records.cesta.o.d build/.objects/93e8a7acc874f49e/src/program_record_memory.cesta.o.d build/.objects/ee2f71846910690d/src/program_record_tasks.cesta.o.d build/.objects/9ac6c21ec2e573dc/src/kernel_records.cesta.o.d build/.objects/5c8a8be44e917664/src/kernel_current_boot.cesta.o.d build/.objects/b68a72593bd3eb02/src/kernel_paging.cesta.o.d build/.objects/fa43ab82c5b00d71/src/kernel_task_wait.cesta.o.d build/.objects/145b35cbc54eb019/src/pager_state.cesta.o.d build/.objects/e6e2a0f72e32a140/src/source_policy.cesta.o.d build/.objects/b596d9e2286030fc/src/kernel_sources.cesta.o.d build/.objects/724ad31eab3ac631/src/network_config.cesta.o.d build/.objects/5d48a43b3db4080f/src/pci_policy.cesta.o.d build/.objects/d805b10cfc8f7c66/src/kernel_pci.cesta.o.d build/.objects/a6c3d4299b7d34fe/src/history_recovery.cesta.o.d build/.objects/33e8eeed5295d45e/src/kernel_history.cesta.o.d build/.objects/9a2a651c28d41fe7/runtime/kernel/kernel_log.c.o.d build/.objects/d5247f505bb49b6f/runtime/kernel/entry_x86_64.S.o.d build/.objects/5d66cc14d65071ca/src/silk_shell_models.cesta.o.d build/.objects/cc56761a9d323c4a/src/silk_shell_commands.cesta.o.d build/.objects/90067874a21561db/src/silk_shell_models_root.cesta.o.d build/.objects/cc0528db9f8411d0/src/silk_shell_models_bell.cesta.o.d build/.objects/b7c41f1aa53ab8f7/src/silk_shell_models_linen.cesta.o.d build/.objects/1f4ede46e2b21a58/src/silk_shell_models_projection.cesta.o.d build/.objects/999ed027716364ba/src/silk_shell_models_publish.cesta.o.d build/.objects/2748179c8f891e06/src/silk_shell_models_publish_native.cesta.o.d build/.objects/14bce8598562815b/src/silk_shell_models_render.cesta.o.d build/.objects/bf4380968b4ec6fd/src/silk_shell_models_render_native.cesta.o.d build/.objects/08e1bb93efd1a543/src/silk_shell_models_paint.cesta.o.d build/.objects/f3db094a04826125/src/silk_shell_models_paint_native.cesta.o.d build/.objects/64c1f7cf4424ecec/src/silk_shell_models_workspace.cesta.o.d build/.objects/a2c02c445cd002d0/src/silk_shell_models_workspace_native.cesta.o.d build/.objects/845b9b1a5383fa54/src/linen_capture.cesta.o.d build/.objects/aed60e62d91bb4b4/src/linen_capture_client.cesta.o.d build/.objects/18c0533873de667a/src/linen_capture_transport.cesta.o.d build/.objects/48e68aed90876805/src/shell_linen_snapshot.cesta.o.d build/.objects/a45bcfb07857a969/src/shell_linen_list.cesta.o.d build/.objects/c092e7b22817268c/src/shell_text_delivery.cesta.o.d build/.objects/2c705a127a579430/src/shell_text_delivery_native.cesta.o.d build/.objects/a3914c98944506d7/src/shell_text_delivery_display.cesta.o.d build/.objects/5d98c798dd257e49/src/shell_linen_presentation.cesta.o.d build/.objects/c6e885b3982513eb/src/silk_shell_custody.cesta.o.d build/.objects/7938783cebaf3601/src/linen_protocol_probe.cesta.o.d build/.objects/dbe0500c8f303b69/src/silk_shell_models_linen.cesta.o.d build/.objects/6082cf75c340150c/src/silk_shell_models_projection.cesta.o.d build/.objects/d19302beb8fe17f2/src/silk_shell_models_publish.cesta.o.d build/.objects/4d9916a5973a9ee6/src/silk_shell_models_publish_native.cesta.o.d build/.objects/58bc65bcef97d010/src/linen_capture.cesta.o.d build/.objects/5c101235663d4cf0/src/linen_capture_client.cesta.o.d build/.objects/d941e32a76a9fb32/src/linen_capture_transport.cesta.o.d build/.objects/04039a1d0dd9370b/src/shell_linen_snapshot.cesta.o.d build/.objects/413bb3970f653068/src/shell_text_delivery.cesta.o.d build/.objects/46257244f08e5c64/src/shell_text_delivery_native.cesta.o.d build/.objects/61799c2112d17249/src/shell_text_delivery_display.cesta.o.d build/.objects/2360bd6b23ce2f97/src/shell_linen_presentation.cesta.o.d build/.objects/d058faa75db37800/runtime/client_runtime.c.o.d build/.objects/b7144ac63e0b9804/runtime/traps_x86_64.c.o.d build/.objects/5c0b9f1df62c4728/runtime/rpc_traps_x86_64.c.o.d build/.objects/65b46107433a53d0/runtime/start_x86_64.S.o.d build/.objects/9a341b76481cffad/generated/runtime/format_bytes.c.o.d build/.objects/6e255aeabc6023a0/generated/runtime/packed_words.c.o.d build/.objects/8be2c011fb3ce56f/src/linen_capture.cesta.o.d build/.objects/855cf409c208294f/src/linen_capture_client.cesta.o.d build/.objects/bf7572eb35fe3349/src/linen_capture_transport.cesta.o.d build/.objects/f9549877b4e91dd3/src/linen_capture_service.cesta.o.d build/.objects/85cdac3968fd4ac2/src/shell_linen_snapshot.cesta.o.d build/.objects/8428c2651c3d0fb0/src/silk_shell_models_linen.cesta.o.d build/.objects/b1e8ea3303c5440b/src/silk_shell_models_projection.cesta.o.d build/.objects/1b5a2f5d57c44b09/src/silk_shell_models_publish.cesta.o.d build/.objects/5edf5677ecce46fd/src/silk_shell_models_publish_native.cesta.o.d build/.objects/c7012c49e102c817/src/shell_text_delivery.cesta.o.d build/.objects/6bbc66393ab49813/src/shell_text_delivery_native.cesta.o.d build/.objects/3bd3fa3dfebffe90/src/shell_text_delivery_display.cesta.o.d build/.objects/56b06fdba4219bae/src/shell_linen_presentation.cesta.o.d build/.objects/8bc92584a1bc60bc/src/silk_shell_models_render.cesta.o.d build/.objects/9e867867d35cc70a/src/silk_shell_models_render_native.cesta.o.d build/.objects/d096b29b4e65a714/src/silk_shell_models_paint.cesta.o.d build/.objects/4d154c6352e69c62/src/silk_shell_models_paint_native.cesta.o.d build/.objects/abfbd4f34ae3d0ce/src/shell_linen_list.cesta.o.d build/.objects/586b63906ab18df9/src/silk_shell_diagnostics_a.cesta.o.d build/.objects/1e8ca0cf268ab0ed/src/silk_shell_diagnostics_a_native.cesta.o.d build/.objects/fd90c076105550fe/src/silk_shell_proofs_a.cesta.o.d build/.objects/43d7f060ae34d5d1/src/silk_shell_proofs_a_root.cesta.o.d build/.objects/542e98710af5dcaf/src/silk_shell_proofs_a_routes.cesta.o.d build/.objects/7a2dffcbbff43b2b/src/silk_shell_proofs_a_routes_native.cesta.o.d build/.objects/22f296b27cc61ba6/src/shell_proof_lifecycle_fixture.cesta.o.d build/.objects/69291b03f5c2d657/src/silk_shell_proofs_a_lifecycle.cesta.o.d build/.objects/b3225b4ae7fee353/src/silk_shell_proofs_a_lifecycle_native.cesta.o.d build/.objects/097f472246023bf9/src/silk_shell_proofs_b_palette.cesta.o.d build/.objects/5bf20b0b5d4b8aed/src/silk_shell_proofs_b_palette_native.cesta.o.d build/.objects/4cd6003c48d7cc0d/src/silk_shell_proofs_b_atlas.cesta.o.d build/.objects/229e6db3cfb10931/src/silk_shell_proofs_b_atlas_native.cesta.o.d build/.objects/f56b0da36cb87098/src/silk_shell_proofs_b_input.cesta.o.d build/.objects/078d4efafbaa08d6/src/silk_shell_proofs_b_input_native.cesta.o.d build/.objects/a03373d4dd99b420/src/silk_shell_proofs_b_root.cesta.o.d build/.objects/03a810070a584965/src/silk_shell_core_root.cesta.o.d build/.objects/bcb131a54bbae66f/src/silk_shell_core_dispatch.cesta.o.d build/.objects/a3066bb833682a36/src/silk_shell_startup.cesta.o.d build/.objects/7cfc24c6a2cae60a/src/silk_shell_core_main.cesta.o.d build/.objects/e8a08c226cf54157/src/silk_shell_join.cesta.o.d build/.objects/71b563a6f4e34a01/src/silk_shell_input.cesta.o.d build/.objects/da1f6cad4ad6f709/src/silk_shell_input_adapter.cesta.o.d build/.objects/25a1a6c5e9145498/src/silk_shell_input_diagnostics.cesta.o.d build/.objects/82deb16b8491ce24/src/silk_shell_input_apps.cesta.o.d build/.objects/d3de41d482095a03/src/silk_shell_chrome_atomic.cesta.o.d build/.objects/318917eac90e568c/src/silk_shell_custody.cesta.o.d build/.objects/9a34f0c200d44680/src/silk_shell_atlas.cesta.o.d build/.objects/6578356dc544545c/src/silk_shell_atlas_adapter.cesta.o.d build/.objects/8680e8dba55106dd/src/silk_shell_chrome.cesta.o.d build/.objects/4ca431533f2a9801/src/silk_shell_viewport.cesta.o.d build/.objects/b273779147a5b9ad/src/silkbar_geometry.cesta.o.d build/.objects/75703275afdd8d3c/src/silkbar_model.cesta.o.d build/.objects/0da0f4564068a4db/src/silk_shell_surfaces.cesta.o.d build/.objects/df5b33e129550817/src/silk_shell_surfaces_native.cesta.o.d build/.objects/0fade820838c2599/src/silk_shell_commands.cesta.o.d build/.objects/269ace6ffec6523e/src/silk_shell_command_count.cesta.o.d build/.objects/ad36d7b1050a569c/src/silk_shell_entry.cesta.o.d build/.objects/c15ee7431716832a/runtime/client_runtime.c.o.d build/.objects/5c3e1ccf60d99346/runtime/server_runtime.c.o.d build/.objects/bd1440bf39fc845e/runtime/traps_x86_64.c.o.d build/.objects/d480912068b8dcea/runtime/server_traps_x86_64.c.o.d build/.objects/79ffa47f373abe9a/runtime/start_x86_64.S.o.d build/.objects/3bf24639e5833dd2/runtime/rpc_traps_x86_64.c.o.d build/.objects/ccfc3357f2199433/generated/runtime/format_bytes.c.o.d build/.objects/36b5fd63c24ac30a/generated/runtime/packed_words.c.o.d build/.objects/a62e5922dd8c3c8b/tests/pager_boot/client.cesta.o.d build/.objects/dda77889cda65e0f/runtime/client_runtime.c.o.d build/.objects/d358e8a82b99adb3/runtime/traps_x86_64.c.o.d build/.objects/15474164bcb3d2ff/runtime/start_x86_64.S.o.d build/.objects/d72a42f7281f5d80/generated/runtime/format_bytes.c.o.d build/.objects/9a1dfed1a02bf0ef/generated/runtime/packed_words.c.o.d build/.objects/919adea1445a6082/tests/kernel_sources/probe.cesta.o.d build/.objects/3023b88ae6bb5507/runtime/client_runtime.c.o.d build/.objects/b17bb058d1baeb2b/runtime/traps_x86_64.c.o.d build/.objects/3ba865a8a62be037/runtime/start_x86_64.S.o.d build/.objects/c8494a256898d9da/generated/runtime/format_bytes.c.o.d build/.objects/cb6fe1a102bba967/generated/runtime/packed_words.c.o.d build/.objects/6220d230814ccdd1/tests/kernel_pci/probe.cesta.o.d build/.objects/ae07bfb64a68deb4/runtime/client_runtime.c.o.d build/.objects/3168376a1459dc68/runtime/traps_x86_64.c.o.d build/.objects/11744d0c50cf8a04/runtime/start_x86_64.S.o.d build/.objects/16d95ffca3b78843/generated/runtime/format_bytes.c.o.d build/.objects/be9016d95693fe14/generated/runtime/packed_words.c.o.d build/.objects/5edd7c83164cfcd1/tests/region_writer/probe.cesta.o.d build/.objects/f23c2820b4ff15e6/runtime/client_runtime.c.o.d build/.objects/3c1224cb5ab9ebf2/runtime/server_runtime.c.o.d build/.objects/ce497601b0b0924a/runtime/traps_x86_64.c.o.d build/.objects/0448ac03a5f16426/runtime/server_traps_x86_64.c.o.d build/.objects/13b2f4fc5ba8ae7b/runtime/region_writer_traps_x86_64.c.o.d build/.objects/74dbdb8697d0f456/runtime/start_x86_64.S.o.d build/.objects/624f143532dc80b9/generated/runtime/format_bytes.c.o.d build/.objects/6e5c2f8f0e823d86/generated/runtime/packed_words.c.o.d build/.objects/1f42e0be5d979d2b/tests/fpu_context/probe.cesta.o.d build/.objects/3084e5f95c3eefed/runtime/client_runtime.c.o.d build/.objects/6d483866b2e2b771/runtime/traps_x86_64.c.o.d build/.objects/562fb9929acb631d/runtime/start_x86_64.S.o.d build/.objects/e6d07e2f1d3e4f7c/generated/runtime/format_bytes.c.o.d build/.objects/9a40f61525e9b40d/generated/runtime/packed_words.c.o.d build/.objects/1f3ee0be5d93b454/tests/fpu_context/probe.cesta.o.d build/.objects/308ee5f95c474472/runtime/client_runtime.c.o.d build/.objects/6d523866b2eb0bf6/runtime/traps_x86_64.c.o.d build/.objects/5639b9929ad3b7a2/runtime/start_x86_64.S.o.d build/.objects/e6d47e2f1d423853/generated/runtime/format_bytes.c.o.d build/.objects/9a4af61525f20892/generated/runtime/packed_words.c.o.d build/.objects/a4cbe9e6c889cdb1/tests/history_recovery/probe.cesta.o.d build/.objects/3bb970543f06ba05/src/history_recovery.cesta.o.d build/.objects/34042c3a5c92b3e6/src/history_decode.cesta.o.d build/.objects/6f36173c8d4f799e/runtime/history_native.c.o.d build/.objects/e623acb4b02b133c/runtime/client_runtime.c.o.d build/.objects/850610202a0b2480/runtime/traps_x86_64.c.o.d build/.objects/0e0592afba2a432c/runtime/start_x86_64.S.o.d build/.objects/fd1ffafa56df85b3/generated/runtime/format_bytes.c.o.d build/.objects/7cc80f39caddb61c/generated/runtime/packed_words.c.o.d build/.objects/aae765b14158881f/tests/kernel_retire/probe.cesta.o.d build/.objects/fe07ae6525b26c81/runtime/client_runtime.c.o.d build/.objects/7d2d780e24066335/runtime/traps_x86_64.c.o.d build/.objects/4a36d0f14981fef1/runtime/start_x86_64.S.o.d build/.objects/a0173ed5a409d760/generated/runtime/format_bytes.c.o.d build/.objects/3cc21fc0369cd1e1/generated/runtime/packed_words.c.o.d build/.objects/84966b113f482211/src/sext.cesta.o.d build/.objects/e1779b41157bd3a9/src/sext_service.cesta.o.d build/.objects/69587def0e8eda0f/src/sext_workspace.cesta.o.d build/.objects/c164688ded225ee8/runtime/sext_native.c.o.d build/.objects/8788a4d079d41430/runtime/sext_worker_probe.c.o.d build/.objects/23a039077581fb1a/runtime/client_runtime.c.o.d build/.objects/9cc8ce7abfe2323e/runtime/server_runtime.c.o.d build/.objects/57f79c97e94bd526/runtime/traps_x86_64.c.o.d build/.objects/34ffe0f95f151cda/runtime/server_traps_x86_64.c.o.d build/.objects/23742f284ace3f4a/runtime/start_x86_64.S.o.d build/.objects/5a4dcd12385aefed/generated/runtime/format_bytes.c.o.d build/.objects/5874f19e28db9b7a/generated/runtime/packed_words.c.o.d build/.objects/50c6330903177953/src/sext.cesta.o.d build/.objects/62c86b32fc45c89b/src/sext_service.cesta.o.d build/.objects/3721b9a6cf3260ad/src/sext_workspace.cesta.o.d build/.objects/0940b94d2f086948/runtime/sext_native.c.o.d build/.objects/43432250fb269bc0/runtime/sext_worker_probe.c.o.d build/.objects/f37259d15ccd5cbe/runtime/client_runtime.c.o.d build/.objects/f353014a79ea2fa2/runtime/server_runtime.c.o.d build/.objects/82c773c5ff98e25a/runtime/traps_x86_64.c.o.d build/.objects/062ea218e25a86fe/runtime/server_traps_x86_64.c.o.d build/.objects/620dee108e34fe6e/runtime/start_x86_64.S.o.d build/.objects/60b53b011ddbd56f/generated/runtime/format_bytes.c.o.d build/.objects/53bde9131eae7c5e/generated/runtime/packed_words.c.o.d build/.objects/b5a2a01165d1af0d/src/frame_lease.cesta.o.d build/.objects/323075e01144cc38/src/kernel_memory.cesta.o.d build/.objects/e29678b3862941ef/src/program_memory.cesta.o.d build/.objects/968d70ab50d14dcd/src/program_workers.cesta.o.d build/.objects/3554b602c7bab30c/src/kernel_workers.cesta.o.d build/.objects/3555ba7b062329e0/src/kernel_platform.cesta.o.d build/.objects/98b4c43de8088ca6/src/kernel_domains.cesta.o.d build/.objects/5c706a61bb81a41b/src/program_image.cesta.o.d build/.objects/8430152c97e435c7/src/program_image_bytes.cesta.o.d build/.objects/3c0480fc5b8db57a/runtime/kernel/image_failure.c.o.d build/.objects/5f97dc379026952f/src/kernel_entry.cesta.o.d build/.objects/d06874f74a39a7f1/src/kernel_syscalls.cesta.o.d build/.objects/eb934cd0a0c48c78/src/kernel_rpc.cesta.o.d build/.objects/a05184d10afe5934/src/memory_regions.cesta.o.d build/.objects/07215957f1ea61b3/src/region_writer.cesta.o.d build/.objects/2ed075df6a116f4b/src/device_reset.cesta.o.d build/.objects/90892a5849d262cc/src/epoch_journal.cesta.o.d build/.objects/dfbc03653ac1b4bf/src/kernel_epoch_log.cesta.o.d build/.objects/c64f3c4a33313f55/src/kernel_boot_memory.cesta.o.d build/.objects/02ced040bc276485/src/kernel_region_writer.cesta.o.d build/.objects/b3127bab89e25ca1/src/kernel_device_interrupt.cesta.o.d build/.objects/516323a7112e9af5/src/kernel_device_reset.cesta.o.d build/.objects/7fd566ff9708915e/src/app_private.cesta.o.d build/.objects/e1164d1ee98146c3/src/kernel_apps.cesta.o.d build/.objects/f2ddc432636e28c0/src/program_records.cesta.o.d build/.objects/02106af68353e893/src/program_record_memory.cesta.o.d build/.objects/60e5ae78a6348834/src/program_record_tasks.cesta.o.d build/.objects/9a0ad0f2fc854b89/src/kernel_records.cesta.o.d build/.objects/a2ae5e0658e6a2f1/src/kernel_current_boot.cesta.o.d build/.objects/e86b4df0404e87e7/src/kernel_paging.cesta.o.d build/.objects/4b48ee7099daa748/src/kernel_task_wait.cesta.o.d build/.objects/a48364eb87797250/src/pager_state.cesta.o.d build/.objects/c288925a3d40a21d/src/source_policy.cesta.o.d build/.objects/adad61e9dae54da9/src/kernel_sources.cesta.o.d build/.objects/31642976f28c3308/src/network_config.cesta.o.d build/.objects/126bf10281c7e18e/src/pci_policy.cesta.o.d build/.objects/416345b45e45d93b/src/kernel_pci.cesta.o.d build/.objects/d8c9bf243033b073/src/history_recovery.cesta.o.d build/.objects/d5add77b51d4fc53/src/kernel_history.cesta.o.d build/.objects/1cb94bddb465d4d8/src/smp_park.cesta.o.d build/.objects/a49f1bc859b99f2f/src/kernel_smp.cesta.o.d build/.objects/d941a5248c413233/src/nvme_msix.cesta.o.d build/.objects/1c26d17757c16526/runtime/kernel/kernel_log.c.o.d build/.objects/27390210d6aa74ee/runtime/kernel/entry_x86_64.S.o.d build/.objects/012832936d1b3152/src/frame_lease.cesta.o.d build/.objects/4b6c3dd6e367682b/src/kernel_memory.cesta.o.d build/.objects/ad0222d9c5f63fa0/src/program_memory.cesta.o.d build/.objects/e725e2cdb88c7b92/src/program_workers.cesta.o.d build/.objects/d9dca9af6f1587ef/src/kernel_workers.cesta.o.d build/.objects/bc3100f9009a5ed3/src/kernel_platform.cesta.o.d build/.objects/3e9e9d561cbc0b1d/src/kernel_domains.cesta.o.d build/.objects/8b3c452215fd79ac/src/program_image.cesta.o.d build/.objects/bf585c79346b0598/src/program_image_bytes.cesta.o.d build/.objects/9958c14727943b11/runtime/kernel/image_failure.c.o.d build/.objects/c100c487da11cd60/src/kernel_entry.cesta.o.d build/.objects/de53b3ed53df6666/src/kernel_syscalls.cesta.o.d build/.objects/ddd836cda75041eb/src/kernel_rpc.cesta.o.d build/.objects/f7ae317a9a1eac17/src/memory_regions.cesta.o.d build/.objects/862901d20385f304/src/region_writer.cesta.o.d build/.objects/08e60b291673ee1c/src/device_reset.cesta.o.d build/.objects/57e3ed9140ef5e2f/src/epoch_journal.cesta.o.d build/.objects/5c100ec5912487f0/src/kernel_epoch_log.cesta.o.d build/.objects/8ef0e9941653aeda/src/kernel_boot_memory.cesta.o.d build/.objects/62e64bfb4d1ce38a/src/kernel_region_writer.cesta.o.d build/.objects/26be9bc4ffab7b96/src/kernel_device_interrupt.cesta.o.d build/.objects/aba6e6598ec6237a/src/kernel_device_reset.cesta.o.d build/.objects/3605d45eff7bb3d5/src/app_private.cesta.o.d build/.objects/bf36f48a017e61d4/src/kernel_apps.cesta.o.d build/.objects/fa05c5da99790b33/src/program_records.cesta.o.d build/.objects/5e255764dc41d4e4/src/program_record_memory.cesta.o.d build/.objects/796dc29a9562cb17/src/program_record_tasks.cesta.o.d build/.objects/a05c3142e3b20b1e/src/kernel_records.cesta.o.d build/.objects/ae0b5cac27031166/src/kernel_current_boot.cesta.o.d build/.objects/c6fd9f4b38a9fb38/src/kernel_paging.cesta.o.d build/.objects/48f8c07a314691bb/src/kernel_task_wait.cesta.o.d build/.objects/2d2a74b24c5f5503/src/pager_state.cesta.o.d build/.objects/d45907a0080d8762/src/source_policy.cesta.o.d build/.objects/fa1d06156fd1d33e/src/kernel_sources.cesta.o.d build/.objects/bc7ab41fa23da6fb/src/network_config.cesta.o.d build/.objects/68b22ba9b3144e85/src/pci_policy.cesta.o.d build/.objects/346b5252e0ddc8cc/src/kernel_pci.cesta.o.d build/.objects/2a550027ef5b5244/src/history_recovery.cesta.o.d build/.objects/8dd4452add544124/src/kernel_history.cesta.o.d build/.objects/5a0f16e06a8b15cb/src/smp_park.cesta.o.d build/.objects/a77a0c5b6a877760/src/kernel_smp.cesta.o.d build/.objects/ab7c1623d07a9884/src/nvme_msix.cesta.o.d build/.objects/601cb46df15b389d/runtime/kernel/kernel_log.c.o.d build/.objects/89a93f157b5d71e5/runtime/kernel/entry_x86_64.S.o.d build/.objects/86d16ae28840bb54/src/frame_lease.cesta.o.d build/.objects/a9ea9e0c1184fadd/src/kernel_memory.cesta.o.d build/.objects/059e9a1516998dfa/src/program_memory.cesta.o.d build/.objects/6d8ce2d4ce1ac894/src/program_workers.cesta.o.d build/.objects/caa69b2a3b1c0c29/src/kernel_workers.cesta.o.d build/.objects/1c85c728624201e5/src/kernel_platform.cesta.o.d build/.objects/00b5d34311986a6f/src/kernel_domains.cesta.o.d build/.objects/eb64be6bed76eede/src/program_image.cesta.o.d build/.objects/399b51fbe2e82a32/src/program_image_bytes.cesta.o.d build/.objects/d707ffc0ba0f054b/runtime/kernel/image_failure.c.o.d build/.objects/fc7b39e3d73198ba/src/kernel_entry.cesta.o.d build/.objects/5cc478917fbf8c70/src/kernel_syscalls.cesta.o.d build/.objects/3c08714bbd59bb9d/src/kernel_rpc.cesta.o.d build/.objects/617e58147dd5fbd1/src/memory_regions.cesta.o.d build/.objects/0c88752e1d853976/src/region_writer.cesta.o.d build/.objects/e7d5c679263f0bee/src/device_reset.cesta.o.d build/.objects/c21160de7f584f69/src/epoch_journal.cesta.o.d build/.objects/07c9d0c04699ba2a/src/kernel_epoch_log.cesta.o.d build/.objects/cee9d22d6268961c/src/kernel_boot_memory.cesta.o.d build/.objects/00accb0eeaa92bac/src/kernel_region_writer.cesta.o.d build/.objects/246e587d304df6c0/src/kernel_device_interrupt.cesta.o.d build/.objects/aa150810d8e97bbc/src/kernel_device_reset.cesta.o.d build/.objects/3a50a5701de808a7/src/app_private.cesta.o.d build/.objects/20ff552677f8fb26/src/kernel_apps.cesta.o.d build/.objects/6dafcf3f1cdd2cc5/src/program_records.cesta.o.d build/.objects/a235394a4c6a1c56/src/program_record_memory.cesta.o.d build/.objects/755d8548f11c3ad1/src/program_record_tasks.cesta.o.d build/.objects/85f24a6d61afb988/src/kernel_records.cesta.o.d build/.objects/9bd8bf6ba2abd770/src/kernel_current_boot.cesta.o.d build/.objects/909942c67ce05ad2/src/kernel_paging.cesta.o.d build/.objects/f436aadde869abcd/src/kernel_task_wait.cesta.o.d build/.objects/f48c71ca1af8ed35/src/pager_state.cesta.o.d build/.objects/76b499397f04f784/src/source_policy.cesta.o.d build/.objects/a29e7ba0f4e001a8/src/kernel_sources.cesta.o.d build/.objects/0893de9f728ff60d/src/network_config.cesta.o.d build/.objects/af47148609338177/src/pci_policy.cesta.o.d build/.objects/dfe1da6baa274c7e/src/kernel_pci.cesta.o.d build/.objects/415d7776e3cfabb6/src/history_recovery.cesta.o.d build/.objects/b000d70e2e5f0f96/src/kernel_history.cesta.o.d build/.objects/bde128e4130da5fd/src/smp_park.cesta.o.d build/.objects/1e3a5985427602ba/src/kernel_smp.cesta.o.d build/.objects/69796f8406fe4af6/src/nvme_msix.cesta.o.d build/.objects/c3e5f87c0dca49ef/runtime/kernel/kernel_log.c.o.d build/.objects/16a2cf044ddac257/runtime/kernel/entry_x86_64.S.o.d build/.objects/7cffa22ad4439fe7/src/frame_lease.cesta.o.d build/.objects/b87c12279a819c32/src/kernel_memory.cesta.o.d build/.objects/1528d478616ada09/src/program_memory.cesta.o.d build/.objects/9905b13a93f1ad27/src/program_workers.cesta.o.d build/.objects/22cd2c870dde1fd6/src/kernel_workers.cesta.o.d build/.objects/13ced179a4c6a33a/src/kernel_platform.cesta.o.d build/.objects/78c9e19f23ef4dc0/src/kernel_domains.cesta.o.d build/.objects/9651cfe8987a58e5/src/program_image.cesta.o.d build/.objects/31cbdabc276fde21/src/program_image_bytes.cesta.o.d build/.objects/72ad8d50e8079804/runtime/kernel/image_failure.c.o.d build/.objects/31c53a389be871c9/src/kernel_entry.cesta.o.d build/.objects/c26f35445322d63b/src/kernel_syscalls.cesta.o.d build/.objects/1e178d6f5ccbf0f2/src/kernel_rpc.cesta.o.d build/.objects/4c3edfad1775291e/src/memory_regions.cesta.o.d build/.objects/26c7289f162ad31d/src/region_writer.cesta.o.d build/.objects/9a0a9c31cc535e95/src/device_reset.cesta.o.d build/.objects/c283c0ab74868b16/src/epoch_journal.cesta.o.d build/.objects/8c5b83d45ff03419/src/kernel_epoch_log.cesta.o.d build/.objects/82c9b0925b0853ef/src/kernel_boot_memory.cesta.o.d build/.objects/6d4704673e3f879f/src/kernel_region_writer.cesta.o.d build/.objects/af034ccbf6ef5d6b/src/kernel_device_interrupt.cesta.o.d build/.objects/ac87056cc54faa0f/src/kernel_device_reset.cesta.o.d build/.objects/4d9da9c2a8fd6918/src/app_private.cesta.o.d build/.objects/9efd4a6bed35bfed/src/kernel_apps.cesta.o.d build/.objects/83096829fd10411a/src/program_records.cesta.o.d build/.objects/b91577d00068697d/src/program_record_memory.cesta.o.d build/.objects/51535cf765d3301e/src/program_record_tasks.cesta.o.d build/.objects/9ec458d8afd46333/src/kernel_records.cesta.o.d build/.objects/33e1faba76c1893b/src/kernel_current_boot.cesta.o.d build/.objects/f3c6320fb3fe4241/src/kernel_paging.cesta.o.d build/.objects/0823a4733e976cc2/src/kernel_task_wait.cesta.o.d build/.objects/0cc2af90ae05a12a/src/pager_state.cesta.o.d build/.objects/70e3b93c1a8d4d37/src/source_policy.cesta.o.d build/.objects/59ba92bd68c736d3/src/kernel_sources.cesta.o.d build/.objects/54d255915fbd6702/src/network_config.cesta.o.d build/.objects/0bce08f80da1db08/src/pci_policy.cesta.o.d build/.objects/0b053bc99aab1705/src/kernel_pci.cesta.o.d build/.objects/6d2c76fc1e0e205d/src/history_recovery.cesta.o.d build/.objects/98cdd20a4b7831bd/src/kernel_history.cesta.o.d build/.objects/30671ec126408852/src/smp_park.cesta.o.d build/.objects/87d323aa32200bc9/src/kernel_smp.cesta.o.d build/.objects/cf2aa7fb645d1e9d/src/nvme_msix.cesta.o.d build/.objects/e3ab2da04bbc9d40/runtime/kernel/kernel_log.c.o.d build/.objects/423f48a237a09d68/runtime/kernel/entry_x86_64.S.o.d build/.objects/09bfeb12b2497b81/src/frame_lease.cesta.o.d build/.objects/3410d1e3cbe41d10/src/kernel_memory.cesta.o.d build/.objects/67b28dd741c3381b/src/program_memory.cesta.o.d build/.objects/b3ea2afde0bfc7c1/src/program_workers.cesta.o.d build/.objects/a87fb56601f28c6c/src/kernel_workers.cesta.o.d build/.objects/a3b1ddb8b33faa88/src/kernel_platform.cesta.o.d build/.objects/641855720cc8cdb6/src/kernel_domains.cesta.o.d build/.objects/1178520fac410cef/src/program_image.cesta.o.d build/.objects/2535ea340f396583/src/program_image_bytes.cesta.o.d build/.objects/8baa5a08aeb9b3b2/runtime/kernel/image_failure.c.o.d build/.objects/5a4eeb8b1bd46adb/src/kernel_entry.cesta.o.d build/.objects/0f3596d06cfd5efd/src/kernel_syscalls.cesta.o.d build/.objects/e2fcc53f015a02d0/src/kernel_rpc.cesta.o.d build/.objects/a09a3114a6028c84/src/memory_regions.cesta.o.d build/.objects/a844c7b166db5cd7/src/region_writer.cesta.o.d build/.objects/90604c9ebe9afd5f/src/device_reset.cesta.o.d build/.objects/b2071afe6e07f6ac/src/epoch_journal.cesta.o.d build/.objects/df0678e9bcd1fb6b/src/kernel_epoch_log.cesta.o.d build/.objects/430f35f76588f199/src/kernel_boot_memory.cesta.o.d build/.objects/a161c12c02ecd8c9/src/kernel_region_writer.cesta.o.d build/.objects/8143c69a2d57132d/src/kernel_device_interrupt.cesta.o.d build/.objects/aae207fa2c8d9eb9/src/kernel_device_reset.cesta.o.d build/.objects/4a47e88da6675f3e/src/app_private.cesta.o.d build/.objects/50eaa5984c76f427/src/kernel_apps.cesta.o.d build/.objects/95a85035e44298e8/src/program_records.cesta.o.d build/.objects/2c67389215c30237/src/program_record_memory.cesta.o.d build/.objects/e7f31f25c34acb84/src/program_record_tasks.cesta.o.d build/.objects/0b8c2670e8e5bbe5/src/kernel_records.cesta.o.d build/.objects/8eceb612dfd3a9fd/src/kernel_current_boot.cesta.o.d build/.objects/802a1dd6d6e5d923/src/kernel_paging.cesta.o.d build/.objects/e1bd7873d5ff9f60/src/kernel_task_wait.cesta.o.d build/.objects/8a2ec29333cc89f8/src/pager_state.cesta.o.d build/.objects/3df8e33e6f898c51/src/source_policy.cesta.o.d build/.objects/f42779a309c20985/src/kernel_sources.cesta.o.d build/.objects/f6217fa807c7d1a0/src/network_config.cesta.o.d build/.objects/f3644a368e270dae/src/pci_policy.cesta.o.d build/.objects/a0c5117fff6dd70f/src/kernel_pci.cesta.o.d build/.objects/eb02612c40b82e17/src/history_recovery.cesta.o.d build/.objects/3fa443e197af8677/src/kernel_history.cesta.o.d build/.objects/e1985e6411290336/runtime/kernel/kernel_log.c.o.d build/.objects/0a335b9e8f88498e/runtime/kernel/entry_x86_64.S.o.d
