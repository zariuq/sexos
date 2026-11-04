#!/usr/bin/env python3
"""Compare the generated client record and constants with the pinned Rust ABI."""

import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import tempfile
import tomllib

HERE = Path(__file__).resolve().parent.parent
REPO = HERE.parent


def run(command):
    result = subprocess.run(list(map(str, command)), cwd=REPO, capture_output=True, timeout=60)
    if result.returncode:
        raise RuntimeError((command, result.stderr.decode(errors="replace")))
    return result.stdout


def main():
    parser = argparse.ArgumentParser()
    parser.parse_args()
    revision = "4a040a86"
    source = run(["git", "show", revision + ":crates/sex-pdx/src/lib.rs"]).decode()
    contract_source = run(["git", "show", revision + ":sexos_contract.toml"])
    contract = tomllib.loads(contract_source.decode())
    record = re.search(r"pub struct PdxMessage \{[^}]*\}", source)
    assert record, "the upstream message record changed"
    constants = {}
    for spelling in ("SLOT_DISPLAY", "SLOT_NET"):
        value = re.search(r"pub\s+const\s+" + spelling + r":\s*u64\s*=\s*(\d+);", source)
        assert value, spelling
        constants[spelling] = int(value[1])
    pci_id = re.search(r"pub const SYS_MAP_PCI_BAR:\s*u64\s*=\s*(\d+);", source)
    drive = run(["git", "show", revision + ":apps/sexdrive/src/main.rs"]).decode()
    shared_id = re.search(r'in\("rax"\) (\d+)u64,\s*in\("rdi"\) fb_size as u64', drive)
    network = run(["git", "show", revision + ":servers/sexnet/src/main.rs"]).decode()
    diag_id = re.search(r'inout\("rax"\) (\d+)u64 => result,\s*in\("rdi"\) selector', network)
    assert pci_id and shared_id and diag_id, "the device syscall reference sites changed"
    work = Path(tempfile.mkdtemp(prefix="client-abi-", dir=HERE / "build"))
    rust = work / "probe.rs"
    rust.write_text("#[repr(C)]\n" + record[0] + r'''
fn main() {
    println!("{} {} {} {} {} {} {} {}",
      std::mem::size_of::<PdxMessage>(), std::mem::align_of::<PdxMessage>(),
      std::mem::offset_of!(PdxMessage, type_id), std::mem::offset_of!(PdxMessage, arg0),
      std::mem::offset_of!(PdxMessage, arg1), std::mem::offset_of!(PdxMessage, arg2),
      std::mem::offset_of!(PdxMessage, caller_pd), std::mem::offset_of!(PdxMessage, _pad));
}
''')
    run(["rustc", "--edition=2021", rust, "-o", work / "rust-probe"])
    expected = run([work / "rust-probe"])
    current_revision = json.loads((HERE / "interfaces/sext.source.json").read_text())["pin"]
    current_source = run(["git", "show", current_revision + ":crates/sex-pdx/src/lib.rs"]).decode()
    current_record = re.search(r"pub struct PdxMessage \{[^}]*\}", current_source)
    assert current_record and "caller_task" in current_record[0]
    current_probe = work / "current.rs"
    current_probe.write_text("#[repr(C)]\n" + current_record[0] + r'''
fn main() {
    println!("{} {} {} {} {} {} {} {}",
      std::mem::size_of::<PdxMessage>(), std::mem::align_of::<PdxMessage>(),
      std::mem::offset_of!(PdxMessage, type_id), std::mem::offset_of!(PdxMessage, arg0),
      std::mem::offset_of!(PdxMessage, arg1), std::mem::offset_of!(PdxMessage, arg2),
      std::mem::offset_of!(PdxMessage, caller_pd), std::mem::offset_of!(PdxMessage, caller_task));
}
''')
    run(["rustc", "--edition=2021", current_probe, "-o", work / "current-probe"])
    assert run([work / "current-probe"]) == expected, "current message layout differs from historical layout"
    c = work / "probe.c"
    c.write_text(r'''
#include "sexos_client.h"
#include <stdio.h>
_Static_assert(SEXOS_SYSCALL_call == EXPECT_CALL, "call syscall changed");
_Static_assert(SEXOS_SYSCALL_listen == EXPECT_LISTEN, "listen syscall changed");
_Static_assert(SEXOS_SLOT_display == EXPECT_DISPLAY, "display slot changed");
_Static_assert(SEXOS_SLOT_net == EXPECT_NET, "network slot changed");
_Static_assert(SEXOS_SYSCALL_map_pci_bar == EXPECT_PCI, "PCI mapping syscall changed");
_Static_assert(SEXOS_SYSCALL_alloc_shared_buffer == EXPECT_SHARED, "shared buffer syscall changed");
_Static_assert(SEXOS_SYSCALL_net_diag == EXPECT_DIAG, "network diagnostic syscall changed");
int main(void) {
    printf("%zu %zu %zu %zu %zu %zu %zu %zu\n", sizeof(SexosMessage), _Alignof(SexosMessage),
      offsetof(SexosMessage,type_id), offsetof(SexosMessage,arg0), offsetof(SexosMessage,arg1),
      offsetof(SexosMessage,arg2), offsetof(SexosMessage,caller_pd), offsetof(SexosMessage,caller_task));
    return 0;
}
''')
    flags = ["-std=c11", "-Wall", "-Wextra", "-Wpedantic", "-Werror", "-I", HERE / "generated/include",
             "-DEXPECT_CALL=" + str(contract["ipc"]["pdx_call"]["syscall"]),
             "-DEXPECT_LISTEN=" + str(contract["ipc"]["pdx_listen"]["syscall"]),
             "-DEXPECT_DISPLAY=" + str(constants["SLOT_DISPLAY"]), "-DEXPECT_NET=" + str(constants["SLOT_NET"]),
             "-DEXPECT_PCI=" + pci_id[1], "-DEXPECT_SHARED=" + shared_id[1], "-DEXPECT_DIAG=" + diag_id[1]]
    for compiler in ("gcc", "clang"):
        executable = work / compiler
        run([compiler, *flags, c, "-o", executable])
        assert run([executable]) == expected, compiler + ": generated record differs from Rust"
        print(compiler + ": message size/alignment, all six field offsets, syscall and slot constants agree with Rust")
    summary = {"revision": revision, "current_layout_revision": current_revision,
               "current_rust_source_sha256": hashlib.sha256(current_source.encode()).hexdigest(),
               "rust_source_sha256": hashlib.sha256(source.encode()).hexdigest(),
               "contract_sha256": hashlib.sha256(contract_source).hexdigest(), "record": expected.decode().strip(),
               "boundary": "message record layout and constants; register trap behavior is exercised by QEMU"}
    (work / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")
    (HERE / "build/client-abi-summary.json").write_text(json.dumps(summary, indent=2) + "\n")


if __name__ == "__main__":
    main()
