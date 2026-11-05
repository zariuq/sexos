#!/usr/bin/env python3
"""Compare the bounded Cesta HTML reader with the pinned Rust reader."""

import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
from layout_text import LayoutText

HERE = Path(__file__).resolve().parent.parent
REPO = HERE.parent
PIN = "44053ea9b017fe89008413f7cc8a6b1a4d28d015"
HASHES = {
    "html_text.rs": "f0e6f6bdbb8444d0c356ba75ebdbc18cb1d4b335dde0e20bb25f05e5c14a5c09",
    "script.rs": "61b42d63cb819a938d34f8cdb11ce9682e4def5a45348a190394bfdda38c41a1",
    "regex.rs": "67075f4d19803445a01d05b518a27a21547f098a5a2a888b4c0d182b36ee4f2d",
}


def run(argv, *, cwd=HERE, env=None):
    result = subprocess.run(list(map(str, argv)), cwd=cwd, capture_output=True, env=env)
    if result.returncode:
        raise RuntimeError((argv, result.returncode, result.stdout[-2000:], result.stderr[-5000:]))
    return result.stdout


def corpus():
    cases = [
        (b"<!DOCTYPE html><html><head><title>t</title><style>p{color:red}</style></head>"
         b"<body><h1>SexOS   News</h1><p>First\n  paragraph.</p><p>Second<br>line</p>"
         b"<ul><li>one</li><li>two</li></ul><h3>Small</h3><p>end</p></body></html>", 4096),
        (b"<p>a<script>document.write('<p>x</p>')</script>b<!-- hidden <p>c</p> -->d</p>"
         b"<noscript>ns</noscript><template><p>tp</p></template><SCRIPT type=x>y</SCRIPT>"
         b"<script src=/x.js></script><script>while(1){}</script>e", 512),
        (b"<p>a &amp; b &lt;c&gt; &#65;&#x42; &copy; &bogus; &#0; &mdash;</p>", 512),
        (b"<p>Read <a href=\"/docs\">the docs</a> or <A class=x HREF='https://sexos.example/a?b=1'>this</A>.</p><a>bare</a>", 1024),
        (b"".join(f'<a href="/{n}">l{n}</a> '.encode() for n in range(20)), 8192),
        (b"<p>" + b"word " * 2000 + b"</p>", 17),
        (b"<p>" + b"word " * 2000 + b"</p>", 64),
        (b"<p>" + b"word " * 2000 + b"</p>", 1000),
        (b"<p", 256), (b"<!-- open", 256), (b"<script>never closed", 256),
        (b"&#xFFFFFFFFFF;", 256), (b"<a href=\"/x>t</a>", 256), (b"<<<>>>", 256), (b"&#xD800;", 256),
        (b"<table><tr><th>k</th><th>v</th></tr><tr><td>a</td><td>1</td></tr></table>", 256),
        (b"<table><tr><th>name</th><th>size</th><th>kind</th></tr><tr><td>Quil</td><td>12 KiB</td><td>editor</td></tr>"
         b"<tr><td>Spindle shell</td><td>8</td></tr></table><p>after</p>", 512),
        (b"  <!DOCTYPE html><p>", 256), (b"<HTML>", 256), (b"plain text <p>", 256), (b"", 256),
        (b"<a href='/a/b'>x</a> <a href=\"/guide\">y</a>", 256),
        (b"<p>" + b"w " * 40 + b"</p><a href=\"/a-long-target-path\">x</a>", 120),
        (b"<p>a   b</p><pre>\nfn main() {\n    x\t= 1;\n}\n</pre><p>c   d</p>", 512),
        (b"<pre>  x  <a href=\"/y\">y</a>", 256),
        (b"<p>Logo <img src=\"/art.qoi\" alt=\"SexOS mark\"> and <IMG src=x.png> and <img alt=' '/></p>", 256),
        (b"<html><head><TITLE> Field &amp; notes </TITLE></head><body><p>a</p><blockquote><p>q1</p>q2"
         b"<blockquote>deep</blockquote></blockquote><hr><p>b</p></body></html>", 512),
        (b"<ol><li>one<li>two<ul><li>a</li><li>b</li></ul></li><li>three</li></ol>"
         b"<ol start=7><li>seven</li></ol><dl><dt>term</dt><dd>meaning</dd></dl>", 512),
        (b"<p>Press <kbd>Ctrl</kbd>+<kbd>S</kbd> to <strong>save</strong>, see <code>quil.rs</code> and <em>notes</em>.</p>", 256),
        ("<p>&ldquo;Held&rdquo; &ndash; it&rsquo;s 5&deg; &times; 2 &rarr; &euro;10 &bull; soft&shy;hyphen &nosuch;</p>".encode(), 256),
        (b"<form><label>Name <input name=n placeholder='Your name'></label><input type=hidden value=secret>"
         b"<label><input type=checkbox checked> Remember</label> <input type=radio> A <input type=radio checked> B"
         b"<select><option>One<option selected>Two</select><textarea>hi there</textarea>"
         b"<button>Go now</button><input type=submit value=Send></form>"
         b"<p>E = mc<sup>2</sup>, H<sub>2</sub>O, <abbr title=\"capability\">cap</abbr>, <q>held</q>.</p>"
         b"<details><summary>More</summary><p>Hidden by default.</p></details>", 1024),
        (b"<p>Use <mark>held</mark> routes, not <del>ambient</del><ins>granted</ins> ones; see <cite>ARCHITECTURE</cite> "
         b"<s>old</s> <u>u</u>.</p><figure><img alt=Map><figcaption>The SASOS map</figcaption></figure>", 512),
        (b"<html><head><meta charset=\"windows-1252\"></head><body><p>caf\xe9 \x93q\x94 \x80</p></body></html>", 256),
        ("<p>café</p>".encode(), 256),
    ]
    return cases


