"""Optional exact-kernel qualification of allocation-incarnation detachment.

The selected artifact changes lease metadata. Page-table effects, translation
invalidation, caller authentication and allocator coalescing have separate
contracts and qualification; this gate does not prove whole-kernel execution.
"""

import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shlex
import shutil
import subprocess
import sys
import time

ROOT = Path(__file__).resolve().parents[1]
SYMBOL = "frame_lease_detach"
KERNEL_FLAGS = ["-std=c11", "-O2", "-g0", "-ffreestanding", "-fno-builtin",
                "-fno-pic", "-fno-pie", "-mcmodel=kernel", "-fno-stack-protector",
                "-fno-asynchronous-unwind-tables", "-mno-red-zone", "-mgeneral-regs-only",
                "-Wall", "-Wextra", "-Wpedantic", "-Wformat=2", "-Werror"]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def run(command, cwd, log, accepted=True):
    command = list(map(str, command))
    with log.open("ab") as stream:
        stream.write((shlex.join(command) + "\n").encode())
        stream.flush()
        result = subprocess.run(command, cwd=cwd, stdout=stream, stderr=subprocess.STDOUT)
    if (result.returncode == 0) != accepted:
        raise RuntimeError(f"unexpected status {result.returncode}: {shlex.join(command)}; see {log}")


def declaration(select, label, path):
    symbol = select(path, SYMBOL)
    literal = "[" + ";".join(f"0x{b:02x}" for b in symbol.code) + "]"
    return (f'let detach_{label}_mc = define_assert_word_list "detach_{label}_mc"\n'
            f' (term_of_bytes(Bytes.sub (load_file {json.dumps(str(path))}) '
            f'{symbol.file_offset} {symbol.size})) {literal};;\n'
            f'let DETACH_{label.upper()}_EXEC = X86_MK_EXEC_RULE detach_{label}_mc;;\n')


def runtime(args, work, select, assert_identity, log):
    run([args.cesta, "export-c", ROOT / "src/frame_lease_interface.cesta", "-o", work / "frame_lease.h"], ROOT, log)
    run([args.cesta, "export-c", ROOT / "src/frame_lease.cesta", "-o", work / "frame_lease.c",
         "--plan", work / "frame_lease.plan", "--", "-I" + str(work)], ROOT, log)
    if (work / "frame_lease.h").read_bytes().rstrip(b"\n") != (
            ROOT / "generated/include/frame_lease.h").read_bytes().rstrip(b"\n"):
        raise RuntimeError("kernel header differs from authoritative Cesta header")
    result = {}
    for cc, original in (("gcc", args.gcc_image), ("clang", args.clang_image)):
        image = work / f"kernel-{cc}.elf"
        image.write_bytes(original.read_bytes())
        symbol = select(image, SYMBOL)
        flags = [*KERNEL_FLAGS, "-I", work]
        run([cc, *flags, "-c", work / "frame_lease.c", "-o", work / f"{cc}.o"], ROOT, log)
        run(["ld", "--build-id=none", "-e", SYMBOL, work / f"{cc}.o", "-o", work / f"{cc}.elf"], ROOT, log)
        if select(work / f"{cc}.elf", SYMBOL).code != symbol.code:
            raise RuntimeError(f"{cc}: current source's selected bytes differ from kernel")
        (work / f"{cc}.bytes").write_bytes(symbol.code)
        run(["objcopy", "--dump-section", f".text={work / (cc + '.text')}", image,
             work / f"{cc}-section-copy.elf"], ROOT, log)
        text = (work / f"{cc}.text").read_bytes()
        if text != symbol.text or text[symbol.offset:symbol.offset + symbol.size] != symbol.code:
            raise RuntimeError("independent ELF section extraction disagrees")
        caller = subprocess.check_output(["objdump", "-d", "--disassemble=program_frame_detach", image], text=True)
        (work / f"{cc}-caller.txt").write_text(caller)
        if not re.search(r"\b(?:call\w*|jmp\w*)\s+[^\n]*<frame_lease_detach>", caller):
            raise RuntimeError("kernel caller does not reach selected detach artifact")
        if "invlpg" not in caller:
            raise RuntimeError("native mapping-detachment caller lacks invalidation instruction")
        assembly = ('.section .text\n.globl detach_under_test\n.type detach_under_test,@function\n'
                    'detach_under_test:\n.incbin "' + str(work / f"{cc}.bytes") + '"\n'
                    '.size detach_under_test,.-detach_under_test\n.section .note.GNU-stack,"",@progbits\n')
        (work / f"{cc}-runtime.S").write_text(assembly)
        host = ROOT / "tests/frame_lease_detach/host.c"
        normal = ["-std=c11", "-O2", "-g0", "-Wall", "-Wextra", "-Werror", "-I", work, "-no-pie"]
        blob_host = work / f"{cc}-blob-host"
        run([cc, *normal, host, work / f"{cc}-runtime.S", "-o", blob_host], ROOT, log)
        run([blob_host], ROOT, log)
        for mode, extra in (("plain", []), ("sanitized", ["-fsanitize=address,undefined", "-fno-sanitize-recover=all"])):
            binary = work / f"{cc}-{mode}"
            run([cc, *normal, *extra, "-DDETACH_NATIVE", host, work / "frame_lease.c", "-o", binary], ROOT, log)
            run([binary], ROOT, log)
        for label, offset, value in (("instruction", symbol.file_offset, symbol.code[0] ^ 1),
                                     ("extent", symbol.symbol_offset + 16, 0), ("target", 18, 183)):
            changed = bytearray(symbol.image)
            changed[offset] = value
            bad = work / f"{cc}-bad-{label}.elf"
            bad.write_bytes(changed)
            try:
                assert_identity(bad, SYMBOL, symbol.identity())
            except ValueError:
                pass
            else:
                raise AssertionError(f"artifact mutation accepted: {cc}/{label}")
        result[cc] = {**symbol.identity(), "symbol_size": symbol.size,
                      "symbol_sha256": hashlib.sha256(symbol.code).hexdigest(),
                      "compiler_command": list(map(str, [cc, *flags, "-c", "frame_lease.c"])),
                      "runtime_cases_per_route": 9217, "runtime_routes": 3,
                      "caller_contains_invalidation": True,
                      "invalidation_formal_proof": False}
    # Retire to FREE instead of DETACHED, leaving valid x86 and the same extent.
    clang = select(work / "kernel-clang.elf", SYMBOL)
    pattern = bytes.fromhex("c644071902")
    if clang.code.count(pattern) != 1:
        raise RuntimeError("unrecognized Clang phase store; update semantic mutation")
    changed = bytearray(clang.image)
    changed[clang.file_offset + clang.code.index(pattern) + len(pattern) - 1] = 3
    mutant = work / "kernel-mutant.elf"
    mutant.write_bytes(changed)
    (work / "mutant.bytes").write_bytes(select(mutant, SYMBOL).code)
    mutated_assembly = (work / "clang-runtime.S").read_text().replace("clang.bytes", "mutant.bytes")
    (work / "mutant.S").write_text(mutated_assembly)
    run(["clang", *normal, host, work / "mutant.S", "-o", work / "mutant-host"], ROOT, log)
    run([work / "mutant-host"], ROOT, log, accepted=False)
    return result


