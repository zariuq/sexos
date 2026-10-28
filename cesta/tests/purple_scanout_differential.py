#!/usr/bin/env python3
"""Compare purple-scanout with its Cesta translation."""

import argparse
from pathlib import Path
import random
import re
import shutil

from differential_support import (
    C_MAIN,
    RUST_FFI,
    RUST_MAIN,
    first_difference,
    pinned_function,
    run,
)

HERE = Path(__file__).resolve().parent.parent
REPO = HERE.parent
REVISION = "4a040a86"

PROVIDER = r'''
#include "sexos_server.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
    uint64_t pfn_base;
} SexWindowCreateParams;

static struct {
    uint64_t create_status;
    uint64_t window_id;
    uint64_t submit_status;
    uint64_t limit;
} scenario;

static uint64_t yields;
static uint64_t expected_fb_ptr;

void sexos_spin_loop_hint(void) {}
_Noreturn void provider_panic(void) { printf("PANIC\n"); exit(0); }
_Noreturn void sexos_spin_forever(void) { provider_panic(); }
SexosMessage sexos_listen_once(uint64_t slot) { (void)slot; printf("UNEXPECTED listen\n"); exit(3); }
uint64_t sexos_reply(uint32_t target_pd, uint64_t value) { (void)target_pd; (void)value; printf("UNEXPECTED reply\n"); exit(3); }
uint64_t sexos_allocate_memory(uint64_t size) { (void)size; printf("UNEXPECTED allocate\n"); exit(3); }
uint64_t sexos_map_memory(uint64_t pfn, uint64_t size) { (void)pfn; (void)size; printf("UNEXPECTED map\n"); exit(3); }

SexosReply sexos_pdx_call(uint64_t slot, uint64_t opcode, uint64_t a0, uint64_t a1, uint64_t a2) {
    if (slot == 0 && opcode == 69) {
        const char *text = (const char *)(uintptr_t)a0;
        size_t length = (size_t)a1;
        if (length && text[length - 1] == '\n') --length;
        fputs("L ", stdout);
        fwrite(text, 1, length, stdout);
        fputc('\n', stdout);
        return (SexosReply){0, 0};
    }
    if (slot == 5 && opcode == 0xDE) {
        const SexWindowCreateParams *p = (const SexWindowCreateParams *)(uintptr_t)a0;
        expected_fb_ptr = p->pfn_base;
        unsigned aligned = (p->pfn_base & 4095u) == 0 ? 1u : 0u;
        uint32_t init_px = *(const volatile uint32_t *)(uintptr_t)p->pfn_base;
        printf("CREATE x=%u y=%u w=%u h=%u aligned=%u init_px=%#x a1=%" PRIu64 " a2=%" PRIu64 " -> status=%#" PRIx64 " id=%" PRIu64 "\n",
               p->x, p->y, p->width, p->height, aligned, init_px, a1, a2,
               scenario.create_status, scenario.window_id);
        return (SexosReply){scenario.create_status, scenario.window_id};
    }
    if (slot == 5 && opcode == 0xDD) {
        const uint64_t *pfn_list = (const uint64_t *)(uintptr_t)a1;
        unsigned match_fb = (pfn_list[0] == expected_fb_ptr) ? 1u : 0u;
        uint32_t px = match_fb ? *(const volatile uint32_t *)(uintptr_t)pfn_list[0] : 0u;
        printf("SUBMIT id=%" PRIu64 " match_fb=%u px=%#x pages=%" PRIu64 " a2=%" PRIu64 " -> status=%#" PRIx64 "\n",
               a0, match_fb, px, a2, a2, scenario.submit_status);
        return (SexosReply){scenario.submit_status, 0};
    }
    printf("C %" PRIu64 " %#" PRIx64 " %" PRIu64 " %" PRIu64 " %" PRIu64 "\n", slot, opcode, a0, a1, a2);
    return (SexosReply){0, 0};
}

void sexos_yield(void) {
    printf("Y\n");
    if (++yields >= scenario.limit) {
        printf("END\n");
        exit(0);
    }
}

void provider_start(int argc, char **argv) {
    if (argc != 5) exit(2);
    scenario.create_status = strtoull(argv[1], NULL, 10);
    scenario.window_id = strtoull(argv[2], NULL, 10);
    scenario.submit_status = strtoull(argv[3], NULL, 10);
    scenario.limit = strtoull(argv[4], NULL, 10);
}
'''


