# SexOS — Cesta port

This is the **`cesta` branch** of [Andreas Xirtus’s SexOS](https://github.com/xirtus/sexos): an experimental port of its Rust kernel and services to [Cesta](https://github.com/zariuq/cesta), with readable C output.

The aim is to preserve the architecture—one address space, explicit capabilities, protection domains and revisioned state—while making more of its memory and protocol obligations checkable. Upstream supplies both the design and a behavioral reference. Where the implementation falls short of that design, an improvement needs its own specification and evidence.

[Upstream project](https://github.com/xirtus/sexos) · [Architecture](ARCHITECTURE.md) · [Cesta port interfaces](cesta/INTERFACE.txt) · [Ported components](cesta/SERVICES.txt)

![SexOS](docs/hero.jpg)

## What is Cesta?

Cesta is C with optional checks for ownership, lifetimes, memory bounds and protocols. It accepts ordinary C and translates enriched code into readable C for GCC or Clang, without a runtime borrow ledger. Optional proof tools verify selected contracts and compiled routines while keeping normal builds lightweight.

Its structured source forms, macros, typed spans and record contracts make programming intent explicit. A buffer can carry its extent; a returned reference can name the input it borrows; a protocol can require a receipt to be consumed or invalidate a fact after a write. Checked code still contains necessary bounds and failure checks.

The implementation remains inspectable C with ordinary C data layouts and calling conventions. Ordinary C remains available; the extra guarantees apply to code admitted under the corresponding contracts. Cesta does not require a garbage collector, a replacement system compiler, or a proof assistant in an ordinary application build.

## The OS architecture we are preserving

| Principle | Port direction |
|---|---|
| Single global virtual address space | Protection domains share the address space; authority determines permitted access. |
| Capability-mediated PDX communication | Sender identity, held grants, replies and completion obligations remain explicit. |
| Hardware protection keys | Enforce permissions established by the capability model. |
| BOOT capability genesis | Establish and seal initial authority before admitting user tasks. |
| Snapshot → transform → swap → commit | Extend checked state transitions and retain the inputs needed for replay. |
| Services in protection domains | Keep paging, display, files, input and other services outside the kernel where the design places them. |

These are architectural requirements. Their presence in a declaration or document is not evidence that every runtime path already enforces them.

## What runs today

The tree contains a Cesta x86-64 kernel and executable ports of services and applications, including Sext, Display, SilkBar, Input, Store, Files, USB, Net, Linen, Quil, Spindle, SexDrive and Kaleidoscope. QEMU qualification exercises assembled images with the available Cesta replacements.

A joined Cesta Silk Shell executable combines modules for custody, input, surfaces, publication and the main loop. Some joins remain explicit native boundaries, and newer upstream shell and renderer behavior still needs reconciliation. **The whole OS port is not complete.** Historical and compatibility images may also include Rust components.

The last broad source-reconciliation baseline is upstream [`b6be1b45`](https://github.com/xirtus/sexos/tree/b6be1b45). Newer upstream work is being incorporated selectively and qualified; fetching a newer revision does not establish port parity. [UPSTREAM.txt](cesta/UPSTREAM.txt) and the component interfaces describe the reference profiles and intentional differences.

The [image loader](cesta/PROGRAM_IMAGE.txt) now validates an executable's load plan before allocating frames or changing mappings. Cesta retains the source buffer through a checked borrowed result; kernel preflight also checks existing mappings and private-image capacity. Post-admission failures roll back unpublished mappings and transient leases before the image is committed to pinned BOOT ownership.

A small checked-Cesta loader decoder now has [exact-byte x86 proofs](cesta/proofs/program_image_u32.ml) for GCC and Clang kernel images. Under stated span and ABI assumptions, they establish its result, call/return behavior and bounded memory accesses. The [optional proof gate](cesta/tests/program_image_hol.py) replays both proofs and rejects a deliberately incorrect load. Both proved kernel files have passed paired boots with the current Cesta userland and device fixtures, including complete post-readiness soaks.

## Build the Cesta sources

A Linux x86-64 development environment needs a C11 compiler, Make and GNU binutils. Python 3 runs the regeneration and test tools. QEMU, Limine and ISO tools are needed for boot qualification; Rust and Lean are needed only by the reference/proof gates that use them.

For sibling checkouts:

```bash
git clone --branch cesta https://github.com/zariuq/sexos.git
git clone https://github.com/zariuq/cesta.git
make -C cesta
CESTA="$(pwd)/cesta/build/cesta"

cd sexos/cesta
"$CESTA" build -j 2 sexos-kernel-current
"$CESTA" build -j 2 sext linen quil
```

Use `--cc clang` to select Clang. The project declares the remaining executable and archive targets in [cesta.project](cesta/cesta.project).

The checked-in [readable-C export](cesta/c/) has its own Makefile and can be built without Cesta or Rust:

```bash
# From sexos/cesta
make -C c -j 2 CC=gcc build/sexos-kernel-current/sexos-kernel-current
```

To regenerate that export from the current sources:

```bash
make export CESTA="$CESTA"
```

The exported snapshot and a newer worktree may differ until regeneration. Qualification checks their correspondence explicitly.

Building a kernel binary is not the same as packaging a complete boot image. [package_client.py](cesta/tools/package_client.py) combines selected replacements with a reference staging tree; [boot_client.py](cesta/tools/boot_client.py) runs the resulting image. The [kernel interface](cesta/KERNEL.txt), [current BOOT checks](cesta/CURRENT_BOOT.txt) and component gates document the required profiles.

## What the evidence establishes

| Evidence | Scope |
|---|---|
| Cesta ownership and protocol checks | Declared resource, lifetime and control-flow obligations in admitted bodies. Native boundaries remain explicit. |
| Lean admission | Properties of the actual emitted permission plans and their mathematical models. |
| Rust differential tests | Agreement with a named upstream reference on the exercised inputs and observations. |
| Independent models and mutation controls | Checks of intended behavior, including cases where the port deliberately strengthens upstream. |
| GCC/Clang, sanitizers and readable-C comparisons | Compiler-route behavior, exercised memory errors and export correspondence. |
| QEMU gates | Particular images, devices, boot paths and runtime scenarios. |
| Optional HOL machine proofs | Exact selected instruction bytes under stated ABI and memory assumptions. |

These layers answer different questions. Passing a permission checker does not prove arbitrary C behavior; passing a test suite does not prove all executions; proving a leaf routine does not prove its entire caller or the OS.

Developer gates live in [cesta/Makefile](cesta/Makefile). `make check` runs the local port checks; `make qualify` additionally needs the declared proof/reference dependencies. The individual targets allow focused qualification.

## Remaining work

- Reconcile current shell, service, renderer and browser behavior, and strengthen their remaining native joins.
- Incorporate newer kernel program, memory and concurrency changes without weakening capability boundaries.
- Extend staged epoch and replay mechanisms to the remaining effects and execution paths.
- Connect source, permission, ABI and machine-code evidence more completely.
- Broaden hardware coverage and independent security review.

The current Cesta multicore profile enrolls and parks application processors; this is not active SMP execution. Metadata rollback is not rollback of published IPC or hardware effects. Full deterministic replay, production isolation and general-purpose desktop readiness are not claimed.

The current kernel retains a bounded suffix of at most 8,192 epoch events plus a checkpoint of the elided boundary. Total epochs and event counts do not wrap; elided or future event reads are refused. Complete replay of the elided prefix remains unimplemented.

## Repository map

```text
cesta/src/                Cesta kernel, services, applications and models
cesta/runtime/            explicit native and architecture boundaries
cesta/generated/          generated interfaces and helper code
cesta/c/                  readable C export
cesta/tests/              differential, model, admission and boot gates
cesta/cesta.project       x86-64 build graph
cesta/cesta.arm64.project separate ARM64 bring-up targets
kernel/, servers/,       Rust reference implementation
crates/, apps/
```

The upstream Rust build remains available through `make full` at the repository root. [Upstream releases](https://github.com/xirtus/sexos/releases) are upstream artifacts, not releases of this Cesta port.

## Licensing and contributing

The OS port follows upstream’s **GPLv3** licensing; see [LICENSE](LICENSE). Third-party components retain their own licences. Cesta is a separate compiler project.

Contributions should preserve the architecture, identify the intended observation, and provide evidence for behavior changes. Start with [ARCHITECTURE.md](ARCHITECTURE.md), [the port interface](cesta/INTERFACE.txt), and the relevant component tests. See [CONTRIBUTING.md](CONTRIBUTING.md) and [SECURITY.md](SECURITY.md) for contribution and security-reporting guidance.
