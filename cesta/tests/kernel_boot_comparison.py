#!/usr/bin/env python3
"""Exact log corrections do not hide other boot changes or stale expectations."""
import copy
import importlib.util
import json
from pathlib import Path
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent.parent
spec = importlib.util.spec_from_file_location("boot_oracle", HERE / "oracle/oracle.py")
oracle = importlib.util.module_from_spec(spec)
spec.loader.exec_module(oracle)


def main():
    ref = json.loads((HERE / "oracle/reference/4a040a86.json").read_text())
    policy = json.loads((HERE / "oracle/kernel-log-differences.json").read_text())
    new = copy.deepcopy(ref)
    new["grants"] = sorted(new["grants"] + [policy["changes"][0]["after"]])
    new["handoffs"][-1] = ["kaleidoscope", 14]
    before = copy.deepcopy(ref)
    adjusted, names = oracle.expected_reference(ref, new, policy)
    assert adjusted == new and ref == before and len(names) == 2
    # These differences must survive every permitted correction.
    mutants = []
    for section, value in [("faults", {"#GP": 1}), ("first_sends", ["unexpected-send"]),
                           ("checker", {"exit": 1}), ("ready", []), ("clock", {"monotonic": False})]:
        mutant = copy.deepcopy(new)
        mutant[section] = value
        mutants.append(mutant)
    mutant = copy.deepcopy(new)
    mutant["grants"].append("another-grant")
    mutants.append(mutant)
    for mutant in mutants:
        adjusted, _ = oracle.expected_reference(ref, mutant, policy)
        assert adjusted != mutant
    invalid = []
    for section in ("faults", "checker", "first_sends"):
        change = copy.deepcopy(policy)
        change["changes"][0]["section"] = section
        invalid.append((new, change))
    duplicate = copy.deepcopy(policy)
    duplicate["changes"].append(duplicate["changes"][0])
    invalid.append((new, duplicate))
    unknown = copy.deepcopy(policy)
    unknown["changes"][0]["wildcard"] = True
    invalid.append((new, unknown))
    invalid.append((ref, policy))
    stale = copy.deepcopy(new)
    stale["handoffs"][-1] = ["kaleidoscope", 15]
    invalid.append((stale, policy))
    for candidate, manifest in invalid:
        try:
            oracle.expected_reference(ref, candidate, manifest)
        except ValueError:
            pass
        else:
            raise AssertionError("unsafe or stale expected differences accepted")
    work = Path(tempfile.mkdtemp(prefix="kernel-boot-comparison-", dir=HERE / "build"))
    def compare(reference, candidate, *arguments):
        for name, value in (("reference", reference), ("candidate", candidate)):
            (work / (name + ".json")).write_text(json.dumps(value))
        return subprocess.run(["python3", HERE / "oracle/oracle.py", "compare", work / "reference.json",
                               work / "candidate.json", *arguments], capture_output=True).returncode
    assert compare(ref, ref) == 0
    assert compare(ref, new) == 1
    assert compare(ref, new, "--allow", HERE / "oracle/kernel-log-differences.json") == 0
    faulted = copy.deepcopy(ref)
    faulted["faults"]["#PF"] = 1
    assert compare(faulted, faulted) == 1
    unsure = copy.deepcopy(ref)
    unsure["inconclusive"] = ["ready"]
    assert compare(unsure, unsure) == 2
    assert compare(ref, ref, "--sections", "unknown-section") == 2
    for index, marker in enumerate(("panic", "KERNEL PANIC", "[cesta.client.fault]")):
        log = work / f"panic-{index}.log"
        log.write_text(marker + "\n")
        assert oracle.observe(oracle.Log(log))["faults"]["panic"] == 1
    print(f"kernel log comparison: two exact corrections, {len(mutants)} surviving changes, "
          f"{len(invalid)} refused policies/candidates")


if __name__ == "__main__":
    main()