def rust_reference(source):
    probe = pinned_function(
        source,
        "fn debug_syscall_probe() {",
        {"0u64": "unsafe { crate::sexos_pdx_call(0, 0x5151, 0, 0, 0) };"},
    )
    start = source.index("fn debug_syscall_probe() {")
    end = source.index("\n}\n", start) + 3
    source = source[:start] + probe + source[end:]
    for old, new in (
        ("#![no_std]\n", ""),
        ("#![no_main]\n", ""),
        ("#[global_allocator]\nstatic ALLOCATOR: DummyAllocator = DummyAllocator;\n", ""),
        ("#[panic_handler]\nfn panic(info: &core::panic::PanicInfo) -> ! {\n    serial_println!(\"{}\", info);\n    loop { core::hint::spin_loop(); }\n}\n", ""),
        ('#[no_mangle]\npub extern "C" fn _start() -> ! {', "fn purple_scanout_start() -> ! {"),
    ):
        assert source.count(old) == 1, old
        source = source.replace(old, new)
    return source


def scenarios():
    r = random.Random(0x5151)
    statuses = [0, 0, 1, 0xFFFFFFFFFFFFFFFE, 0xFFFFFFFFFFFFFFFC, r.getrandbits(64) | 1]
    ids = [1, 7, 42, 0x123456789ABCDEF0]
    cases = []
    for cs in statuses:
        for ss in statuses:
            wid = ids[len(cases) % len(ids)]
            limit = 1 + (len(cases) % 4)
            cases.append([cs, wid, ss, limit])
    return cases


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cesta", default="cesta")
    args = parser.parse_args()
    cesta = shutil.which(args.cesta) or args.cesta
    work = HERE / "build/purple-scanout-differential"
    if work.exists():
        shutil.rmtree(work)
    work.mkdir(parents=True)

    show = lambda path: run(["git", "-C", REPO, "show", f"{REVISION}:{path}"]).stdout.decode()
    main_rs = show("purple-scanout/src/main.rs")
    pdx_rs = show("crates/sex-pdx/src/lib.rs")
    translation = (HERE / "src/purple_scanout.cesta").read_text()

    pinned = [
        pinned_function(
            pdx_rs,
            "pub fn pdx_call_checked(slot: u64, opcode: u64, arg0: u64, arg1: u64, arg2: u64) -> Result<u64, u64> {",
            {},
        ),
    ]
    rust = work / "reference.rs"
    rust.write_text(
        RUST_FFI
        + "".join(pinned)
        + "}\n"
        + rust_reference(main_rs)
        + RUST_MAIN % "purple_scanout_start"
    )
    (work / "provider.c").write_text(PROVIDER)
    (work / "main.c").write_text(C_MAIN % "purple_scanout_main")

    include = ["-I", HERE / "generated/include"]
    runtime = [
        HERE / "runtime/client_runtime.c",
        HERE / "runtime/server_runtime.c",
        *sorted((HERE / "generated/runtime").glob("*.c")),
    ]
    sanitizers = ["-fsanitize=address,undefined", "-fno-sanitize-recover=all"]
    warnings = ["-Wall", "-Wextra", "-Wpedantic", "-Wformat=2", "-Werror"]
    plain_provider = work / "provider-plain.o"
    run(
        [
            "gcc",
            "-std=c11",
            "-O2",
            "-DSEXOS_HOST",
            *warnings,
            *include,
            "-c",
            work / "provider.c",
            "-o",
            plain_provider,
        ]
    )
    reference = work / "reference"
    run(
        [
            "rustc",
            "--edition",
            "2021",
            "-O",
            "-A",
            "warnings",
            rust,
            "-o",
            reference,
            "-C",
            f"link-arg={plain_provider}",
        ]
    )
    harnesses = []
    for compiler in ("gcc", "clang"):
        obj = work / f"purple-scanout-{compiler}.o"
        run(
            [
                cesta,
                "cc",
                "--cc",
                compiler,
                "-std=c11",
                "-O2",
                *warnings,
                *sanitizers,
                *include,
                "-c",
                HERE / "src/purple_scanout.cesta",
                "-o",
                obj,
            ]
        )
        harness = work / f"harness-{compiler}"
        run(
            [
                compiler,
                "-std=c11",
                "-O2",
                "-DSEXOS_HOST",
                *sanitizers,
                *include,
                work / "provider.c",
                work / "main.c",
                obj,
                *runtime,
                "-o",
                harness,
            ]
        )
        harnesses.append((compiler, harness))

    markers = [
        rb"\[purple\] single-pixel scanout test start",
        rb"\[purple\] create failed status=0x",
        rb"\[purple\] submit failed status=0x",
        rb"\[purple\] submitted single magenta pixel",
    ]
    unreached = set(markers)
    cases = scenarios()
    references = {}
    compared = 0
    for number, case in enumerate(cases):
        expected = run([reference, *case]).stdout
        assert b"UNEXPECTED" not in expected, number
        unreached -= {m for m in unreached if re.search(m, expected)}
        references[number] = expected
        for compiler, harness in harnesses:
            actual = run([harness, *case]).stdout
            if actual != expected:
                raise SystemExit(
                    first_difference(
                        actual.decode(errors="replace"),
                        expected.decode(errors="replace"),
                        f"scenario {number} {compiler}",
                    )
                )
        compared += len(expected.splitlines())
    assert not unreached, f"no scenario reached {sorted(unreached)}"

    mutations = [
        ("0x5151u", "0x5152u"),
        ("(= (. width) 1u)", "(= (. width) 2u)"),
        ("(= (. height) 1u)", "(= (. height) 2u)"),
        ("0x00FF00FFu", "0x00FF00FEu"),
        ("(= ([] (-> p pfn_list) 0) fb_ptr)", "(= ([] (-> p pfn_list) 0) 0u)"),
        ("window_id (cast uint64_t (cast uintptr_t (-> p pfn_list))) 1", "window_id (cast uint64_t (cast uintptr_t (-> p pfn_list))) 2"),
        ('(define OP_WINDOW_CREATE "UINT64_C(0xDE)")', '(define OP_WINDOW_CREATE "UINT64_C(0xDF)")'),
        ('(define OP_WINDOW_SUBMIT "UINT64_C(0xDD)")', '(define OP_WINDOW_SUBMIT "UINT64_C(0xDC)")'),
    ]
    for number, (old_text, new_text) in enumerate(mutations):
        assert translation.count(old_text) == 1, old_text
        mutant = work / f"purple-scanout-mutant-{number}.cesta"
        mutant.write_text(translation.replace(old_text, new_text))
        obj = work / f"mutant-{number}.o"
        run(
            [
                cesta,
                "cc",
                "--cc",
                "gcc",
                "-std=c11",
                "-O2",
                *include,
                "-c",
                mutant,
                "-o",
                obj,
            ]
        )
        harness = work / f"mutant-{number}"
        run(
            [
                "gcc",
                "-std=c11",
                "-O2",
                "-DSEXOS_HOST",
                *include,
                plain_provider,
                work / "main.c",
                obj,
                *runtime,
                "-o",
                harness,
            ]
        )
        if all(run([harness, *case]).stdout == references[idx] for idx, case in enumerate(cases)):
            raise SystemExit(f"mutation {number} was not detected: {old_text} -> {new_text}")

    print(
        f"purple-scanout: Cesta translation equals the Rust app on {compared} trace lines "
        f"({len(cases)} scenarios, GCC and Clang, address and UB sanitizers); "
        f"{len(mutations)} mutations detected"
    )


if __name__ == "__main__":
    main()
