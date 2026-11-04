#!/usr/bin/env python3
"""Recover bounded cyclic histories against an unbounded logical-clock oracle.

The Rust reference's numeric maximum is intentionally *not* the oracle for
this corrected behavior. Its rollback is reproduced separately on pinned
source. The ordinary codec/server differential remains in sexstore_differential.
"""

import argparse
import json
from pathlib import Path
import random
import shutil
import subprocess

import sexstore_differential as differential

HERE = Path(__file__).resolve().parent.parent
MODULUS = (1 << 32) - 1

HARNESS = r'''
#include "sexstore.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void sexos_line(const char *format, ...) { (void)format; }
void sexos_fail(void) { abort(); }
static int table_equal(const KvTable *a, const KvTable *b) {
    for (size_t i = 0; i < 16; ++i) {
        if (a->slots[i].state != b->slots[i].state || a->slots[i].generation != b->slots[i].generation ||
            a->slots[i].key != b->slots[i].key || a->slots[i].val != b->slots[i].val) return 0;
    }
    return 1;
}
int main(void) {
    uint32_t sa, sb;
    unsigned damaged;
    while (scanf("%" SCNu32 " %" SCNu32 " %u", &sa, &sb, &damaged) == 3) {
        Durable d = sexstore_durable_new();
        KvTable a = {0}, b = {0}, fresh = {0}, loaded = {0};
        a.slots[0] = (KvSlot){1, 1, 1, 101};
        b.slots[0] = (KvSlot){1, 1, 1, 202};
        fresh.slots[0] = (KvSlot){1, 2, 1, 303};
        uint64_t seed = sa ^ ((uint64_t)sb << 32);
        for (size_t i = 1; i < 16; ++i) {
            seed = seed * UINT64_C(6364136223846793005) + 1;
            a.slots[i] = (KvSlot){1, 1, (uint32_t)i, seed};
            b.slots[i] = (KvSlot){1, 2, (uint32_t)i, ~seed};
            fresh.slots[i] = (KvSlot){1, 3, (uint32_t)i, seed ^ (seed >> 17)};
        }
        if (sa) { DurablePage p = sexstore_durable_build_page(&a, sa); memcpy(d.region, p.bytes, 512); }
        if (sb) { DurablePage p = sexstore_durable_build_page(&b, sb); memcpy(d.region + 512, p.bytes, 512); }
        if (damaged == 1) d.region[8] ^= 1;
        if (damaged == 2) d.region[520] ^= 1;
        (void)sexstore_durable_load_into_ram(&d, &loaded);
        if (loaded.slots[0].val == 101 && !table_equal(&loaded, &a)) return 4;
        if (loaded.slots[0].val == 202 && !table_equal(&loaded, &b)) return 4;
        printf("%" PRIu64, loaded.slots[0].val);
        for (unsigned i = 0; i < 4; ++i) {
            Durable before = d;
            if (!sexstore_durable_write_all(&d, &fresh)) return 3;
            memset(&loaded, 0, sizeof loaded);
            (void)sexstore_durable_load_into_ram(&d, &loaded);
            if (!table_equal(&loaded, &fresh)) return 5;
            printf(" %" PRIu64 " %" PRIu32 " %" PRIu32 " %u %u", loaded.slots[0].val,
                sexstore_pages_page_seq((cesta_read_bytes){d.region, 512}),
                sexstore_pages_page_seq((cesta_read_bytes){d.region + 512, 512}),
                memcmp(d.region, before.region, 512) == 0,
                memcmp(d.region + 512, before.region + 512, 512) == 0);
            ++fresh.slots[0].val;
        }
        printf("\n");
    }
    return ferror(stdin) ? 2 : 0;
}
'''


def run(command, **kwargs):
    result = subprocess.run(list(map(str, command)), capture_output=True, **kwargs)
    if result.returncode:
        raise AssertionError((list(map(str, command)), result.returncode,
                              result.stdout.decode()[-4000:], result.stderr.decode()[-8000:]))
    return result


