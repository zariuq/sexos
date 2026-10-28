# Readable C exported by cesta build from cesta.project; builds without Cesta.
CC = clang
AR = ar
.DEFAULT_GOAL := all

.PHONY: all
all: build/kaleidoscope/kaleidoscope build/silkbar/silkbar build/sexinput/sexinput build/sexstore/sexstore build/sexdisplay/sexdisplay build/sexbell/sexbell build/linen/linen build/quil/quil build/spindle/spindle build/purple-scanout/purple-scanout build/sexfiles/sexfiles build/sexdrive/sexdrive build/sexusb/sexusb build/sexnet/sexnet build/kaleidoscope-network/kaleidoscope-network build/sexnet-network/sexnet-network build/sexos-kernel/sexos-kernel build/sexos-kernel-network/sexos-kernel-network

build/kaleidoscope/obj/src/kaleidoscope.c.o: src/kaleidoscope.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kaleidoscope.c' -o "$@"

build/kaleidoscope/obj/runtime/client_runtime.c.o: runtime/client_runtime.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/kaleidoscope/obj/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/kaleidoscope/obj/runtime/start_x86_64.S.o: runtime/start_x86_64.S Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/kaleidoscope/obj/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/kaleidoscope/obj/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/kaleidoscope/kaleidoscope: Makefile build/kaleidoscope/obj/src/kaleidoscope.c.o build/kaleidoscope/obj/runtime/client_runtime.c.o build/kaleidoscope/obj/runtime/traps_x86_64.c.o build/kaleidoscope/obj/runtime/start_x86_64.S.o build/kaleidoscope/obj/generated/runtime/format_bytes.c.o build/kaleidoscope/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/kaleidoscope/obj/src/kaleidoscope.c.o' 'build/kaleidoscope/obj/runtime/client_runtime.c.o' 'build/kaleidoscope/obj/runtime/traps_x86_64.c.o' 'build/kaleidoscope/obj/runtime/start_x86_64.S.o' 'build/kaleidoscope/obj/generated/runtime/format_bytes.c.o' 'build/kaleidoscope/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/silkbar/obj/src/silkbar.c.o: src/silkbar.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=silkbar_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silkbar.c' -o "$@"

build/silkbar/obj/src/silkbar_model.c.o: src/silkbar_model.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=silkbar_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silkbar_model.c' -o "$@"

build/silkbar/obj/runtime/client_runtime.c.o: runtime/client_runtime.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=silkbar_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/silkbar/obj/runtime/server_runtime.c.o: runtime/server_runtime.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=silkbar_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_runtime.c' -o "$@"

build/silkbar/obj/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=silkbar_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/silkbar/obj/runtime/server_traps_x86_64.c.o: runtime/server_traps_x86_64.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=silkbar_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_traps_x86_64.c' -o "$@"

build/silkbar/obj/runtime/start_x86_64.S.o: runtime/start_x86_64.S Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=silkbar_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/silkbar/obj/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=silkbar_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/silkbar/obj/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=silkbar_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/silkbar/silkbar: Makefile build/silkbar/obj/src/silkbar.c.o build/silkbar/obj/src/silkbar_model.c.o build/silkbar/obj/runtime/client_runtime.c.o build/silkbar/obj/runtime/server_runtime.c.o build/silkbar/obj/runtime/traps_x86_64.c.o build/silkbar/obj/runtime/server_traps_x86_64.c.o build/silkbar/obj/runtime/start_x86_64.S.o build/silkbar/obj/generated/runtime/format_bytes.c.o build/silkbar/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/silkbar/obj/src/silkbar.c.o' 'build/silkbar/obj/src/silkbar_model.c.o' 'build/silkbar/obj/runtime/client_runtime.c.o' 'build/silkbar/obj/runtime/server_runtime.c.o' 'build/silkbar/obj/runtime/traps_x86_64.c.o' 'build/silkbar/obj/runtime/server_traps_x86_64.c.o' 'build/silkbar/obj/runtime/start_x86_64.S.o' 'build/silkbar/obj/generated/runtime/format_bytes.c.o' 'build/silkbar/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/sexinput/obj/src/sexinput.c.o: src/sexinput.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexinput_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/sexinput.c' -o "$@"

build/sexinput/obj/runtime/client_runtime.c.o: runtime/client_runtime.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexinput_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/sexinput/obj/runtime/server_runtime.c.o: runtime/server_runtime.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexinput_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_runtime.c' -o "$@"

build/sexinput/obj/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexinput_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/sexinput/obj/runtime/server_traps_x86_64.c.o: runtime/server_traps_x86_64.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexinput_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_traps_x86_64.c' -o "$@"

build/sexinput/obj/runtime/start_x86_64.S.o: runtime/start_x86_64.S Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexinput_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/sexinput/obj/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexinput_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/sexinput/obj/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexinput_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/sexinput/sexinput: Makefile build/sexinput/obj/src/sexinput.c.o build/sexinput/obj/runtime/client_runtime.c.o build/sexinput/obj/runtime/server_runtime.c.o build/sexinput/obj/runtime/traps_x86_64.c.o build/sexinput/obj/runtime/server_traps_x86_64.c.o build/sexinput/obj/runtime/start_x86_64.S.o build/sexinput/obj/generated/runtime/format_bytes.c.o build/sexinput/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/sexinput/obj/src/sexinput.c.o' 'build/sexinput/obj/runtime/client_runtime.c.o' 'build/sexinput/obj/runtime/server_runtime.c.o' 'build/sexinput/obj/runtime/traps_x86_64.c.o' 'build/sexinput/obj/runtime/server_traps_x86_64.c.o' 'build/sexinput/obj/runtime/start_x86_64.S.o' 'build/sexinput/obj/generated/runtime/format_bytes.c.o' 'build/sexinput/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/sexstore/obj/src/sexstore.c.o: src/sexstore.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexstore_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/sexstore.c' -o "$@"

build/sexstore/obj/src/sexstore_durable.c.o: src/sexstore_durable.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexstore_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/sexstore_durable.c' -o "$@"

build/sexstore/obj/src/sexstore_pages.c.o: src/sexstore_pages.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexstore_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/sexstore_pages.c' -o "$@"

build/sexstore/obj/runtime/client_runtime.c.o: runtime/client_runtime.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexstore_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/sexstore/obj/runtime/server_runtime.c.o: runtime/server_runtime.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexstore_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_runtime.c' -o "$@"

build/sexstore/obj/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexstore_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/sexstore/obj/runtime/server_traps_x86_64.c.o: runtime/server_traps_x86_64.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexstore_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_traps_x86_64.c' -o "$@"

