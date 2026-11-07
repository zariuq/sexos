#!/usr/bin/env python3
"""Compare the fixed-memory Kaleidoscope regex engine with pinned Rust."""
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
REPO = HERE.parent
PIN = "b6be1b451a6c21985d00d192674feedf66bb11dd"
SOURCE_SHA256 = "67075f4d19803445a01d05b518a27a21547f098a5a2a888b4c0d182b36ee4f2d"


def run(argv, *, env=None):
    result = subprocess.run(list(map(str, argv)), cwd=HERE, capture_output=True, env=env)
    if result.returncode:
        raise RuntimeError((argv, result.returncode, result.stdout[-1000:], result.stderr[-5000:]))
    return result.stdout


def cases():
    return [
        (b"b+", b"", b"aabbbc", 0), (b"^a.c$", b"", b"abc", 0),
        (b"^a.c$", b"", b"abcd", 0), (b"colou?r", b"", b"my color", 0),
        (rb"\d{2,3}", b"", b"a1234", 0), (rb"\d{2,3}?", b"", b"a1234", 0),
        (b"[^a-c ]+", b"", b"abc xyz", 0), (b"HELLO", b"i", b"say hello", 0),
        (b"[a-z]+", b"i", b"123ABC", 0), (b"cat|dog", b"", b"hotdog", 0),
        (rb"\bis\b", b"", b"this is it", 0), (b"a.*b", b"", b"axxbyyb", 0),
        (b"a.*?b", b"", b"axxbyyb", 0), (b"x*", b"", b"abc", 0),
        (rb"(\w+)@(\w+)\.com", b"", b"mail ann@sex.com now", 0),
        (b"(a)|(b)", b"", b"b", 0), (b"(?:ab)+(c)", b"", b"ababc", 0),
        (b"(a*)+b", b"", b"aab", 0), (rb"\D+", b"", b"12ab 34", 0),
        (rb"\s+", b"", b"a \t\nb", 0), (rb"a\B", b"", b"ab a", 0),
        (b"a.b", b"", b"a\nb", 0), (rb"\w+", b"", b"_x", 0),
        (b"z", b"", b"zz", 1), (b"", b"", b"abc", 2),
        (b"(", b"", b"x", 0), (b"a)", b"", b"x", 0),
        (b"*a", b"", b"x", 0), (b"[a-", b"", b"x", 0),
        (b"a{3,2}", b"", b"aaa", 0), (b"[z-a]", b"", b"x", 0),
        (b"a{1000}", b"", b"a", 0), (b"a", b"x", b"a", 0),
        (b"(a*)*b", b"", b"a" * 40, 0),
    ]


def rust_bytes(data): return "&[" + ",".join(map(str, data)) + "]"


