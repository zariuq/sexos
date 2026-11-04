"""Exercise the real Linen create body at identity exhaustion and retain the Rust counterexample."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess

HERE = Path(__file__).resolve().parent.parent


def main():
    ap = argparse.ArgumentParser(); ap.add_argument('--cesta', required=True)
    ap.add_argument('--output', type=Path, default=HERE / 'build/linen-session-ids'); args = ap.parse_args()
    w = args.output.resolve(); w.mkdir(parents=True, exist_ok=True)
    def run(args):
        result = subprocess.run(list(map(str, args)), cwd=HERE, capture_output=True, text=True)
        assert result.returncode == 0, (args, result.stdout[-2000:], result.stderr[-5000:])
        return result.stdout.strip()
    source = HERE.parent / 'servers/linen/src/session.rs'; reference = source.read_text()
    rust = '''const LINEN_MAX_NAME:usize=24;const LINEN_MAX_OBJECTS:usize=16;
mod session {
SOURCE
pub fn boundary(){let mut s=Session::new();s.next_id=u64::MAX;
 let first=s.create(ObjectKind::Document,b"a",3).unwrap();
 let second=s.create(ObjectKind::Document,b"b",3).unwrap();
 println!("{} {} {}",first,second,s.next_id);}
}
fn main(){session::boundary();}
'''.replace('SOURCE', reference)
    (w / 'reference.rs').write_text(rust)
    run(['rustc', '--edition=2021', '-Adead_code', '-C', 'overflow-checks=off', w / 'reference.rs', '-o', w / 'reference'])
    observed = run([w / 'reference']); assert observed == '18446744073709551615 0 1', observed
    run([args.cesta, 'export-c', 'src/linen.cesta', '--std=c11', '-o', w / 'linen.c', '--', '-Igenerated/include'])
    records = []
    for cc in ('gcc', 'clang'):
        for mode, extra in [('O0', ['-O0']), ('O2', ['-O2']), ('SAN', ['-O1', '-fsanitize=address,undefined', '-fno-sanitize-recover=all', '-fno-pie', '-no-pie'])]:
            binary = w / (cc + '-' + mode)
            run([cc, '-std=c11', '-Wall', '-Wextra', '-Wpedantic', '-Werror', *extra, '-ffunction-sections', '-fdata-sections',
                 '-Wl,--gc-sections', '-I' + str(w), '-Igenerated/include', 'tests/linen_capture/session_host.c', '-o', binary])
            records.append(dict(compiler=cc, mode=mode, output=run([binary])))
    c = w / 'linen.c'; original = c.read_text()
    guard = '  if (s->next_id == 0 || s->next_id == UINT64_MAX) return ERR_ID_EXHAUSTED;\n'
    assert original.count(guard) == 1
    changed = w / 'mutant'; changed.mkdir(exist_ok=True); (changed / 'linen.c').write_text(original.replace(guard, ''))
    binary = w / 'mutant-run'
    run(['gcc', '-std=c11', '-O2', '-ffunction-sections', '-fdata-sections', '-Wl,--gc-sections', '-I' + str(changed),
         '-Igenerated/include', 'tests/linen_capture/session_host.c', '-o', binary])
    result = subprocess.run([str(binary)], cwd=HERE, capture_output=True)
    assert result.returncode != 0, 'identity-wrap mutation survived'
    assert source.read_text() == reference
    (w / 'summary.json').write_text(json.dumps(dict(reference_sha256=hashlib.sha256(source.read_bytes()).hexdigest(),
        legacy_counterexample=observed, runtime=records, mutants_caught=1,
        scope='Intentional exhaustion correction, independently checked against the current Rust boundary. Ordinary request behavior remains separately qualified.'), indent=2) + '\n')
    print('Linen session IDs:', observed, 'reproduced in current Rust;', len(records), 'corrected C configurations; wrap mutation caught')


if __name__ == '__main__': main()