build/sexstore/obj/runtime/start_x86_64.S.o: runtime/start_x86_64.S Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexstore_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/sexstore/obj/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexstore_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/sexstore/obj/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexstore_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/sexstore/sexstore: Makefile build/sexstore/obj/src/sexstore.c.o build/sexstore/obj/src/sexstore_durable.c.o build/sexstore/obj/src/sexstore_pages.c.o build/sexstore/obj/runtime/client_runtime.c.o build/sexstore/obj/runtime/server_runtime.c.o build/sexstore/obj/runtime/traps_x86_64.c.o build/sexstore/obj/runtime/server_traps_x86_64.c.o build/sexstore/obj/runtime/start_x86_64.S.o build/sexstore/obj/generated/runtime/format_bytes.c.o build/sexstore/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/sexstore/obj/src/sexstore.c.o' 'build/sexstore/obj/src/sexstore_durable.c.o' 'build/sexstore/obj/src/sexstore_pages.c.o' 'build/sexstore/obj/runtime/client_runtime.c.o' 'build/sexstore/obj/runtime/server_runtime.c.o' 'build/sexstore/obj/runtime/traps_x86_64.c.o' 'build/sexstore/obj/runtime/server_traps_x86_64.c.o' 'build/sexstore/obj/runtime/start_x86_64.S.o' 'build/sexstore/obj/generated/runtime/format_bytes.c.o' 'build/sexstore/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/sexdisplay/obj/src/sexdisplay.c.o: src/sexdisplay.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdisplay_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/sexdisplay.c' -o "$@"

build/sexdisplay/obj/src/silkbar_model.c.o: src/silkbar_model.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdisplay_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/silkbar_model.c' -o "$@"

build/sexdisplay/obj/runtime/client_runtime.c.o: runtime/client_runtime.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdisplay_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/sexdisplay/obj/runtime/server_runtime.c.o: runtime/server_runtime.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdisplay_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_runtime.c' -o "$@"

build/sexdisplay/obj/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdisplay_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/sexdisplay/obj/runtime/server_traps_x86_64.c.o: runtime/server_traps_x86_64.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdisplay_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_traps_x86_64.c' -o "$@"

build/sexdisplay/obj/runtime/start_x86_64.S.o: runtime/start_x86_64.S Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdisplay_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/sexdisplay/obj/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdisplay_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/sexdisplay/obj/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdisplay_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/sexdisplay/sexdisplay: Makefile build/sexdisplay/obj/src/sexdisplay.c.o build/sexdisplay/obj/src/silkbar_model.c.o build/sexdisplay/obj/runtime/client_runtime.c.o build/sexdisplay/obj/runtime/server_runtime.c.o build/sexdisplay/obj/runtime/traps_x86_64.c.o build/sexdisplay/obj/runtime/server_traps_x86_64.c.o build/sexdisplay/obj/runtime/start_x86_64.S.o build/sexdisplay/obj/generated/runtime/format_bytes.c.o build/sexdisplay/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/sexdisplay/obj/src/sexdisplay.c.o' 'build/sexdisplay/obj/src/silkbar_model.c.o' 'build/sexdisplay/obj/runtime/client_runtime.c.o' 'build/sexdisplay/obj/runtime/server_runtime.c.o' 'build/sexdisplay/obj/runtime/traps_x86_64.c.o' 'build/sexdisplay/obj/runtime/server_traps_x86_64.c.o' 'build/sexdisplay/obj/runtime/start_x86_64.S.o' 'build/sexdisplay/obj/generated/runtime/format_bytes.c.o' 'build/sexdisplay/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/sexbell/obj/src/sexbell.c.o: src/sexbell.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexbell_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/sexbell.c' -o "$@"

build/sexbell/obj/runtime/client_runtime.c.o: runtime/client_runtime.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexbell_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/sexbell/obj/runtime/server_runtime.c.o: runtime/server_runtime.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexbell_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_runtime.c' -o "$@"

build/sexbell/obj/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexbell_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/sexbell/obj/runtime/server_traps_x86_64.c.o: runtime/server_traps_x86_64.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexbell_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_traps_x86_64.c' -o "$@"

build/sexbell/obj/runtime/start_x86_64.S.o: runtime/start_x86_64.S Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexbell_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/sexbell/obj/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexbell_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/sexbell/obj/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexbell_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/sexbell/sexbell: Makefile build/sexbell/obj/src/sexbell.c.o build/sexbell/obj/runtime/client_runtime.c.o build/sexbell/obj/runtime/server_runtime.c.o build/sexbell/obj/runtime/traps_x86_64.c.o build/sexbell/obj/runtime/server_traps_x86_64.c.o build/sexbell/obj/runtime/start_x86_64.S.o build/sexbell/obj/generated/runtime/format_bytes.c.o build/sexbell/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/sexbell/obj/src/sexbell.c.o' 'build/sexbell/obj/runtime/client_runtime.c.o' 'build/sexbell/obj/runtime/server_runtime.c.o' 'build/sexbell/obj/runtime/traps_x86_64.c.o' 'build/sexbell/obj/runtime/server_traps_x86_64.c.o' 'build/sexbell/obj/runtime/start_x86_64.S.o' 'build/sexbell/obj/generated/runtime/format_bytes.c.o' 'build/sexbell/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/linen/obj/src/linen.c.o: src/linen.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=linen_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/linen.c' -o "$@"

build/linen/obj/runtime/client_runtime.c.o: runtime/client_runtime.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=linen_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/linen/obj/runtime/server_runtime.c.o: runtime/server_runtime.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=linen_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_runtime.c' -o "$@"

build/linen/obj/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=linen_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/linen/obj/runtime/server_traps_x86_64.c.o: runtime/server_traps_x86_64.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=linen_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_traps_x86_64.c' -o "$@"

build/linen/obj/runtime/start_x86_64.S.o: runtime/start_x86_64.S Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=linen_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/linen/obj/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=linen_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/linen/obj/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=linen_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/linen/linen: Makefile build/linen/obj/src/linen.c.o build/linen/obj/runtime/client_runtime.c.o build/linen/obj/runtime/server_runtime.c.o build/linen/obj/runtime/traps_x86_64.c.o build/linen/obj/runtime/server_traps_x86_64.c.o build/linen/obj/runtime/start_x86_64.S.o build/linen/obj/generated/runtime/format_bytes.c.o build/linen/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/linen/obj/src/linen.c.o' 'build/linen/obj/runtime/client_runtime.c.o' 'build/linen/obj/runtime/server_runtime.c.o' 'build/linen/obj/runtime/traps_x86_64.c.o' 'build/linen/obj/runtime/server_traps_x86_64.c.o' 'build/linen/obj/runtime/start_x86_64.S.o' 'build/linen/obj/generated/runtime/format_bytes.c.o' 'build/linen/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/quil/obj/src/quil.c.o: src/quil.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=quil_main' '-Dsexfiles_quil_persistence_proof' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/quil.c' -o "$@"

