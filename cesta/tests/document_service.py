#!/usr/bin/env python3
"""Qualify the pinned authenticated document-session service core."""
import argparse
import hashlib
import json
import os
import subprocess
import sys
import tempfile
from pathlib import Path
from layout_text import LayoutText

HERE = Path(__file__).resolve().parent.parent
PIN = "44053ea9b017fe89008413f7cc8a6b1a4d28d015"
SOURCE_SHA256 = "365cbe553ed5c6e92190b0ae9b7255e99fd5a1ca35a3ff0a37a085184c454d2b"


def run(argv, *, env=None):
    result = subprocess.run(list(map(str, argv)), cwd=HERE, capture_output=True, env=env)
    if result.returncode:
        raise RuntimeError((argv, result.returncode, result.stdout[-1000:], result.stderr[-5000:]))
    return result.stdout


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cesta", type=Path, required=True)
    parser.add_argument("--work", type=Path)
    parser.add_argument("--mutations", action="store_true")
    parser.add_argument("--cesta-source", type=Path)
    parser.add_argument("--lean-root", type=Path)
    args = parser.parse_args()
    work = (args.work or Path(tempfile.mkdtemp(
        prefix="document-service-", dir=HERE / "build"
    ))).resolve()
    work.mkdir(parents=True, exist_ok=True)
    rust = subprocess.check_output(
        ["git", "show", PIN + ":servers/sexfiles/src/document.rs"], cwd=HERE.parent
    )
    assert hashlib.sha256(rust).hexdigest() == SOURCE_SHA256
    protocol = work / "document-protocol.c"
    run([args.cesta, "export-c", "src/document_protocol.cesta", "--std=c11", "-o", protocol,
         "--", "-Igenerated/include"])
    sources = {}; plans = {}; builds = []
    common = ["-std=c11", "-g0", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
              "-Igenerated/include"]
    for compiler in ("gcc", "clang"):
        source = work / f"document-service-{compiler}.c"
        plan = work / f"document-service-{compiler}.plan"
        run([args.cesta, "export-c", "src/document_service.cesta", "--cc", compiler,
             "--std=c11", "--plan", plan, "-o", source, "--", "-Igenerated/include"])
        sources[compiler] = source
        plans[hashlib.sha256(plan.read_bytes()).hexdigest()] = plan
        for mode, extra in (
            ("O0", ["-O0"]), ("O2", ["-O2"]),
            ("SAN", ["-O1", "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
                     "-fno-pie", "-no-pie"]),
        ):
            binary = work / f"{compiler}-{mode}"
            run([compiler, *common, *extra, "tests/document_service/host.c", protocol, source,
                 "-o", binary])
            run([binary], env={**os.environ, "ASAN_OPTIONS": "detect_leaks=1:abort_on_error=1"})
            builds.append((compiler, mode))

    mutations = []
    if args.mutations:
        base = LayoutText(sources["gcc"].read_text())
        changes = [
            ("caller-authority", "size_t index=document_target_index(caller);if(index==SIZE_MAX)return(DocumentServiceResult)", "size_t index=document_target_index(caller);if(false)return(DocumentServiceResult)"),
            ("live-authority", "session->dead||!live", "session->dead&&live"),
            ("duplicate-effect", "if(document_request_equal(session->last,request))", "if(false&&document_request_equal(session->last,request))"),
            ("sequence-monotone", "request.seq<=session->last.seq", "request.seq<session->last.seq"),
            ("delete-generation", "if(request.a!=generation)", "if(false)"),
            ("browser-route", "request.kind!=SEXOS_DOCUMENT_INFO&&", "false&&"),
            ("name-alphabet", "||c=='-'||c=='_'||c=='.'", "||true"),
            ("commit-hash", "sexos_document_hash(&session->payload)!=request.a", "false"),
            ("load-recheck", "after!=session->generation", "false"),
            ("retire-dead", "session->dead=true", "session->dead=false"),
        ]
        for name, old, new in changes:
            assert base.count(old) >= 1, (name, base.count(old))
            mutant = work / f"{name}.c"; mutant.write_text(base.replace(old, new, 1))
            binary = work / name
            built = subprocess.run(["gcc", *common, "-O2", "tests/document_service/host.c",
                                    protocol, mutant, "-o", binary], cwd=HERE, capture_output=True)
            if built.returncode == 0:
                result = subprocess.run([binary], cwd=HERE, capture_output=True)
                assert result.returncode != 0, ("survived", name, result.stdout, result.stderr)
            mutations.append(name)

    admission = []
    if args.lean_root:
        assert args.cesta_source
        sys.path.insert(0, str(args.cesta_source / "tests"))
        import ownership_admission
        for index, plan in enumerate(plans.values()):
            record, = ownership_admission.parse(plan.read_bytes())
            admission.append(ownership_admission.admit_record(
                record, work / f"DocumentServiceOwnership{index}.lean", args.lean_root
            ))
    (work / "summary.json").write_text(json.dumps({
        "pin": PIN, "rust_sha256": SOURCE_SHA256, "builds": builds,
        "mutations": mutations, "admission": admission,
    }, indent=2) + "\n")
    print("document service:", len(builds), "builds,", len(mutations), "mutants,",
          sum(item["functions"] for item in admission), "admitted")


if __name__ == "__main__":
    main()
