# Readable C exported by cesta build from cesta.arm64.project; builds without Cesta.
CC = clang
AR = ar
.DEFAULT_GOAL := all
.DELETE_ON_ERROR:

.PHONY: all
all: build-arm64/arm64-phase2/arm64-phase2 build-arm64/arm64-phase3/arm64-phase3 build-arm64/arm64-phase4/arm64-phase4 build-arm64/arm64-phase5/arm64-phase5

build-arm64/.objects/62fb10017ae769b0/src/arm64_boot_policy.cesta.o: src/arm64_boot_policy.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '--target=aarch64-none-elf' '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-stack-protector' '-fno-pic' '-fno-pie' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/arm64/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/arm64_boot_policy.c' -o "$@"

build-arm64/arm64-phase2/obj/src/arm64_boot_policy.cesta.o: build-arm64/.objects/62fb10017ae769b0/src/arm64_boot_policy.cesta.o
	@mkdir -p 'build-arm64/arm64-phase2/obj/src'
	ln -f 'build-arm64/.objects/62fb10017ae769b0/src/arm64_boot_policy.cesta.o' "$@"

build-arm64/.objects/85cd4996f49b5f4c/src/arm64_platform.cesta.o: src/arm64_platform.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '--target=aarch64-none-elf' '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-stack-protector' '-fno-pic' '-fno-pie' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/arm64/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/arm64_platform.c' -o "$@"

build-arm64/arm64-phase2/obj/src/arm64_platform.cesta.o: build-arm64/.objects/85cd4996f49b5f4c/src/arm64_platform.cesta.o
	@mkdir -p 'build-arm64/arm64-phase2/obj/src'
	ln -f 'build-arm64/.objects/85cd4996f49b5f4c/src/arm64_platform.cesta.o' "$@"

build-arm64/.objects/39cbe8bbddc94370/src/arm64_phase2.cesta.o: src/arm64_phase2.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '--target=aarch64-none-elf' '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-stack-protector' '-fno-pic' '-fno-pie' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/arm64/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/arm64_phase2.c' -o "$@"

build-arm64/arm64-phase2/obj/src/arm64_phase2.cesta.o: build-arm64/.objects/39cbe8bbddc94370/src/arm64_phase2.cesta.o
	@mkdir -p 'build-arm64/arm64-phase2/obj/src'
	ln -f 'build-arm64/.objects/39cbe8bbddc94370/src/arm64_phase2.cesta.o' "$@"

build-arm64/.objects/6889cacfec5e0c99/runtime/arm64/entry.S.o: runtime/arm64/entry.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '--target=aarch64-none-elf' '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-stack-protector' '-fno-pic' '-fno-pie' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/arm64/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/arm64/entry.S' -o "$@"

build-arm64/arm64-phase2/obj/runtime/arm64/entry.S.o: build-arm64/.objects/6889cacfec5e0c99/runtime/arm64/entry.S.o
	@mkdir -p 'build-arm64/arm64-phase2/obj/runtime/arm64'
	ln -f 'build-arm64/.objects/6889cacfec5e0c99/runtime/arm64/entry.S.o' "$@"

build-arm64/arm64-phase2/arm64-phase2: GNUmakefile build-arm64/arm64-phase2/obj/src/arm64_boot_policy.cesta.o build-arm64/arm64-phase2/obj/src/arm64_platform.cesta.o build-arm64/arm64-phase2/obj/src/arm64_phase2.cesta.o build-arm64/arm64-phase2/obj/runtime/arm64/entry.S.o runtime/arm64/kernel.ld
	@mkdir -p "$(@D)"
	$(CC) 'build-arm64/arm64-phase2/obj/src/arm64_boot_policy.cesta.o' 'build-arm64/arm64-phase2/obj/src/arm64_platform.cesta.o' 'build-arm64/arm64-phase2/obj/src/arm64_phase2.cesta.o' 'build-arm64/arm64-phase2/obj/runtime/arm64/entry.S.o' '--target=aarch64-none-elf' '-fuse-ld=lld' '-nostdlib' '-static' '-Wl,-T,runtime/arm64/kernel.ld' '-Wl,--build-id=none' -o "$@"