build/quil/obj/runtime/client_runtime.c.o: runtime/client_runtime.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=quil_main' '-Dsexfiles_quil_persistence_proof' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/quil/obj/runtime/server_runtime.c.o: runtime/server_runtime.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=quil_main' '-Dsexfiles_quil_persistence_proof' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_runtime.c' -o "$@"

build/quil/obj/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=quil_main' '-Dsexfiles_quil_persistence_proof' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/quil/obj/runtime/server_traps_x86_64.c.o: runtime/server_traps_x86_64.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=quil_main' '-Dsexfiles_quil_persistence_proof' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_traps_x86_64.c' -o "$@"

build/quil/obj/runtime/start_x86_64.S.o: runtime/start_x86_64.S Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=quil_main' '-Dsexfiles_quil_persistence_proof' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/quil/obj/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=quil_main' '-Dsexfiles_quil_persistence_proof' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/quil/obj/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=quil_main' '-Dsexfiles_quil_persistence_proof' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/quil/quil: Makefile build/quil/obj/src/quil.c.o build/quil/obj/runtime/client_runtime.c.o build/quil/obj/runtime/server_runtime.c.o build/quil/obj/runtime/traps_x86_64.c.o build/quil/obj/runtime/server_traps_x86_64.c.o build/quil/obj/runtime/start_x86_64.S.o build/quil/obj/generated/runtime/format_bytes.c.o build/quil/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/quil/obj/src/quil.c.o' 'build/quil/obj/runtime/client_runtime.c.o' 'build/quil/obj/runtime/server_runtime.c.o' 'build/quil/obj/runtime/traps_x86_64.c.o' 'build/quil/obj/runtime/server_traps_x86_64.c.o' 'build/quil/obj/runtime/start_x86_64.S.o' 'build/quil/obj/generated/runtime/format_bytes.c.o' 'build/quil/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/spindle/obj/src/spindle.c.o: src/spindle.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=spindle_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/spindle.c' -o "$@"

build/spindle/obj/src/sex_graphics.c.o: src/sex_graphics.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=spindle_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/sex_graphics.c' -o "$@"

build/spindle/obj/runtime/client_runtime.c.o: runtime/client_runtime.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=spindle_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/spindle/obj/runtime/server_runtime.c.o: runtime/server_runtime.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=spindle_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_runtime.c' -o "$@"

build/spindle/obj/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=spindle_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/spindle/obj/runtime/server_traps_x86_64.c.o: runtime/server_traps_x86_64.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=spindle_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_traps_x86_64.c' -o "$@"

build/spindle/obj/runtime/start_x86_64.S.o: runtime/start_x86_64.S Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=spindle_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/spindle/obj/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=spindle_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/spindle/obj/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=spindle_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/spindle/spindle: Makefile build/spindle/obj/src/spindle.c.o build/spindle/obj/src/sex_graphics.c.o build/spindle/obj/runtime/client_runtime.c.o build/spindle/obj/runtime/server_runtime.c.o build/spindle/obj/runtime/traps_x86_64.c.o build/spindle/obj/runtime/server_traps_x86_64.c.o build/spindle/obj/runtime/start_x86_64.S.o build/spindle/obj/generated/runtime/format_bytes.c.o build/spindle/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/spindle/obj/src/spindle.c.o' 'build/spindle/obj/src/sex_graphics.c.o' 'build/spindle/obj/runtime/client_runtime.c.o' 'build/spindle/obj/runtime/server_runtime.c.o' 'build/spindle/obj/runtime/traps_x86_64.c.o' 'build/spindle/obj/runtime/server_traps_x86_64.c.o' 'build/spindle/obj/runtime/start_x86_64.S.o' 'build/spindle/obj/generated/runtime/format_bytes.c.o' 'build/spindle/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/purple-scanout/obj/src/purple_scanout.c.o: src/purple_scanout.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=purple_scanout_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/purple_scanout.c' -o "$@"

build/purple-scanout/obj/runtime/client_runtime.c.o: runtime/client_runtime.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=purple_scanout_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/purple-scanout/obj/runtime/server_runtime.c.o: runtime/server_runtime.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=purple_scanout_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_runtime.c' -o "$@"

build/purple-scanout/obj/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=purple_scanout_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/purple-scanout/obj/runtime/server_traps_x86_64.c.o: runtime/server_traps_x86_64.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=purple_scanout_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_traps_x86_64.c' -o "$@"

build/purple-scanout/obj/runtime/start_x86_64.S.o: runtime/start_x86_64.S Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=purple_scanout_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/purple-scanout/obj/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=purple_scanout_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/purple-scanout/obj/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=purple_scanout_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/purple-scanout/purple-scanout: Makefile build/purple-scanout/obj/src/purple_scanout.c.o build/purple-scanout/obj/runtime/client_runtime.c.o build/purple-scanout/obj/runtime/server_runtime.c.o build/purple-scanout/obj/runtime/traps_x86_64.c.o build/purple-scanout/obj/runtime/server_traps_x86_64.c.o build/purple-scanout/obj/runtime/start_x86_64.S.o build/purple-scanout/obj/generated/runtime/format_bytes.c.o build/purple-scanout/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/purple-scanout/obj/src/purple_scanout.c.o' 'build/purple-scanout/obj/runtime/client_runtime.c.o' 'build/purple-scanout/obj/runtime/server_runtime.c.o' 'build/purple-scanout/obj/runtime/traps_x86_64.c.o' 'build/purple-scanout/obj/runtime/server_traps_x86_64.c.o' 'build/purple-scanout/obj/runtime/start_x86_64.S.o' 'build/purple-scanout/obj/generated/runtime/format_bytes.c.o' 'build/purple-scanout/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/sexfiles/obj/src/sexfiles.c.o: src/sexfiles.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexfiles_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/sexfiles.c' -o "$@"

build/sexfiles/obj/src/sexfiles_diskfs.c.o: src/sexfiles_diskfs.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexfiles_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/sexfiles_diskfs.c' -o "$@"

build/sexfiles/obj/src/sexfiles_proof.c.o: src/sexfiles_proof.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexfiles_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/sexfiles_proof.c' -o "$@"

