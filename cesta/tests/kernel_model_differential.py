#!/usr/bin/env python3
"""Compare the actual kernel's exported C with Lean's executable kernel step.

The C side includes the unedited domain and syscall translation units. Only
CPU entry, allocation, page mapping and logging have host providers. The Lean
side executes SingleAddressSpace.step with the generated SexOSGenesis table.
Every result and complete declared state projection is compared after every
request. This is a test of the stated fragment, not a C refinement proof.
"""

import argparse
import hashlib
import json
from pathlib import Path
import random
import re
import shutil
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent.parent


def run(command, *, cwd=HERE, data=None, timeout=180):
    result = subprocess.run(list(map(str, command)), cwd=cwd, input=data,
                            capture_output=True, timeout=timeout)
    if result.returncode:
        raise RuntimeError((command, result.returncode, result.stdout.decode(errors="replace")[-5000:],
                            result.stderr.decode(errors="replace")[-5000:]))
    return result.stdout


def scenarios(domains, grants):
    # Every real genesis ring, and every domain's implicit self ring.
    basic = []
    for holder, slot, target in grants + [(d, 2, d) for d in domains]:
        basic.extend([(holder, 0, slot, 23), (target, 1, 0, 0),
                      (target, 2, holder, 91), (holder, 1, 0, 0),
                      (target, 2, holder, 92), (holder, 1, 0, 0)])
    yield "genesis-ipc", basic
    # Capacity, FIFO order, wraparound and reply-slot priority over a queued call.
    holder, slot, target = grants[0]
    ring = [(holder, 0, slot, n) for n in range(257)]
    ring += [(target, 1, 0, 0) for _ in range(130)]
    ring += [(holder, 0, slot, n + 300) for n in range(130)]
    ring += [(target, 1, 0, 0) for _ in range(257)]
    ring += [(target, 2, holder, 45), (holder, 0, 2, 46), (holder, 1, 0, 0), (holder, 1, 0, 0)]
    yield "fifo-capacity", ring
    # Page-sized allocation/lending; slots 20..31 exclude the input/device
    # capabilities omitted by SingleAddressSpace, and occupied ring slots stay
    # visible to both models. Allocation and mapping providers cannot fail here.
    memory = []
    fresh = 0
    borrowers = set()
    for holder, slot, target in grants:
        memory += [(holder, 3, 0, 0), (holder, 4, fresh, 0),
                   (target, 4, fresh, 0), (holder, 5, slot, 20),
                   (target, 6, 20, 0), (target, 6, 2, 0)]
        fresh += 1  # A repeated lend to the occupied destination allocates nothing.
        if target not in borrowers:
            fresh += 1
            borrowers.add(target)
    yield "page-lending", memory
    for seed in range(6):
        rng = random.Random(0x6B3E100 + seed)
        requests = []
        for _ in range(360):
            d = rng.choice(domains)
            op = rng.choices(range(7), [28, 28, 14, 4, 8, 10, 8])[0]
            slots = [s for h, s, _ in grants if h == d] + [2, 0, 30, 32]
            if op == 0:
                a, b = rng.choice(slots), rng.randrange(500)
            elif op == 2:
                a, b = rng.choice(domains), rng.randrange(500)
            elif op == 4:
                a, b = rng.randrange(32), 0
            elif op == 5:
                a, b = rng.choice(slots), rng.choice([2, *range(20, 32)])
            elif op == 6:
                a, b = rng.choice([2, 20, 21, 30, 32]), 0
            else:
                a, b = 0, 0
            requests.append((d, op, a, b))
        yield f"mixed-{seed}", requests


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cesta", required=True)
    parser.add_argument("--lean-root", type=Path, required=True)
    args = parser.parse_args()
    cesta = Path(shutil.which(args.cesta) or args.cesta).resolve()
    lean_root = args.lean_root.resolve()
    work = Path(tempfile.mkdtemp(prefix="kernel-model-", dir=HERE / "build"))
    modules = ["src/kernel_domains.cesta", "src/kernel_syscalls.cesta", "src/kernel_rpc.cesta", "src/program_record_tasks.cesta"]
    source_names = [*modules, "src/kernel_interface.cesta", "interfaces/kernel.data", "interfaces/kernel.export",
                    "interfaces/client.data", "interfaces/client.export", "tests/kernel_model/host.c",
                    "tests/kernel_memory/worker_stubs.c", "tests/kernel_memory/record_stubs.c",
                    "runtime/kernel/cpu_x86_64.h", "generated/include/sexos_kernel.h",
                    "generated/include/kernel_genesis.h", "generated/include/client_constants.h"]
    inputs = {name: hashlib.sha256((HERE / name).read_bytes()).hexdigest() for name in source_names}
    flags = ["-Iruntime/kernel", "-Igenerated/include"]
    for module in modules:
        (work / (Path(module).stem + ".c")).write_bytes(run([cesta, "export-c", module, "--", *flags]))
    header = run([cesta, "export-c", "src/kernel_interface.cesta", "--", *flags]).rstrip(b"\n") + b"\n"
    assert header == (HERE / "generated/include/sexos_kernel.h").read_bytes(), "stale kernel header"
    for spec, name in [("interfaces/kernel.export", "kernel_genesis.h"), ("interfaces/client.export", "client_constants.h")]:
        generated = work / name
        run([cesta, "export-header", spec, "-o", generated])
        assert generated.read_bytes().rstrip(b"\n") + b"\n" == (HERE / "generated/include" / name).read_bytes(), "stale table header"
    # Regeneration is checked against the actual Lean instance, before it is run.
    run(["python3", "tools/genesis_lean.py", "--cesta", cesta, work / "SexOSGenesis.lean"])
    model_dir = lean_root / "Mettapedia/Machines/Kernel"
    assert (work / "SexOSGenesis.lean").read_bytes() == (model_dir / "SexOSGenesis.lean").read_bytes(), "stale Lean genesis"
    rows = lambda table, *columns: re.findall(r"\(Row ([^()]*)\)",
        run([cesta, "tables", "rows", "interfaces/kernel.data", table, *columns]).decode())
    domains = [int(row.split()[0]) for row in rows("Domains", "id", "name")]
    names = {row.split()[1]: int(row.split()[0]) for row in rows("Domains", "id", "name")}
    grants = [(names[h], int(s), names[t]) for h, s, t in
              (row.split() for row in rows("RingGrants", "holder", "slot", "target"))]
    cases = list(scenarios(domains, grants))
    expected = {}
    run(["lake", "build", "Mettapedia.Machines.Kernel.TraceReplay"], cwd=lean_root)
    lean = ["lake", "env", "lean", "--run", HERE / "tests/kernel_model/reference.lean"]
    for name, requests in cases:
        data = "".join(" ".join(map(str, request)) + "\n" for request in requests).encode()
        (work / (name + ".input")).write_bytes(data)
        expected[name] = run(lean, cwd=lean_root, data=data)
        (work / (name + ".lean.out")).write_bytes(expected[name])
        assert len(expected[name].splitlines()) == len(requests)
    include = ["-I", work, "-I", HERE / "generated/include", "-I", HERE / "runtime/kernel"]
    common = ["-std=c11", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
              "-DSEXOS_WORKER_STUBS_NO_PLATFORM=1",
              "-ffunction-sections", "-fdata-sections", "-Wl,--gc-sections", *include]
    host = HERE / "tests/kernel_model/host.c"
    checked = []
    for compiler in ("gcc", "clang"):
        for mode in ("O0", "O2", "sanitize"):
            binary = work / f"host-{compiler}-{mode}"
            options = (["-O1", "-g", "-fsanitize=address,undefined", "-fno-sanitize-recover=all"]
                       if mode == "sanitize" else ["-" + mode])
            run([compiler, *common, *options, host, work / "program_record_tasks.c",
                 HERE / "tests/kernel_memory/worker_stubs.c", HERE / "tests/kernel_memory/record_stubs.c", "-o", binary])
            assert run([binary, "--bounds"]) == b"kernel boundary controls passed\n"
            for name, _ in cases:
                actual = run([binary], data=(work / (name + ".input")).read_bytes())
                if actual != expected[name]:
                    (work / (name + f".{compiler}-{mode}.out")).write_bytes(actual)
                    line = next((i for i, pair in enumerate(zip(actual.splitlines(), expected[name].splitlines()))
                                 if pair[0] != pair[1]), min(len(actual.splitlines()), len(expected[name].splitlines())))
                    raise AssertionError((name, compiler, mode, "first differing request", line + 1, work))
            checked.append(f"{compiler}-{mode}")
    mutations = [
        ("kernel_domains.c", "c->ring->count >= KERNEL_RING_CAPACITY", "c->ring->count >= KERNEL_RING_CAPACITY - 1"),
        ("kernel_domains.c", "c->tail = (c->head + c->count) % KERNEL_RING_CAPACITY;", "c->tail = c->head;"),
        ("kernel_domains.c", "(c->head + 1u) % KERNEL_RING_CAPACITY", "(c->head + 2u) % KERNEL_RING_CAPACITY"),
        ("kernel_syscalls.c", "call_entry.arg0 = f->rdx;", "call_entry.arg0 = f->rdx + 1;"),
        ("kernel_syscalls.c", "call_entry.arg1 = f->r10;", "call_entry.arg1 = f->r10 + 1;"),
        ("kernel_syscalls.c", "call_entry.arg2 = f->r8;", "call_entry.arg2 = f->r8 + 1;"),
        ("kernel_syscalls.c", "d->reply_rights[out->sender] += 1;", "d->reply_rights[out->sender] = 1;"),
        ("kernel_syscalls.c", "d->reply_rights[target_id] -= 1;", "d->reply_rights[target_id] = 0;"),
        ("kernel_syscalls.c", "target->reply_value = f->rsi;", "target->reply_value = f->rsi + 1;"),
        ("kernel_syscalls.c", "if (d->reply_ready)", "if (d->reply_ready && d->ring.count == 0)"),
        ("kernel_syscalls.c", "record_block(phys, KERNEL_PAGE_SIZE << order, d->id);",
         "record_block(phys, KERNEL_PAGE_SIZE << order, d->id % 14 + 1);"),
        ("kernel_syscalls.c", "PTE_USER | PTE_NO_EXECUTE), d->key)", "PTE_USER | PTE_NO_EXECUTE), (uint8_t)(d->key ^ 1))"),
        ("kernel_syscalls.c", "lent.permissions = 3;", "lent.permissions = 1;"),
    ]
    killed = []
    for index, (filename, old, new) in enumerate(mutations):
        source = (work / filename).read_text()
        assert source.count(old) == 1, (filename, old)
        mutant = work / f"mutant-{index}"
        mutant.mkdir()
        for name in ("kernel_domains.c", "kernel_syscalls.c"):
            text = (work / name).read_text()
            (mutant / name).write_text(text.replace(old, new) if name == filename else text)
        binary = mutant / "host"
        # The mutant directory must precede the ordinary generated units.
        run(["gcc", "-I", mutant, *common, "-O2", host, work / "program_record_tasks.c",
             HERE / "tests/kernel_memory/worker_stubs.c", HERE / "tests/kernel_memory/record_stubs.c", "-o", binary])
        for name, _ in cases:
            result = subprocess.run([binary], input=(work / (name + ".input")).read_bytes(),
                                    capture_output=True, timeout=60)
            if result.returncode or result.stdout != expected[name]:
                killed.append(index)
                break
        else:
            raise AssertionError(("mutation survived", index, old))
    for name, digest in inputs.items():
        assert hashlib.sha256((HERE / name).read_bytes()).hexdigest() == digest, "inputs changed during test"
    total = sum(len(requests) for _, requests in cases)
    summary = {"requests": total, "scenarios": len(cases), "variants": checked, "mutations_detected": len(killed), "inputs": inputs,
               "cesta_sha256": hashlib.sha256(cesta.read_bytes()).hexdigest(),
               "lean_sources": {p.name: hashlib.sha256(p.read_bytes()).hexdigest() for p in model_dir.glob("*.lean")},
               "boundary": "actual exported syscall/ring code against Lean step; infallible page-sized host allocation/mapping, no CPU/device/shared-buffer proof"}
    (work / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")
    print(f"kernel model: {total} requests in {len(cases)} scenarios agree after every step; "
          f"all genesis domains/grants, GCC/Clang O0/O2 and sanitizers, {len(killed)} mutations detected")


if __name__ == "__main__":
    main()
