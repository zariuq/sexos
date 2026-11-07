#!/usr/bin/env python3
"""Compare the checked Cesta decimal unit with Rust's own float formatting and parsing.

The unit carries the exact conversions behind upstream's script engine: Rust's correctly rounded
`str::parse::<f64>` and the Dragon digit generators behind `{:e}` and `{:.N}` (the latter for
magnitudes below 1e21, as `toFixed` uses it). The reference is
Rust's core library on a seeded corpus of doubles (bit patterns, powers, subnormals, boundary
values) and decimal texts (halfway points between neighbours, long digit strings, extreme
exponents).
"""
import argparse
import json
import os
import random
import struct
import subprocess
import sys
import tempfile
from decimal import Decimal, getcontext
from pathlib import Path
from layout_text import LayoutText

HERE = Path(__file__).resolve().parent.parent
PRECISIONS = (40, 0, 1, 2, 3, 5, 10, 17, 20, 26)


def run(argv, *, stdin=None):
    result = subprocess.run(list(map(str, argv)), cwd=HERE, capture_output=True, input=stdin)
    if result.returncode:
        raise RuntimeError((argv, result.returncode, result.stdout[-1000:], result.stderr[-5000:]))
    return result.stdout


def to_double(bits):
    return struct.unpack("<d", struct.pack("<Q", bits))[0]


def to_bits(x):
    return struct.unpack("<Q", struct.pack("<d", x))[0]


def doubles(rng, count):
    out = [0.0, -0.0, 5e-324, -5e-324, 2.2250738585072014e-308, 2.225073858507201e-308,
           1.7976931348623157e308, 1e21, 9.999999999999999e20, 1e-6, 9.999999999999999e-7, 0.1,
           0.3, 1 / 3, 2 / 3, 0.5, 1.5, 2.5, 0.125, 1e23, 8.41e21, 1.0, 100.0, 9007199254740993.0,
           1.005, 2.675, 1.45, 8.345, 0.007, 0.024, 0.044]
    while len(out) < count:
        r = rng.random()
        if r < 0.3:
            x = to_double(rng.getrandbits(64))
        elif r < 0.5:
            x = rng.uniform(-1e6, 1e6)
        elif r < 0.6:
            x = float(rng.randint(-10 ** 17, 10 ** 17))
        elif r < 0.7:
            x = float(Decimal(10) ** rng.randint(-323, 308)) * rng.choice([1, -1])
        elif r < 0.8:
            x = to_double(rng.getrandbits(52) | rng.choice([0, 1, 2, 2046]) << 52)
        elif r < 0.9:
            x = round(rng.uniform(-1000, 1000), rng.randint(0, 6))
        else:
            x = float(Decimal(2) ** rng.randint(-1074, 1023)) * rng.choice([1, -1])
        if x == x and abs(x) != float("inf"):
            out.append(x)
    return out


def halfway(x):
    return (Decimal(x) + Decimal(to_double(to_bits(x) + 1))) / 2


