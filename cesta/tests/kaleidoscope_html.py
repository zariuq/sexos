#!/usr/bin/env python3
"""Compare the bounded Cesta HTML reader with the pinned Rust reader."""

import argparse
import hashlib
import json
import os
import random
from pathlib import Path
import subprocess
import tempfile
from layout_text import LayoutText

HERE = Path(__file__).resolve().parent.parent
REPO = HERE.parent
PIN = "b6be1b451a6c21985d00d192674feedf66bb11dd"
HASHES = {
    "html_text.rs": "70b5791f2cca8c7bbe4ac4862af73352418691702b22c5a993eca96369e8ac8f",
    "script.rs": "1a1a612fab2d9ac343485abeee712cc9a22b414972cc78aa241ca3d590949e35",
    "regex.rs": "67075f4d19803445a01d05b518a27a21547f098a5a2a888b4c0d182b36ee4f2d",
    "fmath.rs": "54338f493cfc6613680c8eb1f4aaeabe8231e746500372174195d4a5fe72e134",
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
        (b"<ol type=a start=26><li>z<li>aa</ol><ol type=I start=1994><li>x</ol><ol type=i><li>i<li>ii</ol>"
         b"<ol start=' 3 '><li>s</ol><ol start=+5><li>p</ol><ol start=70000><li>o</ol><ol start=0><li>z</ol>", 512),
        (b"<p>one<br>two<br><br><br><br>three</p><ul><li><p>para</p><li>x<div>d</div></ul>", 256),
        (b"<table><caption>Sizes</caption><tr><th>k<th>v<tr><td>a<td>1</table><table><tr><th>h</th></tr></table>", 512),
        (b"<p>&AMP; &Amp; &Agrave;&agrave;&Sigma;&Sigmaf;&sigmaf;&le;&ge;&check;a&zwnj;b&lrm;c &#9;t &nbsp;&nbsp;x</p>", 256),
        (b"<video title='Intro'><p>fallback</p></video><audio/>after <iframe src=x></iframe><pre><code>a<kbd>b</kbd></code></pre>", 256),
        (b"<p>a\x0bb</p><p\x0bclass=x>c</p>", 256),
    ]
    cases += generated(20261105, 600)
    return cases


def generated(seed, count):
    """Random documents over the converter's tags, entities, whitespace and scripts."""
    rng = random.Random(seed)
    words = [b"word", b"two words", b"  ", b"\n", b"\t", b"\x0b", b"x", b"caf\xc3\xa9", b"\xe2\x82\xac", b"|"]
    entities = [b"&amp;", b"&AMP;", b"&lt;", b"&nbsp;", b"&Agrave;", b"&agrave;", b"&Sigma;", b"&Sigmaf;",
                b"&omega;", b"&le;", b"&starf;", b"&zwnj;", b"&#9;", b"&#x41;", b"&#0;", b"&bogus;", b"&shy;", b"&times;"]
    opens = [b"p", b"div", b"br", b"br", b"li", b"tr", b"td", b"th", b"caption", b"pre", b"code", b"kbd", b"blockquote",
             b"h2", b"strong", b"em", b"mark", b"hr", b"dd", b"dt", b"img alt=pic", b"a href=/x", b"video title=V",
             b"audio", b"iframe", b"ol", b"ul", b"table", b"ol type=a start=27", b"ol type=I start=4", b"ol start=' 2'",
             b"input type=checkbox checked", b"select", b"option", b"q", b"sup", b"abbr title=t"]
    scripts = [b"document.write(1+2)", b"document.write([3,1,2].toSorted(), ' ', 0.1+0.2)",
               b"document.write((1.005).toFixed(2), (12345.678).toLocaleString())",
               b"var s=new Set([1,1,2]);document.write(s.size, [...s])",
               b"Promise.resolve(4).then(v=>document.write('<b>'+v+'</b>'))",
               b"document.write(`a${`b${1}`}c`, String.raw`\\n`)", b"(", b"document.write('&amp;', '<p>x</p>')",
               b"class C { #x = 2; get x() { return this.#x } } document.write(new C().x)",
               b"document.write(JSON.stringify({a: [1]}, null, 1))"]
    cases = []
    for _ in range(count):
        parts = []
        for _ in range(rng.randrange(1, 40)):
            k = rng.random()
            if k < 0.35:
                parts.append(rng.choice(words))
            elif k < 0.5:
                parts.append(rng.choice(entities))
            elif k < 0.55:
                parts.append(b"<script>" + rng.choice(scripts) + b"</script>")
            elif k < 0.8:
                parts.append(b"<" + rng.choice(opens) + (b"/" if rng.random() < 0.05 else b"") + b">")
            else:
                parts.append(b"</" + rng.choice(opens).split()[0] + b">")
        cases.append((b"".join(parts), rng.choice([64, 256, 1024, 4096])))
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
    text += "mod fmath {\n" + sources["fmath.rs"].replace("//!", "//") + "\n}\n"
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
        for source in ("kaleidoscope_decimal", "kaleidoscope_script_lexer", "kaleidoscope_regex",
                       "kaleidoscope_regex_match", "kaleidoscope_script_core", "kaleidoscope_html"):
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
            ("header-rule-join", "html_aligned(out,start,&made,'+')", "html_aligned(out,start,&made,'|')"),
            ("br-runs", "else if(out.newlines<3)", "else if(out.newlines<2)"),
            ("roman-marker", "style=='I'?(uint8_t)(letter-'a'+'A'):letter", "letter"),
            ("entity-case", "if(name[i]!=(uint8_t)word[i])", "if(html_lower(name[i])!=(uint8_t)word[i])"),
            ("media-skip", "skip=media;", "(void)media;"),
            ("list-start-trim", "while(from<to&&html_white_space(html_scalar(value,from,&n)))", "while(false)"),
        ]
        prefix = [work / f"{name}-gcc.c" for name in (
            "kaleidoscope_decimal", "kaleidoscope_script_lexer", "kaleidoscope_regex",
            "kaleidoscope_regex_match", "kaleidoscope_script_core")]
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
