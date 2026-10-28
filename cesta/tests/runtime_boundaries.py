#!/usr/bin/env python3
"""Check the bounded native line bridge and its typed call boundary."""

from pathlib import Path
import json
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent.parent

HARNESS = r'''
#include "sexos_client.h"
#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

static char captured[512];
static size_t calls, size;
SexosReply sexos_pdx_call(uint64_t slot, uint64_t opcode, uint64_t a0, uint64_t a1, uint64_t a2)
{
    assert(slot == 0 && opcode == 69 && a2 == 0);
    const char *text = (const char *)(uintptr_t)a0;
    if (a1 >= 20 && !memcmp(text, "[cesta.client.fault]", 20)) _Exit(77);
    assert(a1 <= sizeof captured);
    ++calls; size = (size_t)a1;
    memcpy(captured, text, size);
    return (SexosReply){0, 0};
}
SexosMessage sexos_listen_once(uint64_t slot) { (void)slot; return (SexosMessage){65535,0,0,0,0,0}; }
void sexos_yield(void) { _Exit(88); }

int main(int argc, char **argv)
{
    if (argc > 1) {
        if (!strcmp(argv[1], "long")) {
            char text[513]; memset(text, 'x', 512); text[512] = 0;
            sexos_line("%s", text);
        } else {
            const char *invalid[] = {"%", "%n", "%ll", "%#", "%q", "%#u"};
            unsigned int index = (unsigned int)strtoul(argv[1], NULL, 10);
            assert(index < sizeof invalid / sizeof *invalid);
            sexos_line(invalid[index]);
        }
        return 2;
    }
    sexos_line("value=%u/%llu/%zu/%#llx/%s/%%", 0u, ULLONG_MAX, (size_t)987, 0ull, "done");
    static const char expected[] = "value=0/18446744073709551615/987/0x0/done/%\n";
    assert(calls == 1 && size == sizeof expected - 1 && !memcmp(captured, expected, size));
    char limit[512]; memset(limit, 'x', 511); limit[511] = 0;
    sexos_line("%s", limit);
    assert(calls == 2 && size == 512 && captured[511] == '\n');
    for (size_t i = 0; i < 511; ++i) assert(captured[i] == 'x');
    sexos_line("");
    assert(calls == 3 && size == 1 && captured[0] == '\n');
    return 0;
}
'''


def run(command, expected=0):
    result = subprocess.run(list(map(str, command)), cwd=HERE, capture_output=True, timeout=30)
    if result.returncode != expected:
        raise AssertionError((command, result.returncode, result.stderr.decode(errors="replace")))
    return result


def main():
    work = Path(tempfile.mkdtemp(prefix="runtime-boundaries-", dir=HERE / "build"))
    source = work / "harness.c"
    source.write_text(HARNESS)
    libraries = [HERE / "runtime/client_runtime.c", HERE / "generated/runtime/packed_words.c",
                 HERE / "generated/runtime/format_bytes.c"]
    for compiler, sanitizer in (("gcc", False), ("clang", True)):
        flags = ["-std=c11", "-O1" if sanitizer else "-O2", "-DSEXOS_HOST", "-fno-builtin",
                 "-Wall", "-Wextra", "-Wpedantic", "-Werror", "-Wno-format-security", "-Wno-format-nonliteral",
                 "-Wno-format-zero-length",
                 "-I", HERE / "generated/include"]
        if sanitizer:
            flags += ["-fsanitize=address,undefined", "-fno-sanitize-recover=all"]
        executable = work / compiler
        run([compiler, *flags, *libraries, source, "-o", executable])
        run([executable])
        for fault in ("long", "0", "1", "2", "3", "4", "5"):
            run([executable, fault], expected=77)
        bad = work / "wrong_type.c"
        bad.write_text('#include "sexos_client.h"\nvoid f(void) { sexos_line("%llu", 1u); }\n')
        result = subprocess.run([compiler, "-std=c11", "-Wformat=2", "-Werror", "-I", str(HERE / "generated/include"),
                                 "-c", str(bad), "-o", str(work / "wrong_type.o")], capture_output=True)
        assert result.returncode != 0 and b"format" in result.stderr, result.stderr
        print(compiler + ": one syscall per line, exact capacity, numeric extremes, malformed-format refusal and call-type checks passed")
    (work / "summary.json").write_text(json.dumps({"native": "gcc", "sanitized": "clang ASan/UBSan",
        "format_faults": 7, "compile_refusals": 2, "max_line_bytes": 512}, indent=2) + "\n")


if __name__ == "__main__":
    main()