def texts(rng, values, count):
    out = ["0", "-0", "1", "1.", ".5", "0.1", "1e-400", "-1e-400", "1e309",
           "2.4703282292062327e-324", "2.4703282292062328e-324", "2.2250738585072011e-308",
           "2.2250738585072012e-308", "1.7976931348623158e308", "1.7976931348623159e308",
           "9007199254740993", "9007199254740992.5", "123456789012345678901234567890",
           "0." + "0" * 300 + "1", "1" + "0" * 400 + "e-400", "0.30000000000000004", "1e22",
           "1e23", "4.9e-324", "5e-324", "1e-323", "00000.000001e6", "1.e5", ".0e9", "1E+2",
           "1e-0", "3.14159265358979323846264338327950288419716939937510"]
    # Halfway points continued past the 800 kept digits: one nonzero digit far out decides.
    for x in rng.sample(values, 200):
        x = abs(x)
        if x == 0.0 or x >= 1.7976931348623157e308:
            continue
        mantissa, exponent = format(halfway(x).normalize(), "e").split("e")
        digits = mantissa.replace(".", "")
        tail = "0" * (820 - len(digits)) + rng.choice("123456789")
        out.append(digits[0] + "." + digits[1:] + tail + "e" + exponent)
    while len(out) < count:
        r = rng.random()
        if r < 0.35:
            x = abs(rng.choice(values))
            if x == 0.0 or x >= 1.7976931348623157e308:
                continue
            h = halfway(x)
            out.append(format(h, "e"))
            mantissa, exponent = format(h.normalize(), "e").split("e")
            if mantissa[-1].isdigit():
                nudged = (int(mantissa[-1]) + rng.choice([1, 9])) % 10
                out.append(mantissa[:-1] + str(nudged) + "e" + exponent)
        elif r < 0.7:
            n = rng.randint(1, 25)
            digits = "".join(rng.choice("0123456789") for _ in range(n))
            p = rng.randint(0, n)
            text = digits[:p] + ("." if rng.random() < 0.7 else "") + digits[p:]
            if rng.random() < 0.6:
                text += rng.choice("eE") + rng.choice(["", "+", "-"]) + str(rng.randint(0, 330))
            out.append(text)
        elif r < 0.85:
            n = rng.randint(700, 1200)
            out.append("0." + "".join(rng.choice("0123456789") for _ in range(n)) + "e" +
                       str(rng.randint(-330, 330)))
        else:
            out.append(repr(abs(rng.choice(values))))
    return [t for t in out if t and t != "." and not t.startswith(("e", "E", ".e", ".E"))]


REFERENCE = r'''
use std::io::BufRead;
fn main() {
    let mode = std::env::args().nth(1).unwrap();
    for line in std::io::stdin().lock().lines() {
        let line = line.unwrap();
        if mode == "format" {
            let x = f64::from_bits(u64::from_str_radix(&line, 16).unwrap());
            let mut out = if x == 0.0 { String::from("zero") } else { format!("{:e}", x) };
            if x.abs() < 1e21 { for p in PRECISIONS { out.push(' '); out.push_str(&format!("{:.*}", p, x)); } }
            println!("{}", out);
        } else {
            match line.parse::<f64>() { Ok(v) => println!("{:016x}", v.to_bits()), Err(_) => println!("ERR") }
        }
    }
}
'''

