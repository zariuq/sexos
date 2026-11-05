#!/usr/bin/env python3
"""Share of structured Cesta, ownership-checked native C and unchecked native C.

Reads every cesta/src/*.cesta document with Cesta's own quoting rules and
classifies the text of each (native "...") form:

  checked    the unit carries an unconditional (ownership (check-all)), so
             Cesta's native ownership check reads its function bodies;
  unchecked  a function body the check does not read: either a definition the
             unit names (unchecked NAME ... (obligation "...")), or any body of
             a unit without an ownership form.

Named unchecked definitions carry their obligation in the unit. A unit with
native function bodies and no ownership form fails --check.

A unit whose native text is declarations only (types, prototypes, macros and
constant data) has no bodies and takes no ownership form. Its text counts as
checked when the ownership check reads it: when the header generated from it
(as tools/regenerate_client.py records, or matched by include guard) is
included, directly or through other headers, by a checked unit. Otherwise it
is reported as unchecked declarations.

Sizes are counted in non-whitespace bytes, so formatting does not move the
shares; raw byte counts are printed alongside.
"""
import argparse
import json
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
ESCAPES = {'n': '\n', 'r': '\r', 't': '\t', 'b': '\b', 'f': '\f', '"': '"', "'": "'", '\\': '\\'}


class Atom:
    __slots__ = ('kind', 'text', 'items', 'begin', 'end')

    def __init__(self, kind, text=None, items=None, begin=0, end=0):
        self.kind, self.text, self.items, self.begin, self.end = kind, text, items, begin, end


def read(source):
    pos = 0

    def skip():
        nonlocal pos
        while True:
            while pos < len(source) and source[pos] in ' \t\r\n\f\v':
                pos += 1
            if pos < len(source) and source[pos] == ';':
                while pos < len(source) and source[pos] != '\n':
                    pos += 1
                continue
            return

    def one():
        nonlocal pos
        skip()
        begin = pos
        c = source[pos]
        if c == '(':
            pos += 1
            items = []
            while True:
                skip()
                if source[pos] == ')':
                    pos += 1
                    return Atom('list', items=items, begin=begin, end=pos)
                items.append(one())
        if c == '"' or c == "'":
            pos += 1
            out = []
            while source[pos] != c:
                ch = source[pos]
                pos += 1
                if ch == '\\':
                    e = source[pos]
                    pos += 1
                    if e == 'x':
                        out.append(chr(int(source[pos:pos + 2], 16)))
                        pos += 2
                    else:
                        out.append(ESCAPES[e])
                else:
                    out.append(ch)
            pos += 1
            return Atom('text', text=''.join(out), begin=begin, end=pos)
        while pos < len(source) and source[pos] not in ' \t\r\n\f\v();"':
            pos += 1
        return Atom('symbol', text=source[begin:pos], begin=begin, end=pos)

    atom = one()
    skip()
    if pos != len(source):
        raise ValueError('extra input after the document')
    return atom


def head(atom, name):
    return atom.kind == 'list' and atom.items and atom.items[0].kind == 'symbol' and atom.items[0].text == name


def natives(atom):
    if head(atom, 'native') and len(atom.items) == 2 and atom.items[1].kind == 'text':
        yield atom.items[1]
        return
    if atom.kind == 'list':
        for item in atom.items:
            yield from natives(item)


def checks_all(unit):
    return any(head(form, 'ownership') and any(head(x, 'check-all') for x in form.items[1:])
               for form in unit.items[1:])


def named_unchecked(unit):
    """Definitions a checked unit leaves to named obligations."""
    names = set()
    for form in unit.items[1:]:
        if head(form, 'ownership'):
            for x in form.items[1:]:
                if head(x, 'unchecked') and len(x.items) > 1:
                    target = x.items[1]
                    if target.kind == 'symbol':
                        names.add(target.text)
                    elif target.kind == 'list':
                        names.update(n.text for n in target.items if n.kind == 'symbol')
    return names


def solid(text):
    return sum(1 for ch in text if not ch.isspace())


