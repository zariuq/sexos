"""Qualify current sex-net frame-to-job ticket classification."""
import argparse
import hashlib
import importlib
import itertools
import json
import os
from pathlib import Path
import random
import subprocess
import sys
from layout_text import LayoutText

HERE = Path(__file__).resolve().parent.parent


def run(command, data=None, *, cwd=HERE, env=None):
    result = subprocess.run(list(map(str, command)), cwd=cwd, input=data, capture_output=True, env=env)
    assert result.returncode == 0, (command, result.returncode, result.stdout[-2000:], result.stderr[-8000:])
    return result.stdout


HOST = r'''
#include "network_engine_frame.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

static void put16(uint8_t *bytes, uint16_t value) { bytes[0] = (uint8_t)(value >> 8); bytes[1] = (uint8_t)value; }
static void put32(uint8_t *bytes, uint32_t value) {
    bytes[0] = (uint8_t)(value >> 24); bytes[1] = (uint8_t)(value >> 16);
    bytes[2] = (uint8_t)(value >> 8); bytes[3] = (uint8_t)value;
}
int main(void) {
    uint64_t mode, length, ethernet, version_header, protocol, source, port;
    while (scanf("%"SCNu64" %"SCNu64" %"SCNu64" %"SCNu64" %"SCNu64" %"SCNu64" %"SCNu64,
                 &mode, &length, &ethernet, &version_header, &protocol, &source, &port) == 7) {
        NetConfig config = net_config_controlled();
        if (mode == 1) config.prefix = 31;
        else if (mode == 2) config.local = 0;
        else if (mode == 3) config.source_base = 47999;
        NetworkEngineFrame frame; memset(&frame, 0xA5, sizeof(frame)); frame.length = (size_t)length;
        put16(&frame.bytes[12], (uint16_t)ethernet); frame.bytes[14] = (uint8_t)version_header;
        frame.bytes[23] = (uint8_t)protocol; put32(&frame.bytes[26], (uint32_t)source);
        size_t offset = 14 + ((size_t)(frame.bytes[14] & 15) * 4);
        if (offset + 1 < sizeof(frame.bytes)) put16(&frame.bytes[offset], (uint16_t)port);
        NetworkEngineTicket client = network_engine_client_ticket(&frame, config);
        NetworkEngineTicket dns = network_engine_dns_ticket(&frame, config);
        printf("C %u %u D %u %u\n", (unsigned)client.present, client.value, (unsigned)dns.present, dns.value);
    }
    return 0;
}
'''


def cases():
    rows = []
    lengths = [0, 13, 41, 42, 53, 54, 74, 1514, 1515, (1 << 64) - 1]
    headers = [0, 0x40, 0x44, 0x45, 0x46, 0x4F, 0x65, 0xFF]
    for row in itertools.product(range(4), lengths, [0, 0x0800, 0x0806, 0xFFFF], headers,
                                 [0, 6, 17, 255], [0, 0x0A00020F, 0x0A000202, 0xFFFFFFFF],
                                 [0, 47999, 48000, 48001, 65535]):
        if len(rows) % 29 == 0 or row[2] == 0x0800 and row[4] in (6, 17): rows.append(row)
    rng = random.Random(0x5345584E4554)
    for _ in range(20000):
        rows.append((rng.randrange(4), rng.getrandbits(64), rng.getrandbits(16), rng.getrandbits(8),
                     rng.getrandbits(8), rng.getrandbits(32), rng.getrandbits(16)))
    return rows


