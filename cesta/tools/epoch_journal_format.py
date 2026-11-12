#!/usr/bin/env python3
"""Initialize an empty epoch journal extent without scanning it at boot."""
import argparse
import hashlib
import json
import os
import struct

SECTOR = 512
CHECKSUM_OFFSET = 504
MAGIC = b"SEXJHEAD"
VERSION = 1
META_SECTORS = 2


def checksum(sector):
    value = 14695981039346656037
    for byte in sector[:CHECKSUM_OFFSET]:
        value ^= byte
        value = (value * 1099511628211) & 0xFFFFFFFFFFFFFFFF
    return value


def metadata(generation, next_sequence, sessions, next_event):
    sector = bytearray(SECTOR)
    sector[:8] = MAGIC
    struct.pack_into("<IIQQQQ", sector, 8, VERSION, 0, generation,
                     next_sequence, sessions, next_event)
    struct.pack_into("<Q", sector, CHECKSUM_OFFSET, checksum(sector))
    return bytes(sector)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--image", required=True)
    parser.add_argument("--base-lba", type=int, default=4096)
    parser.add_argument("--sectors", type=int, default=16384)
    args = parser.parse_args()
    if args.base_lba < 0 or args.sectors < 1:
        raise SystemExit("invalid journal extent")
    start = args.base_lba * SECTOR
    extent = (args.sectors + META_SECTORS) * SECTOR
    with open(args.image, "r+b", buffering=0) as image:
        image.seek(0, os.SEEK_END)
        size = image.tell()
        if start > size or extent > size - start:
            raise SystemExit("journal extent exceeds image")
        image.seek(start)
        if any(image.read(extent)):
            raise SystemExit("refusing to format a nonempty journal extent")
        record = metadata(1, 1, 0, 1)
        image.seek(start + args.sectors * SECTOR)
        if image.write(record * META_SECTORS) != len(record) * META_SECTORS:
            raise SystemExit("short metadata write")
        os.fsync(image.fileno())
        image.seek(start + args.sectors * SECTOR)
        if image.read(len(record) * META_SECTORS) != record * META_SECTORS:
            raise SystemExit("metadata readback mismatch")
    with open(args.image, "rb") as image:
        digest = hashlib.file_digest(image, "sha256").hexdigest()
    print(json.dumps({"image": args.image, "image_bytes": size, "base_lba": args.base_lba,
                      "data_sectors": args.sectors, "metadata_sectors": META_SECTORS,
                      "next_sequence": 1, "sessions": 0, "next_event": 1,
                      "commit_generation": 1, "image_sha256": digest}, indent=2))


if __name__ == "__main__":
    main()
