#!/usr/bin/env python3
"""Boot-log observations for comparing SexOS images.

  oracle.py observe LOG [--checker CHECKER] > OBS.json
  oracle.py compare REF.json NEW.json [--sections a,b,...]

`observe` reads a serial log, removes the timer and scheduler prints that
interrupts splice into other lines, and records what the boot did: modules
loaded, protection domains spawned, capabilities granted, handoffs, servers
ready, first sends on each edge, contract fingerprints, the clock, faults, and
the verdicts of the upstream boot-graph checker. Facts that depend on the exact
binaries (segment addresses, entry points) are kept in their own section.

`compare` reports every section in which two observations differ. A section
is inconclusive when the log could be read more than one way there (see Log);
compare exits 1 on a conclusive difference, 2 when only inconclusive sections
differ, and 0 when the observations are equal.
"""
from __future__ import annotations

import argparse
import copy
import json
import re
import subprocess
import sys
from pathlib import Path

ANSI = re.compile(r"\x1b\[[0-9;?]*[A-Za-z]")
# Prints made from the timer interrupt and the preempting scheduler
# (kernel/src/interrupts.rs, kernel/src/scheduler.rs, kernel/src/memory/manager.rs).
INTERRUPT = re.compile(
    r"timer\.tick\.enter"
    r"|timer\.fire\.count=\d+"
    r"|\[timer\.tick\.defer\] reason=\S+ phase=\d+"
    r"|scheduler\.tick\.enter core=\d+ phase=\d+ rq_depth=\d+"
    r"|scheduler\.pick_next pd_id=\d+"
    r"|scheduler\.restore_context pd_id=\d+ kstack_top=0x[0-9a-f]+ rip=0x[0-9a-f]+"
    r"|task\.running id=\d+ pd_id=\d+ rip=0x[0-9a-f]+ rsp=0x[0-9a-f]+"
    r"|task\.requeued id=\d+ pd_id=\d+"
    r"|\[perf\.noise\.summary\] name=\S+ count=\d+ suppressed=\d+"
)

# How lines begin: a bracketed marker, "name: ", an arrow or tick, or a dotted
# event name followed by a space or the end of the line.
LINE_START = re.compile(r"\[|[A-Za-z][\w.-]*: |\s*[→✓]|[a-z_]+(?:\.[a-z_]+)+(?: |$)")

BEHAVIOUR = [
    "modules", "spawns", "spawned_without_ok", "grants", "collar_grants", "memlend", "phase25_complete",
    "handoffs", "scheduler_ready", "first_scheduled", "ready", "first_sends",
    "fingerprints", "clock", "faults", "checker",
]
BINARY = ["segments", "entries"]


def expected_reference(ref: dict, new: dict, policy: dict) -> tuple[dict, list[str]]:
    """Apply exact, named log corrections; every declared correction must occur.

    Only grant/handoff records may be corrected. Faults, checker verdicts and
    program sends remain ordinary comparisons. The default comparison never
    loads a policy.
    """
    if set(policy) != {"schema", "changes"} or policy["schema"] != "sexos-log-differences/1":
        raise ValueError("unknown expected-difference schema")
    changes = policy["changes"]
    if not isinstance(changes, list) or not changes:
        raise ValueError("expected differences must be a nonempty list")
    adjusted = copy.deepcopy(ref)
    names = []
    for change in changes:
        if not isinstance(change, dict):
            raise ValueError("a correction must be an object")
        operation = change.get("operation")
        fields = {"id", "reason", "section", "operation", "after"}
        if operation == "replace-item":
            fields.add("before")
        elif operation == "insert-item":
            fields.add("index")
        else:
            raise ValueError("unknown correction operation")
        if set(change) != fields:
            raise ValueError("unknown or missing correction field")
        name, reason, section = (change[k] for k in ("id", "reason", "section"))
        if (not isinstance(name, str) or not re.fullmatch(r"[a-z][a-z0-9-]*", name)
                or name in names or not isinstance(reason, str) or not reason.strip()):
            raise ValueError("corrections need unique names and a reason")
        if section not in ("grants", "handoffs"):
            raise ValueError("only grant and handoff log corrections are supported")
        old, actual, after = adjusted.get(section), new.get(section), change["after"]
        def record_ok(value):
            if section == "handoffs":
                return (isinstance(value, list) and len(value) == 2 and isinstance(value[0], str)
                        and type(value[1]) is int and value[1] > 0)
            return (isinstance(value, str) and re.fullmatch(
                r"from=\S+ to=\S+ slot=\S+ target=\S+ ok=[01](?: optional=[01])?", value) is not None)
        if not record_ok(after) or (operation == "replace-item" and not record_ok(change["before"])):
            raise ValueError(name + ": invalid log-record shape")
        if not isinstance(old, list) or not isinstance(actual, list) or actual.count(after) != 1:
            raise ValueError(name + ": expected candidate record is absent or duplicated")
        if operation == "replace-item":
            before = change["before"]
            if before == after or old.count(before) != 1 or after in old:
                raise ValueError(name + ": reference record is absent, duplicated or unchanged")
            old[old.index(before)] = after
        else:
            index = change["index"]
            if type(index) is not int or not 0 <= index <= len(old) or after in old:
                raise ValueError(name + ": invalid insertion or already present in reference")
            old.insert(index, after)
        names.append(name)
    return adjusted, names


