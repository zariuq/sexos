#!/usr/bin/env python3
"""Lay out every (native "...") fragment of src/*.cesta by the port's one rule.

The rule is cesta/.clang-format, applied by clang-format 21.1.8. A fragment
may change only in whitespace: the tool refuses any result whose
non-whitespace bytes differ from the original. The C text is written back
with real newlines; only backslash and double quote are escaped, following
Cesta's quoting rules.

  tools/native_format.py            rewrite the sources in place
  tools/native_format.py --check    list sources that differ from the rule
"""
import argparse
import os
import pathlib
import shutil
import subprocess
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import native_census  # noqa: E402

ROOT = pathlib.Path(__file__).resolve().parents[1]
STYLE = ROOT / '.clang-format'
VERSION = '21.1.8'


def formatter():
    explicit = os.environ.get('CLANG_FORMAT')
    if explicit:
        command = explicit.split()
    elif shutil.which('clang-format'):
        command = ['clang-format']
    else:
        command = ['uvx', '--from', f'clang-format=={VERSION}', 'clang-format']
    found = subprocess.run(command + ['--version'], capture_output=True, text=True, check=True).stdout
    if f'version {VERSION}' not in found:
        sys.exit(f'native_format: the layout rule is defined by clang-format {VERSION}; found {found.strip()}')
    return command


def encode(text):
    for ch in text:
        if ch == '\0':
            raise ValueError('NUL byte in native C source')
    return '"' + text.replace('\\', '\\\\').replace('"', '\\"') + '"'


def layout(command, text):
    result = subprocess.run(command + [f'--style=file:{STYLE}', '--assume-filename=fragment.c'],
                            input=text, capture_output=True, text=True, check=True).stdout
    return '\n' + result.strip('\n') + '\n'


def solid(text):
    """Non-whitespace bytes after C's line splicing (translation phase 2)."""
    return ''.join(ch for ch in text.replace('\\\n', '') if not ch.isspace())


def rewrite(command, path):
    source = path.read_text()
    doc = native_census.read(source)
    pieces, last = [], 0
    for atom in native_census.natives(doc):
        formatted = layout(command, atom.text)
        if solid(formatted) != solid(atom.text):
            raise SystemExit(f'{path}: byte {atom.begin}: formatting changed more than whitespace')
        pieces.append(source[last:atom.begin])
        pieces.append(encode(formatted))
        last = atom.end
    pieces.append(source[last:])
    result = ''.join(pieces)
    reread = [a.text for a in native_census.natives(native_census.read(result))]
    if [solid(t) for t in reread] != [solid(a.text) for a in native_census.natives(doc)]:
        raise SystemExit(f'{path}: rewritten document does not read back to the same fragments')
    return source, result


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument('--check', action='store_true')
    parser.add_argument('sources', nargs='*', type=pathlib.Path)
    args = parser.parse_args()
    command = formatter()
    sources = args.sources or sorted((ROOT / 'src').glob('*.cesta'))
    results = [(path, *rewrite(command, path)) for path in sources]
    differing = [(path, after) for path, before, after in results if before != after]
    if not args.check:
        for path, after in differing:
            path.write_text(after)
    if args.check:
        for path, _ in differing:
            print(f'not laid out by the native rule: {path.relative_to(ROOT)}')
        return 1 if differing else 0
    print(f'native_format: {len(differing)} of {len(sources)} sources rewritten')
    return 0


if __name__ == '__main__':
    sys.exit(main())