def strip_c(text):
    """Remove comments, string and character literals and preprocessor lines,
    keeping offsets (replaced by spaces)."""
    out = list(text)
    i, n = 0, len(text)
    line_start = True
    while i < n:
        c = text[i]
        if line_start and c == '#':
            j = i
            while j < n and not (text[j] == '\n' and text[j - 1] != '\\'):
                j += 1
            for k in range(i, j):
                if out[k] != '\n':
                    out[k] = ' '
            i = j
            continue
        if c == '\n':
            line_start = True
            i += 1
            continue
        if not c.isspace():
            line_start = False
        if text.startswith('//', i):
            j = text.find('\n', i)
            j = n if j < 0 else j
            for k in range(i, j):
                out[k] = ' '
            i = j
            continue
        if text.startswith('/*', i):
            j = text.find('*/', i + 2)
            j = n if j < 0 else j + 2
            for k in range(i, j):
                if out[k] != '\n':
                    out[k] = ' '
            i = j
            continue
        if c in '"\'':
            j = i + 1
            while j < n and text[j] != c:
                j += 2 if text[j] == '\\' else 1
            for k in range(i + 1, min(j, n)):
                out[k] = ' '
            i = j + 1
            continue
        i += 1
    return ''.join(out)


NAME = re.compile(r'([A-Za-z_]\w*)\s*\($')


def function_bodies(text):
    """Top-level brace blocks whose opener follows a parameter list."""
    code = strip_c(text)
    bodies = []
    depth, i, n = 0, 0, len(code)
    while i < n:
        c = code[i]
        if c == '{':
            if depth == 0:
                k = i - 1
                while k >= 0 and code[k].isspace():
                    k -= 1
                if k >= 0 and code[k] == ')':
                    level, m = 0, k
                    while m >= 0:
                        if code[m] == ')':
                            level += 1
                        elif code[m] == '(':
                            level -= 1
                            if level == 0:
                                break
                        m -= 1
                    found = NAME.search(code[max(0, m - 200):m + 1])
                    start = i
                    level, j = 0, i
                    while j < n:
                        if code[j] == '{':
                            level += 1
                        elif code[j] == '}':
                            level -= 1
                            if level == 0:
                                break
                        j += 1
                    line_begin = code.rfind(';', 0, m)
                    line_begin = max(line_begin, code.rfind('}', 0, m)) + 1
                    bodies.append((found.group(1) if found else '?', line_begin, j + 1))
                    i = j + 1
                    continue
            depth += 1
        elif c == '}':
            depth -= 1
        i += 1
    return bodies


INCLUDE = re.compile(r'#include\s+"([\w./-]+)"')
GUARD = re.compile(r'#ifndef\s+(\w+)\s*\n\s*#define\s+\1\b')
HEADER_DIRS = ('generated/include', 'generated/arm64/include', 'runtime/kernel', 'runtime/arm64', 'runtime')


def includes(doc, text):
    found = set(INCLUDE.findall(text))
    def walk(atom):
        if head(atom, 'include') and len(atom.items) == 2 and atom.items[1].kind == 'text':
            found.add(atom.items[1].text)
        elif atom.kind == 'list':
            for item in atom.items:
                walk(item)
    if doc is not None:
        walk(doc)
    return found


def read_by_checked(root, rows):
    """Headers the ownership check reads: those included by checked units."""
    headers = {}
    for directory in HEADER_DIRS:
        for path in (root / directory).glob('*.h'):
            headers.setdefault(path.name, path.read_text(errors='replace'))
    seen, stack = set(), []
    for r in rows:
        if r['checked']:
            stack.extend(r['includes'])
    while stack:
        name = stack.pop()
        if name in seen or name not in headers:
            continue
        seen.add(name)
        stack.extend(INCLUDE.findall(headers[name]))
    guards = {}
    for name in seen:
        g = GUARD.search(headers[name])
        if g:
            guards[g.group(1)] = name
    return guards, seen


