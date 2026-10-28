#!/usr/bin/env python3
"""A matching observation is insufficient if either boot faults or is ambiguous."""

import copy
import importlib.util
import json
from pathlib import Path

HERE = Path(__file__).resolve().parent.parent


def main():
    spec = importlib.util.spec_from_file_location("qualify_client", HERE / "tools/qualify_client.py")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    reference = json.loads((HERE / "oracle/reference/4a040a86.json").read_text())
    module.require_clean_observation(reference)
    changes = []
    for name in ("#GP", "#PF", "fault.kill", "panic"):
        changed = copy.deepcopy(reference)
        changed["faults"][name] = 1
        changes.append(changed)
    ambiguous = copy.deepcopy(reference)
    ambiguous["inconclusive"] = ["ready"]
    changes.append(ambiguous)
    unknown = copy.deepcopy(reference)
    unknown["faults"]["new_fault"] = 0
    changes.append(unknown)
    unknown_value = copy.deepcopy(reference)
    unknown_value["faults"]["#PF"] = None
    changes.append(unknown_value)
    for changed in changes:
        try:
            module.require_clean_observation(changed)
        except RuntimeError:
            pass
        else:
            raise AssertionError("boot qualification accepted a fault or ambiguity")
    print("boot observation: clean reference accepted; four faults, ambiguity and unknown schema/value refused")


if __name__ == "__main__":
    main()
