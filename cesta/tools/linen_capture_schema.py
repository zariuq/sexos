#!/usr/bin/env python3
"""Generate the capture layout and its Lean instance from one wire declaration."""
import argparse
import hashlib
import json
from pathlib import Path

HERE = Path(__file__).resolve().parent.parent


def products(schema):
    assert schema['version'] == 1 and schema['byte_order'] == 'little'
    assert schema['header'] == [['version', 2], ['length', 2], ['revision', 8], ['occupancy', 2]]
    assert schema['row'] == [['id', 8], ['kind', 1], ['disk', 1], ['name_length', 1], ['name', 'name_length']]
    assert schema['slots'] == 16 and schema['name_capacity'] == 24
    assert schema['read_payload_bytes'] == 7
    assert schema['reserved_object_ids'] == [0, (1 << 64) - 1]
    digest = hashlib.sha256(json.dumps(schema, sort_keys=True, separators=(',', ':')).encode()).hexdigest()
    header = sum(size for _, size in schema['header'])
    row = sum(size for _, size in schema['row'] if isinstance(size, int))
    maximum = header + schema['slots'] * (row + schema['name_capacity'])
    offsets = {}; at = 0
    for name, size in schema['header']:
        offsets[name.upper()] = at; at += size
    macros = dict(VERSION=schema['version'], SLOTS=schema['slots'], NAME_CAPACITY=schema['name_capacity'],
                  HEADER_BYTES=header, ROW_BYTES=row, MAX_BYTES=maximum, READ_BYTES=schema['read_payload_bytes'])
    macros.update({'OFFSET_' + k: v for k, v in offsets.items()})
    macros.update({'OP_' + k.upper(): v for k, v in schema['opcodes'].items()})
    c = '#ifndef SEXOS_LINEN_CAPTURE_LAYOUT_H\n#define SEXOS_LINEN_CAPTURE_LAYOUT_H\n'
    c += '/* Generated from linen_capture.schema.json; canonical SHA256 ' + digest + '. */\n'
    c += ''.join('#define LINEN_CAPTURE_' + key + ' ' + str(value) + '\n' for key, value in macros.items())
    c += '#endif\n'
    lean = '''import Mettapedia.Machines.Resources.CaptureLayout

namespace Mettapedia.Machines.Resources.LinenCaptureLayout
def layout : CaptureLayout :=
  ⟨VERSION, SLOTS, NAME, HEADER, ROW, READ⟩
theorem maximum_frame : layout.maximum = MAXIMUM := by decide
theorem header_fits_length : layout.maximum < 2^16 := by decide
theorem chunk_payload_disjoint_from_tag : 8 * layout.readBytes = 56 := by decide
theorem occupied_mask_fits : layout.slots ≤ 16 := by decide
theorem every_frame_length_bound (lengths : List Nat)
    (count : lengths.length ≤ layout.slots)
    (names : ∀ n ∈ lengths, n ≤ layout.nameBytes) :
    layout.headerBytes + (lengths.map (layout.rowBytes + ·)).sum ≤ MAXIMUM :=
  capture_length_bound layout lengths count names
end Mettapedia.Machines.Resources.LinenCaptureLayout
'''
    for key, value in dict(VERSION=schema['version'], SLOTS=schema['slots'], NAME=schema['name_capacity'],
                           HEADER=header, ROW=row, READ=schema['read_payload_bytes'], MAXIMUM=maximum).items():
        lean = lean.replace(key, str(value))
    return c, lean


def main():
    parser = argparse.ArgumentParser(); parser.add_argument('--check', action='store_true')
    parser.add_argument('--lean-output', type=Path); args = parser.parse_args()
    schema = json.loads((HERE / 'interfaces/linen_capture.schema.json').read_text())
    c, lean = products(schema)
    outputs = [(HERE / 'generated/include/linen_capture_layout.h', c)]
    if args.lean_output: outputs.append((args.lean_output, lean))
    for path, text in outputs:
        if args.check: assert path.read_text() == text, str(path)
        else: path.write_text(text)


if __name__ == '__main__': main()