def prove(args, work, select, log):
    started = time.monotonic()
    declarations = work / "bytes.ml"
    declarations.write_text("".join(declaration(select, cc, work / f"kernel-{cc}.elf")
                                      for cc in ("gcc", "clang")) +
                            declaration(select, "mutant", work / "kernel-mutant.elf"))
    support = args.cesta_source / "adapters/hol"
    files = [declarations, support / "pilot/span_words.ml", support / "x86_support.ml",
             support / "state_reads.ml"]
    files += [ROOT / "proofs" / ("frame_lease_" + name + ".ml")
              for name in ("spec", "detach", "scan", "hit", "exhaust", "invalid", "function")]
    text = 'needs "x86/proofs/base.ml";;\n'
    text += "\n".join("loadt " + json.dumps(str(path)) + ";;" for path in files) + "\n"
    theorems = []
    for cc in ("gcc", "clang"):
        prefix = "FRAME_LEASE_" + cc.upper()
        mc, execute, mode = f"detach_{cc}_mc", f"DETACH_{cc.upper()}_EXEC", str(cc == "gcc").lower()
        recipes = [("ENTRY", f"frame_detach_entry_rule {mc} {execute} {mode}"),
                   ("MISS_STEP", f"frame_lease_miss_rule {mc} {execute} {mode}"),
                   ("PREFIX", f"frame_lease_prefix_rule {mc} {execute} {mode} {prefix}_MISS_STEP"),
                   ("HIT", f"frame_lease_hit_rule {mc} {execute} {mode}"),
                   ("EXHAUST", f"frame_lease_exhaust_rule {mc} {execute} {mode}"),
                   ("INVALID", f"frame_lease_invalid_rule {mc} {execute} {mode}")]
        for suffix, recipe in recipes:
            name = prefix + "_" + suffix
            text += f"let {name} = {recipe};;\n"
            theorems.append(name)
        full = "SEXOS_" + prefix + "_CORRECT"
        text += (f"let {full} = frame_lease_function_rule {mc} {execute} "
                 f"{prefix}_ENTRY {prefix}_PREFIX {prefix}_HIT {prefix}_EXHAUST {prefix}_INVALID;;\n")
        theorems.append(full)
        text += f'print_endline "FRAME_DETACH_COMPILER_CHECKED {cc}";;\n'
    text += '''let changed_phase_refused =
  try ignore(frame_lease_hit_rule detach_mutant_mc DETACH_MUTANT_EXEC false); false
  with Failure _ -> true;;
if not changed_phase_refused then failwith "incorrect retirement phase admitted";;
print_endline "FRAME_DETACH_SEMANTIC_MUTANT_REFUSED";;
check_axioms ();;
print_endline "FRAME_DETACH_AXIOMS_CHECKED";;
'''
    text += "\n".join(f'Printf.printf "FRAME_DETACH_THEOREM {name} %s\\n" '
                      f'(string_of_thm {name});;' for name in theorems) + "\n"
    driver, inline = work / "proof.ml", work / "proof.inlined.ml"
    driver.write_text(text)
    env = {**os.environ, "HOLLIGHT_DIR": str(args.hol), "HOLLIGHT_USE_MODULE": "1"}

    def proof_command(command, target):
        with target.open("ab") as stream:
            stream.write((shlex.join(list(map(str, command))) + "\n").encode())
            stream.flush()
            result = subprocess.run(list(map(str, command)), cwd=args.s2n, env=env,
                                    stdout=stream, stderr=subprocess.STDOUT)
        if result.returncode:
            raise RuntimeError(f"proof command failed ({result.returncode}); see {target}")

    compiled, binary = work / "proof.inlined.cmx", work / "proof.native"
    proof_command([args.hol / "hol.sh", "inline-load", driver, inline], log)
    proof_command([args.hol / "hol.sh", "compile", inline, "-o", compiled, "-w", "-a"], log)
    proof_command([args.hol / "hol.sh", "link", compiled, "-o", binary], log)
    proof_log = work / "proof.log"
    proof_command([binary], proof_log)
    output = proof_log.read_text()
    required = ["FRAME_DETACH_AXIOMS_CHECKED", "FRAME_DETACH_SEMANTIC_MUTANT_REFUSED",
                *["FRAME_DETACH_THEOREM " + name + " " for name in theorems]]
    if any(marker not in output for marker in required):
        raise RuntimeError("required complete-function proof or audit results missing")

    def revision(path):
        return subprocess.check_output(["git", "-C", path, "rev-parse", "HEAD"], text=True).strip()

    return {"theorems": theorems, "fresh_process": True, "axiom_audit": True,
            "semantic_mutant_refused": True, "seconds": time.monotonic() - started,
            "proof_input_sha256": sha(inline), "proof_executable_sha256": sha(binary),
            "proof_log_sha256": sha(proof_log), "hol_commit": revision(args.hol),
            "s2n_commit": revision(args.s2n), "hol_library_sha256": sha(args.hol / "hol_lib.cmxa")}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("gcc-image", "clang-image", "cesta-source", "out"):
        parser.add_argument("--" + name, type=Path, required=True)
    parser.add_argument("--cesta", type=Path)
    parser.add_argument("--hol", type=Path)
    parser.add_argument("--s2n", type=Path)
    parser.add_argument("--runtime-only", action="store_true")
    args = parser.parse_args()
    for name, value in vars(args).items():
        if isinstance(value, Path):
            if name == "cesta":
                value = Path(shutil.which(str(value)) or value)
            setattr(args, name, value.resolve())
    args.cesta = args.cesta or args.cesta_source / "build/cesta"
    if not args.runtime_only and (args.hol is None or args.s2n is None):
        parser.error("proof replay needs --hol and --s2n; use --runtime-only for runtime evidence")
    work = args.out
    work.mkdir(parents=True, exist_ok=False)
    sys.path.insert(0, str(args.cesta_source / "adapters/hol/pilot"))
    from elf_symbol import select, assert_identity
    log = work / "commands.log"
    inputs = [ROOT / "src/frame_lease.cesta", ROOT / "src/frame_lease_interface.cesta",
              ROOT / "cesta.project", args.cesta, args.gcc_image, args.clang_image,
              Path(__file__).resolve(), ROOT / "tests/frame_lease_detach/host.c",
              *sorted((ROOT / "proofs").glob("frame_lease_*.ml")),
              *[args.cesta_source / "adapters/hol" / name
                for name in ("state_reads.ml", "x86_support.ml", "pilot/span_words.ml", "pilot/elf_symbol.py")]]
    before = {str(path): sha(path) for path in inputs}
    artifacts = runtime(args, work, select, assert_identity, log)
    result = {"format": "sexos-frame-detach/1", "proof_status": "not-run",
              "scope": "selected metadata detachment and native caller connection",
              "whole_kernel_refinement": False, "inputs": before, "artifacts": artifacts,
              "source_to_machine_refinement": False, "boot_qualification": False,
              "caller_authority_formal_proof": False, "translation_invalidation_formal_proof": False}
    if args.runtime_only:
        name = "runtime.json"
        message = "detach: GCC/Clang actual kernel bytes and source controls passed; HOL not run"
    else:
        result["proof"] = prove(args, work, select, log)
        result["proof_status"] = "accepted"
        name = "receipt.json"
        message = "detach: both complete kernel-artifact proofs replayed; semantic mutant refused; no new axioms"
    if before != {str(path): sha(path) for path in inputs}:
        raise RuntimeError("qualification inputs changed; no current receipt issued")
    (work / name).write_text(json.dumps(result, indent=2, sort_keys=True) + "\n")
    print(message)


if __name__ == "__main__":
    main()
