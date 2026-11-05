"""Qualify Bell's actual ownership envelope and its emitted permission plans."""

import argparse
import hashlib
import importlib
from itertools import combinations
import json
from pathlib import Path
import subprocess
import sys
from layout_text import LayoutText

HERE = Path(__file__).resolve().parent.parent
FLAGS = ["SEXOS_BELL_DELIVERY_PROOF","SEXOS_BELL_BRIDGE_STUB_PROOF","SEXOS_BELL_LANE_RENAME_PROOF"]
VARIANTS = [c for n in range(len(FLAGS)+1) for c in combinations(FLAGS,n)]


def run(command, accepted=True, needle=None):
    result = subprocess.run(list(map(str,command)),cwd=HERE,capture_output=True,text=True)
    assert (result.returncode == 0) == accepted,(command,result.stdout,result.stderr)
    if needle:
        assert "ownership" in result.stderr and needle in result.stderr,(needle,result.stderr)
    return result


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cesta",default="cesta")
    parser.add_argument("--cesta-source",type=Path)
    parser.add_argument("--lean-root",type=Path)
    parser.add_argument("--output",type=Path,default=HERE/"build/sexbell-ownership")
    args = parser.parse_args()
    args.output.mkdir(parents=True,exist_ok=True)
    source = HERE/"src/sexbell.cesta"
    include = HERE/"generated/include"
    records = []
    for compiler in ("gcc","clang"):
        for index, variant in enumerate(VARIANTS):
            plan = args.output/f"{compiler}-{index}.plan"
            exported = args.output/f"{compiler}-{index}.c"
            flags = ["-I"+str(include),*("-D"+f for f in variant)]
            run([args.cesta,"export-c",source,"--cc",compiler,"--std=c11","--plan",plan,"-o",exported,"--",*flags])
            run([compiler,"-std=c11","-Wall","-Wextra","-Wpedantic","-Werror","-fsyntax-only",*flags,exported])
            records.append({"compiler":compiler,"flags":list(variant),"plan":str(plan.relative_to(args.output)),
                "sha256":hashlib.sha256(plan.read_bytes()).hexdigest()})
    original = LayoutText(source.read_text())
    mutations = [
        ("(call bell_queue_find_lowest_priority_index q (& drop_idx))",
         "(call bell_queue_find_lowest_priority_index q (& (-> q tail)))", "overlapping loan"),
        ("(return (& ([] (-> s policy_table) i)))",
         "(block (var PolicyEntry escaped (compound PolicyEntry 0)) (return (& escaped)))", "escapes"),
        ('(= (* downgrade) (?: (== urgency_hint 0) NULL "no_caps_untrusted"))',
         "(var char transient 0) (= (* downgrade) (& transient))", "non-static"),
        ("(function find_policy (returns-borrow 0 policy_table nullable))",
         "(function find_policy (returns-static nullable))", "static lifetime"),
    ]
    for index,(before,after,needle) in enumerate(mutations):
        assert original.count(before) == 1,before
        mutant = args.output/f"mutant-{index}.cesta"
        mutant.write_text(original.replace(before,after))
        run([args.cesta,"cc","--cc","gcc","-std=c11","-fsyntax-only","-I"+str(include),mutant],False,needle)
    admission = []
    if args.lean_root:
        if not args.cesta_source:
            parser.error("--lean-root requires --cesta-source for the shared ownership-plan reader")
        sys.path.insert(0,str(args.cesta_source.resolve()/"tests"))
        checker = importlib.import_module("ownership_admission")
        for index, digest in enumerate(sorted({r["sha256"] for r in records})):
            record = next(r for r in records if r["sha256"] == digest)
            forms = checker.parse((args.output/record["plan"]).read_bytes())
            assert len(forms) == 1
            proof = args.output/f"BellAdmission{index}.lean"
            admission.append(checker.admit_record(forms[0],proof,args.lean_root.resolve()))
    summary = {"source_sha256":hashlib.sha256(source.read_bytes()).hexdigest(),"configurations":records,
        "source_mutants_refused":len(mutations),"unique_plans":len({r["sha256"] for r in records}),"admission":admission}
    (args.output/"result.json").write_text(json.dumps(summary,indent=2)+"\n")
    print(f"sexbell ownership: {len(records)} compiler/configuration plans; {len(mutations)} source mutations refused; {summary['unique_plans']} distinct plan(s); {sum(r['functions'] for r in admission)} function plans kernel-admitted")


if __name__ == "__main__":
    main()
