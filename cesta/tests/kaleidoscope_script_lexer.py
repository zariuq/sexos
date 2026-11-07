#!/usr/bin/env python3
"""Compare the checked Cesta script lexer with upstream Machine::next."""
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
SOURCE_SHA256 = "1a1a612fab2d9ac343485abeee712cc9a22b414972cc78aa241ca3d590949e35"


def run(argv, *, env=None):
    result = subprocess.run(list(map(str, argv)), cwd=HERE, capture_output=True, env=env)
    if result.returncode:
        raise RuntimeError((argv, result.returncode, result.stdout[-1000:], result.stderr[-5000:]))
    return result.stdout


def cases():
    return [
        b"document.write(2+2, 'x'); // tail\nvar n = 0x2a;",
        rb"var r=/a[b\/]c+/gi; x=x/2; return /x/; typeof /z/; case /c/:",
        b"var t=`hello ${name}`; var s=\"tab\\tq\";",
        b"a===b !== c == d != e <= f >= g &&= h ||= i ??= j **= k >>> l ... m => n ++ -- += -= *= /= %= && || ?? ** |= &= ^= << >> ?.x",
        b"0 .5 0.25 1e21 1e-7 1E+3 0xFF 0X1a 123456789000",
        b"a?.b ? .5 : c; a?.[x]", b"a?.5",
        b"\n\t/* block\ncomment */\nidentifier_$9",
        rb"/[/]/ /[a-z_\d]+/i /a\/b/",
        b"'unterminated", b"/* unterminated", b"/unterminated", b"@",
        b"0xgg", b"1e+", b"/a\nb/",
        # Since b6be1b45: nested templates, private names, exact decimal literals.
        b"`a${`b${c}d`}e` `${'}'}` `${\"`\"}` `x\\`y` `${ {a: 1}.a }`",
        b"`${", b"`${`", b"`a${'b}`", b"#x this.#y #1 # z",
        b"1e-300 5e-324 2.4703282292062327e-324 2.4703282292062328e-324 1e309 1.7976931348623157e308 1.7976931348623159e308",
        b"9007199254740993 0.1 0.30000000000000004 123456789012345678901234567890 .5e-3 1. 00012 0.000001e6",
        b"0." + b"0" * 300 + b"1e300 " + b"1" + b"0" * 400 + b"e-400 3.14159265358979323846264338327950288419716939937510",
        b"2.2250738585072011e-308 2.2250738585072012e-308 4.9406564584124654e-324 1e-400 7e22 1e23 8.98846567431158e307",
    ]


def byte_slice(data):
    return "&[" + ",".join(map(str, data)) + "]"


def c_array(data):
    return "{" + (",".join(map(str, data)) if data else "0") + "}"


RUST_PREFIX = r'''
#[derive(Clone,Copy,Debug,PartialEq)]
enum Tok<'a>{Num(f64),Str(&'a[u8]),Template(&'a[u8]),Regex(&'a[u8],&'a[u8]),Ident(&'a[u8]),Punct(u8),End}
#[derive(Clone,Copy,Debug,PartialEq,Eq)]
enum Error{Unsupported,Syntax,Limit,Thrown}
struct Machine<'a>{src:&'a[u8],at:usize,tok:Tok<'a>,nl:bool}
fn ident_start(b:u8)->bool{b.is_ascii_alphabetic()||b==b'_'||b==b'$'}
impl<'a> Machine<'a>{
'''