def cases(count):
    # Expected recovery uses logical history, not an implementation of the
    # cyclic comparison being tested. All non-tie histories fit a half-cycle.
    ordinary = [(0, 0, 0), (1, 0, 101), (0, 1, 202), (1, 1, 101),
              (MODULUS, 0, 101), (0, MODULUS, 202),
              (MODULUS, MODULUS - 1, 101), (MODULUS - 1, MODULUS, 202),
              (1, MODULUS, 101), (MODULUS, 1, 202)]
    result = [(*case, 0) for case in ordinary]
    r = random.Random(0xC1C1E)
    for _ in range(count):
        older = r.randrange(1 << 64)
        gap = r.choice([1, 2, 7, MODULUS // 2, r.randrange(1, MODULUS // 2 + 1)])
        newer = older + gap
        old_serial, new_serial = older % MODULUS + 1, newer % MODULUS + 1
        if r.randrange(2):
            case = (new_serial, old_serial, 101)
        else:
            case = (old_serial, new_serial, 202)
        damaged = r.choice([0, 0, 0, 1, 2])
        a, b, expected = case
        if damaged: expected = 202 if damaged == 1 else 101
        result.append((a, b, expected, damaged))
    return result


def check(output, scenarios, label):
    lines = output.decode().splitlines()
    assert len(lines) == len(scenarios), label
    for i, ((a, b, initial, damaged), line) in enumerate(zip(scenarios, lines)):
        if damaged == 1: a = 0
        if damaged == 2: b = 0
        values = list(map(int, line.split()))
        expected = [initial]
        a_current = initial != 202
        for value in range(303, 307):
            serial = a if a_current else b
            fresh = serial % MODULUS + 1
            if a_current:
                b = fresh
            else:
                a = fresh
            expected.extend([value, a, b, int(a_current), int(not a_current)])
            a_current = not a_current
        assert values == expected, (label, i, scenarios[i], values, expected)


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--cesta", default="cesta")
    p.add_argument("--cases", type=int, default=1000)
    p.add_argument("--output", type=Path, default=HERE / "build/sexstore-versions")
    args = p.parse_args()
    work = args.output
    work.mkdir(parents=True, exist_ok=True)
    cesta = shutil.which(args.cesta) or args.cesta
    scenarios = cases(args.cases)
    input_bytes = "".join(f"{a} {b} {damaged}\n" for a, b, _, damaged in scenarios).encode()
    (work / "harness.c").write_text(HARNESS)
    include = ["-I", HERE / "generated/include"]
    flags = ["-std=c11", "-O2", "-Wall", "-Wextra", "-Wpedantic", "-Werror"]
    sanitize = ["-fsanitize=address,undefined", "-fno-sanitize-recover=all"]
    source = (HERE / "src/sexstore_durable.cesta").read_text()

    # Run the actual pinned Rust durable functions; only process scaffolding
    # and unused imports are replaced by the existing host provider.
    pinned = run(["git", "-C", HERE.parent, "show", "4a040a86:servers/sexstore/src/main.rs"]).stdout.decode()
    legacy_main = r'''
fn main() {
    let mut old = [KvSlot { state: 0, generation: 0, key: 0, val: 0 }; 16];
    old[0] = KvSlot { state: 1, generation: 1, key: 1, val: 101 };
    let a = durable_build_page(&old, u32::MAX);
    old[0].val = 202;
    let b = durable_build_page(&old, u32::MAX - 1);
    unsafe {
        let region = &mut *core::ptr::addr_of_mut!(DURABLE_REGION);
        region[..512].copy_from_slice(&a);
        region[512..].copy_from_slice(&b);
    }
    old[0].val = 303;
    assert!(unsafe { durable_write_all(&old) });
    let mut loaded = [KvSlot { state: 0, generation: 0, key: 0, val: 0 }; 16];
    unsafe { durable_load_into_ram(&mut loaded); }
    println!("LEGACY {}", loaded[0].val);
}
'''
    from differential_support import LISTEN, pinned_function
    pdx = run(["git", "-C", HERE.parent, "show", "4a040a86:crates/sex-pdx/src/lib.rs"]).stdout.decode()
    provider = differential.RUST_PROVIDER + pinned_function(pdx,
        "pub fn pdx_listen_raw(slot: u64) -> PdxMessage {", LISTEN) + "}\n"
    rust = work / "legacy.rs"
    rust.write_text(provider + differential.rust_reference(pinned) + legacy_main)
    run(["rustc", "--edition", "2021", "-O", "-A", "warnings", rust, "-o", work / "legacy"])
    legacy = run([work / "legacy"]).stdout
    assert b"LEGACY 101\n" in legacy, legacy
    (work / "legacy.log").write_bytes(legacy)

    def compile_case(compiler, label, durable_source, authored):
        objects = []
        for unit, path in [("durable", durable_source), ("pages", HERE / "src/sexstore_pages.cesta")]:
            obj = work / f"{label}-{unit}.o"
            if authored:
                command = [cesta, "cc", "--cc", compiler, *flags, *sanitize, *include, "-c", path, "-o", obj]
            else:
                exported = work / f"{label}-{unit}.c"
                run([cesta, "export-c", path, "--std=c11", "--cc", compiler, "-o", exported, "--", *include])
                command = [compiler, *flags, *sanitize, *include, "-c", exported, "-o", obj]
            run(command)
            objects.append(obj)
        binary = work / label
        run([compiler, *flags, *sanitize, *include, work / "harness.c", *objects, "-o", binary])
        return binary

    for compiler in ("gcc", "clang"):
        for authored in (True, False):
            label = compiler + ("-authored" if authored else "-exported")
            binary = compile_case(compiler, label, HERE / "src/sexstore_durable.cesta", authored)
            check(run([binary], input=input_bytes).stdout, scenarios, label)

    mutations = [
        ("numeric-maximum", "(<= distance (/ UINT32_MAX 2u))", "(&& (> distance 0) (>= first second))"),
        ("zero-at-wrap", "(if (== next_seq 0) (= next_seq 1))", "(if (== next_seq 0) (= next_seq 0))"),
        ("skip-first-serial", "(if (== next_seq 0) (= next_seq 1))", "(if (== next_seq 0) (= next_seq 2))"),
        ("wrong-cycle", "(+ (- UINT32_MAX second) first)", "(+ (+ (- UINT32_MAX second) 1u) first)"),
        ("half-window", "(<= distance (/ UINT32_MAX 2u))", "(< distance (/ UINT32_MAX 2u))"),
        ("overwrite-current", "(?: a_current DURABLE_PAGE_B_OFFSET DURABLE_PAGE_A_OFFSET)", "(?: a_current DURABLE_PAGE_A_OFFSET DURABLE_PAGE_B_OFFSET)"),
        ("select-stale", "(return (call load_page (& page_a) table))", "(return (call load_page (& page_b) table))"),
    ]
    detected = []
    for label, old, new in mutations:
        assert source.count(old) == 1, old
        path = work / (label + ".cesta")
        path.write_text(source.replace(old, new))
        binary = compile_case("gcc", label, path, True)
        result = subprocess.run([str(binary)], input=input_bytes, capture_output=True)
        try:
            assert result.returncode == 0, (label, result.returncode)
            check(result.stdout, scenarios, label)
        except AssertionError:
            assert result.returncode in (0, 3, 4, 5), (label, result.returncode, result.stderr[-4000:])
            detected.append(label)
        else:
            raise AssertionError(("surviving mutation", label))

    (work / "result.json").write_text(json.dumps({"histories": len(scenarios), "builds": 4,
        "mutations_detected": detected}, indent=2) + "\n")
    print(f"sexstore versions: pinned Rust rollback reproduced; {len(scenarios)} logical histories, four authored/exported GCC/Clang sanitizer builds; {len(detected)} mutations detected")


if __name__ == "__main__":
    main()