build/sexfiles/obj/runtime/client_runtime.c.o: runtime/client_runtime.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexfiles_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/sexfiles/obj/runtime/server_runtime.c.o: runtime/server_runtime.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexfiles_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_runtime.c' -o "$@"

build/sexfiles/obj/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexfiles_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/sexfiles/obj/runtime/server_traps_x86_64.c.o: runtime/server_traps_x86_64.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexfiles_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_traps_x86_64.c' -o "$@"

build/sexfiles/obj/runtime/start_x86_64.S.o: runtime/start_x86_64.S Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexfiles_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/sexfiles/obj/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexfiles_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/sexfiles/obj/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexfiles_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/sexfiles/sexfiles: Makefile build/sexfiles/obj/src/sexfiles.c.o build/sexfiles/obj/src/sexfiles_diskfs.c.o build/sexfiles/obj/src/sexfiles_proof.c.o build/sexfiles/obj/runtime/client_runtime.c.o build/sexfiles/obj/runtime/server_runtime.c.o build/sexfiles/obj/runtime/traps_x86_64.c.o build/sexfiles/obj/runtime/server_traps_x86_64.c.o build/sexfiles/obj/runtime/start_x86_64.S.o build/sexfiles/obj/generated/runtime/format_bytes.c.o build/sexfiles/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/sexfiles/obj/src/sexfiles.c.o' 'build/sexfiles/obj/src/sexfiles_diskfs.c.o' 'build/sexfiles/obj/src/sexfiles_proof.c.o' 'build/sexfiles/obj/runtime/client_runtime.c.o' 'build/sexfiles/obj/runtime/server_runtime.c.o' 'build/sexfiles/obj/runtime/traps_x86_64.c.o' 'build/sexfiles/obj/runtime/server_traps_x86_64.c.o' 'build/sexfiles/obj/runtime/start_x86_64.S.o' 'build/sexfiles/obj/generated/runtime/format_bytes.c.o' 'build/sexfiles/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/sexdrive/obj/src/sexdrive.c.o: src/sexdrive.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdrive_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/sexdrive.c' -o "$@"

build/sexdrive/obj/runtime/client_runtime.c.o: runtime/client_runtime.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdrive_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/sexdrive/obj/runtime/server_runtime.c.o: runtime/server_runtime.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdrive_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_runtime.c' -o "$@"

build/sexdrive/obj/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdrive_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/sexdrive/obj/runtime/server_traps_x86_64.c.o: runtime/server_traps_x86_64.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdrive_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_traps_x86_64.c' -o "$@"

build/sexdrive/obj/runtime/mmio_traps_x86_64.c.o: runtime/mmio_traps_x86_64.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdrive_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/mmio_traps_x86_64.c' -o "$@"

build/sexdrive/obj/runtime/start_x86_64.S.o: runtime/start_x86_64.S Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdrive_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/sexdrive/obj/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdrive_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/sexdrive/obj/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexdrive_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/sexdrive/sexdrive: Makefile build/sexdrive/obj/src/sexdrive.c.o build/sexdrive/obj/runtime/client_runtime.c.o build/sexdrive/obj/runtime/server_runtime.c.o build/sexdrive/obj/runtime/traps_x86_64.c.o build/sexdrive/obj/runtime/server_traps_x86_64.c.o build/sexdrive/obj/runtime/mmio_traps_x86_64.c.o build/sexdrive/obj/runtime/start_x86_64.S.o build/sexdrive/obj/generated/runtime/format_bytes.c.o build/sexdrive/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/sexdrive/obj/src/sexdrive.c.o' 'build/sexdrive/obj/runtime/client_runtime.c.o' 'build/sexdrive/obj/runtime/server_runtime.c.o' 'build/sexdrive/obj/runtime/traps_x86_64.c.o' 'build/sexdrive/obj/runtime/server_traps_x86_64.c.o' 'build/sexdrive/obj/runtime/mmio_traps_x86_64.c.o' 'build/sexdrive/obj/runtime/start_x86_64.S.o' 'build/sexdrive/obj/generated/runtime/format_bytes.c.o' 'build/sexdrive/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/sexusb/obj/src/sexusb.c.o: src/sexusb.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexusb_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/sexusb.c' -o "$@"

build/sexusb/obj/runtime/client_runtime.c.o: runtime/client_runtime.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexusb_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/sexusb/obj/runtime/server_runtime.c.o: runtime/server_runtime.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexusb_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_runtime.c' -o "$@"

build/sexusb/obj/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexusb_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/sexusb/obj/runtime/server_traps_x86_64.c.o: runtime/server_traps_x86_64.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexusb_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_traps_x86_64.c' -o "$@"

build/sexusb/obj/runtime/mmio_traps_x86_64.c.o: runtime/mmio_traps_x86_64.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexusb_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/mmio_traps_x86_64.c' -o "$@"

build/sexusb/obj/runtime/start_x86_64.S.o: runtime/start_x86_64.S Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexusb_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/sexusb/obj/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexusb_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/sexusb/obj/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexusb_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/sexusb/sexusb: Makefile build/sexusb/obj/src/sexusb.c.o build/sexusb/obj/runtime/client_runtime.c.o build/sexusb/obj/runtime/server_runtime.c.o build/sexusb/obj/runtime/traps_x86_64.c.o build/sexusb/obj/runtime/server_traps_x86_64.c.o build/sexusb/obj/runtime/mmio_traps_x86_64.c.o build/sexusb/obj/runtime/start_x86_64.S.o build/sexusb/obj/generated/runtime/format_bytes.c.o build/sexusb/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/sexusb/obj/src/sexusb.c.o' 'build/sexusb/obj/runtime/client_runtime.c.o' 'build/sexusb/obj/runtime/server_runtime.c.o' 'build/sexusb/obj/runtime/traps_x86_64.c.o' 'build/sexusb/obj/runtime/server_traps_x86_64.c.o' 'build/sexusb/obj/runtime/mmio_traps_x86_64.c.o' 'build/sexusb/obj/runtime/start_x86_64.S.o' 'build/sexusb/obj/generated/runtime/format_bytes.c.o' 'build/sexusb/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/sexnet/obj/src/sexnet.c.o: src/sexnet.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexnet_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/sexnet.c' -o "$@"

build/sexnet/obj/runtime/client_runtime.c.o: runtime/client_runtime.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexnet_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/sexnet/obj/runtime/server_runtime.c.o: runtime/server_runtime.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexnet_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_runtime.c' -o "$@"

build/sexnet/obj/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexnet_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/sexnet/obj/runtime/server_traps_x86_64.c.o: runtime/server_traps_x86_64.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexnet_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_traps_x86_64.c' -o "$@"

