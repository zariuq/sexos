#!/usr/bin/env python3
"""Check QEMU device arguments and preservation of earlier serial evidence."""
import json
import os
from pathlib import Path
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent.parent


def main():
    work = Path(tempfile.mkdtemp(prefix="kernel-boot-driver-", dir=HERE / "build"))
    fake = work / "qemu-system-x86_64"
    fake.write_text('''#!/usr/bin/env python3
import json
from pathlib import Path
import sys
import os
import time
args = sys.argv[1:]
serial = Path(args[args.index('-serial') + 1].removeprefix('file:'))
serial.with_suffix('.args.json').write_text(json.dumps(args))
serial.write_text('[test.ready]\\n')
time.sleep(float(os.environ.get('TEST_BOOT_HOLD', '0')))
''')
    fake.chmod(0o755)
    log = work / "serial.log"
    env = dict(os.environ, PATH=str(work) + os.pathsep + os.environ["PATH"])
    command = [HERE / "oracle/run.sh", work / "image.iso", log, "2", "--fixed-clock", "--until", "[test.ready]",
               "--cpu", "max,-pku", "--nvme-image", work / "disk.raw", "--network",
               "--network-gateway", "10.0.2.1", "--usb-keyboard"]
    subprocess.run(list(map(str, command)), env=env, check=True, capture_output=True)
    args = json.loads(log.with_suffix(".args.json").read_text())
    assert args.count("-cpu") == 1 and args[args.index("-cpu") + 1] == "max,-pku"
    assert "snapshot=on" in args[args.index("-drive") + 1]
    assert args[args.index("-netdev") + 1] == "user,id=net0,restrict=on,host=10.0.2.1"
    assert "usb-kbd,bus=xhci.0" in args and "nvme,drive=nvme0,serial=SEXOS-CHECK" in args
    assert "shift=4,sleep=off" in args and "base=2026-01-01T10:00:00,clock=vm" in args
    original = log.read_bytes()
    assert subprocess.run(list(map(str, command)), env=env, capture_output=True).returncode == 2
    assert log.read_bytes() == original
    invalid = [HERE / "oracle/run.sh", work / "image.iso", work / "invalid.log", "2",
               "--network-gateway", "invalid,gateway"]
    assert subprocess.run(list(map(str, invalid)), env=env, capture_output=True).returncode == 2
    assert not (work / "invalid.log").exists()
    for name, seconds, post, hold, expected in (
        ('early-exit', 4, 1, 0, 3), ('short-interval', 1, 3, 4, 3),
        ('complete-interval', 4, 1, 4, 0)):
        probe = [HERE/'oracle/run.sh', work/'image.iso', work/(name+'.log'), str(seconds),
                 '--until', '[test.ready]', '--post-ready', str(post)]
        result = subprocess.run(list(map(str, probe)), env={**env, 'TEST_BOOT_HOLD': str(hold)},
                                capture_output=True)
        assert result.returncode == expected, (name, result.returncode, result.stderr)
    print("kernel boot driver: CPU/devices, preserved logs, early-exit and incomplete-soak refusals")


if __name__ == "__main__":
    main()
