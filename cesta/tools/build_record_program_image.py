#!/usr/bin/env python3
import argparse
import hashlib
import struct
from pathlib import Path

DIRECTORY_LBA = 2048
PAYLOAD_LBA = 2049
SECTOR = 512


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--base', type=Path, required=True)
    parser.add_argument('--payload', type=Path, required=True)
    parser.add_argument('--out', type=Path, required=True)
    args = parser.parse_args()
    if args.base.resolve() == args.out.resolve():
        raise ValueError('base and output must differ')
    image = bytearray(args.base.read_bytes())
    payload = args.payload.read_bytes()
    if not payload or len(payload) > 32 * 1024 * 1024:
        raise ValueError('payload is empty or too large')
    end = PAYLOAD_LBA * SECTOR + len(payload)
    if end > len(image):
        raise ValueError('base image is too small')
    used = image[DIRECTORY_LBA * SECTOR:end]
    if any(used):
        raise ValueError('program-store area is not free')
    directory = bytearray(SECTOR)
    directory[:8] = b'SEXPROG1'
    struct.pack_into('<I', directory, 8, 1)
    directory[16:19] = b'hum'
    struct.pack_into('<II', directory, 32, PAYLOAD_LBA, len(payload))
    digest = 0xCBF29CE484222325
    for byte in payload:
        digest ^= byte
        digest = (digest * 0x100000001B3) & 0xFFFFFFFFFFFFFFFF
    struct.pack_into('<Q', directory, 40, digest)
    image[DIRECTORY_LBA * SECTOR:(DIRECTORY_LBA + 1) * SECTOR] = directory
    image[PAYLOAD_LBA * SECTOR:end] = payload
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_bytes(image)
    print(f'program-store: payload_bytes={len(payload)} lba={PAYLOAD_LBA} hash={digest:#x} '
          f'image_sha256={hashlib.sha256(image).hexdigest()}')


if __name__ == '__main__':
    main()
