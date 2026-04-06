# /docs

This directory contains all architecture documentation, design references, and decision records for ghOSt.

## Key Documents

| Document                   | File                      | Purpose                                                                                     |
| -------------------------- | ------------------------- | ------------------------------------------------------------------------------------------- |
| **Development Bible**      | `DEVELOPMENT_BIBLE.md`    | Canonical design reference. The law of the land.                                            |
| **Issue Tracker**          | `ISSUES.md`               | Issue tracking, planned work, and open questions.                                           |
| **Windows Phase 0 Setup**  | `PHASE0_WINDOWS_SETUP.md` | Low-risk Windows bootstrap guidance for toolchain and QEMU/OVMF setup.                      |
| **Phase 1 Bring-up Guide** | `PHASE1_BRINGUP.md`       | Exact build, boot, and debug workflow for the first UEFI sign-of-life and image milestones. |
| **Architecture Decisions** | `decisions/`              | Individual ADRs for significant design choices.                                             |

## Rules

- The Development Bible is a living document. Changes to it are architectural decisions.
- ADRs are never deleted, only superseded.
- Every contributor should read the Bible before making architecture-impacting changes.

## Current ADRs

- `ADR-0001` — license selection: GPL-3.0-or-later
- `ADR-0002` — kernel architecture: microkernel
- `ADR-0003` — toolchain selection: Clang/LLD + NASM
- `ADR-0004` — UEFI boot approach: minimal from-scratch loader first