build-arm64/arm64-phase3/obj/src/arm64_boot_policy.cesta.o: build-arm64/.objects/62fb10017ae769b0/src/arm64_boot_policy.cesta.o
	@mkdir -p 'build-arm64/arm64-phase3/obj/src'
	ln -f 'build-arm64/.objects/62fb10017ae769b0/src/arm64_boot_policy.cesta.o' "$@"

build-arm64/.objects/f874d2e7874ea064/src/arm64_phase3_policy.cesta.o: src/arm64_phase3_policy.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '--target=aarch64-none-elf' '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-stack-protector' '-fno-pic' '-fno-pie' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/arm64/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/arm64_phase3_policy.c' -o "$@"

build-arm64/arm64-phase3/obj/src/arm64_phase3_policy.cesta.o: build-arm64/.objects/f874d2e7874ea064/src/arm64_phase3_policy.cesta.o
	@mkdir -p 'build-arm64/arm64-phase3/obj/src'
	ln -f 'build-arm64/.objects/f874d2e7874ea064/src/arm64_phase3_policy.cesta.o' "$@"

build-arm64/arm64-phase3/obj/src/arm64_platform.cesta.o: build-arm64/.objects/85cd4996f49b5f4c/src/arm64_platform.cesta.o
	@mkdir -p 'build-arm64/arm64-phase3/obj/src'
	ln -f 'build-arm64/.objects/85cd4996f49b5f4c/src/arm64_platform.cesta.o' "$@"

build-arm64/.objects/a40e7b437b5272fd/src/arm64_phase3.cesta.o: src/arm64_phase3.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '--target=aarch64-none-elf' '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-stack-protector' '-fno-pic' '-fno-pie' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/arm64/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/arm64_phase3.c' -o "$@"

build-arm64/arm64-phase3/obj/src/arm64_phase3.cesta.o: build-arm64/.objects/a40e7b437b5272fd/src/arm64_phase3.cesta.o
	@mkdir -p 'build-arm64/arm64-phase3/obj/src'
	ln -f 'build-arm64/.objects/a40e7b437b5272fd/src/arm64_phase3.cesta.o' "$@"

build-arm64/.objects/df59441f16427b66/runtime/arm64/phase3_entry.S.o: runtime/arm64/phase3_entry.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '--target=aarch64-none-elf' '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-stack-protector' '-fno-pic' '-fno-pie' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/arm64/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/arm64/phase3_entry.S' -o "$@"

build-arm64/arm64-phase3/obj/runtime/arm64/phase3_entry.S.o: build-arm64/.objects/df59441f16427b66/runtime/arm64/phase3_entry.S.o
	@mkdir -p 'build-arm64/arm64-phase3/obj/runtime/arm64'
	ln -f 'build-arm64/.objects/df59441f16427b66/runtime/arm64/phase3_entry.S.o' "$@"

build-arm64/.objects/ec1dbba15de9e042/runtime/arm64/phase3_vectors.S.o: runtime/arm64/phase3_vectors.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '--target=aarch64-none-elf' '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-stack-protector' '-fno-pic' '-fno-pie' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/arm64/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/arm64/phase3_vectors.S' -o "$@"

build-arm64/arm64-phase3/obj/runtime/arm64/phase3_vectors.S.o: build-arm64/.objects/ec1dbba15de9e042/runtime/arm64/phase3_vectors.S.o
	@mkdir -p 'build-arm64/arm64-phase3/obj/runtime/arm64'
	ln -f 'build-arm64/.objects/ec1dbba15de9e042/runtime/arm64/phase3_vectors.S.o' "$@"