def c_array(data): return "{" + (",".join(map(str, data)) if data else "0") + "}"


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cesta", type=Path, required=True)
    parser.add_argument("--work", type=Path)
    parser.add_argument("--mutations", action="store_true")
    parser.add_argument("--cesta-source", type=Path)
    parser.add_argument("--lean-root", type=Path)
    args = parser.parse_args()
    work = (args.work or Path(tempfile.mkdtemp(prefix="kaleidoscope-regex-", dir=HERE / "build"))).resolve()
    work.mkdir(parents=True, exist_ok=True)
    source = subprocess.check_output(["git", "show", PIN + ":apps/kaleidoscope/src/regex.rs"], cwd=REPO)
    assert hashlib.sha256(source).hexdigest() == SOURCE_SHA256
    (work / "regex.rs").write_bytes(source)
    selected = cases()
    rust_rows = ",\n".join("(" + ",".join((rust_bytes(p), rust_bytes(f), rust_bytes(t), str(start))) + ")"
                              for p, f, t, start in selected)
    (work / "reference.rs").write_text(
        "extern crate alloc;mod regex;use regex::{Regex,Error};\n"
        "static CASES:&[(&[u8],&[u8],&[u8],usize)]=&[" + rust_rows + "];\n"
        "fn e(x:Error)->u8{match x{Error::Syntax=>1,Error::Limit=>2}}\n"
        "fn main(){let n:usize=std::env::args().nth(1).unwrap().parse().unwrap();let(p,f,t,s)=CASES[n];"
        "match Regex::new(p,f){Err(x)=>print!(\"C {}\",e(x)),Ok(r)=>match r.find(t,s){Err(x)=>print!(\"C 0 F {}\",e(x)),"
        "Ok(None)=>print!(\"C 0 F 0 0 {}\",r.groups),Ok(Some(c))=>{print!(\"C 0 F 0 1 {}\",r.groups);"
        "for x in c.iter().take(r.groups+1){match x{Some((a,b))=>print!(\" {}-{}\",a,b),None=>print!(\" -\")}}}}}}\n"
    )
    reference = work / "reference"
    run(["rustc", "--edition=2021", "-C", "opt-level=2", work / "reference.rs", "-o", reference])
    expected = [run([reference, i]) for i in range(len(selected))]
    rows = []
    for pattern, flags, text, start in selected:
        rows.append("{" + c_array(pattern) + f",{len(pattern)}," + c_array(flags) + f",{len(flags)}," +
                    c_array(text) + f",{len(text)},{start}" + "}")
    (work / "cases.h").write_text(
        "typedef struct Case{uint8_t pattern[256];size_t pattern_len;uint8_t flags[8];size_t flags_len;uint8_t text[4096];size_t text_len,from;}Case;\n"
        "static const Case cases[]={" + ",\n".join(rows) + "};\n")
    (work / "host.c").write_text(
        "#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>\n#include \"kaleidoscope_regex.h\"\n#include \"cases.h\"\n"
        "int main(int argc,char **argv){if(argc!=2)return 2;size_t n=strtoull(argv[1],0,10);if(n>=sizeof cases/sizeof *cases)return 2;"
        "KaleidoscopeRegexSource s={0};KaleidoscopeRegexText t={0};KaleidoscopeRegex r;memcpy(s.pattern,cases[n].pattern,cases[n].pattern_len);s.pattern_len=cases[n].pattern_len;"
        "memcpy(s.flags,cases[n].flags,cases[n].flags_len);s.flags_len=cases[n].flags_len;memcpy(t.bytes,cases[n].text,cases[n].text_len);t.len=cases[n].text_len;"
        "unsigned e=kaleidoscope_regex_compile(&s,&r);if(e){printf(\"C %u\",e);return 0;}KaleidoscopeRegexFind f=kaleidoscope_regex_find(&r,&t,cases[n].from);"
        "if(f.error){printf(\"C 0 F %u\",f.error);return 0;}printf(\"C 0 F 0 %u %zu\",(unsigned)f.matched,f.groups);if(f.matched)for(size_t i=0;i<=f.groups;i++){if(f.captures[i].present)printf(\" %zu-%zu\",f.captures[i].start,f.captures[i].end);else printf(\" -\");}return 0;}\n")
    common = ["-std=c11", "-g0", "-Wall", "-Wextra", "-Wpedantic", "-Werror", "-Igenerated/include", "-I", work]
    exported = {}; plans = {}
    builds = []
    for compiler in ("gcc", "clang"):
        compile_source = work / f"regex-compile-{compiler}.c"
        match_source = work / f"regex-match-{compiler}.c"
        plan = work / f"regex-match-{compiler}.plan"
        run([args.cesta, "export-c", "src/kaleidoscope_regex.cesta", "--cc", compiler, "--std=c11", "-o", compile_source, "--", "-Igenerated/include"])
        run([args.cesta, "export-c", "src/kaleidoscope_regex_match.cesta", "--cc", compiler, "--std=c11", "--plan", plan, "-o", match_source, "--", "-Igenerated/include"])
        plans[hashlib.sha256(plan.read_bytes()).hexdigest()] = plan
        csource = work / f"regex-{compiler}.c"
        csource.write_bytes(compile_source.read_bytes() + match_source.read_bytes())
        exported[compiler] = csource
        for mode, extra in (("O0", ["-O0"]), ("O2", ["-O2"]),
            ("SAN", ["-O1", "-fsanitize=address,undefined", "-fno-sanitize-recover=all", "-fno-pie", "-no-pie"])):
            binary = work / f"{compiler}-{mode}"; run([compiler, *common, *extra, work / "host.c", csource, "-o", binary])
            for i, wanted in enumerate(expected):
                actual = run([binary, i], env={**os.environ, "ASAN_OPTIONS": "detect_leaks=1:abort_on_error=1"})
                assert actual == wanted, (compiler, mode, i, selected[i], actual, wanted)
            builds.append((compiler, mode))
    mutations = []
    if args.mutations:
        base = LayoutText(exported["gcc"].read_text())
        changes = [
            ("step-budget", "RE_STEPS UINT32_C(200000)", "RE_STEPS UINT32_C(20)"),
            ("any-newline", "text->bytes[pos]!='\\n'", "true"),
            ("case-fold", "regex->icase?re_match_lower(text->bytes[pos])==re_match_lower(in.byte)", "false"),
            ("word-underscore", "||b=='_'", ""),
            ("lazy-order", "n.lazy?out:(uint16_t)(split+1)", "n.lazy?(uint16_t)(split+1):out"),
            ("progress", "pos!=marks[in.a]", "true"),
        ]
        for name, old, new in changes:
            assert base.count(old) >= 1, (name, base.count(old))
            mutant = work / f"{name}.c"; mutant.write_text(base.replace(old, new, 1)); binary = work / name
            built = subprocess.run(["gcc", *common, "-O2", work / "host.c", mutant, "-o", binary], cwd=HERE, capture_output=True)
            if built.returncode == 0:
                assert any(subprocess.run([binary, str(i)], cwd=HERE, capture_output=True).stdout != wanted
                           for i, wanted in enumerate(expected)), ("survived", name)
            mutations.append(name)
    admission = []
    if args.lean_root:
        assert args.cesta_source
        sys.path.insert(0, str(args.cesta_source / "tests"))
        import ownership_admission
        for index, plan in enumerate(plans.values()):
            record, = ownership_admission.parse(plan.read_bytes())
            admission.append(ownership_admission.admit_record(
                record, work / f"KaleidoscopeRegexOwnership{index}.lean", args.lean_root
            ))
    (work / "summary.json").write_text(json.dumps({"pin": PIN, "source_sha256": SOURCE_SHA256,
        "cases": len(selected), "builds": builds, "mutations": mutations,
        "admission": admission}, indent=2) + "\n")
    print("kaleidoscope regex:", len(selected), "Rust cases,", len(builds), "builds,", len(mutations), "mutants,",
          sum(item["functions"] for item in admission), "admitted")


if __name__ == "__main__": main()