SPECIAL = set("[](){}*+?|^$.\\")


def literal_prefix(pattern: str) -> str:
    """The literal text every match of `pattern` starts with."""
    out, i = [], 0
    while i < len(pattern):
        c = pattern[i]
        if c == "\\" and i + 1 < len(pattern) and pattern[i + 1] in SPECIAL:
            lit, i = pattern[i + 1], i + 2
        elif c in SPECIAL:
            break
        else:
            lit, i = c, i + 1
        if i < len(pattern) and pattern[i] in "?*{":
            break
        out.append(lit)
    return "".join(out)


class Log:
    """A serial log with the interrupt prints removed.

    Output from several protection domains shares one serial line. A domain can
    be preempted in the middle of a line, with or without an interrupt print;
    the rest of its line then appears later, at the start of a physical line,
    after whatever other domains printed meanwhile. The log does not say which
    domain printed a line, so lines are not rejoined wholesale. Instead, for
    each pattern, `matches` treats a position as cut when the pattern's literal
    prefix starts there without a complete match, or when the line ends inside
    that prefix or inside an unclosed bracket. The cut text ends at the end of
    the line or where another domain's bracketed marker begins; the first of the
    next WINDOW lines whose text completes a match across the join is taken.
    """

    WINDOW = 400

    def __init__(self, path: Path):
        text = ANSI.sub("", path.read_text(errors="replace")).replace("\r", "")
        self.lines: list[str] = []
        self.cut_at_end: set[int] = set()
        for line in text.split("\n"):
            cut = False
            while (m := INTERRUPT.search(line)) is not None:
                cut = cut or (bool(line[: m.start()].strip()) and not line[m.end():].strip())
                line = line[: m.start()] + line[m.end():]
            if line.strip():
                self.lines.append(line)
                if cut:
                    self.cut_at_end.add(len(self.lines) - 1)
        self.interrupted = len(self.cut_at_end)
        self.ambiguous: list[tuple[str, str, int]] = []

    def boundaries(self, i: int) -> list[int]:
        """Positions in line i where text may have been cut: before another
        domain's bracketed marker, and the end if an interrupt print followed."""
        line = self.lines[i]
        out = [c for c in range(1, len(line)) if line[c] == "["]
        if i in self.cut_at_end:
            out.append(len(line))
        return out

    def _cut_positions(self, i: int, anchor: str, starts: set[int]) -> list[int]:
        line = self.lines[i]
        cuts = set()
        if anchor:
            k = line.find(anchor)
            while k >= 0:
                if k not in starts:
                    cuts.add(k)
                k = line.find(anchor, k + 1)
        for c in self.boundaries(i):
            for j in range(min(len(anchor) - 1, c), 0, -1):
                if line[c - j:c] == anchor[:j]:
                    cuts.add(c - j)
                    break
            if anchor in ("", "["):
                k = line.rfind("[", 0, c)
                if k >= 0 and "]" not in line[k:c] and k not in starts:
                    cuts.add(k)
        return sorted(cuts)

    def matches(self, pattern: re.Pattern) -> list[tuple[int, re.Match]]:
        """Matches in line order, as (index of the line where the match starts,
        match). Each cut takes the earliest continuation no earlier cut took.
        The reading is ambiguous, and `self.ambiguous` grows, when a cut has
        continuations giving different texts, or when cuts compete for too few
        continuations; the observation then marks the section inconclusive."""
        anchor = literal_prefix(pattern.pattern)
        found, pending = [], []
        for i, line in enumerate(self.lines):
            whole = list(pattern.finditer(line))
            for m in whole:
                # A match followed only by part of its record before a cut may
                # be a shortened record.
                ends = [c for c in self.boundaries(i) if c >= m.end() and "]" not in line[m.end():c]]
                options = self._continuations(i, m.start(), pattern, ends) if ends else []
                # A continuation that adds a field (it starts with a delimiter)
                # extends the record. One that would continue the match's last
                # token mid-value is as likely to belong to another record.
                if any(self._continues_token(m, e) for _, e in options):
                    self.ambiguous.append((pattern.pattern, "cut after a complete value", i))
                    options = []
                if options:
                    pending.append((i, options))
                else:
                    found.append((i, m))
            for k in self._cut_positions(i, anchor, {m.start() for m in whole}):
                ends = [c for c in self.boundaries(i) if c > k] or [len(line)]
                if options := self._continuations(i, k, pattern, ends):
                    pending.append((i, options))
        used: set[int] = set()
        for i, options in pending:
            if len({m.group(0) for _, m in options}) > 1:
                self.ambiguous.append((pattern.pattern, "cut with continuations giving different texts", i))
            free = [(j, m) for j, m in options if j not in used]
            if not free:
                self.ambiguous.append((pattern.pattern, "cuts competing for one continuation", i))
                free = options
            j, m = free[0]
            used.add(j)
            found.append((i, m))
        found.sort(key=lambda e: e[0])
        return found

    @staticmethod
    def _continues_token(short: re.Match, longer: re.Match) -> bool:
        text, added = short.group(0), longer.group(0)[len(short.group(0)):]
        return bool(text) and bool(added) and text[-1].isalnum() and (added[0].isalnum() or added[0] in "_.-")

    def _continuations(self, i: int, k: int, pattern: re.Pattern, ends: list[int]) -> list[tuple[int, re.Match]]:
        """Later lines that complete the text from k, cut at the first of
        `ends` for which any line does. A continuation never looks like the
        start of a line, so such lines are skipped, unless the cut lies inside
        the pattern's literal prefix: the continuation must then spell out the
        rest of that literal, which is specific enough on its own."""
        line = self.lines[i]
        literal = literal_prefix(pattern.pattern)
        for c in ends:
            inside_literal = c - k < len(literal)
            options = []
            for j in range(i + 1, min(len(self.lines), i + 1 + self.WINDOW)):
                later = self.lines[j]
                if not inside_literal and LINE_START.match(later):
                    continue
                m = pattern.match(line[:c] + later, k)
                if m is not None and m.end() > c:
                    options.append((j, m))
            if options:
                return options
        return []

    def first(self, needle: str) -> int | None:
        hits = self.matches(re.compile(re.escape(needle)))
        return hits[0][0] if hits else None


