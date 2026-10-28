#!/usr/bin/env python3
"""For each ported server, compare which of the markers its Cesta source prints
appear in the boot log with those in the reference boot (a proof lane or a path
present in one and absent in the other). Values and interleaving depend on
timing, so the full line sequences are compared only for information.

usage: tools/compare_markers.py REFERENCE_LOG NEW_LOG SERVER...   (SERVER names src/SERVER.cesta)

Both boots should run until the same markers: one that stops as soon as the
milestones appear can miss a late line the other printed.
"""

from pathlib import Path
import re
import sys

CESTA = Path(__file__).resolve().parent.parent / "src"


def markers(server):
    text = (CESTA / f"{server}.cesta").read_text()
    return set(re.findall(r'"(\[[a-z_][a-z0-9_.]*\])', text))


def lines(log, names):
    out = []
    for line in Path(log).read_text(errors="replace").splitlines():
        for m in re.finditer(r"\[[a-z_][a-z0-9_.]*\][^\[]*", line):
            name = re.match(r"\[[^\]]*\]", m.group(0)).group(0)
            if name in names:
                out.append(m.group(0).rstrip())
    return out


def main():
    ref, new, servers = sys.argv[1], sys.argv[2], sys.argv[3:]
    status = 0
    for server in servers:
        names = markers(server)
        a, b = lines(ref, names), lines(new, names)
        seen_a = {re.match(r"\[[^\]]*\]", x).group(0) for x in a}
        seen_b = {re.match(r"\[[^\]]*\]", x).group(0) for x in b}
        same_lines = "lines equal" if a == b else "lines differ in values or order"
        if seen_a == seen_b:
            print(f"{server}: the same {len(seen_a)} markers ({len(a)} vs {len(b)} lines, {same_lines})")
            continue
        status = 1
        print(f"{server}: markers differ; only in the reference: {sorted(seen_a - seen_b)}; only in the new boot: {sorted(seen_b - seen_a)}")
    return status


if __name__ == "__main__":
    sys.exit(main())
