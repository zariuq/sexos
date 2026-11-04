"""Check the durable wrapper and admit its two permission projections."""

import argparse
import hashlib
import importlib
import json
from pathlib import Path
import subprocess
import sys

HERE = Path(__file__).resolve().parent.parent

PROBE = r'''
#include <stdlib.h>
#include "DURABLE_IMPLEMENTATION"
void sexos_line(const char *format, ...) { (void)format; }
void sexos_fail(void) { exit(77); }
int main(int argc, char **argv) {
    if (argc != 3) return 2;
    size_t offset = (size_t)strtoull(argv[1], NULL, 10);
    Durable durable = sexstore_durable_new();
    DurablePage page = {{0}};
    if (argv[2][0] == 'w') return page_write(&durable, offset, &page) ? 0 : 3;
    page_read(&durable, offset, &page);
    return 0;
}
'''


def run(command, accepted=True, needle=None, **kwargs):
    result = subprocess.run(list(map(str, command)), capture_output=True, text=True, **kwargs)
    assert (result.returncode == 0) == accepted, (command, result.stdout[-3000:], result.stderr[-6000:])
    if needle:
        assert "ownership" in result.stderr and needle in result.stderr, result.stderr
    return result


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--cesta", default="cesta")
    p.add_argument("--cesta-source", type=Path)
    p.add_argument("--lean-root", type=Path)
    p.add_argument("--output", type=Path, default=HERE / "build/sexstore-ownership")
    args = p.parse_args()
    work = args.output.resolve()
    work.mkdir(parents=True, exist_ok=True)
    source = HERE / "src/sexstore_durable.cesta"
    include = "-I" + str(HERE / "generated/include")
    plans = []
    codec_c = work / "codec.c"
    run([args.cesta, "export-c", HERE / "src/sexstore_pages.cesta", "-o", codec_c])

    def probe(compiler, implementation, label):
        source = work / (label + ".c")
        source.write_text(PROBE.replace("DURABLE_IMPLEMENTATION", implementation.name))
        binary = work / label
        run([compiler, "-std=c11", "-O2", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
             "-fsanitize=address,undefined", "-fno-sanitize-recover=all", include,
             source, codec_c, "-o", binary])
        return binary

    def check_bounds(binary):
        for offset in (0, 512, 1, 513, (1 << 64)-1):
            for mode in ("r", "w"):
                invalid = offset not in (0, 512)
                result = run([binary, offset, mode], not invalid)
                assert result.returncode == (77 if invalid else 0), (offset, mode, result.returncode)

    for compiler in ("gcc", "clang"):
        exported, plan = work / (compiler + ".c"), work / (compiler + ".plan")
        run([args.cesta, "export-c", source, "--cc", compiler, "--std=c11", "-o", exported, "--plan", plan, "--", include])
        run([compiler, "-std=c11", "-Wall", "-Wextra", "-Wpedantic", "-Werror", "-fsyntax-only", include, exported])
        plans.append({"compiler": compiler, "sha256": hashlib.sha256(plan.read_bytes()).hexdigest(), "plan": plan.name})
        check_bounds(probe(compiler, exported, "bounds-"+compiler))
    original = source.read_text()
    before = "(return (compound cesta_read_bytes (= (. data) (-> page bytes)) (= (. len) DURABLE_PAGE_SIZE)))"
    mutations = [
        (before, before.replace("DURABLE_PAGE_SIZE", "(+ DURABLE_PAGE_SIZE 1)"), "exceeds"),
        (before, "(block (var DurablePage escaped (init 0)) (return (compound cesta_read_bytes (= (. data) (. escaped bytes)) (= (. len) DURABLE_PAGE_SIZE))))", "escapes"),
        ("(call sexstore_pages_page_begin out seq)", "(block (= (. out len) 1) (call sexstore_pages_page_begin out seq))", "independently"),
        ("(call sexstore_pages_page_begin out seq)", "(block (var cesta_read_bytes alias (compound cesta_read_bytes (. page bytes) DURABLE_PAGE_SIZE)) (call sexstore_pages_page_begin out seq))", "overlapping"),
    ]
    for index, (old, new, needle) in enumerate(mutations):
        assert original.count(old) == 1, old
        mutant = work / f"mutant-{index}.cesta"
        mutant.write_text(original.replace(old, new))
        run([args.cesta, "cc", "--cc", "gcc", "-std=c11", "-fsyntax-only", include, mutant], False, needle)
    guard = '(if (! (|| (== page_offset DURABLE_PAGE_A_OFFSET) (== page_offset DURABLE_PAGE_B_OFFSET)))\n        (call sexos_fail))'
    assert original.count(guard) == 2
    mutant = work / "missing-page-guard.cesta"
    mutant.write_text(original.replace(guard, "", 1))
    exported = work / "missing-page-guard.c"
    run([args.cesta, "export-c", mutant, "--std=c11", "-o", exported, "--", include])
    mutant_binary = probe("gcc", exported, "missing-guard-probe")
    try:
        check_bounds(mutant_binary)
    except AssertionError:
        pass
    else:
        raise AssertionError("page bounds gate accepted the missing guard")
    admission = []
    byte_mutants = 0
    if args.lean_root:
        if not args.cesta_source:
            p.error("--lean-root requires --cesta-source")
        sys.path.insert(0, str(args.cesta_source.resolve() / "tests"))
        native = importlib.import_module("ownership_admission")
        byte = importlib.import_module("checked_admission")
        for digest in sorted({plan["sha256"] for plan in plans}):
            item = next(plan for plan in plans if plan["sha256"] == digest)
            record, = native.parse((work / item["plan"]).read_bytes())
            admission.append(native.admit_record(record, work / f"Durable{len(admission)}.lean", args.lean_root.resolve()))
        codec = HERE / "src/sexstore_pages.cesta"
        recorded = run([args.cesta, "checked", "plan", codec]).stdout
        (work / "codec.plan").write_text(recorded)
        record, = byte.parse(recorded.encode())
        lines = ["import Mettapedia.GSLT.LanguageDef.CheckedByteModules",
            "set_option maxRecDepth 200000", "set_option maxHeartbeats 0",
            "open Mettapedia.GSLT.LanguageDef.CheckedBytes", "def codec : Module := " + byte.module(record),
            "theorem actual_codec_admitted : admit codec = true := by decide +kernel",
            "#print axioms actual_codec_admitted"]
        for _, changed in byte.mutations(record):
            lines.append("example : admit ("+byte.module(changed)+" : Module) = false := by decide +kernel")
            byte_mutants += 1
        proof = work / "PageCodec.lean"
        proof.write_text("\n".join(lines)+"\n")
        checked = run(["lake", "env", "lean", proof], cwd=args.lean_root.resolve())
        (work / "PageCodec.log").write_text(checked.stdout)
        # The reusable serial/publication laws are built as part of this
        # qualification, not counted as a proof of the native byte writer.
        checked = run(["lake", "build", "Mettapedia.Machines.Storage.VersionedPair"], cwd=args.lean_root.resolve())
        (work / "StorageModel.log").write_text(checked.stdout)
    (work / "result.json").write_text(json.dumps({"plans": plans, "source_mutants_refused": len(mutations),
        "native_admission": admission, "codec_mutants_refused": byte_mutants,
        "page_bounds_executions": 20, "missing_guard_mutation_detected": True}, indent=2)+"\n")
    print(f"sexstore ownership: two compiler plans; {len(mutations)} source mutants refused; "
          f"{sum(item['functions'] for item in admission)} native function plans admitted; {byte_mutants} codec mutations refused; 20 page-bound executions and missing-guard control")


if __name__ == "__main__":
    main()