def observe(log: Log) -> dict:
    obs: dict = {}
    inconclusive: set[str] = set()

    def matches(sections: tuple[str, ...], pattern: str) -> list[tuple[int, re.Match]]:
        before = len(log.ambiguous)
        found = log.matches(re.compile(pattern))
        if len(log.ambiguous) > before:
            inconclusive.update(sections)
        return found

    def first(section: str, needle: str) -> int | None:
        hits = matches((section,), re.escape(needle))
        return hits[0][0] if hits else None

    obs["modules"] = [m.group(1) for _, m in matches(("modules",), r"limine: Loading module `boot:///([^`]+)`")]

    spawn_sections = ("spawns", "spawned_without_ok", "segments", "entries")
    events = sorted(
        [(i, "begin", m) for i, m in matches(spawn_sections, r"\[bootgraph\.pd\.spawn\.begin\] pd=(\S+)")]
        + [(i, "ok", m) for i, m in matches(spawn_sections, r"\[bootgraph\.pd\.spawn\.ok\] pd=(\S+) id=(\d+) pkey=(\d+)")]
        + [(i, "segment", m) for i, m in matches(spawn_sections, r"ELF: Loading segment: vaddr=(0x[0-9a-f]+), memsz=(0x[0-9a-f]+)")]
        + [(i, "entry", m) for i, m in matches(spawn_sections, r"checkpoint=elf\.after_load entry=(0x[0-9a-f]+)")],
        key=lambda e: e[0])
    spawns, begun = [], []
    segments: dict[str, list] = {}
    entries: dict[str, str] = {}
    current = None
    for _, kind, m in events:
        if kind == "begin":
            current = m.group(1)
            begun.append(current)
            segments[current] = []
        elif kind == "ok":
            spawns.append([m.group(1), int(m.group(2)), int(m.group(3))])
            current = None
        elif kind == "segment" and current:
            segments[current].append([m.group(1), m.group(2)])
        elif kind == "entry" and current:
            entries[current] = m.group(1)
    obs["spawns"] = spawns
    ok = {s[0] for s in spawns}
    obs["spawned_without_ok"] = [p for p in begun if p not in ok]

    # Grants are printed as "[bootgraph.cap.grant] from=…" and as
    # "[bootgraph.cap.grant from=… ok=1]"; some only in one form.
    obs["grants"] = sorted({m.group(1) for _, m in matches(("grants",),
        r"\[bootgraph\.cap\.grant\]? (from=\S+ to=\S+ slot=\S+ target=\S+ ok=\d+(?: optional=\d+)?)")})
    obs["collar_grants"] = sorted({f"{m.group(1)} {m.group(2)}" for _, m in matches(("collar_grants",),
        r"\[collar\.grant\.(auto|init)\] ((?:grant_id|count)=\d+ (?:subject=\d+ object=\d+ op=\w+|generation=\d+))")})
    obs["memlend"] = [f"{m.group(1)}{m.group(2) or ''}" for _, m in matches(("memlend",),
        r"\[kernel\.memlend\.grant\.(begin|ok|err)\]( caller_pd=\d+ target_pd=\d+ slot=\d+| reason=\w+)?")]
    b = first("phase25_complete", "[bootgraph.phase25.begin]")
    c = first("phase25_complete", "[bootgraph.phase25.complete]")
    obs["phase25_complete"] = b is not None and c is not None and b < c

    obs["handoffs"] = [[m.group(1), int(m.group(2))] for _, m in matches(("handoffs",),
        r"\[bootgraph\.boot\.handoff\] target=(\S+) id=(\d+)")]
    obs["scheduler_ready"] = first("scheduler_ready", "init: Ready for Scheduler.") is not None
    fs = matches(("first_scheduled",), r"first scheduled pd_id=(\d+)")
    obs["first_scheduled"] = int(fs[0][1].group(1)) if fs else None

    obs["ready"] = sorted({m.group(1) for _, m in matches(("ready",), r"\[([A-Za-z0-9_.-]+?)\.ready\]")})
    obs["first_sends"] = sorted({m.group(1) for _, m in matches(("first_sends",),
        r"\[bootgraph\.edge\.send (from=\S+ to=\S+ slot=\S+ op=\S+) first=1\]")})
    obs["fingerprints"] = sorted({m.group(0) for _, m in matches(("fingerprints",),
        r"abi=\d+ layout=\d+ theme=\d+ fp=0x[0-9a-f]{16}")})

    ticks = [tuple(int(g) for g in m.groups()) for _, m in matches(("clock",),
             r"\[silkbar\.clock\.send\] hh=(\d+) mm=(\d+) ss=(\d+)")]
    secs = [h * 3600 + mi * 60 + s for h, mi, s in ticks]
    obs["clock"] = {"sends": len(ticks) > 0, "monotonic": all(a <= b for a, b in zip(secs, secs[1:]))}

    fault_patterns = {"#PF": r"\#PF", "#GP": r"\#GP", "fault.kill": r"fault\.kill",
                      "panic": r"panic|KERNEL PANIC|\[cesta\.client\.fault\]"}
    obs["faults"] = {name: len(matches(("faults",), pattern)) for name, pattern in fault_patterns.items()}
    obs["interrupted_lines"] = log.interrupted
    obs["segments"] = segments
    obs["entries"] = entries
    obs["inconclusive"] = sorted(inconclusive)
    return obs