def census(root):
    rows = []
    for path in sorted((root / 'src').glob('*.cesta')):
        source = path.read_text()
        doc = read(source)
        unit = next((x for x in doc.items if head(x, 'unit')), None) if doc.kind == 'list' else None
        texts = list(natives(doc))
        native_source = sum(t.end - t.begin for t in texts)
        native_solid = sum(solid(t.text) for t in texts)
        checked = bool(unit and checks_all(unit))
        named = named_unchecked(unit) if checked else set()
        functions, obligated = [], []
        body_solid = named_solid = 0
        for t in texts:
            for name, begin, end in function_bodies(t.text):
                functions.append(name)
                size = solid(t.text[begin:end])
                body_solid += size
                if name in named:
                    obligated.append(name)
                    named_solid += size
        native_text = ''.join(t.text for t in texts)
        guard = GUARD.search(native_text)
        rows.append(dict(unit=path.stem, raw=len(source.encode()), solid=solid(source),
                         native_raw=native_source, native_solid=native_solid, checked=checked,
                         functions=functions, body_solid=body_solid,
                         named=obligated, named_solid=named_solid,
                         includes=sorted(includes(doc, native_text)),
                         guard=guard.group(1) if guard else None))
    guards, seen = read_by_checked(root, rows)
    generated = {}
    record = root / 'tools' / 'regenerate_client.py'
    if record.exists():
        for m in re.finditer(r'"src/(\w+)\.cesta"[^\]]*\],\s*\w+\s*/\s*"([\w.-]+)"', record.read_text()):
            generated[m.group(1)] = m.group(2)
    for r in rows:
        reached = r['guard'] in guards or generated.get(r['unit']) in seen
        r['read_by_checked'] = bool(not r['checked'] and not r['functions'] and reached)
    return rows


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument('--root', type=pathlib.Path, default=ROOT)
    parser.add_argument('--json', action='store_true')
    parser.add_argument('--units', type=int, default=15, help='largest unchecked units to list')
    parser.add_argument('--check', action='store_true',
                        help='fail if an unchecked function body has no named obligation')
    args = parser.parse_args()
    rows = census(args.root)

    total = sum(r['solid'] for r in rows)
    native = sum(r['native_solid'] for r in rows)
    in_checked = sum(r['native_solid'] for r in rows if r['checked'])
    named_bodies = sum(r['named_solid'] for r in rows if r['checked'])
    named_count = sum(len(r['named']) for r in rows if r['checked'])
    read = sum(r['native_solid'] for r in rows if r['read_by_checked'])
    checked = in_checked - named_bodies + read
    open_units = native - in_checked - read
    bodies = sum(r['body_solid'] for r in rows if not r['checked'])
    unchecked = open_units + named_bodies
    missing = [r['unit'] for r in rows if not r['checked'] and r['functions']]
    summary = dict(units=len(rows), total=total, structured=total - native, checked_native=checked,
                   unchecked_native=unchecked, named_unchecked_bodies=named_bodies,
                   named_unchecked_functions=named_count, open_unit_native=open_units,
                   unchecked_bodies=bodies, unchecked_declarations=open_units - bodies,
                   declarations_read_by_checked=read,
                   unread_declaration_units=[r['unit'] for r in rows
                                             if not r['checked'] and not r['functions']
                                             and r['native_solid'] and not r['read_by_checked']],
                   raw=sum(r['raw'] for r in rows),
                   raw_open_unit_native=sum(r['native_raw'] for r in rows if not r['checked']),
                   unchecked_units_with_bodies=len(missing))
    if args.json:
        json.dump(dict(summary=summary, units=rows), sys.stdout, indent=1)
        print()
    else:
        pct = lambda x: f'{100 * x / total:5.1f}%'
        print(f'{len(rows)} units, {total:,} non-whitespace bytes ({summary["raw"]:,} raw)')
        print(f'  structured Cesta     {total - native:>10,}  {pct(total - native)}')
        print(f'  checked native C     {checked:>10,}  {pct(checked)}'
              f'  (of which {read:,} declaration-only units read by checked units)')
        print(f'  unchecked native C   {unchecked:>10,}  {pct(unchecked)}')
        print(f'    named unchecked functions in checked units  {named_bodies:>9,}  '
              f'{pct(named_bodies)}  ({named_count} functions)')
        print(f'    other native text outside checked units     {open_units:>9,}  {pct(open_units)}')
        print(f'      function bodies                           {bodies:>9,}  {pct(bodies)}')
        print(f'      declarations not read by a checked unit   {open_units - bodies:>9,}  '
              f'{pct(open_units - bodies)}  {summary["unread_declaration_units"]}')
        print(f'  units with function bodies and no ownership form: {len(missing)}')
        largest = sorted((r for r in rows if not r['checked'] and r['body_solid']),
                         key=lambda r: -r['body_solid'])[:args.units]
        if largest:
            print('largest unchecked units (function-body bytes, functions):')
            for r in largest:
                print(f'  {r["unit"]:<40} {r["body_solid"]:>8,}  {len(r["functions"])}')
    if args.check and missing:
        for unit in missing:
            print(f'unit with function bodies and no ownership form: {unit}', file=sys.stderr)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