build/sexnet/obj/runtime/mmio_traps_x86_64.c.o: runtime/mmio_traps_x86_64.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexnet_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/mmio_traps_x86_64.c' -o "$@"

build/sexnet/obj/runtime/start_x86_64.S.o: runtime/start_x86_64.S Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexnet_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/sexnet/obj/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexnet_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/sexnet/obj/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=sexnet_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/sexnet/sexnet: Makefile build/sexnet/obj/src/sexnet.c.o build/sexnet/obj/runtime/client_runtime.c.o build/sexnet/obj/runtime/server_runtime.c.o build/sexnet/obj/runtime/traps_x86_64.c.o build/sexnet/obj/runtime/server_traps_x86_64.c.o build/sexnet/obj/runtime/mmio_traps_x86_64.c.o build/sexnet/obj/runtime/start_x86_64.S.o build/sexnet/obj/generated/runtime/format_bytes.c.o build/sexnet/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/sexnet/obj/src/sexnet.c.o' 'build/sexnet/obj/runtime/client_runtime.c.o' 'build/sexnet/obj/runtime/server_runtime.c.o' 'build/sexnet/obj/runtime/traps_x86_64.c.o' 'build/sexnet/obj/runtime/server_traps_x86_64.c.o' 'build/sexnet/obj/runtime/mmio_traps_x86_64.c.o' 'build/sexnet/obj/runtime/start_x86_64.S.o' 'build/sexnet/obj/generated/runtime/format_bytes.c.o' 'build/sexnet/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/kaleidoscope-network/obj/src/network_browser.c.o: src/network_browser.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_browser_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_browser.c' -o "$@"

build/kaleidoscope-network/obj/src/network_observation.c.o: src/network_observation.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_browser_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_observation.c' -o "$@"

build/kaleidoscope-network/obj/src/rpc_runtime.c.o: src/rpc_runtime.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_browser_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/rpc_runtime.c' -o "$@"

build/kaleidoscope-network/obj/src/rpc_wait.c.o: src/rpc_wait.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_browser_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/rpc_wait.c' -o "$@"

build/kaleidoscope-network/obj/runtime/client_runtime.c.o: runtime/client_runtime.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_browser_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/kaleidoscope-network/obj/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_browser_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/kaleidoscope-network/obj/runtime/server_traps_x86_64.c.o: runtime/server_traps_x86_64.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_browser_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_traps_x86_64.c' -o "$@"

build/kaleidoscope-network/obj/runtime/rpc_traps_x86_64.c.o: runtime/rpc_traps_x86_64.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_browser_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/rpc_traps_x86_64.c' -o "$@"

build/kaleidoscope-network/obj/runtime/start_x86_64.S.o: runtime/start_x86_64.S Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_browser_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/kaleidoscope-network/obj/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_browser_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/kaleidoscope-network/obj/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_browser_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/kaleidoscope-network/kaleidoscope-network: Makefile build/kaleidoscope-network/obj/src/network_browser.c.o build/kaleidoscope-network/obj/src/network_observation.c.o build/kaleidoscope-network/obj/src/rpc_runtime.c.o build/kaleidoscope-network/obj/src/rpc_wait.c.o build/kaleidoscope-network/obj/runtime/client_runtime.c.o build/kaleidoscope-network/obj/runtime/traps_x86_64.c.o build/kaleidoscope-network/obj/runtime/server_traps_x86_64.c.o build/kaleidoscope-network/obj/runtime/rpc_traps_x86_64.c.o build/kaleidoscope-network/obj/runtime/start_x86_64.S.o build/kaleidoscope-network/obj/generated/runtime/format_bytes.c.o build/kaleidoscope-network/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/kaleidoscope-network/obj/src/network_browser.c.o' 'build/kaleidoscope-network/obj/src/network_observation.c.o' 'build/kaleidoscope-network/obj/src/rpc_runtime.c.o' 'build/kaleidoscope-network/obj/src/rpc_wait.c.o' 'build/kaleidoscope-network/obj/runtime/client_runtime.c.o' 'build/kaleidoscope-network/obj/runtime/traps_x86_64.c.o' 'build/kaleidoscope-network/obj/runtime/server_traps_x86_64.c.o' 'build/kaleidoscope-network/obj/runtime/rpc_traps_x86_64.c.o' 'build/kaleidoscope-network/obj/runtime/start_x86_64.S.o' 'build/kaleidoscope-network/obj/generated/runtime/format_bytes.c.o' 'build/kaleidoscope-network/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/sexnet-network/obj/src/network_service.c.o: src/network_service.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_service.c' -o "$@"

build/sexnet-network/obj/src/network_observation.c.o: src/network_observation.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_observation.c' -o "$@"

build/sexnet-network/obj/src/network_device.c.o: src/network_device.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_device.c' -o "$@"

build/sexnet-network/obj/src/network_send.c.o: src/network_send.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_send.c' -o "$@"

build/sexnet-network/obj/src/network_tcp.c.o: src/network_tcp.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_tcp.c' -o "$@"

build/sexnet-network/obj/src/network_http.c.o: src/network_http.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/network_http.c' -o "$@"

build/sexnet-network/obj/src/rpc_runtime.c.o: src/rpc_runtime.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/rpc_runtime.c' -o "$@"

build/sexnet-network/obj/src/rpc_wait.c.o: src/rpc_wait.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/rpc_wait.c' -o "$@"

build/sexnet-network/obj/runtime/client_runtime.c.o: runtime/client_runtime.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/client_runtime.c' -o "$@"

build/sexnet-network/obj/runtime/server_runtime.c.o: runtime/server_runtime.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_runtime.c' -o "$@"

build/sexnet-network/obj/runtime/traps_x86_64.c.o: runtime/traps_x86_64.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/traps_x86_64.c' -o "$@"

build/sexnet-network/obj/runtime/server_traps_x86_64.c.o: runtime/server_traps_x86_64.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/server_traps_x86_64.c' -o "$@"

build/sexnet-network/obj/runtime/rpc_traps_x86_64.c.o: runtime/rpc_traps_x86_64.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/rpc_traps_x86_64.c' -o "$@"

build/sexnet-network/obj/runtime/mmio_traps_x86_64.c.o: runtime/mmio_traps_x86_64.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/mmio_traps_x86_64.c' -o "$@"

build/sexnet-network/obj/runtime/start_x86_64.S.o: runtime/start_x86_64.S Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/start_x86_64.S' -o "$@"