build-arm64/arm64-phase3/arm64-phase3: GNUmakefile build-arm64/arm64-phase3/obj/src/arm64_boot_policy.cesta.o build-arm64/arm64-phase3/obj/src/arm64_phase3_policy.cesta.o build-arm64/arm64-phase3/obj/src/arm64_platform.cesta.o build-arm64/arm64-phase3/obj/src/arm64_phase3.cesta.o build-arm64/arm64-phase3/obj/runtime/arm64/phase3_entry.S.o build-arm64/arm64-phase3/obj/runtime/arm64/phase3_vectors.S.o runtime/arm64/kernel.ld
	@mkdir -p "$(@D)"
	$(CC) 'build-arm64/arm64-phase3/obj/src/arm64_boot_policy.cesta.o' 'build-arm64/arm64-phase3/obj/src/arm64_phase3_policy.cesta.o' 'build-arm64/arm64-phase3/obj/src/arm64_platform.cesta.o' 'build-arm64/arm64-phase3/obj/src/arm64_phase3.cesta.o' 'build-arm64/arm64-phase3/obj/runtime/arm64/phase3_entry.S.o' 'build-arm64/arm64-phase3/obj/runtime/arm64/phase3_vectors.S.o' '--target=aarch64-none-elf' '-fuse-ld=lld' '-nostdlib' '-static' '-Wl,-T,runtime/arm64/kernel.ld' '-Wl,--build-id=none' -o "$@"

build-arm64/arm64-phase4/obj/src/arm64_boot_policy.cesta.o: build-arm64/.objects/62fb10017ae769b0/src/arm64_boot_policy.cesta.o
	@mkdir -p 'build-arm64/arm64-phase4/obj/src'
	ln -f 'build-arm64/.objects/62fb10017ae769b0/src/arm64_boot_policy.cesta.o' "$@"

build-arm64/arm64-phase4/obj/src/arm64_phase3_policy.cesta.o: build-arm64/.objects/f874d2e7874ea064/src/arm64_phase3_policy.cesta.o
	@mkdir -p 'build-arm64/arm64-phase4/obj/src'
	ln -f 'build-arm64/.objects/f874d2e7874ea064/src/arm64_phase3_policy.cesta.o' "$@"

build-arm64/.objects/2a35c8c2dee474ed/src/arm64_phase4_policy.cesta.o: src/arm64_phase4_policy.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '--target=aarch64-none-elf' '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-stack-protector' '-fno-pic' '-fno-pie' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/arm64/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/arm64_phase4_policy.c' -o "$@"

build-arm64/arm64-phase4/obj/src/arm64_phase4_policy.cesta.o: build-arm64/.objects/2a35c8c2dee474ed/src/arm64_phase4_policy.cesta.o
	@mkdir -p 'build-arm64/arm64-phase4/obj/src'
	ln -f 'build-arm64/.objects/2a35c8c2dee474ed/src/arm64_phase4_policy.cesta.o' "$@"

build-arm64/arm64-phase4/obj/src/arm64_platform.cesta.o: build-arm64/.objects/85cd4996f49b5f4c/src/arm64_platform.cesta.o
	@mkdir -p 'build-arm64/arm64-phase4/obj/src'
	ln -f 'build-arm64/.objects/85cd4996f49b5f4c/src/arm64_platform.cesta.o' "$@"

build-arm64/arm64-phase4/obj/src/arm64_phase3.cesta.o: build-arm64/.objects/a40e7b437b5272fd/src/arm64_phase3.cesta.o
	@mkdir -p 'build-arm64/arm64-phase4/obj/src'
	ln -f 'build-arm64/.objects/a40e7b437b5272fd/src/arm64_phase3.cesta.o' "$@"

build-arm64/.objects/97897b0817218b32/src/arm64_phase4.cesta.o: src/arm64_phase4.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '--target=aarch64-none-elf' '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-stack-protector' '-fno-pic' '-fno-pie' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/arm64/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/arm64_phase4.c' -o "$@"

build-arm64/arm64-phase4/obj/src/arm64_phase4.cesta.o: build-arm64/.objects/97897b0817218b32/src/arm64_phase4.cesta.o
	@mkdir -p 'build-arm64/arm64-phase4/obj/src'
	ln -f 'build-arm64/.objects/97897b0817218b32/src/arm64_phase4.cesta.o' "$@"

build-arm64/arm64-phase4/obj/runtime/arm64/phase3_entry.S.o: build-arm64/.objects/df59441f16427b66/runtime/arm64/phase3_entry.S.o
	@mkdir -p 'build-arm64/arm64-phase4/obj/runtime/arm64'
	ln -f 'build-arm64/.objects/df59441f16427b66/runtime/arm64/phase3_entry.S.o' "$@"

