#!/usr/bin/env python3
"""Positive and negative controls for oracle.py on a reference boot log.

  test_oracle.py LOG

Changing what the boot did must change the observation: every mutation has to
be detected. Splitting marker lines the way preemption does must never change
a section silently: either the observation stays the same, or the changed
section is marked inconclusive (the log can be read more than one way). The
share of inconclusive splits is reported and must stay at most MAX_INCONCLUSIVE.
"""
from __future__ import annotations

import random
import re
import sys
import tempfile
from pathlib import Path

sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parent))
import oracle  # noqa: E402

SPLITS = 300
MAX_INCONCLUSIVE = 0.10
MARKER = re.compile(
    r"\[bootgraph\.(pd\.spawn\.ok|cap\.grant|boot\.handoff|edge\.send)|\[collar\.grant|\.ready\]|fp=0x")


def observe_text(text: str) -> tuple[dict, set[str]]:
    with tempfile.TemporaryDirectory() as d:
        p = Path(d) / "log"
        p.write_text(text)
        obs = oracle.observe(oracle.Log(p))
    return {k: obs[k] for k in oracle.BEHAVIOUR if k != "checker"}, set(obs["inconclusive"])


def split(lines: list[str], i: int, cut: int, later: int, same_line: bool) -> list[str]:
    """Cut line i at `cut` the way preemption does; its tail reappears `later`
    lines on. The cut is followed either by an interrupt print or, with
    same_line, by the next line's text on the same physical line."""
    head, tail = lines[i][:cut], lines[i][cut:]
    out = lines[:i]
    if same_line and i + 1 < len(lines):
        out.append(head + lines[i + 1])
        rest = lines[i + 2:]
    else:
        out.append(head + "scheduler.tick.enter core=0 phase=4 rq_depth=13")
        rest = lines[i + 1:]
    out.extend(rest[:later] + [tail] + rest[later:])
    return out


def main() -> int:
    log = Path(sys.argv[1])
    text = oracle.ANSI.sub("", log.read_text(errors="replace")).replace("\r", "")
    lines = text.split("\n")
    base, base_unsure = observe_text(text)
    failures = 0
    silent = inconclusive = 0
    rng = random.Random(0)

    # Split whole records only: lines that preemption has not already split
    # (no other domain's marker glued on, no interrupt print, not ending in a
    # key with its value still to come).
    marked = [i for i, l in enumerate(lines) if MARKER.search(l) and len(l) > 12
              and "[" not in l[1:] and not oracle.INTERRUPT.search(l) and not l.rstrip().endswith("=")]
    for trial in range(SPLITS):
        i = rng.choice(marked)
        cut = rng.randrange(2, len(lines[i]) - 1)
        later = rng.randrange(1, 40)
        same = rng.random() < 0.5 and i + 1 < len(lines) and lines[i + 1].startswith("[")
        got, unsure = observe_text("\n".join(split(lines, i, cut, later, same)))
        diff = [k for k in base if base[k] != got[k]]
        if [k for k in diff if k not in unsure]:
            silent += 1
            print(f"split silently changed: line {i} cut {cut} later {later} same_line {same}: {diff}")
        elif diff:
            inconclusive += 1

    def mutate(name: str, f) -> None:
        nonlocal failures
        new = f(list(lines))
        if observe_text("\n".join(new))[0] == base:
            failures += 1
            print(f"mutation not detected: {name}")

    def drop(pattern: str):
        return lambda ls: [l for l in ls if pattern not in l]

    def replace_first(old: str, new: str):
        def f(ls):
            for j, l in enumerate(ls):
                if old in l:
                    ls[j] = l.replace(old, new, 1)
                    break
            return ls
        return f

    mutate("missing ready marker", drop("[linen.ready]"))
    mutate("missing grant", drop("slot=SLOT_INPUT target=input_ring"))
    mutate("missing collar grant", drop("grant_id=7 "))
    mutate("changed protection key", replace_first("pd=sexdrive id=2 pkey=2", "pd=sexdrive id=2 pkey=3"))
    mutate("changed handoff target", replace_first("target=sexdrive id=2", "target=sexdrive id=3"))
    mutate("fault marker", lambda ls: ls[:200] + ["#PF at 0x0"] + ls[200:])
    mutate("missing first send", drop("first=1]"))
    mutate("changed fingerprint", lambda ls: [l.replace("fp=0x00201004584e4745", "fp=0x00201004584e4746") for l in ls])
    mutate("first scheduled domain", replace_first("first scheduled pd_id=1", "first scheduled pd_id=2"))
    mutate("module dropped", drop("boot:///apps/spindle"))

    def swap_spawns(ls):
        ok = [j for j, l in enumerate(ls) if "[bootgraph.pd.spawn.ok]" in l]
        a, b = ok[0], ok[1]
        ls[a], ls[b] = ls[b], ls[a]
        return ls
    mutate("spawn order", swap_spawns)

    rate = inconclusive / SPLITS
    ok = failures == 0 and silent == 0 and not base_unsure and rate <= MAX_INCONCLUSIVE
    print(f"oracle controls: {'passed' if ok else 'FAILED'}: 11 mutations, {failures} undetected; "
          f"{SPLITS} forced splits: {silent} silent changes, {inconclusive} inconclusive "
          f"({rate:.1%}, limit {MAX_INCONCLUSIVE:.0%}); reference inconclusive: {sorted(base_unsure) or 'none'}; "
          f"{len(marked)} marker lines")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