build/sexnet-network/obj/generated/runtime/format_bytes.c.o: generated/runtime/format_bytes.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/format_bytes.c' -o "$@"

build/sexnet-network/obj/generated/runtime/packed_words.c.o: generated/runtime/packed_words.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fPIE' '-fno-stack-protector' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/include' '-DSEXOS_MAIN=network_service_main' -MMD -MP -MF "$@.d" -MQ "$@" -c 'generated/runtime/packed_words.c' -o "$@"

build/sexnet-network/sexnet-network: Makefile build/sexnet-network/obj/src/network_service.c.o build/sexnet-network/obj/src/network_observation.c.o build/sexnet-network/obj/src/network_device.c.o build/sexnet-network/obj/src/network_send.c.o build/sexnet-network/obj/src/network_tcp.c.o build/sexnet-network/obj/src/network_http.c.o build/sexnet-network/obj/src/rpc_runtime.c.o build/sexnet-network/obj/src/rpc_wait.c.o build/sexnet-network/obj/runtime/client_runtime.c.o build/sexnet-network/obj/runtime/server_runtime.c.o build/sexnet-network/obj/runtime/traps_x86_64.c.o build/sexnet-network/obj/runtime/server_traps_x86_64.c.o build/sexnet-network/obj/runtime/rpc_traps_x86_64.c.o build/sexnet-network/obj/runtime/mmio_traps_x86_64.c.o build/sexnet-network/obj/runtime/start_x86_64.S.o build/sexnet-network/obj/generated/runtime/format_bytes.c.o build/sexnet-network/obj/generated/runtime/packed_words.c.o
	@mkdir -p "$(@D)"
	$(CC) 'build/sexnet-network/obj/src/network_service.c.o' 'build/sexnet-network/obj/src/network_observation.c.o' 'build/sexnet-network/obj/src/network_device.c.o' 'build/sexnet-network/obj/src/network_send.c.o' 'build/sexnet-network/obj/src/network_tcp.c.o' 'build/sexnet-network/obj/src/network_http.c.o' 'build/sexnet-network/obj/src/rpc_runtime.c.o' 'build/sexnet-network/obj/src/rpc_wait.c.o' 'build/sexnet-network/obj/runtime/client_runtime.c.o' 'build/sexnet-network/obj/runtime/server_runtime.c.o' 'build/sexnet-network/obj/runtime/traps_x86_64.c.o' 'build/sexnet-network/obj/runtime/server_traps_x86_64.c.o' 'build/sexnet-network/obj/runtime/rpc_traps_x86_64.c.o' 'build/sexnet-network/obj/runtime/mmio_traps_x86_64.c.o' 'build/sexnet-network/obj/runtime/start_x86_64.S.o' 'build/sexnet-network/obj/generated/runtime/format_bytes.c.o' 'build/sexnet-network/obj/generated/runtime/packed_words.c.o' '-nostdlib' '-static-pie' '-Wl,-e,_start' '-Wl,--build-id=none' -o "$@"

build/sexos-kernel/obj/src/kernel_memory.c.o: src/kernel_memory.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_memory.c' -o "$@"

build/sexos-kernel/obj/src/kernel_platform.c.o: src/kernel_platform.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_platform.c' -o "$@"

build/sexos-kernel/obj/src/kernel_domains.c.o: src/kernel_domains.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_domains.c' -o "$@"

build/sexos-kernel/obj/src/kernel_entry.c.o: src/kernel_entry.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_entry.c' -o "$@"

build/sexos-kernel/obj/src/kernel_syscalls.c.o: src/kernel_syscalls.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_syscalls.c' -o "$@"

build/sexos-kernel/obj/src/kernel_rpc.c.o: src/kernel_rpc.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_rpc.c' -o "$@"

build/sexos-kernel/obj/runtime/kernel/kernel_log.c.o: runtime/kernel/kernel_log.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/kernel/kernel_log.c' -o "$@"

build/sexos-kernel/obj/runtime/kernel/entry_x86_64.S.o: runtime/kernel/entry_x86_64.S Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/kernel/entry_x86_64.S' -o "$@"

build/sexos-kernel/sexos-kernel: Makefile build/sexos-kernel/obj/src/kernel_memory.c.o build/sexos-kernel/obj/src/kernel_platform.c.o build/sexos-kernel/obj/src/kernel_domains.c.o build/sexos-kernel/obj/src/kernel_entry.c.o build/sexos-kernel/obj/src/kernel_syscalls.c.o build/sexos-kernel/obj/src/kernel_rpc.c.o build/sexos-kernel/obj/runtime/kernel/kernel_log.c.o build/sexos-kernel/obj/runtime/kernel/entry_x86_64.S.o runtime/kernel/kernel.ld
	@mkdir -p "$(@D)"
	$(CC) 'build/sexos-kernel/obj/src/kernel_memory.c.o' 'build/sexos-kernel/obj/src/kernel_platform.c.o' 'build/sexos-kernel/obj/src/kernel_domains.c.o' 'build/sexos-kernel/obj/src/kernel_entry.c.o' 'build/sexos-kernel/obj/src/kernel_syscalls.c.o' 'build/sexos-kernel/obj/src/kernel_rpc.c.o' 'build/sexos-kernel/obj/runtime/kernel/kernel_log.c.o' 'build/sexos-kernel/obj/runtime/kernel/entry_x86_64.S.o' '-nostdlib' '-static' '-Wl,-T,runtime/kernel/kernel.ld' '-Wl,--build-id=none' '-Wl,-z,max-page-size=0x1000' -o "$@"

build/sexos-kernel-network/obj/src/kernel_memory.c.o: src/kernel_memory.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_memory.c' -o "$@"

build/sexos-kernel-network/obj/src/kernel_platform.c.o: src/kernel_platform.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_platform.c' -o "$@"

build/sexos-kernel-network/obj/src/kernel_domains.c.o: src/kernel_domains.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_domains.c' -o "$@"

build/sexos-kernel-network/obj/src/kernel_entry.c.o: src/kernel_entry.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_entry.c' -o "$@"

build/sexos-kernel-network/obj/src/kernel_syscalls.c.o: src/kernel_syscalls.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_syscalls.c' -o "$@"

build/sexos-kernel-network/obj/src/kernel_rpc.c.o: src/kernel_rpc.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/kernel_rpc.c' -o "$@"

build/sexos-kernel-network/obj/runtime/kernel/kernel_log.c.o: runtime/kernel/kernel_log.c Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/kernel/kernel_log.c' -o "$@"