RUST_SUFFIX = r'''
}
fn hex(x:&[u8]){for b in x{print!("{:02x}",b)}}
fn err(e:Error)->u8{match e{Error::Unsupported=>1,Error::Syntax=>2,Error::Limit=>3,Error::Thrown=>4}}
fn main(){
 let n:usize=std::env::args().nth(1).unwrap().parse().unwrap();
 let source=CASES[n];let mut m=Machine{src:source,at:0,tok:Tok::End,nl:false};
 loop{match m.next(){
  Err(e)=>{println!("E {}",err(e));break},
  Ok(())=>{print!("T {} ",m.nl as u8);match m.tok{
   Tok::End=>{println!("0");break},Tok::Num(x)=>println!("1 {:016x}",x.to_bits()),
   Tok::Str(s)=>{print!("2 ");hex(s);println!()},Tok::Template(s)=>{print!("3 ");hex(s);println!()},
   Tok::Regex(p,f)=>{print!("4 ");hex(p);print!(" /");hex(f);println!()},
   Tok::Ident(s)=>{print!("5 ");hex(s);println!()},Tok::Punct(p)=>println!("6 {}",p)
  }}
 }}
}
'''


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cesta", type=Path, required=True)
    parser.add_argument("--work", type=Path)
    parser.add_argument("--mutations", action="store_true")
    parser.add_argument("--cesta-source", type=Path)
    parser.add_argument("--lean-root", type=Path)
    args = parser.parse_args()
    work = (args.work or Path(tempfile.mkdtemp(
        prefix="kaleidoscope-script-lexer-", dir=HERE / "build"
    ))).resolve()
    work.mkdir(parents=True, exist_ok=True)
    raw = subprocess.check_output(
        ["git", "show", PIN + ":apps/kaleidoscope/src/script.rs"], cwd=REPO
    )
    assert hashlib.sha256(raw).hexdigest() == SOURCE_SHA256
    upstream = raw.decode()
    start = upstream.index("    fn next(&mut self)")
    end = upstream.index("    fn expect(&mut self", start)
    scanners = upstream[upstream.index("fn template_end("):upstream.index("/// JavaScript ToInt32")]
    selected = cases()
    rows = ",\n".join(byte_slice(case) for case in selected)
    suffix = RUST_SUFFIX.replace("fn main()", "static CASES:&[&[u8]]=&[" + rows + "];\nfn main()")
    (work / "reference.rs").write_text(RUST_PREFIX + upstream[start:end] + suffix + scanners)
    reference = work / "reference"
    run(["rustc", "--edition=2021", "-C", "opt-level=2", work / "reference.rs", "-o", reference])
    expected = [run([reference, i]) for i in range(len(selected))]
    c_rows = ["{" + c_array(case) + f",{len(case)}" + "}" for case in selected]
    (work / "cases.h").write_text(
        "typedef struct Case{uint8_t bytes[4096];size_t len;}Case;\n"
        "static const Case cases[]={" + ",\n".join(c_rows) + "};\n"
    )
    common = ["-std=c11", "-g0", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
              "-Igenerated/include", "-I", work]
    sources = {}; plans = {}; builds = []
    host = HERE / "tests/kaleidoscope_script_lexer/host.c"
    decimals = {}
    for compiler in ("gcc", "clang"):
        decimal = work / f"decimal-{compiler}.c"
        run([args.cesta, "export-c", "src/kaleidoscope_decimal.cesta", "--cc", compiler,
             "--std=c11", "-o", decimal, "--", "-Igenerated/include"])
        decimals[compiler] = decimal
        source = work / f"lexer-{compiler}.c"
        plan = work / f"lexer-{compiler}.plan"
        run([args.cesta, "export-c", "src/kaleidoscope_script_lexer.cesta", "--cc", compiler,
             "--std=c11", "--plan", plan, "-o", source, "--", "-Igenerated/include"])
        sources[compiler] = source
        plans[hashlib.sha256(plan.read_bytes()).hexdigest()] = plan
        for mode, extra in (
            ("O0", ["-O0"]), ("O2", ["-O2"]),
            ("SAN", ["-O1", "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
                     "-fno-pie", "-no-pie"]),
        ):
            binary = work / f"{compiler}-{mode}"
            run([compiler, *common, *extra, host, source, decimals[compiler], "-o", binary])
            for i, wanted in enumerate(expected):
                actual = run([binary, i], env={**os.environ,
                    "ASAN_OPTIONS": "detect_leaks=1:abort_on_error=1"})
                assert actual == wanted, (compiler, mode, i, selected[i], actual, wanted)
            builds.append((compiler, mode))
    mutations = []
    if args.mutations:
        base = LayoutText(sources["gcc"].read_text())
        changes = [
            ("regex-context", "UINT64_C(0x6e7275746572)", "UINT64_C(0)"),
            ("regex-class", "else if(source->bytes[i]=='[')in_class=true", "else if(false)in_class=true"),
            ("block-comment", "script_two(source,i,'*','/')", "false"),
            ("optional-digit", "!script_digit(source->bytes[state->at+2])", "true"),
            ("newline", "newline|=source->bytes[state->at]=='\\n'", "newline=false"),
            ("triple-equals", "SCRIPT_OP3('=','=','=', 'E')", "SCRIPT_OP3('=','=','=', 'e')"),
            ("hex", "value=value*16.0+d", "value=value*10.0+d"),
            ("template-nesting", "uint8_t found=script_substitution_end(source,i+2,nesting+1,&i);", "uint8_t found=SCRIPT_SCAN_FOUND;i+=2;"),
            ("substitution-string", "}else if(b=='\\''||b=='\"'){", "}else if(false){"),
            ("private-name", "(c=='#'&&state->at+1<source->len", "(false&&state->at+1<source->len"),
            ("decimal-fraction", "kaleidoscope_decimal_digit(&decimal,(uint8_t)(source->bytes[at]-'0'),true);", "kaleidoscope_decimal_digit(&decimal,(uint8_t)(source->bytes[at]-'0'),false);"),
        ]
        for name, old, new in changes:
            assert base.count(old) >= 1, (name, base.count(old))
            mutant = work / f"{name}.c"
            mutant.write_text(base.replace(old, new, 1))
            binary = work / name
            built = subprocess.run(["gcc", *common, "-O2", host, mutant, decimals["gcc"], "-o", binary],
                                   cwd=HERE, capture_output=True)
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
                record, work / f"KaleidoscopeScriptLexerOwnership{index}.lean", args.lean_root
            ))
    (work / "summary.json").write_text(json.dumps({
        "pin": PIN, "source_sha256": SOURCE_SHA256, "cases": len(selected),
        "builds": builds, "mutations": mutations, "admission": admission,
    }, indent=2) + "\n")
    print("kaleidoscope script lexer:", len(selected), "Rust cases,", len(builds),
          "builds,", len(mutations), "mutants,",
          sum(item["functions"] for item in admission), "admitted")


if __name__ == "__main__":
    main()
