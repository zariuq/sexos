#!/usr/bin/env python3
"""Exercise the real current Kaleidoscope client through the bounded HTML/script path."""

import argparse
import json
import os
from pathlib import Path
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent.parent


def run(argv, *, env=None):
    result = subprocess.run(list(map(str, argv)), cwd=HERE, capture_output=True, env=env)
    if result.returncode:
        raise RuntimeError((argv, result.returncode, result.stdout[-3000:], result.stderr[-5000:]))
    return result.stdout


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cesta", type=Path, required=True)
    parser.add_argument("--work", type=Path)
    args = parser.parse_args()
    work = (args.work or Path(tempfile.mkdtemp(prefix="kaleidoscope-html-live-", dir=HERE / "build"))).resolve()
    work.mkdir(parents=True, exist_ok=True)
    units = ("kaleidoscope", "kaleidoscope_body", "kaleidoscope_script_lexer",
             "kaleidoscope_regex", "kaleidoscope_regex_match", "kaleidoscope_script_core",
             "kaleidoscope_html")
    host = HERE / "tests/kaleidoscope_html_live/host.c"
    rows = []
    for compiler in ("gcc", "clang"):
        sources = []
        for unit in units:
            source = work / f"{unit}-{compiler}.c"
            run([args.cesta, "export-c", f"src/{unit}.cesta", "--cc", compiler, "--std=c11",
                 "-o", source, "--", "-Igenerated/include"])
            sources.append(source)
        for mode, flags in (("O2", ["-O2"]), ("SAN", ["-O1", "-fsanitize=address,undefined",
                                                       "-fno-sanitize-recover=all", "-fno-pie", "-no-pie"])):
            binary = work / f"{compiler}-{mode}"
            run([compiler, "-std=c11", "-g0", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
                 "-DSEXOS_HOST", "-DSEXOS_KALEIDOSCOPE_CURRENT=1", "-Igenerated/include",
                 *flags, host, *sources, "runtime/client_runtime.c",
                 "generated/runtime/packed_words.c", "generated/runtime/format_bytes.c", "-o", binary])
            output = run([binary], env={**os.environ, "ASAN_OPTIONS": "detect_leaks=1:abort_on_error=1"})
            assert b"[browser.html.converted]" in output
            assert b"scripts_run=1 scripts_refused=0" in output
            assert output.rstrip().endswith(b"P 414243"), output[-2000:]
            rows.append((compiler, mode))
    summary = {"builds": rows, "body": body_text(), "presented_hex": "414243"}
    (work / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")
    print("kaleidoscope live html:", len(rows), "builds, real client/core path")


def body_text():
    return "<html><p>A<script>document.write('B')</script>C"


if __name__ == "__main__":
    main()
