# ADR-0004: UEFI Boot Approach — Minimal From-Scratch Loader First

**Status:** accepted
**Date:** 2026-03-08
**Author(s):** AnotherLaughingMan

## Context

ghOSt reached the point where Phase 1 boot work could not continue without choosing how
the first UEFI executable would be built.

The open options were:

- GNU-EFI;
- POSIX-UEFI;
- EDK2; or
- a minimal from-scratch UEFI application and loader path.

This decision matters because ghOSt is not trying to ship a generic firmware application
framework. It needs a small, explicit, debuggable boot path that can:

- build cleanly with the project's chosen Clang + LLD + NASM toolchain;
- work on the current Windows Phase 0 setup without introducing a large secondary build
  ecosystem;
- keep the boot handoff contract under direct project control; and
- stay small enough that the Minimal Rescue profile is not forced into a heavyweight
  firmware-development stack.

The first concrete milestone after this decision is the Phase 1 sign-of-life artifact:
a bootable `.efi` image that proves QEMU/OVMF bring-up and establishes the shape of the
future boot handoff.

## Decision

ghOSt will use a **minimal from-scratch UEFI application** for early bring-up and the
first bootloader milestones.

Concretely, this means:

- the project will define and maintain its own minimal UEFI header subset and loader code
  needed for Phase 1 and early Phase 2;
- the first `.efi` artifact will be built directly with Clang and LLD in PE/COFF form,
  without adopting EDK2 as the default build system;
- only the specific protocols required for early boot will be implemented first
  (initially console output, image/file access as needed, memory map acquisition, and
  `ExitBootServices()` preparation);
- EDK2 remains an allowed reference and later validation path, but not the default ghOSt
  boot implementation strategy; and
- GNU-EFI and POSIX-UEFI are not chosen as the primary foundation for ghOSt boot code.

## Rationale

### Why from scratch first

- **Fits the chosen toolchain:** ghOSt already standardized on Clang + LLD + NASM. A
  direct PE/COFF build path keeps Phase 1 aligned with that decision instead of layering
  an additional framework-specific toolchain on top.
- **Minimizes moving parts:** early boot debugging is hard enough without also debugging a
  large firmware build system, package layout, or external helper library conventions.
- **Keeps control of the ABI boundary:** the boot handoff structure from firmware to ghOSt
  is an OS API boundary. Owning the loader code directly keeps that boundary explicit.
- **Keeps the boot path small:** ghOSt's minimal-media goals argue for the smallest viable
  boot artifact and the fewest hidden dependencies.
- **Works well with the current host setup:** the Windows Phase 0 environment is already
  validated around QEMU, bundled edk2 firmware, Clang, LLD, and NASM.

### Why not EDK2 as the default

- **Too much framework for the first milestone:** EDK2 is powerful and mature, but it is a
  full firmware-development environment with its own build workflows, package model,
  Python tooling, and repo/submodule expectations.
- **Wrong optimization target for Phase 1:** ghOSt is building a tiny boot path, not a
  firmware product or a general-purpose UEFI package ecosystem.
- **Higher setup and maintenance cost:** adopting EDK2 as the default now would slow the
  path to the first sign-of-life result.

EDK2 still remains useful later as a reference implementation source, comparison target,
and possible validation environment if ghOSt eventually benefits from it.

### Why not GNU-EFI

- **Less aligned with the chosen Windows-first Phase 0 workflow:** GNU-EFI is strongly
  associated with GCC-style build assumptions and is not the cleanest fit for the current
  Clang + LLD + Windows setup.
- **Adds another compatibility layer:** ghOSt does not need a portability wrapper for its
  first, tightly-scoped x86-64 UEFI loader.

### Why not POSIX-UEFI

- **Not enough advantage over owning the tiny code directly:** for a minimal sign-of-life
  loader, a thin abstraction layer still adds an external dependency without removing the
  core need to understand UEFI semantics.
- **Smaller ecosystem and less strategic value:** if ghOSt is going to absorb dependency
  cost, EDK2 would be the stronger long-term reference point. For the tiny early path,
  direct implementation is cleaner.

## Alternatives Considered

### EDK2

- **Pros:** mature, feature-rich, widely used, strong protocol/library coverage, excellent
  long-term reference value.
- **Cons:** heavy build system, additional tooling complexity, larger conceptual surface
  area than ghOSt needs for first bring-up.
- **Verdict:** rejected as the default early boot path; retained as a later reference and
  optional validation path.

### GNU-EFI

- **Pros:** smaller than EDK2, traditional path for simple UEFI apps, established in hobby
  OS work.
- **Cons:** poorer fit for the project's chosen toolchain and Windows setup, still adds an
  external shim layer.
- **Verdict:** rejected.

### POSIX-UEFI

- **Pros:** lightweight approach, simpler than EDK2.
- **Cons:** narrower ecosystem, less strategic benefit than either direct ownership or
  EDK2, and still not zero-cost in integration and maintenance.
- **Verdict:** rejected.

## Consequences

- ghOSt must define a minimal internal UEFI interface layer rather than relying on a large
  external boot framework.
- The build system must emit a valid x86-64 PE/COFF UEFI application using Clang/LLD.
- Early boot code must stay disciplined and limited to the minimum protocols actually
  needed.
- Documentation should treat EDK2 as a reference source and optional later tool, not a
  prerequisite for contributing to the first boot milestones.

## Remaining Work

- Define the canonical Clang and LLD flags for the first x86-64 UEFI executable.
- Implement the sign-of-life `.efi` artifact and package it into a bootable image.
- Define the minimal internal header set and calling-convention assumptions for x86-64
  UEFI.
- Confirm serial and/or console output under QEMU/OVMF.
- Extend the loader toward memory map acquisition, GOP discovery, kernel loading, and
  `ExitBootServices()`.