HOST = r'''
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kaleidoscope_decimal.h"

static double from_bits(uint64_t b) { double d; memcpy(&d, &b, 8); return d; }
static uint64_t to_bits(double d) { uint64_t b; memcpy(&b, &d, 8); return b; }

/* Rust's `{:.N}`: the sign, then the exact digits in positional form. */
static void exact_fixed(double v, size_t frac) {
    if (to_bits(v) >> 63) putchar('-');
    KaleidoscopeDigits d = {0};
    if (v != 0.0) kaleidoscope_decimal_exact(v, -(int32_t)frac, &d);
    if (v == 0.0 || d.k <= -(int32_t)frac) {
        putchar('0');
        if (frac) { putchar('.'); for (size_t i = 0; i < frac; i++) putchar('0'); }
        return;
    }
    if (d.k <= 0) {
        size_t zeros = (size_t)-d.k;
        printf("0.");
        for (size_t i = 0; i < zeros; i++) putchar('0');
        fwrite(d.digits, 1, d.len, stdout);
        for (size_t i = d.len + zeros; i < frac; i++) putchar('0');
    } else if ((size_t)d.k < d.len) {
        fwrite(d.digits, 1, (size_t)d.k, stdout);
        putchar('.');
        fwrite(d.digits + d.k, 1, d.len - (size_t)d.k, stdout);
        for (size_t i = d.len - (size_t)d.k; i < frac; i++) putchar('0');
    } else {
        fwrite(d.digits, 1, d.len, stdout);
        for (size_t i = d.len; i < (size_t)d.k; i++) putchar('0');
        if (frac) { putchar('.'); for (size_t i = 0; i < frac; i++) putchar('0'); }
    }
}

/* Rust's `{:e}`: the shortest digits as d.ddde<exponent>. */
static void shortest(double v) {
    if (v == 0.0) { printf("zero"); return; }
    KaleidoscopeDigits d = {0};
    kaleidoscope_decimal_shortest(v, &d);
    if (v < 0.0) putchar('-');
    putchar(d.digits[0]);
    if (d.len > 1) { putchar('.'); fwrite(d.digits + 1, 1, d.len - 1, stdout); }
    printf("e%d", (int)d.k - 1);
}

/* Rust's `parse::<f64>` grammar for plain decimals, digits fed to the unit. */
static int parse(const char *text, double *out) {
    size_t at = 0, n = strlen(text);
    KaleidoscopeDecimal decimal = {0};
    if (at < n && (text[at] == '+' || text[at] == '-')) decimal.negative = text[at++] == '-';
    int digits = 0;
    while (at < n && text[at] >= '0' && text[at] <= '9') { kaleidoscope_decimal_digit(&decimal, (uint8_t)(text[at++] - '0'), false); digits = 1; }
    if (at < n && text[at] == '.') {
        at++;
        while (at < n && text[at] >= '0' && text[at] <= '9') { kaleidoscope_decimal_digit(&decimal, (uint8_t)(text[at++] - '0'), true); digits = 1; }
    }
    if (!digits) return 0;
    int64_t exponent = 0;
    if (at < n && (text[at] == 'e' || text[at] == 'E')) {
        at++;
        int negative = 0;
        if (at < n && (text[at] == '+' || text[at] == '-')) negative = text[at++] == '-';
        if (at == n || text[at] < '0' || text[at] > '9') return 0;
        while (at < n && text[at] >= '0' && text[at] <= '9') { if (exponent < 1000000000000) exponent = exponent * 10 + (text[at] - '0'); at++; }
        if (negative) exponent = -exponent;
    }
    if (at != n) return 0;
    *out = kaleidoscope_decimal_value(&decimal, exponent);
    return 1;
}

int main(int argc, char **argv) {
    static const size_t precisions[] = {PRECISIONS};
    static char line[4096];
    int format = argc > 1 && strcmp(argv[1], "format") == 0;
    while (fgets(line, sizeof line, stdin)) {
        size_t len = strlen(line);
        if (len && line[len - 1] == '\n') line[--len] = 0;
        if (format) {
            double x = from_bits(strtoull(line, NULL, 16));
            shortest(x);
            if (x < 1e21 && x > -1e21)
                for (size_t i = 0; i < sizeof precisions / sizeof precisions[0]; i++) { putchar(' '); exact_fixed(x, precisions[i]); }
            putchar('\n');
        } else {
            double v;
            if (parse(line, &v)) printf("%016llx\n", (unsigned long long)to_bits(v)); else puts("ERR");
        }
    }
    return 0;
}
'''


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cesta", type=Path, required=True)
    parser.add_argument("--work", type=Path)
    parser.add_argument("--cases", type=int, default=40000)
    parser.add_argument("--mutations", action="store_true")
    parser.add_argument("--cesta-source", type=Path)
    parser.add_argument("--lean-root", type=Path)
    args = parser.parse_args()
    getcontext().prec = 2000
    work = (args.work or Path(tempfile.mkdtemp(prefix="kaleidoscope-decimal-", dir=HERE / "build"))).resolve()
    work.mkdir(parents=True, exist_ok=True)
    rng = random.Random(20261105)
    values = doubles(rng, args.cases)
    corpus = {
        "format": "".join("%016x\n" % to_bits(x) for x in values).encode(),
        "parse": "".join(t + "\n" for t in texts(rng, values, args.cases // 3)).encode(),
    }
    precisions = ", ".join(map(str, PRECISIONS))
    (work / "reference.rs").write_text(REFERENCE.replace("PRECISIONS", f"[{precisions}]"))
    run(["rustc", "--edition=2021", "-C", "opt-level=2", work / "reference.rs", "-o", work / "reference"])
    expected = {mode: run([work / "reference", mode], stdin=data) for mode, data in corpus.items()}
    (work / "host.c").write_text(HOST.replace("{PRECISIONS}", "{" + precisions + "}"))
    common = ["-std=c11", "-g0", "-Wall", "-Wextra", "-Wpedantic", "-Werror", "-ffp-contract=off",
              "-Igenerated/include"]
    sources, plans, builds = {}, {}, []
    for compiler in ("gcc", "clang"):
        source = work / f"decimal-{compiler}.c"
        plan = work / f"decimal-{compiler}.plan"
        run([args.cesta, "export-c", "src/kaleidoscope_decimal.cesta", "--cc", compiler, "--std=c11",
             "--plan", plan, "-o", source, "--", "-Igenerated/include"])
        sources[compiler] = source
        plans[plan.read_bytes()] = plan
        for mode, extra in (("O0", ["-O0"]), ("O2", ["-O2"]),
                            ("SAN", ["-O1", "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
                                     "-fno-pie", "-no-pie"])):
            binary = work / f"{compiler}-{mode}"
            run([compiler, *common, *extra, work / "host.c", source, "-o", binary])
            for kind, data in corpus.items():
                actual = run([binary, kind], stdin=data)
                if actual != expected[kind]:
                    rows = [(a, b) for a, b in zip(expected[kind].split(b"\n"), actual.split(b"\n")) if a != b]
                    raise AssertionError((compiler, mode, kind, len(rows), rows[:3]))
            builds.append((compiler, mode))
    mutations = []
    if args.mutations:
        base = LayoutText(sources["gcc"].read_text())
        changes = [
            ("parse-ties-even", "remainder==half&&(rest||(kept&1))", "remainder==half&&rest"),
            ("parse-sticky", "bool rest=num.n!=0||decimal->sticky;", "bool rest=num.n!=0;"),
            ("shortest-tie", "increase=decimal_big_cmp(&sum,&scale)>=0;", "increase=decimal_big_cmp(&sum,&scale)>0;"),
            ("shortest-inclusive", "int rounding=d.inclusive?1:0;", "int rounding=0;"),
            ("exact-half-even", "(order==0&&length>0&&(out->digits[length-1]&1))", "(order==0&&length>0)"),
            ("exact-room", "size_t room=21+", "size_t room=17+"),
            ("subnormal-scale", "drop+=-1022-lead;", "drop+=-1021-lead;"),
            ("fast-path", "count<=15&&exp10>=-22&&exp10<=22", "count<=17&&exp10>=-22&&exp10<=22"),
        ]
        for name, old, new in changes:
            assert base.count(old) == 1, (name, base.count(old))
            mutant = work / f"{name}.c"
            mutant.write_text(base.replace(old, new))
            binary = work / name
            built = subprocess.run(["gcc", *common, "-O2", work / "host.c", mutant, "-o", binary], cwd=HERE,
                                   capture_output=True)
            if built.returncode == 0:
                assert any(subprocess.run([binary, kind], cwd=HERE, input=data, capture_output=True).stdout
                           != expected[kind] for kind, data in corpus.items()), ("survived", name)
            mutations.append(name)
    admission = []
    if args.lean_root:
        assert args.cesta_source
        sys.path.insert(0, str(args.cesta_source / "tests"))
        import ownership_admission
        for index, plan in enumerate(plans.values()):
            record, = ownership_admission.parse(plan.read_bytes())
            admission.append(ownership_admission.admit_record(
                record, work / f"KaleidoscopeDecimalOwnership{index}.lean", args.lean_root))
    (work / "summary.json").write_text(json.dumps({
        "doubles": len(values), "texts": corpus["parse"].count(b"\n"), "precisions": list(PRECISIONS),
        "builds": builds, "mutations": mutations, "admission": admission,
    }, indent=2) + "\n")
    print("kaleidoscope decimal:", len(values), "doubles x", 1 + len(PRECISIONS), "formats,",
          corpus["parse"].count(b"\n"), "texts,", len(builds), "builds,", len(mutations), "mutants,",
          sum(item["functions"] for item in admission), "admitted")


if __name__ == "__main__":
    main()