build/sexos-kernel-network/obj/runtime/kernel/entry_x86_64.S.o: runtime/kernel/entry_x86_64.S Makefile
	@mkdir -p "$(@D)"
	$(CC) '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-pic' '-fno-pie' '-mcmodel=kernel' '-fno-stack-protector' '-fno-asynchronous-unwind-tables' '-mno-red-zone' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Iruntime/kernel' '-Igenerated/include' '-DSEXOS_NETWORK_PROFILE=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/kernel/entry_x86_64.S' -o "$@"

build/sexos-kernel-network/sexos-kernel-network: Makefile build/sexos-kernel-network/obj/src/kernel_memory.c.o build/sexos-kernel-network/obj/src/kernel_platform.c.o build/sexos-kernel-network/obj/src/kernel_domains.c.o build/sexos-kernel-network/obj/src/kernel_entry.c.o build/sexos-kernel-network/obj/src/kernel_syscalls.c.o build/sexos-kernel-network/obj/src/kernel_rpc.c.o build/sexos-kernel-network/obj/runtime/kernel/kernel_log.c.o build/sexos-kernel-network/obj/runtime/kernel/entry_x86_64.S.o runtime/kernel/kernel.ld
	@mkdir -p "$(@D)"
	$(CC) 'build/sexos-kernel-network/obj/src/kernel_memory.c.o' 'build/sexos-kernel-network/obj/src/kernel_platform.c.o' 'build/sexos-kernel-network/obj/src/kernel_domains.c.o' 'build/sexos-kernel-network/obj/src/kernel_entry.c.o' 'build/sexos-kernel-network/obj/src/kernel_syscalls.c.o' 'build/sexos-kernel-network/obj/src/kernel_rpc.c.o' 'build/sexos-kernel-network/obj/runtime/kernel/kernel_log.c.o' 'build/sexos-kernel-network/obj/runtime/kernel/entry_x86_64.S.o' '-nostdlib' '-static' '-Wl,-T,runtime/kernel/kernel.ld' '-Wl,--build-id=none' '-Wl,-z,max-page-size=0x1000' -o "$@"