def checker_verdicts(checker: Path, log: Path) -> dict:
    """Gate verdicts of the upstream checker, with line numbers removed."""
    run = subprocess.run([sys.executable, "-I", str(checker), str(log)],
                         capture_output=True, text=True)
    gates = {}
    for l in run.stdout.splitlines():
        if m := re.match(r"([A-Z_]+_GATE): (PASS|FAIL)\s*(.*)", l):
            reason = re.sub(r"\d+", "N", m.group(3))
            gates[m.group(1)] = [m.group(2), reason]
    return {"exit": run.returncode, "gates": gates}


def main() -> int:
    ap = argparse.ArgumentParser()
    sub = ap.add_subparsers(dest="cmd", required=True)
    o = sub.add_parser("observe")
    o.add_argument("log", type=Path)
    o.add_argument("--checker", type=Path)
    c = sub.add_parser("compare")
    c.add_argument("ref", type=Path)
    c.add_argument("new", type=Path)
    c.add_argument("--sections", default=",".join(BEHAVIOUR))
    c.add_argument("--allow", type=Path, help="explicit exact grant/handoff log corrections; strict by default")
    a = ap.parse_args()

    if a.cmd == "observe":
        obs = observe(Log(a.log))
        obs["checker"] = checker_verdicts(a.checker, a.log) if a.checker else None
        json.dump(obs, sys.stdout, indent=1, sort_keys=True)
        print()
        return 0

    ref, new = json.loads(a.ref.read_text()), json.loads(a.new.read_text())
    sections = a.sections.split(",")
    if any(s not in BEHAVIOUR + BINARY for s in sections) or len(set(sections)) != len(sections):
        ap.error("sections must be distinct known observation sections")
    if any(s not in ref or s not in new for s in sections):
        ap.error("an observation is missing a requested section")
    if a.allow:
        try:
            ref, corrections = expected_reference(ref, new, json.loads(a.allow.read_text()))
        except (ValueError, TypeError, KeyError) as e:
            ap.error(str(e))
        print("exact log corrections: " + ", ".join(corrections))
    if "faults" in sections:
        for label, observed in (("reference", ref), ("candidate", new)):
            faults = observed["faults"]
            if (not isinstance(faults, dict) or set(faults) != {"#GP", "#PF", "fault.kill", "panic"}
                    or any(type(count) is not int or count != 0 for count in faults.values())):
                print(label + " has faults or an unknown fault schema")
                return 1
    unsure = set(ref.get("inconclusive", [])) | set(new.get("inconclusive", []))
    differ = [s for s in sections if ref[s] != new[s]]
    for s in differ:
        label = "inconclusive" if s in unsure else "differs"
        print(f"{s} ({label}):\n  ref: {json.dumps(ref.get(s))}\n  new: {json.dumps(new.get(s))}")
    conclusive = [s for s in differ if s not in unsure]
    if conclusive:
        print(f"{len(conclusive)} section(s) differ")
        return 1
    if unsure.intersection(sections):
        print(f"no conclusive difference; {len(unsure.intersection(sections))} section(s) inconclusive (ambiguous split lines)")
        return 2
    print("equal")
    return 0


if __name__ == "__main__":
    sys.exit(main())
