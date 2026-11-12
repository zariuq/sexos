#!/usr/bin/env python3
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent.parent
TOOL = HERE / "tools/epoch_journal_format.py"


def invoke(image, *arguments):
    return subprocess.run(
        ["python3", str(TOOL), "--image", str(image), "--base-lba", "2", "--sectors", "4", *arguments],
        cwd=HERE, capture_output=True, text=True)


def main():
    with tempfile.TemporaryDirectory(prefix="epoch-format-", dir=HERE / "build") as directory:
        image = Path(directory) / "disk.raw"
        original = bytes(8 * 512)
        image.write_bytes(original)
        result = invoke(image)
        assert result.returncode == 0, result.stderr
        summary = json.loads(result.stdout)
        assert summary["commit_generation"] == 1 and summary["next_sequence"] == 1
        assert summary["data_sectors"] == 4 and summary["metadata_sectors"] == 2
        contents = image.read_bytes()
        assert contents[:6 * 512] == original[:6 * 512]
        first, second = contents[6 * 512:7 * 512], contents[7 * 512:]
        assert first == second and first[:8] == b"SEXJHEAD"
        version, flags, generation, next_sequence, sessions, next_event = struct.unpack_from("<IIQQQQ", first, 8)
        assert (version, flags, generation, next_sequence, sessions, next_event) == (1, 0, 1, 1, 0, 1)
        value = 14695981039346656037
        for byte in first[:504]:
            value ^= byte
            value = (value * 1099511628211) & 0xFFFFFFFFFFFFFFFF
        assert struct.unpack_from("<Q", first, 504)[0] == value
        assert hashlib.sha256(contents).hexdigest() == summary["image_sha256"]
        before = image.read_bytes()
        refused = invoke(image)
        assert refused.returncode != 0 and "nonempty" in refused.stderr
        assert image.read_bytes() == before
        with image.open("r+b") as stream:
            stream.truncate(5 * 512)
        assert invoke(image).returncode != 0
    print("epoch formatter: initial metadata, independent checksum, preservation and refusal controls passed")


if __name__ == "__main__":
    main()