-include build/kaleidoscope/obj/src/kaleidoscope.c.o.d build/kaleidoscope/obj/runtime/client_runtime.c.o.d build/kaleidoscope/obj/runtime/traps_x86_64.c.o.d build/kaleidoscope/obj/runtime/start_x86_64.S.o.d build/kaleidoscope/obj/generated/runtime/format_bytes.c.o.d build/kaleidoscope/obj/generated/runtime/packed_words.c.o.d build/silkbar/obj/src/silkbar.c.o.d build/silkbar/obj/src/silkbar_model.c.o.d build/silkbar/obj/runtime/client_runtime.c.o.d build/silkbar/obj/runtime/server_runtime.c.o.d build/silkbar/obj/runtime/traps_x86_64.c.o.d build/silkbar/obj/runtime/server_traps_x86_64.c.o.d build/silkbar/obj/runtime/start_x86_64.S.o.d build/silkbar/obj/generated/runtime/format_bytes.c.o.d build/silkbar/obj/generated/runtime/packed_words.c.o.d build/sexinput/obj/src/sexinput.c.o.d build/sexinput/obj/runtime/client_runtime.c.o.d build/sexinput/obj/runtime/server_runtime.c.o.d build/sexinput/obj/runtime/traps_x86_64.c.o.d build/sexinput/obj/runtime/server_traps_x86_64.c.o.d build/sexinput/obj/runtime/start_x86_64.S.o.d build/sexinput/obj/generated/runtime/format_bytes.c.o.d build/sexinput/obj/generated/runtime/packed_words.c.o.d build/sexstore/obj/src/sexstore.c.o.d build/sexstore/obj/src/sexstore_durable.c.o.d build/sexstore/obj/src/sexstore_pages.c.o.d build/sexstore/obj/runtime/client_runtime.c.o.d build/sexstore/obj/runtime/server_runtime.c.o.d build/sexstore/obj/runtime/traps_x86_64.c.o.d build/sexstore/obj/runtime/server_traps_x86_64.c.o.d build/sexstore/obj/runtime/start_x86_64.S.o.d build/sexstore/obj/generated/runtime/format_bytes.c.o.d build/sexstore/obj/generated/runtime/packed_words.c.o.d build/sexdisplay/obj/src/sexdisplay.c.o.d build/sexdisplay/obj/src/silkbar_model.c.o.d build/sexdisplay/obj/runtime/client_runtime.c.o.d build/sexdisplay/obj/runtime/server_runtime.c.o.d build/sexdisplay/obj/runtime/traps_x86_64.c.o.d build/sexdisplay/obj/runtime/server_traps_x86_64.c.o.d build/sexdisplay/obj/runtime/start_x86_64.S.o.d build/sexdisplay/obj/generated/runtime/format_bytes.c.o.d build/sexdisplay/obj/generated/runtime/packed_words.c.o.d build/sexbell/obj/src/sexbell.c.o.d build/sexbell/obj/runtime/client_runtime.c.o.d build/sexbell/obj/runtime/server_runtime.c.o.d build/sexbell/obj/runtime/traps_x86_64.c.o.d build/sexbell/obj/runtime/server_traps_x86_64.c.o.d build/sexbell/obj/runtime/start_x86_64.S.o.d build/sexbell/obj/generated/runtime/format_bytes.c.o.d build/sexbell/obj/generated/runtime/packed_words.c.o.d build/linen/obj/src/linen.c.o.d build/linen/obj/runtime/client_runtime.c.o.d build/linen/obj/runtime/server_runtime.c.o.d build/linen/obj/runtime/traps_x86_64.c.o.d build/linen/obj/runtime/server_traps_x86_64.c.o.d build/linen/obj/runtime/start_x86_64.S.o.d build/linen/obj/generated/runtime/format_bytes.c.o.d build/linen/obj/generated/runtime/packed_words.c.o.d build/quil/obj/src/quil.c.o.d build/quil/obj/runtime/client_runtime.c.o.d build/quil/obj/runtime/server_runtime.c.o.d build/quil/obj/runtime/traps_x86_64.c.o.d build/quil/obj/runtime/server_traps_x86_64.c.o.d build/quil/obj/runtime/start_x86_64.S.o.d build/quil/obj/generated/runtime/format_bytes.c.o.d build/quil/obj/generated/runtime/packed_words.c.o.d build/spindle/obj/src/spindle.c.o.d build/spindle/obj/src/sex_graphics.c.o.d build/spindle/obj/runtime/client_runtime.c.o.d build/spindle/obj/runtime/server_runtime.c.o.d build/spindle/obj/runtime/traps_x86_64.c.o.d build/spindle/obj/runtime/server_traps_x86_64.c.o.d build/spindle/obj/runtime/start_x86_64.S.o.d build/spindle/obj/generated/runtime/format_bytes.c.o.d build/spindle/obj/generated/runtime/packed_words.c.o.d build/purple-scanout/obj/src/purple_scanout.c.o.d build/purple-scanout/obj/runtime/client_runtime.c.o.d build/purple-scanout/obj/runtime/server_runtime.c.o.d build/purple-scanout/obj/runtime/traps_x86_64.c.o.d build/purple-scanout/obj/runtime/server_traps_x86_64.c.o.d build/purple-scanout/obj/runtime/start_x86_64.S.o.d build/purple-scanout/obj/generated/runtime/format_bytes.c.o.d build/purple-scanout/obj/generated/runtime/packed_words.c.o.d build/sexfiles/obj/src/sexfiles.c.o.d build/sexfiles/obj/src/sexfiles_diskfs.c.o.d build/sexfiles/obj/src/sexfiles_proof.c.o.d build/sexfiles/obj/runtime/client_runtime.c.o.d build/sexfiles/obj/runtime/server_runtime.c.o.d build/sexfiles/obj/runtime/traps_x86_64.c.o.d build/sexfiles/obj/runtime/server_traps_x86_64.c.o.d build/sexfiles/obj/runtime/start_x86_64.S.o.d build/sexfiles/obj/generated/runtime/format_bytes.c.o.d build/sexfiles/obj/generated/runtime/packed_words.c.o.d build/sexdrive/obj/src/sexdrive.c.o.d build/sexdrive/obj/runtime/client_runtime.c.o.d build/sexdrive/obj/runtime/server_runtime.c.o.d build/sexdrive/obj/runtime/traps_x86_64.c.o.d build/sexdrive/obj/runtime/server_traps_x86_64.c.o.d build/sexdrive/obj/runtime/mmio_traps_x86_64.c.o.d build/sexdrive/obj/runtime/start_x86_64.S.o.d build/sexdrive/obj/generated/runtime/format_bytes.c.o.d build/sexdrive/obj/generated/runtime/packed_words.c.o.d build/sexusb/obj/src/sexusb.c.o.d build/sexusb/obj/runtime/client_runtime.c.o.d build/sexusb/obj/runtime/server_runtime.c.o.d build/sexusb/obj/runtime/traps_x86_64.c.o.d build/sexusb/obj/runtime/server_traps_x86_64.c.o.d build/sexusb/obj/runtime/mmio_traps_x86_64.c.o.d build/sexusb/obj/runtime/start_x86_64.S.o.d build/sexusb/obj/generated/runtime/format_bytes.c.o.d build/sexusb/obj/generated/runtime/packed_words.c.o.d build/sexnet/obj/src/sexnet.c.o.d build/sexnet/obj/runtime/client_runtime.c.o.d build/sexnet/obj/runtime/server_runtime.c.o.d build/sexnet/obj/runtime/traps_x86_64.c.o.d build/sexnet/obj/runtime/server_traps_x86_64.c.o.d build/sexnet/obj/runtime/mmio_traps_x86_64.c.o.d build/sexnet/obj/runtime/start_x86_64.S.o.d build/sexnet/obj/generated/runtime/format_bytes.c.o.d build/sexnet/obj/generated/runtime/packed_words.c.o.d build/kaleidoscope-network/obj/src/network_browser.c.o.d build/kaleidoscope-network/obj/src/network_observation.c.o.d build/kaleidoscope-network/obj/src/rpc_runtime.c.o.d build/kaleidoscope-network/obj/src/rpc_wait.c.o.d build/kaleidoscope-network/obj/runtime/client_runtime.c.o.d build/kaleidoscope-network/obj/runtime/traps_x86_64.c.o.d build/kaleidoscope-network/obj/runtime/server_traps_x86_64.c.o.d build/kaleidoscope-network/obj/runtime/rpc_traps_x86_64.c.o.d build/kaleidoscope-network/obj/runtime/start_x86_64.S.o.d build/kaleidoscope-network/obj/generated/runtime/format_bytes.c.o.d build/kaleidoscope-network/obj/generated/runtime/packed_words.c.o.d build/sexnet-network/obj/src/network_service.c.o.d build/sexnet-network/obj/src/network_observation.c.o.d build/sexnet-network/obj/src/network_device.c.o.d build/sexnet-network/obj/src/network_send.c.o.d build/sexnet-network/obj/src/network_tcp.c.o.d build/sexnet-network/obj/src/network_http.c.o.d build/sexnet-network/obj/src/rpc_runtime.c.o.d build/sexnet-network/obj/src/rpc_wait.c.o.d build/sexnet-network/obj/runtime/client_runtime.c.o.d build/sexnet-network/obj/runtime/server_runtime.c.o.d build/sexnet-network/obj/runtime/traps_x86_64.c.o.d build/sexnet-network/obj/runtime/server_traps_x86_64.c.o.d build/sexnet-network/obj/runtime/rpc_traps_x86_64.c.o.d build/sexnet-network/obj/runtime/mmio_traps_x86_64.c.o.d build/sexnet-network/obj/runtime/start_x86_64.S.o.d build/sexnet-network/obj/generated/runtime/format_bytes.c.o.d build/sexnet-network/obj/generated/runtime/packed_words.c.o.d build/sexos-kernel/obj/src/kernel_memory.c.o.d build/sexos-kernel/obj/src/kernel_platform.c.o.d build/sexos-kernel/obj/src/kernel_domains.c.o.d build/sexos-kernel/obj/src/kernel_entry.c.o.d build/sexos-kernel/obj/src/kernel_syscalls.c.o.d build/sexos-kernel/obj/src/kernel_rpc.c.o.d build/sexos-kernel/obj/runtime/kernel/kernel_log.c.o.d build/sexos-kernel/obj/runtime/kernel/entry_x86_64.S.o.d build/sexos-kernel-network/obj/src/kernel_memory.c.o.d build/sexos-kernel-network/obj/src/kernel_platform.c.o.d build/sexos-kernel-network/obj/src/kernel_domains.c.o.d build/sexos-kernel-network/obj/src/kernel_entry.c.o.d build/sexos-kernel-network/obj/src/kernel_syscalls.c.o.d build/sexos-kernel-network/obj/src/kernel_rpc.c.o.d build/sexos-kernel-network/obj/runtime/kernel/kernel_log.c.o.d build/sexos-kernel-network/obj/runtime/kernel/entry_x86_64.S.o.d
