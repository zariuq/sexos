#!/usr/bin/env python3
"""Qualify open IPv4 flows joined to exact provider ownership."""
import argparse
import json
import os
from pathlib import Path
import subprocess
import tempfile
from layout_text import LayoutText

HERE = Path(__file__).resolve().parent.parent
UNITS = ("network_config", "network_engine_frame", "network_engine_packet", "network_engine_tcp_packet",
         "network_tcp_client", "network_engine_neighbor", "network_dns_codec", "network_engine_dns",
         "network_open", "network_web", "network_web_open")


def run(argv, *, env=None):
    result = subprocess.run(list(map(str, argv)), cwd=HERE, capture_output=True, env=env)
    if result.returncode:
        raise RuntimeError((argv, result.returncode, result.stdout[-2000:], result.stderr[-6000:]))
    return result.stdout


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cesta", type=Path, required=True)
    parser.add_argument("--work", type=Path)
    parser.add_argument("--mutations", action="store_true")
    args = parser.parse_args()
    work = (args.work or Path(tempfile.mkdtemp(prefix="network-open-", dir=HERE / "build"))).resolve()
    work.mkdir(parents=True, exist_ok=True)
    for unit in ("network_open", "network_web", "network_web_open"):
        (work / f"{unit}.h").write_bytes(run([args.cesta, "export-c", f"src/{unit}_interface.cesta", "--"]))
    sources = {}
    for unit in UNITS:
        source = work / f"{unit}.c"
        source.write_bytes(run([args.cesta, "export-c", f"src/{unit}.cesta", "--std=c11", "--", "-Igenerated/include"]))
        sources[unit] = source
    common = ["-std=c11", "-g0", "-DSEXOS_HOST", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
              "-I" + str(work), "-Igenerated/include"]

    def inputs(replacement=None):
        return [replacement[1] if replacement and unit == replacement[0] else source
                for unit, source in sources.items()]

    builds = []
    for compiler in ("gcc", "clang"):
        for mode, extra in (("O0", ["-O0"]), ("O2", ["-O2"]),
                            ("SAN", ["-O1", "-fsanitize=address,undefined", "-fno-sanitize-recover=all", "-fno-pie", "-no-pie"])):
            binary = work / f"{compiler}-{mode}"
            run([compiler, *common, *extra, *inputs(), "runtime/server_runtime.c", "tests/network_open/host.c", "-o", binary])
            output = run([binary], env={**os.environ, "ASAN_OPTIONS": "detect_leaks=1:abort_on_error=1"})
            builds.append({"compiler": compiler, "mode": mode, "output": output.decode().strip()})
    mutations = []
    if args.mutations:
        changes = [
            ("pending_opens_early", "network_open",
             "if (flow->pending || flow->client.state == NETWORK_TCP_SYN_SENT)",
             "if (false && flow->pending)"),
            ("send_skips_owner", "network_web_open",
             "if (index == NETWORK_WEB_NO_INDEX)\n        return (NetworkTcpClientCount){false, 0, NETWORK_TCP_CLIENT_STATE};",
             "if (index != NETWORK_WEB_NO_INDEX)\n        return (NetworkTcpClientCount){false, 0, NETWORK_TCP_CLIENT_STATE};"),
            ("close_skips_engine", "network_web_open",
             "bool network_web_open_close(NetworkWebOpen *service, NetworkJobIdentity owner, uint8_t number) {\n    size_t index = network_web_flow_find(&service->owners, owner, number);\n    if (index == NETWORK_WEB_NO_INDEX)\n        return false;\n    network_open_close(&service->engine, index);\n    return network_web_flow_release(&service->owners, owner, number);\n}",
             "bool network_web_open_close(NetworkWebOpen *service, NetworkJobIdentity owner, uint8_t number) {\n    size_t index = network_web_flow_find(&service->owners, owner, number);\n    if (index == NETWORK_WEB_NO_INDEX) {\n        return false;\n    }\n    (void)network_open_state(&service->engine, index);\n    return network_web_flow_release(&service->owners, owner, number);\n}"),
            ("rollback_leaks_metadata", "network_web_open",
             "if (index == NETWORK_OPEN_NO_INDEX) {\n            (void)network_web_flow_release(&service->owners, owner, number);",
             "if (index == NETWORK_OPEN_NO_INDEX) {\n            (void)0;"),
            ("ingress_only_first", "network_open",
             "NetworkEngineFrame input = engine->base.rx;\n    for (size_t i = 0; i < engine->count; ++i) {",
             "NetworkEngineFrame input = engine->base.rx;\n    for (size_t i = 0; i < 1 && i < engine->count; ++i) {"),
        ]
        for name, unit, before, after in changes:
            original = LayoutText(sources[unit].read_text())
            assert original.count(before) == 1, (name, original.count(before))
            mutant = work / f"{name}.c"
            mutant.write_text(original.replace(before, after, 1))
            binary = work / name
            run(["gcc", *common, "-O2", *inputs((unit, mutant)), "runtime/server_runtime.c",
                 "tests/network_open/host.c", "-o", binary])
            result = subprocess.run([binary], cwd=HERE, capture_output=True)
            assert result.returncode != 0, ("mutation survived", name)
            mutations.append(name)
    summary = {
        "builds": builds,
        "mutations": mutations,
        "scope": "actual open IPv4 TCP flows joined to exact domain/task/flow ownership; two simultaneous connections, independent frames, final reads, close/reuse and allocation rollback; static IPv4 profile only, no DHCP/IPv6/UDP/TLS/browser claim",
    }
    (work / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")
    print("network open qualification:", len(builds), "routes;", len(mutations), "mutants")


if __name__ == "__main__":
    main()