build-arm64/.objects/fc82ed6c4dc5d9e5/runtime/arm64/phase4_vectors.S.o: runtime/arm64/phase4_vectors.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '--target=aarch64-none-elf' '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-stack-protector' '-fno-pic' '-fno-pie' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/arm64/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/arm64/phase4_vectors.S' -o "$@"

build-arm64/arm64-phase4/obj/runtime/arm64/phase4_vectors.S.o: build-arm64/.objects/fc82ed6c4dc5d9e5/runtime/arm64/phase4_vectors.S.o
	@mkdir -p 'build-arm64/arm64-phase4/obj/runtime/arm64'
	ln -f 'build-arm64/.objects/fc82ed6c4dc5d9e5/runtime/arm64/phase4_vectors.S.o' "$@"

build-arm64/arm64-phase4/arm64-phase4: GNUmakefile build-arm64/arm64-phase4/obj/src/arm64_boot_policy.cesta.o build-arm64/arm64-phase4/obj/src/arm64_phase3_policy.cesta.o build-arm64/arm64-phase4/obj/src/arm64_phase4_policy.cesta.o build-arm64/arm64-phase4/obj/src/arm64_platform.cesta.o build-arm64/arm64-phase4/obj/src/arm64_phase3.cesta.o build-arm64/arm64-phase4/obj/src/arm64_phase4.cesta.o build-arm64/arm64-phase4/obj/runtime/arm64/phase3_entry.S.o build-arm64/arm64-phase4/obj/runtime/arm64/phase4_vectors.S.o runtime/arm64/kernel.ld
	@mkdir -p "$(@D)"
	$(CC) 'build-arm64/arm64-phase4/obj/src/arm64_boot_policy.cesta.o' 'build-arm64/arm64-phase4/obj/src/arm64_phase3_policy.cesta.o' 'build-arm64/arm64-phase4/obj/src/arm64_phase4_policy.cesta.o' 'build-arm64/arm64-phase4/obj/src/arm64_platform.cesta.o' 'build-arm64/arm64-phase4/obj/src/arm64_phase3.cesta.o' 'build-arm64/arm64-phase4/obj/src/arm64_phase4.cesta.o' 'build-arm64/arm64-phase4/obj/runtime/arm64/phase3_entry.S.o' 'build-arm64/arm64-phase4/obj/runtime/arm64/phase4_vectors.S.o' '--target=aarch64-none-elf' '-fuse-ld=lld' '-nostdlib' '-static' '-Wl,-T,runtime/arm64/kernel.ld' '-Wl,--build-id=none' -o "$@"

build-arm64/arm64-phase5/obj/src/arm64_boot_policy.cesta.o: build-arm64/.objects/62fb10017ae769b0/src/arm64_boot_policy.cesta.o
	@mkdir -p 'build-arm64/arm64-phase5/obj/src'
	ln -f 'build-arm64/.objects/62fb10017ae769b0/src/arm64_boot_policy.cesta.o' "$@"

build-arm64/arm64-phase5/obj/src/arm64_phase3_policy.cesta.o: build-arm64/.objects/f874d2e7874ea064/src/arm64_phase3_policy.cesta.o
	@mkdir -p 'build-arm64/arm64-phase5/obj/src'
	ln -f 'build-arm64/.objects/f874d2e7874ea064/src/arm64_phase3_policy.cesta.o' "$@"

build-arm64/arm64-phase5/obj/src/arm64_phase4_policy.cesta.o: build-arm64/.objects/2a35c8c2dee474ed/src/arm64_phase4_policy.cesta.o
	@mkdir -p 'build-arm64/arm64-phase5/obj/src'
	ln -f 'build-arm64/.objects/2a35c8c2dee474ed/src/arm64_phase4_policy.cesta.o' "$@"

build-arm64/.objects/a895d91f28a93e66/src/arm64_phase5_policy.cesta.o: src/arm64_phase5_policy.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '--target=aarch64-none-elf' '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-stack-protector' '-fno-pic' '-fno-pie' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/arm64/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/arm64_phase5_policy.c' -o "$@"

