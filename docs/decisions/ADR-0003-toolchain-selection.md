# ADR-0003: Toolchain Selection — Clang/LLD + NASM

**Status:** accepted
**Date:** 2026-03-08
**Author(s):** AnotherLaughingMan

## Context

ghOSt needs a reproducible Phase 0 toolchain before any kernel, bootloader, or driver code can be written safely. The project needed to choose:

- the primary freestanding C compiler;
- the linker family;
- the primary assembler;
- the assembly syntax convention; and
- the practical path for building on developer machines, especially Windows.

The unresolved options were essentially:

- GCC + GNU binutils + GNU as;
- Clang + LLD + integrated LLVM tools;
- NASM vs GNU as for standalone assembly; and
- Intel vs AT&T assembly syntax.

## Decision

ghOSt will use the following primary toolchain direction:

- **Compiler:** LLVM/Clang
- **Linker:** LLD
- **Assembler:** NASM for standalone assembly sources
- **Assembly syntax:** Intel syntax

This means the default source model is:

- freestanding C compiled with Clang;
- standalone Assembly written in NASM Intel syntax; and
- linking performed with LLD for reproducible kernel and boot artifacts.

## Rationale

This choice is primarily pragmatic.

### Why Clang/LLD

- **Cross-platform practicality:** LLVM binaries are easy to obtain on Windows, Linux, and macOS.
- **One family for multiple artifact types:** Clang/LLD can target freestanding ELF for the kernel and also keeps PE/COFF-based UEFI paths viable without forcing a separate primary compiler family.
- **Good diagnostics:** Clang generally provides clearer diagnostics for early bring-up work.
- **Toolchain consistency:** using one primary compiler/linker family reduces drift in flags, object formats, and debug behavior.

### Why NASM

- **Clear standalone assembly workflow:** NASM is explicit and predictable for low-level OS work.
- **Intel syntax by default:** Intel syntax is the more readable choice for this project and aligns better with most firmware, chipset, and CPU documentation.
- **Avoids dual syntax confusion:** choosing NASM avoids mixing GNU as dialect details into early boot and ISR code unless explicitly required later.

## Alternatives Considered

### GCC + binutils + GNU as

- **Pros:** traditional OS-dev path, mature freestanding support, large body of examples.
- **Cons:** more awkward Windows setup, less attractive as a single-family answer when UEFI/PE-COFF flexibility is also needed.
- **Verdict:** rejected as the primary toolchain, though compatibility testing with GCC may still be useful later.

### GNU as with AT&T syntax

- **Pros:** tightly integrated with GCC/binutils workflows.
- **Cons:** less readable for the intended audience, diverges from most chipset/CPU documentation, and increases onboarding friction.
- **Verdict:** rejected.

### GNU as with Intel syntax

- **Pros:** possible compromise if GNU as were required.
- **Cons:** still inherits GNU as quirks without the simplicity benefit of NASM.
- **Verdict:** rejected as the default.

## Consequences

- Build scripts and documentation should assume Clang/LLD + NASM first.
- Standalone assembly sources should use NASM Intel syntax by default.
- Kernel and boot flags must be documented in a reproducible toolchain manifest or setup guide.
- GCC/binutils support may be added later as a validation path, but not as the primary development baseline.

## Remaining Work

- Pin exact tool versions for reproducible builds.
- Define the canonical target triples, object formats, and linker scripts.
- Document installation/setup steps for Windows and Linux developer machines.
- Prove the chosen stack by producing a bootable sign-of-life image in QEMU/OVMF. VMware Workstation Pro 17 may be used as a manual secondary validation path on machines where it is the only currently available VM platform.
