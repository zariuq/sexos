#!/usr/bin/env python3
"""Exercise the kernel's actual freestanding memory routines against libc.

Symbol renaming happens after compilation, so a generated self-call stays a
self-call and cannot accidentally be satisfied by the reference libc routine.
"""
import argparse
from pathlib import Path
import subprocess
import tempfile

from memory_primitives import HARNESS

HERE = Path(__file__).resolve().parent.parent
NAMES = ("memcpy", "memmove", "memset", "memcmp")


def run(command, timeout=60):
    result = subprocess.run(list(map(str, command)), cwd=HERE, capture_output=True, timeout=timeout)
    if result.returncode:
        raise RuntimeError((command, result.returncode, result.stderr.decode(errors="replace")[-3000:]))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cesta", required=True)
    args = parser.parse_args()
    work = Path(tempfile.mkdtemp(prefix="kernel-memory-", dir=HERE / "build"))
    harness = work / "harness.c"
    harness.write_text(HARNESS)
    for compiler in ("gcc", "clang"):
        lease = work / (compiler + "-frame-lease.o")
        run([args.cesta, "cc", "--cc", compiler, "-std=c11", "-O2",
             "-ffreestanding", "-fno-builtin", "-mgeneral-regs-only",
             "-ffunction-sections", "-fdata-sections", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
             "-Igenerated/include", "-c", "src/frame_lease.cesta", "-o", lease])
        for optimization in ("-O2", "-O3"):
            tag = compiler + optimization[1:]
            obj = work / (tag + ".o")
            run([args.cesta, "cc", "--cc", compiler, "-std=c11", optimization,
                 "-ffreestanding", "-fno-builtin", "-mgeneral-regs-only",
                 "-ffunction-sections", "-fdata-sections", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
                 "-Iruntime/kernel", "-Igenerated/include", "-c", "src/kernel_memory.cesta", "-o", obj])
            renamed = work / (tag + "-renamed.o")
            options = [argument for name in NAMES for argument in ("--redefine-sym", name + "=sexos_" + name)]
            run(["objcopy", *options, obj, renamed])
            binary = work / tag
            run([compiler, "-std=c11", "-O2", "-fno-builtin", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
                 "-Igenerated/include", "-Wl,--gc-sections", harness, lease, renamed, "-o", binary])
            run([binary], timeout=30)
        # Compile the same production unit with only the CPU access boundary
        # replaced, so allocation and page-table tests execute the real code.
        lease_pages = work / (compiler + "-frame-lease-pages.o")
        run([args.cesta, "cc", "--cc", compiler, "-std=c11", "-O1", "-ffreestanding", "-fno-builtin",
             "-ffunction-sections", "-fdata-sections", "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
             "-Wall", "-Wextra", "-Wpedantic", "-Werror", "-Itests/kernel_memory", "-Igenerated/include",
             "-Iruntime/kernel", "-c", "src/frame_lease.cesta", "-o", lease_pages])
        obj = work / (compiler + "-pages.o")
        run([args.cesta, "cc", "--cc", compiler, "-std=c11", "-O1", "-ffreestanding", "-fno-builtin",
             "-ffunction-sections", "-fdata-sections", "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
             "-Wall", "-Wextra", "-Wpedantic", "-Werror", "-Itests/kernel_memory", "-Igenerated/include",
             "-Iruntime/kernel", "-c", "src/kernel_memory.cesta", "-o", obj])
        renamed = work / (compiler + "-pages-renamed.o")
        run(["objcopy", *options, obj, renamed])
        binary = work / (compiler + "-pages")
        run([compiler, "-std=c11", "-O1", "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
             "-Wall", "-Wextra", "-Wpedantic", "-Werror", "-Itests/kernel_memory", "-Igenerated/include",
             "-Iruntime/kernel", "-Wl,--gc-sections", "tests/kernel_memory/host.c", lease_pages, renamed, "-o", binary])
        run([binary])
    print("kernel memory: actual memcpy/memmove/memset/memcmp agree with libc, GCC/Clang O2/O3; "
          "real allocator/page walks, failed-allocation preservation, access checks and NX 1-GiB splitting pass sanitizers")


if __name__ == "__main__":
    main()