build-arm64/arm64-phase5/obj/src/arm64_phase5_policy.cesta.o: build-arm64/.objects/a895d91f28a93e66/src/arm64_phase5_policy.cesta.o
	@mkdir -p 'build-arm64/arm64-phase5/obj/src'
	ln -f 'build-arm64/.objects/a895d91f28a93e66/src/arm64_phase5_policy.cesta.o' "$@"

build-arm64/arm64-phase5/obj/src/arm64_platform.cesta.o: build-arm64/.objects/85cd4996f49b5f4c/src/arm64_platform.cesta.o
	@mkdir -p 'build-arm64/arm64-phase5/obj/src'
	ln -f 'build-arm64/.objects/85cd4996f49b5f4c/src/arm64_platform.cesta.o' "$@"

build-arm64/.objects/87abde1f77471e57/src/arm64_phase5.cesta.o: src/arm64_phase5.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '--target=aarch64-none-elf' '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-stack-protector' '-fno-pic' '-fno-pie' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/arm64/include' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/arm64_phase5.c' -o "$@"

build-arm64/arm64-phase5/obj/src/arm64_phase5.cesta.o: build-arm64/.objects/87abde1f77471e57/src/arm64_phase5.cesta.o
	@mkdir -p 'build-arm64/arm64-phase5/obj/src'
	ln -f 'build-arm64/.objects/87abde1f77471e57/src/arm64_phase5.cesta.o' "$@"

build-arm64/arm64-phase5/obj/runtime/arm64/phase3_entry.S.o: build-arm64/.objects/df59441f16427b66/runtime/arm64/phase3_entry.S.o
	@mkdir -p 'build-arm64/arm64-phase5/obj/runtime/arm64'
	ln -f 'build-arm64/.objects/df59441f16427b66/runtime/arm64/phase3_entry.S.o' "$@"

build-arm64/.objects/5ee3877cd015a0c2/src/arm64_phase3.cesta.o: src/arm64_phase3.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '--target=aarch64-none-elf' '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-stack-protector' '-fno-pic' '-fno-pie' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/arm64/include' '-DARM64_PHASE5=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/arm64_phase3.c' -o "$@"

build-arm64/arm64-phase5/obj/src/arm64_phase3.cesta.o: build-arm64/.objects/5ee3877cd015a0c2/src/arm64_phase3.cesta.o
	@mkdir -p 'build-arm64/arm64-phase5/obj/src'
	ln -f 'build-arm64/.objects/5ee3877cd015a0c2/src/arm64_phase3.cesta.o' "$@"

build-arm64/.objects/28532124acf6fbc3/src/arm64_phase4.cesta.o: src/arm64_phase4.c GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '--target=aarch64-none-elf' '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-stack-protector' '-fno-pic' '-fno-pie' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/arm64/include' '-DARM64_PHASE5=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'src/arm64_phase4.c' -o "$@"

build-arm64/arm64-phase5/obj/src/arm64_phase4.cesta.o: build-arm64/.objects/28532124acf6fbc3/src/arm64_phase4.cesta.o
	@mkdir -p 'build-arm64/arm64-phase5/obj/src'
	ln -f 'build-arm64/.objects/28532124acf6fbc3/src/arm64_phase4.cesta.o' "$@"

build-arm64/.objects/eb07c8b1a607c51a/runtime/arm64/phase4_vectors.S.o: runtime/arm64/phase4_vectors.S GNUmakefile
	@mkdir -p "$(@D)"
	$(CC) '--target=aarch64-none-elf' '-std=c11' '-O2' '-g0' '-ffreestanding' '-fno-builtin' '-fno-stack-protector' '-fno-pic' '-fno-pie' '-mgeneral-regs-only' '-Wall' '-Wextra' '-Wpedantic' '-Wformat=2' '-Werror' '-Igenerated/arm64/include' '-DARM64_PHASE5=1' -MMD -MP -MF "$@.d" -MQ "$@" -c 'runtime/arm64/phase4_vectors.S' -o "$@"

