"""Optional qualification of the actual linked-kernel u32 image decoder.

HOL proves the selected instructions under the stated call/memory contract.
This does not prove the ELF selector, C compiler, loader, or source refinement.
Ordinary kernel builds do not invoke this gate.
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

ROOT = Path(__file__).resolve().parents[1]
SYMBOL = "program_image_bytes_u32_at"
THEOREMS = ("SEXOS_IMAGE_U32_GCC_CORRECT", "SEXOS_IMAGE_U32_CLANG_CORRECT")


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def run(command, cwd, log, expected=0, env=None):
    command = list(map(str, command))
    with log.open("ab") as output:
        output.write((shlex.join(command) + "\n").encode())
        output.flush()
        result = subprocess.run(command, cwd=cwd, stdout=output,
                                stderr=subprocess.STDOUT, env=env)
    if result.returncode != expected:
        raise RuntimeError(f"status {result.returncode}, expected {expected}: "
                           f"{shlex.join(command)}; see {log}")


def bytes_declaration(select, name, image):
    symbol = select(image, SYMBOL)
    literal = "[" + ";".join(f"0x{byte:02x}" for byte in symbol.code) + "]"
    # Linked kernels use higher-half symbol addresses. The exact file slice
    # avoids treating those addresses as section-relative offsets. The gate
    # separately compares this slice with GNU objcopy's .text extraction.
    return f'''let image_u32_{name}_mc = define_assert_word_list "image_u32_{name}_mc"
  (term_of_bytes(Bytes.sub (load_file {json.dumps(str(image))})
    {symbol.file_offset} {symbol.size})) {literal};;
let IMAGE_U32_{name.upper()}_EXEC = X86_MK_EXEC_RULE image_u32_{name}_mc;;
'''


def runtime_controls(args, work, select, assert_identity, log):
    source = ROOT / "src/program_image_bytes.cesta"
    run([args.cesta, "export-c", source, "-o", work / "decoder.c",
         "--plan", work / "decoder.plan"], ROOT, log)
    run([args.cesta, "checked", "header", source, "-o",
         work / "program_image_bytes.h"], ROOT, log)
    run([args.cesta, "checked", "contract", source, "-o",
         work / "decoder.contract.json"], ROOT, log)
    # regenerate_client canonicalizes the number of final newlines.
    if (work / "program_image_bytes.h").read_bytes().rstrip(b"\n") != (
            ROOT / "generated/include/program_image_bytes.h").read_bytes().rstrip(b"\n"):
        raise RuntimeError("kernel header differs from current checked declaration")
    artifacts = {}
    for cc, original in (("gcc", args.gcc_image), ("clang", args.clang_image)):
        image = work / f"kernel-{cc}.elf"
        image.write_bytes(original.read_bytes())
        symbol = select(image, SYMBOL)
        code = work / f"{cc}.bytes"
        code.write_bytes(symbol.code)
        section = work / f"{cc}.text"
        run(["objcopy", "--dump-section", f".text={section}", image,
             work / f"{cc}-section-copy.elf"], ROOT, log)
        if section.read_bytes() != symbol.text or section.read_bytes()[
                symbol.offset:symbol.offset + symbol.size] != symbol.code:
            raise RuntimeError("independent ELF section extraction disagrees")
        caller = "program_image_parse"
        disassembly = subprocess.check_output(
            ["objdump", "-d", f"--disassemble={caller}", str(image)], text=True)
        (work / f"{cc}-caller.txt").write_text(disassembly)
        if not re.search(r"\bcall\w*\s+[^\n]*<" + SYMBOL + r">", disassembly):
            raise RuntimeError("selected kernel caller does not call the proved decoder")
        loader = "elf_load" if cc == "gcc" else "domain_spawn_at"
        loader_code = subprocess.check_output(
            ["objdump", "-d", f"--disassemble={loader}", str(image)], text=True)
        (work / f"{cc}-loader.txt").write_text(loader_code)
        if not re.search(r"\bcall\w*\s+[^\n]*<program_image_parse>", loader_code):
            raise RuntimeError("kernel loader does not call the admitted parser")
        flags = ["-std=c11", "-O2", "-g", "-Wall", "-Wextra", "-Werror",
                 "-fno-pie", "-no-pie", "-I", work]
        host = ROOT / "tests/program_image_bytes_host.c"
        run([cc, *flags, host, "-o", work / f"raw-{cc}"], ROOT, log)
        run([work / f"raw-{cc}", code], ROOT, log)
        for mode, extra in (("plain", []), ("sanitized", ["-fsanitize=address,undefined",
                                                           "-fno-sanitize-recover=all"])):
            binary = work / f"native-{cc}-{mode}"
            run([cc, *flags, *extra, "-DIMAGE_BYTES_NATIVE", host,
                 work / "decoder.c", "-o", binary], ROOT, log)
            run([binary], ROOT, log)
            for invalid in range(5):
                run([binary, invalid], ROOT, log, expected=73)
        for label, offset, value in (("instruction", symbol.file_offset, symbol.code[0] ^ 1),
                                     ("extent", symbol.symbol_offset + 16, 0),
                                     ("target", 18, 183)):
            changed = bytearray(symbol.image)
            changed[offset] = value
            path = work / f"{cc}-bad-{label}.elf"
            path.write_bytes(changed)
            try:
                assert_identity(path, SYMBOL, symbol.identity())
            except ValueError:
                pass
            else:
                raise AssertionError(f"artifact mutation accepted: {cc}/{label}")
        artifacts[cc] = {**symbol.identity(), "symbol": SYMBOL,
                         "symbol_file_offset": symbol.file_offset,
                         "symbol_sha256": hashlib.sha256(symbol.code).hexdigest(),
                         "caller": caller, "loader": loader}

    # Scale an indexed load by two. This is still a valid x86 instruction;
    # both independent runtime observations and the fresh proof must reject it.
    symbol = select(work / "kernel-clang.elf", SYMBOL)
    pattern = bytes.fromhex("8b0417")
    if symbol.code.count(pattern) != 1:
        raise RuntimeError("unrecognized Clang decoder; update the semantic control")
    offset = symbol.file_offset + symbol.code.index(pattern) + 2
    changed = bytearray(symbol.image)
    changed[offset] = 0x57
    mutant = work / "kernel-mutant.elf"
    mutant.write_bytes(changed)
    (work / "mutant.bytes").write_bytes(select(mutant, SYMBOL).code)
    run([work / "raw-clang", work / "mutant.bytes"], ROOT, log, expected=1)
    return artifacts


def prove(args, work, select, log):
    support = args.cesta_source / "adapters/hol"
    declarations = work / "bytes.ml"
    declarations.write_text("".join(bytes_declaration(select, cc, work / f"kernel-{cc}.elf")
                                     for cc in ("gcc", "clang")))
    mutant = work / "mutant.ml"
    mutant.write_text(bytes_declaration(select, "mutant", work / "kernel-mutant.elf") + '''
let mutant_refused =
  try ignore(image_u32_rule image_u32_mutant_mc IMAGE_U32_MUTANT_EXEC 16); false
  with Failure _ -> true;;
if not mutant_refused then failwith "incorrect indexed load admitted";;
print_endline "SEXOS_IMAGE_SEMANTIC_MUTANT_REFUSED";;
''')
    driver = work / "proof.ml"
    inputs = [declarations, support / "pilot/span_words.ml", support / "x86_support.ml",
              ROOT / "proofs/program_image_u32.ml", mutant]
    driver.write_text('needs "x86/proofs/base.ml";;\n' +
                      "\n".join("loadt " + json.dumps(str(p)) + ";;" for p in inputs) +
                      '\ncheck_axioms ();;\nprint_endline "SEXOS_IMAGE_AXIOMS_CHECKED";;\n' +
                      "\n".join(f'Printf.printf "SEXOS_IMAGE_THEOREM {name} %s\\n" '
                                f'(string_of_thm {name});;' for name in THEOREMS) + "\n")
    inlined = work / "proof.inlined.ml"
    compiled = work / "proof.inlined.cmx"
    binary = work / "proof.native"
    env = {**os.environ, "HOLLIGHT_DIR": str(args.hol), "HOLLIGHT_USE_MODULE": "1"}
    run([args.hol / "hol.sh", "inline-load", driver, inlined], args.s2n, log, env=env)
    run([args.hol / "hol.sh", "compile", inlined, "-o", compiled, "-w", "-a"],
        args.s2n, log, env=env)
    run([args.hol / "hol.sh", "link", compiled, "-o", binary], args.s2n, log, env=env)
    proof_log = work / "proof.log"
    run([binary], args.s2n, proof_log)
    output = proof_log.read_text()
    required = ["SEXOS_IMAGE_AXIOMS_CHECKED", "SEXOS_IMAGE_SEMANTIC_MUTANT_REFUSED"]
    required += [f"SEXOS_IMAGE_THEOREM {name} " for name in THEOREMS]
    if any(marker not in output for marker in required):
        raise RuntimeError("required kernel proof results missing")
    return {"theorems": list(THEOREMS), "proof_input_sha256": sha(inlined),
            "proof_executable_sha256": sha(binary),
            "hol_commit": subprocess.check_output(
                ["git", "-C", str(args.hol), "rev-parse", "HEAD"], text=True).strip(),
            "s2n_commit": subprocess.check_output(
                ["git", "-C", str(args.s2n), "rev-parse", "HEAD"], text=True).strip()}


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
            path = Path(shutil.which(str(value)) or value) if name == "cesta" else value
            setattr(args, name, path.resolve())
    args.cesta = args.cesta or args.cesta_source / "build/cesta"
    if not args.runtime_only and (args.hol is None or args.s2n is None):
        parser.error("proof replay needs --hol and --s2n; use --runtime-only for C controls")
    work = args.out
    work.mkdir(parents=True, exist_ok=True)
    if any(work.iterdir()):
        parser.error("--out must be empty; previous evidence is retained")
    sys.path.insert(0, str(args.cesta_source / "adapters/hol/pilot"))
    from elf_symbol import select, assert_identity
    log = work / "commands.log"
    artifacts = runtime_controls(args, work, select, assert_identity, log)
    report = {"format": "sexos-image-u32-proof/1", "artifacts": artifacts,
              "source_sha256": sha(ROOT / "src/program_image_bytes.cesta"),
              "exported_c_sha256": sha(work / "decoder.c"),
              "contract_sha256": sha(work / "decoder.contract.json"),
              "permission_plan_sha256": sha(work / "decoder.plan"),
              "proof_source_sha256": sha(ROOT / "proofs/program_image_u32.ml"),
              "runtime_cases_per_route": 7875, "runtime_routes": 6,
              "invalid_input_controls": 20, "identity_mutations_refused": 6,
              "runtime_semantic_mutants_refused": 1,
              "source_to_machine_refinement": False,
              "whole_loader_proof": False, "boot_qualification": False}
    if args.runtime_only:
        name = "runtime.json"
        print("Decoder runtime and artifact controls passed; HOL was not run")
    else:
        report.update(prove(args, work, select, log))
        name = "receipt.json"
        print("Both linked-kernel decoder proofs replayed; semantic mutant refused; no new axioms")
    (work / name).write_text(json.dumps(report, indent=2, sort_keys=True) + "\n")


if __name__ == "__main__":
    main()
