#!/usr/bin/env python3
"""Compare freestanding memory primitives with the native C library."""

from pathlib import Path
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent.parent

HARNESS = r'''
#include "sexos_client.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void *sexos_memcpy(void *, const void *, size_t);
void *sexos_memmove(void *, const void *, size_t);
void *sexos_memset(void *, int, size_t);
int sexos_memcmp(const void *, const void *, size_t);
SexosReply sexos_pdx_call(uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e)
{ (void)a; (void)b; (void)c; (void)d; (void)e; _Exit(91); }
SexosMessage sexos_listen_once(uint64_t slot) { (void)slot; _Exit(92); }
void sexos_yield(void) { _Exit(93); }

static void seed(unsigned char *a, unsigned char *b, size_t pattern)
{
    for (size_t i = 0; i < 128; ++i) a[i] = b[i] = (unsigned char)(i * 37 + pattern * 11);
}
static int sign(int value) { return (value > 0) - (value < 0); }
int main(void)
{
    unsigned char actual[128], expected[128], source[128];
    for (size_t count = 0; count <= 64; ++count) {
        for (size_t from = 0; from <= 32; ++from) {
            for (size_t to = 0; to <= 32; ++to) {
                seed(actual, expected, count);
                assert(sexos_memmove(actual + to, actual + from, count) == actual + to);
                memmove(expected + to, expected + from, count);
                assert(!memcmp(actual, expected, sizeof actual));
            }
            seed(actual, expected, from);
            for (size_t i = 0; i < sizeof source; ++i) source[i] = (unsigned char)(i * 19 + count);
            assert(sexos_memcpy(actual + from, source + from, count) == actual + from);
            memcpy(expected + from, source + from, count);
            assert(!memcmp(actual, expected, sizeof actual));
            assert(sign(sexos_memcmp(actual + from, source + from, count)) ==
                   sign(memcmp(actual + from, source + from, count)));
            for (int value = -300; value <= 300; value += 13) {
                seed(actual, expected, count);
                assert(sexos_memset(actual + from, value, count) == actual + from);
                memset(expected + from, value, count);
                assert(!memcmp(actual, expected, sizeof actual));
            }
        }
    }
    for (size_t offset = 0; offset < 64; ++offset) {
        seed(actual, expected, offset);
        expected[offset] ^= 255;
        assert(sign(sexos_memcmp(actual, expected, 64)) == sign(memcmp(actual, expected, 64)));
        assert(sign(sexos_memcmp(expected, actual, 64)) == sign(memcmp(expected, actual, 64)));
    }
    return 0;
}
'''


def run(command):
    result = subprocess.run(list(map(str, command)), cwd=HERE, capture_output=True, timeout=60)
    if result.returncode:
        raise RuntimeError((command, result.returncode, result.stderr.decode(errors="replace")))


def main():
    work = Path(tempfile.mkdtemp(prefix="memory-primitives-", dir=HERE / "build"))
    source = work / "harness.c"
    source.write_text(HARNESS)
    for compiler, sanitizer in (("gcc", False), ("clang", True)):
        flags = ["-std=c11", "-O1" if sanitizer else "-O2", "-fno-builtin", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
                 "-I", HERE / "generated/include"]
        if sanitizer:
            flags += ["-fsanitize=address,undefined", "-fno-sanitize-recover=all"]
        # Rename only the definitions so the harness can call libc separately.
        obj = work / (compiler + ".o")
        run([compiler, *flags, *("-D" + name + "=sexos_" + name for name in
             ("memcpy", "memmove", "memset", "memcmp")), "-c", HERE / "runtime/client_runtime.c", "-o", obj])
        executable = work / compiler
        run([compiler, *flags, obj, HERE / "generated/runtime/packed_words.c",
             HERE / "generated/runtime/format_bytes.c", source, "-o", executable])
        run([executable])
        print(compiler + ": 70,785 overlapping moves, copy/fill/compare boundaries and libc agreement passed")


if __name__ == "__main__":
    main()