def expected(row):
    mode, length, ethernet, version_header, protocol, source, port = row
    port &= 0xFFFF; ethernet &= 0xFFFF; version_header &= 0xFF; protocol &= 0xFF; source &= 0xFFFFFFFF
    def classify(wanted, minimum, transport):
        if mode != 0 or length > 1514 or length < minimum: return (0, 0)
        if ethernet != 0x0800 or version_header >> 4 != 4 or protocol != wanted or source != 0x0A00020F: return (0, 0)
        header = (version_header & 15) * 4
        if header < 20 or length < 14 + header + transport or port <= 48000: return (0, 0)
        return (1, port - 48000)
    client = classify(6, 54, 20); dns = classify(17, 42, 8)
    return f"C {client[0]} {client[1]} D {dns[0]} {dns[1]}\n"


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cesta", type=Path, required=True)
    parser.add_argument("--cesta-source", type=Path)
    parser.add_argument("--lean-root", type=Path)
    parser.add_argument("--output", type=Path, default=HERE / "build/network-engine-frame-tests")
    args = parser.parse_args(); output = args.output.resolve(); output.mkdir(parents=True, exist_ok=True)

    inventory = json.loads((HERE / "interfaces/network_engine_frame.source.json").read_text())
    source = run(["git", "show", f"{inventory['pin']}:{inventory['path']}"], cwd=HERE.parent)
    tests = run(["git", "show", f"{inventory['pin']}:{inventory['tests_path']}"], cwd=HERE.parent)
    assert hashlib.sha256(source).hexdigest() == inventory["sha256"]
    assert hashlib.sha256(tests).hexdigest() == inventory["tests_sha256"]
    cargo = run(["cargo", "test", "-p", "sex-net", "--lib", "--", "--test-threads=1"], cwd=HERE.parent)
    assert b"54 passed" in cargo, cargo[-2000:]

    rows = cases(); commands = "".join(" ".join(map(str, row)) + "\n" for row in rows).encode()
    oracle = "".join(expected(row) for row in rows).encode(); (output / "commands.txt").write_bytes(commands)
    host = output / "host.c"; host.write_text(HOST)
    configurations = []; plans = {}; exports = {}
    for compiler in ("gcc", "clang"):
        config_c = output / f"network-config-{compiler}.c"
        frame_c = output / f"network-engine-frame-{compiler}.c"
        plan = output / f"network-engine-frame-{compiler}.plan"
        run([args.cesta, "export-c", "src/network_config.cesta", "--cc", compiler, "--std=c11", "-o", config_c, "--", "-Igenerated/include"])
        run([args.cesta, "export-c", "src/network_engine_frame.cesta", "--cc", compiler, "--std=c11", "-o", frame_c,
             "--plan", plan, "--", "-Igenerated/include"])
        plans[compiler] = plan; exports[compiler] = frame_c
        for mode, flags in (("O0", ["-O0"]), ("O2", ["-O2"]),
                            ("SAN", ["-O1", "-fsanitize=address,undefined", "-fno-sanitize-recover=all", "-fno-pie", "-no-pie"])):
            binary = output / f"{compiler}-{mode}"
            run([compiler, "-std=c11", "-Wall", "-Wextra", "-Wpedantic", "-Werror", *flags,
                 "-Igenerated/include", config_c, frame_c, host, "-o", binary])
            actual = run([binary], commands, env={**os.environ, "ASAN_OPTIONS": "detect_leaks=1:abort_on_error=1"})
            assert actual == oracle, (compiler, mode)
            configurations.append({"compiler": compiler, "mode": mode, "sha256": hashlib.sha256(actual).hexdigest()})

    base = LayoutText(exports["gcc"].read_text())
    mutations = [
        ("validate-config", "!net_config_valid(config)", "false"),
        ("maximum-frame", "frame->length>NETWORK_ENGINE_MAX_FRAME", "frame->length>=NETWORK_ENGINE_MAX_FRAME"),
        ("minimum-frame", "frame->length<minimum", "frame->length<=minimum"),
        ("ether-type", "frame->bytes[12]!=8", "frame->bytes[12]!=7"),
        ("ipv4-version", "frame->bytes[14]>>4!=4", "frame->bytes[14]>>4!=5"),
        ("local-source", "source!=config.local", "false"),
        ("header-bound", "header<20", "header<16"),
        ("transport-bound", "frame->length<14+header+transport", "frame->length<14+header"),
        ("ticket-zero", "port<=config.source_base", "port<config.source_base"),
    ]
    caught = []; config_c = output / "network-config-gcc.c"
    for name, old, new in mutations:
        assert base.count(old) == 1, (name, base.count(old))
        mutant = output / f"mutant-{name}.c"; mutant.write_text(base.replace(old, new)); binary = output / f"mutant-{name}"
        run(["gcc", "-std=c11", "-O2", "-Igenerated/include", config_c, mutant, host, "-o", binary])
        assert run([binary], commands) != oracle, ("survived", name); caught.append(name)

    admission = []
    if args.lean_root:
        assert args.cesta_source; sys.path.insert(0, str(args.cesta_source.resolve() / "tests"))
        helper = importlib.import_module("ownership_admission"); unique = {}
        for plan in plans.values(): unique[hashlib.sha256(plan.read_bytes()).hexdigest()] = plan
        for index, plan in enumerate(unique.values()):
            (record,) = helper.parse(plan.read_bytes())
            admission.append(helper.admit_record(record, output / f"EngineFrame{index}.lean", args.lean_root.resolve()))

    summary = {"pin": inventory["pin"], "upstream_rust_tests": 54, "commands": len(rows),
               "configurations": configurations, "mutants_caught": caught, "admission": admission}
    (output / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")
    print("network engine frame:", len(rows), "classifications; 54 upstream tests;", len(configurations), "C configurations;", len(caught), "mutants;", sum(x["functions"] for x in admission), "plans admitted")


if __name__ == "__main__":
    main()