def rust_slice(data):
    return "&[" + ",".join(map(str, data)) + "]"


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cesta", type=Path, required=True)
    parser.add_argument("--work", type=Path)
    parser.add_argument("--mutations", action="store_true")
    args = parser.parse_args()
    work = (args.work or Path(tempfile.mkdtemp(prefix="kaleidoscope-html-", dir=HERE / "build")).resolve())
    work.mkdir(parents=True, exist_ok=True)
    sources = {}
    for name in HASHES:
        data = subprocess.check_output(["git", "show", f"{PIN}:apps/kaleidoscope/src/{name}"], cwd=REPO)
        assert hashlib.sha256(data).hexdigest() == HASHES[name]
        sources[name] = data.decode()
    cases = corpus()
    text = "extern crate alloc;\nmod regex {\n" + sources["regex.rs"].replace("//!", "//") + "\n}\n"
    text += "mod script {\n" + sources["script.rs"].replace("//!", "//") + "\n}\n"
    text += "mod html_text {\n" + sources["html_text.rs"].replace("//!", "//") + "\n}\n"
    text += "static CASES:&[(&[u8],usize)]=&[\n" + ",\n".join(
        f"({rust_slice(data)},{cap})" for data, cap in cases) + "\n];\n"
    text += r'''
fn main(){let n:usize=std::env::args().nth(1).unwrap().parse().unwrap();let(input,cap)=CASES[n];let mut out=vec![0u8;cap];let c=html_text::convert(input,&mut out);print!("O ");for b in &out[..c.len]{print!("{:02x}",b)}print!(" {} {} {} {} {} {}",c.len,c.links,c.truncated as u8,c.scripts_run,c.scripts_refused,html_text::looks_like_html(input) as u8);for i in 0..c.links{print!(" {}:{}",c.targets[i].0,c.targets[i].1)}println!()}
'''
    (work / "reference.rs").write_text(text)
    reference = work / "reference"
    run(["rustc", "--edition=2021", "-C", "opt-level=2", work / "reference.rs", "-o", reference])
    expected = [run([reference, i]) for i in range(len(cases))]
    definitions = []
    rows = []
    for i, (data, cap) in enumerate(cases):
        body = ",".join(map(str, data)) if data else "0"
        definitions.append(f"static const uint8_t case_{i}[]={{ {body} }};")
        rows.append(f"{{case_{i},{len(data)},{cap}}}")
    (work / "cases.h").write_text(
        "typedef struct HtmlCase {const uint8_t *bytes;size_t length,capacity;} HtmlCase;\n" +
        "\n".join(definitions) + "\nstatic const HtmlCase cases[]={\n" + ",\n".join(rows) + "\n};\n"
    )
    host = HERE / "tests/kaleidoscope_html/host.c"
    exports = {}
    builds = []
    for compiler in ("gcc", "clang"):
        units = []
        for source in ("kaleidoscope_script_lexer", "kaleidoscope_regex", "kaleidoscope_regex_match",
                       "kaleidoscope_script_core", "kaleidoscope_html"):
            target = work / f"{source}-{compiler}.c"
            run([args.cesta, "export-c", f"src/{source}.cesta", "--cc", compiler, "--std=c11",
                 "-o", target, "--", "-Igenerated/include"])
            units.append(target)
            if source == "kaleidoscope_html":
                exports[compiler] = target
        common = ["-std=c11", "-g0", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
                  "-Igenerated/include", "-I", work]
        for mode, flags in (("O0", ["-O0"]), ("O2", ["-O2"]),
                            ("SAN", ["-O1", "-fsanitize=address,undefined",
                                     "-fno-sanitize-recover=all", "-fno-pie", "-no-pie"])):
            binary = work / f"{compiler}-{mode}"
            run([compiler, *common, *flags, host, *units, "-o", binary])
            for i, wanted in enumerate(expected):
                actual = run([binary, i], env={**os.environ, "ASAN_OPTIONS": "detect_leaks=1:abort_on_error=1"})
                assert actual == wanted, (compiler, mode, i, cases[i], actual, wanted)
            builds.append((compiler, mode))
    mutations = []
    if args.mutations:
        base = LayoutText(exports["gcc"].read_text())
        changes = [
            ("script-src", "bool plain=!src.present&&", "bool plain=true&&"),
            ("truncation-reserve", "return out->capacity>reserve?out->capacity-reserve:0", "return out->capacity"),
            ("link-bound", "link_count<KALEIDOSCOPE_HTML_MAX_LINKS", "link_count<=KALEIDOSCOPE_HTML_MAX_LINKS"),
            ("amp-entity", "{\"amp\",'&'}", "{\"amp\",'X'}"),
            ("skip-style", "if(kind&&!html_tag_self_closing", "if(false&&kind&&!html_tag_self_closing"),
            ("latin-decode", "if(latin&&byte>=0x80)", "if(false&&byte>=0x80)"),
        ]
        prefix = [work / f"{name}-gcc.c" for name in (
            "kaleidoscope_script_lexer", "kaleidoscope_regex", "kaleidoscope_regex_match",
            "kaleidoscope_script_core")]
        common = ["-std=c11", "-O2", "-g0", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
                  "-Igenerated/include", "-I", work]
        for name, old, new in changes:
            assert base.count(old) == 1, (name, base.count(old))
            mutant = work / f"mutant-{name}.c"
            mutant.write_text(base.replace(old, new))
            binary = work / f"mutant-{name}"
            built = subprocess.run(["gcc", *common, host, *prefix, mutant, "-o", binary],
                                   cwd=HERE, capture_output=True)
            if built.returncode == 0:
                assert any(subprocess.run([binary, str(i)], cwd=HERE, capture_output=True).stdout != wanted
                           for i, wanted in enumerate(expected)), ("survived", name)
            mutations.append(name)
    summary = {"pin": PIN, "hashes": HASHES, "cases": len(cases), "builds": builds,
               "mutations": mutations}
    (work / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")
    print("kaleidoscope html:", len(cases), "Rust cases,", len(builds), "builds,",
          len(mutations), "mutants")


if __name__ == "__main__":
    main()