build-arm64/arm64-phase5/obj/runtime/arm64/phase4_vectors.S.o: build-arm64/.objects/eb07c8b1a607c51a/runtime/arm64/phase4_vectors.S.o
	@mkdir -p 'build-arm64/arm64-phase5/obj/runtime/arm64'
	ln -f 'build-arm64/.objects/eb07c8b1a607c51a/runtime/arm64/phase4_vectors.S.o' "$@"

build-arm64/arm64-phase5/arm64-phase5: GNUmakefile build-arm64/arm64-phase5/obj/src/arm64_boot_policy.cesta.o build-arm64/arm64-phase5/obj/src/arm64_phase3_policy.cesta.o build-arm64/arm64-phase5/obj/src/arm64_phase4_policy.cesta.o build-arm64/arm64-phase5/obj/src/arm64_phase5_policy.cesta.o build-arm64/arm64-phase5/obj/src/arm64_platform.cesta.o build-arm64/arm64-phase5/obj/src/arm64_phase5.cesta.o build-arm64/arm64-phase5/obj/runtime/arm64/phase3_entry.S.o build-arm64/arm64-phase5/obj/src/arm64_phase3.cesta.o build-arm64/arm64-phase5/obj/src/arm64_phase4.cesta.o build-arm64/arm64-phase5/obj/runtime/arm64/phase4_vectors.S.o runtime/arm64/kernel.ld
	@mkdir -p "$(@D)"
	$(CC) 'build-arm64/arm64-phase5/obj/src/arm64_boot_policy.cesta.o' 'build-arm64/arm64-phase5/obj/src/arm64_phase3_policy.cesta.o' 'build-arm64/arm64-phase5/obj/src/arm64_phase4_policy.cesta.o' 'build-arm64/arm64-phase5/obj/src/arm64_phase5_policy.cesta.o' 'build-arm64/arm64-phase5/obj/src/arm64_platform.cesta.o' 'build-arm64/arm64-phase5/obj/src/arm64_phase5.cesta.o' 'build-arm64/arm64-phase5/obj/runtime/arm64/phase3_entry.S.o' 'build-arm64/arm64-phase5/obj/src/arm64_phase3.cesta.o' 'build-arm64/arm64-phase5/obj/src/arm64_phase4.cesta.o' 'build-arm64/arm64-phase5/obj/runtime/arm64/phase4_vectors.S.o' '--target=aarch64-none-elf' '-fuse-ld=lld' '-nostdlib' '-static' '-Wl,-T,runtime/arm64/kernel.ld' '-Wl,--build-id=none' -o "$@"

-include build-arm64/.objects/62fb10017ae769b0/src/arm64_boot_policy.cesta.o.d build-arm64/.objects/85cd4996f49b5f4c/src/arm64_platform.cesta.o.d build-arm64/.objects/39cbe8bbddc94370/src/arm64_phase2.cesta.o.d build-arm64/.objects/6889cacfec5e0c99/runtime/arm64/entry.S.o.d build-arm64/.objects/f874d2e7874ea064/src/arm64_phase3_policy.cesta.o.d build-arm64/.objects/a40e7b437b5272fd/src/arm64_phase3.cesta.o.d build-arm64/.objects/df59441f16427b66/runtime/arm64/phase3_entry.S.o.d build-arm64/.objects/ec1dbba15de9e042/runtime/arm64/phase3_vectors.S.o.d build-arm64/.objects/2a35c8c2dee474ed/src/arm64_phase4_policy.cesta.o.d build-arm64/.objects/97897b0817218b32/src/arm64_phase4.cesta.o.d build-arm64/.objects/fc82ed6c4dc5d9e5/runtime/arm64/phase4_vectors.S.o.d build-arm64/.objects/a895d91f28a93e66/src/arm64_phase5_policy.cesta.o.d build-arm64/.objects/87abde1f77471e57/src/arm64_phase5.cesta.o.d build-arm64/.objects/5ee3877cd015a0c2/src/arm64_phase3.cesta.o.d build-arm64/.objects/28532124acf6fbc3/src/arm64_phase4.cesta.o.d build-arm64/.objects/eb07c8b1a607c51a/runtime/arm64/phase4_vectors.S.o.d
