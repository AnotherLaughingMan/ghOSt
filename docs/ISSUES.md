# ghOSt Issue Tracker

> **Project:** ghOSt — [https://github.com/AnotherLaughingMan/ghOSt](https://github.com/AnotherLaughingMan/ghOSt)
> **Owner:** AnotherLaughingMan
> **Last updated:** 2026-03-07

This document tracks known issues, planned work items, and open questions for the ghOSt project. For architectural decisions, see `/docs/decisions/`. For the full project roadmap, see the [Development Bible](DEVELOPMENT_BIBLE.md) Section 17.

---

## Status Definitions

| Status          | Meaning                                                    |
| --------------- | ---------------------------------------------------------- |
| **Open**        | Acknowledged, not yet started.                             |
| **In Progress** | Actively being worked on.                                  |
| **Blocked**     | Cannot proceed until a dependency or decision is resolved. |
| **Resolved**    | Completed and verified.                                    |
| **Deferred**    | Intentionally postponed to a future phase.                 |
| **Wontfix**     | Reviewed and determined not to address.                    |

## Priority Definitions

| Priority | Meaning                                            |
| -------- | -------------------------------------------------- |
| **P0**   | Critical — blocks all forward progress.            |
| **P1**   | High — required for current phase milestone.       |
| **P2**   | Medium — important but not blocking current phase. |
| **P3**   | Low — nice to have, future consideration.          |

## Category Labels

| Category        | Scope                                               |
| --------------- | --------------------------------------------------- |
| `architecture`  | System design, structural decisions.                |
| `kernel`        | Kernel core, memory, scheduling, IPC.               |
| `boot`          | Bootloader, UEFI, boot manager.                     |
| `drivers`       | Hardware drivers.                                   |
| `filesystem`    | VFS, filesystem implementations, storage.           |
| `ux`            | GUI, compositor, theming, UX shell.                 |
| `networking`    | TCP/IP stack, NIC drivers, DNS, DHCP.               |
| `compatibility` | COM runtime, 32-bit subsystem, app support.         |
| `security`      | Security boundaries, encryption, access control.    |
| `toolchain`     | Build system, cross-compiler, CI, image generation. |
| `documentation` | Bible, ADRs, READMEs, specs.                        |
| `meta`          | Project governance, process, repo structure.        |

---

## Open Issues

### ISS-0001: Select open-source license

| Field    | Value              |
| -------- | ------------------ |
| Status   | **Open**           |
| Priority | **P1**             |
| Category | `meta`             |
| Phase    | Pre-Phase 0        |
| Created  | 2026-03-07         |
| Assignee | AnotherLaughingMan |

**Description:** The project needs a license before accepting external contributions or distributing artifacts. README currently shows "TBD." Evaluate GPL v3, MIT, Apache 2.0, BSD, and MPL 2.0 against project values (user sovereignty, anti-censorship, open source). Record decision as an ADR.

---

### ISS-0002: Decide kernel architecture (monolithic vs micro vs hybrid)

| Field    | Value              |
| -------- | ------------------ |
| Status   | **Open**           |
| Priority | **P0**             |
| Category | `architecture`     |
| Phase    | Phase 0            |
| Created  | 2026-03-07         |
| Assignee | AnotherLaughingMan |

**Description:** Fundamental architecture decision that affects every subsequent design choice. Monolithic is simpler to start, microkernel is more modular/secure, hybrid attempts to balance both. Must be decided and recorded as ADR before Phase 2 kernel work begins. See Bible Appendix C, item 1.

---

### ISS-0003: Select and configure cross-compiler toolchain

| Field    | Value              |
| -------- | ------------------ |
| Status   | **Open**           |
| Priority | **P0**             |
| Category | `toolchain`        |
| Phase    | Phase 0            |
| Created  | 2026-03-07         |
| Assignee | AnotherLaughingMan |

**Description:** Set up GCC or Clang cross-compiler targeting x86-64 freestanding. Choose assembler (NASM vs GNU as) and syntax convention (Intel vs AT&T). Pin toolchain versions for reproducible builds. This is the Phase 0 exit gate — no OS code can be written without it.

---

### ISS-0004: Choose UEFI development approach

| Field    | Value              |
| -------- | ------------------ |
| Status   | **Open**           |
| Priority | **P1**             |
| Category | `boot`             |
| Phase    | Phase 1            |
| Created  | 2026-03-07         |
| Assignee | AnotherLaughingMan |

**Description:** Decide between GNU-EFI, POSIX-UEFI, EDK2, or writing UEFI interaction from scratch. Each has trade-offs in complexity, size, and control. See Bible Phase 1 notes.

---

### ISS-0005: Design custom filesystem or adopt existing

| Field    | Value              |
| -------- | ------------------ |
| Status   | **Open**           |
| Priority | **P2**             |
| Category | `filesystem`       |
| Phase    | Phase 4            |
| Created  | 2026-03-07         |
| Assignee | AnotherLaughingMan |

**Description:** Decide whether to implement a custom VFS and root filesystem or adopt/port an existing one (ext2 as starting point). Consider journaling needs, data integrity, and long-term maintenance burden. See Bible Appendix C, item 2.

---

### ISS-0006: UX rendering engine strategy

| Field    | Value              |
| -------- | ------------------ |
| Status   | **Open**           |
| Priority | **P2**             |
| Category | `ux`               |
| Phase    | Phase 10           |
| Created  | 2026-03-07         |
| Assignee | AnotherLaughingMan |

**Description:** The "browser-like UX" requires a rendering engine for HTML/CSS/JS. Options: embed an existing engine (Servo, WebKit subset), build a custom lightweight renderer, or use a native widget toolkit with web-view component. This is a massive scope decision. See Bible Appendix C, item 3.

---

### ISS-0007: Name the 32-bit compatibility subsystem

| Field    | Value              |
| -------- | ------------------ |
| Status   | **Open**           |
| Priority | **P3**             |
| Category | `compatibility`    |
| Phase    | Phase 11           |
| Created  | 2026-03-07         |
| Assignee | AnotherLaughingMan |

**Description:** The 32-bit compatibility subsystem needs a ghOSt-native name. Must not be "WoW" or any Windows branding. Should be memorable, on-brand, and technically descriptive enough. See Bible Appendix C, item 9.

---

### ISS-0008: Local AI inference runtime selection

| Field    | Value              |
| -------- | ------------------ |
| Status   | **Open**           |
| Priority | **P3**             |
| Category | `architecture`     |
| Phase    | Phase 12           |
| Created  | 2026-03-07         |
| Assignee | AnotherLaughingMan |

**Description:** Select or build the local inference engine for the AI subsystem. Candidates: ONNX Runtime, llama.cpp-style engine, custom runtime. Must evaluate model format support, memory footprint, CPU vs GPU dispatch, and licensing. See Bible Appendix C, item 10.

---

### ISS-0009: Set up QEMU + OVMF development environment

| Field    | Value              |
| -------- | ------------------ |
| Status   | **Open**           |
| Priority | **P0**             |
| Category | `toolchain`        |
| Phase    | Phase 0            |
| Created  | 2026-03-07         |
| Assignee | AnotherLaughingMan |

**Description:** Configure QEMU with OVMF (UEFI firmware) for testing bootable images without real hardware. Set up GDB remote debugging integration for kernel-level debugging. Essential for iterating on boot and kernel code.

---

### ISS-0010: GPU driver strategy — target GPU family selection

| Field    | Value              |
| -------- | ------------------ |
| Status   | **Open**           |
| Priority | **P2**             |
| Category | `drivers`          |
| Phase    | Phase 5+           |
| Created  | 2026-03-07         |
| Assignee | AnotherLaughingMan |

**Description:** Select the first GPU family to target for native driver support beyond GOP framebuffer. AMD is the leading candidate (published open-source register specs). Evaluate AMD AMDGPU documentation, complexity, and minimum viable feature set (modesetting + 2D acceleration). This decision gates GPU-accelerated UX features.

---

### ISS-0011: Confirm C coding style standard

| Field    | Value              |
| -------- | ------------------ |
| Status   | **Open**           |
| Priority | **P0**             |
| Category | `meta`             |
| Phase    | Phase 0            |
| Created  | 2026-03-07         |
| Assignee | AnotherLaughingMan |

**Description:** Bible Section 7.4 defines a C coding style (snake_case functions, PascalCase_t types, Allman braces for functions, K&R for control flow, 4-space indent, etc.). Confirm or amend before the first `.c` file is written. Record as ADR.

---

### ISS-0012: Select C unit test framework

| Field    | Value              |
| -------- | ------------------ |
| Status   | **Open**           |
| Priority | **P0**             |
| Category | `toolchain`        |
| Phase    | Phase 0            |
| Created  | 2026-03-07         |
| Assignee | AnotherLaughingMan |

**Description:** Choose a lightweight C test framework for kernel unit tests (candidates: Unity, CMocka, custom minimal). Must run in hosted environment for CI, and be small enough to not conflict with freestanding kernel constraints. See Bible Section 15.4.

---

### ISS-0013: Set up CI pipeline (GitHub Actions)

| Field    | Value              |
| -------- | ------------------ |
| Status   | **Open**           |
| Priority | **P0**             |
| Category | `toolchain`        |
| Phase    | Phase 0            |
| Created  | 2026-03-07         |
| Assignee | AnotherLaughingMan |

**Description:** Create GitHub Actions workflow for automated build, static analysis, unit tests, and QEMU boot smoke test. Must run on every push to `main`. Broken main is not acceptable. See Bible Section 15.5.

---

### ISS-0014: Define error handling conventions (ghost_status_t)

| Field    | Value              |
| -------- | ------------------ |
| Status   | **Open**           |
| Priority | **P1**             |
| Category | `kernel`           |
| Phase    | Phase 0–1          |
| Created  | 2026-03-07         |
| Assignee | AnotherLaughingMan |

**Description:** Define the kernel error type (`ghost_status_t` or equivalent), error propagation rules, panic severity levels (PANIC/OOPS/WARN), and cross-subsystem error translation. Must be decided before writing any multi-function kernel code. See Bible Section 8.4.

---

### ISS-0015: Design logging infrastructure

| Field    | Value              |
| -------- | ------------------ |
| Status   | **Open**           |
| Priority | **P1**             |
| Category | `kernel`           |
| Phase    | Phase 0–1          |
| Created  | 2026-03-07         |
| Assignee | AnotherLaughingMan |

**Description:** Implement kernel logging with log levels (PANIC/ERROR/WARN/INFO/DEBUG/TRACE), subsystem tags, timestamping, kernel ring buffer, and crash dump support. Serial output is the first target. See Bible Section 8.5.

---

### ISS-0016: Decide permission model (capabilities vs UNIX-style)

| Field    | Value              |
| -------- | ------------------ |
| Status   | **Open**           |
| Priority | **P1**             |
| Category | `security`         |
| Phase    | Phase 3            |
| Created  | 2026-03-07         |
| Assignee | AnotherLaughingMan |

**Description:** Choose between capability-based permissions and traditional UNIX-style uid/gid model. Affects syscall design, process model, and filesystem. Must be decided before userland process model is implemented. See Bible Section 10.4.

---

### ISS-0017: Design disk encryption architecture

| Field    | Value              |
| -------- | ------------------ |
| Status   | **Open**           |
| Priority | **P2**             |
| Category | `security`         |
| Phase    | Phase 4            |
| Created  | 2026-03-07         |
| Assignee | AnotherLaughingMan |

**Description:** Design FDE (full-disk encryption) or partition encryption. Decide key derivation algorithm, boot-time unlock flow, and recovery key mechanism. Privacy commitment requires this to be architecturally planned before filesystem work, even if implemented later. See Bible Section 10.5.

---

### ISS-0018: Design init system and process lifecycle

| Field    | Value              |
| -------- | ------------------ |
| Status   | **Open**           |
| Priority | **P1**             |
| Category | `kernel`           |
| Phase    | Phase 3            |
| Created  | 2026-03-07         |
| Assignee | AnotherLaughingMan |

**Description:** Design PID 1 / init system, process hierarchy, service management (JSON-driven), daemon lifecycle, zombie reaping, and orderly shutdown sequencing. ghOSt needs its own init — not systemd, not SysV. See Bible Section 13.3.

---

### ISS-0019: Confirm UTF-8 as canonical text encoding

| Field    | Value              |
| -------- | ------------------ |
| Status   | **Open**           |
| Priority | **P0**             |
| Category | `architecture`     |
| Phase    | Phase 0            |
| Created  | 2026-03-07         |
| Assignee | AnotherLaughingMan |

**Description:** Bible Section 7.5 establishes UTF-8 as the canonical encoding for all strings, filenames, configs, and APIs. Confirm this decision and record as ADR before any string-handling code is written. Retrofitting encoding is extremely costly.

---

### ISS-0020: Define ABI stability policy

| Field    | Value              |
| -------- | ------------------ |
| Status   | **Open**           |
| Priority | **P1**             |
| Category | `architecture`     |
| Phase    | Phase 3            |
| Created  | 2026-03-07         |
| Assignee | AnotherLaughingMan |

**Description:** Establish when the syscall ABI freezes (currently: unstable during 0.x, frozen at 1.0.0). UX shell and userland must version-check the kernel at startup. See Bible Section 13.2.

---

### ISS-0021: Select font rendering engine

| Field    | Value              |
| -------- | ------------------ |
| Status   | **Open**           |
| Priority | **P3**             |
| Category | `ux`               |
| Phase    | Phase 10           |
| Created  | 2026-03-07         |
| Assignee | AnotherLaughingMan |

**Description:** Choose font rendering engine for the GUI compositor. Candidates: FreeType, stb_truetype, custom. Must support hinting and subpixel rendering. Early boot uses bitmap font only. See Bible Section 12.9.

---

### ISS-0022: Design update mechanism and trust model

| Field    | Value              |
| -------- | ------------------ |
| Status   | **Open**           |
| Priority | **P2**             |
| Category | `security`         |
| Phase    | Phase 7–8          |
| Created  | 2026-03-07         |
| Assignee | AnotherLaughingMan |

**Description:** Design how the OS updates itself safely. Consider A/B partitions, recovery fallback, signed updates, and the trust model for update distribution. Critical for a privacy-focused OS. See Bible Appendix C.

---

### ISS-0023: Design package management / software distribution

| Field    | Value              |
| -------- | ------------------ |
| Status   | **Open**           |
| Priority | **P2**             |
| Category | `architecture`     |
| Phase    | Phase 6+           |
| Created  | 2026-03-07         |
| Assignee | AnotherLaughingMan |

**Description:** How do users install software? Package manager, app bundles, or both? Decide package format, repository model, dependency resolution. Shapes filesystem layout and permission model.

---

## Resolved Issues

_No issues resolved yet._

---

## Issue Log Format

When adding new issues, use this template:

```markdown
### ISS-NNNN: [Short descriptive title]

| Field    | Value      |
| -------- | ---------- |
| Status   | **Open**   |
| Priority | **P0–P3**  |
| Category | `category` |
| Phase    | Phase N    |
| Created  | YYYY-MM-DD |
| Assignee | [name]     |

**Description:** [Clear description of the issue, decision needed, or work item. Include references to Bible sections or ADRs where relevant.]
```

When resolving an issue, move it to the **Resolved Issues** section with:

- Resolution date
- Brief resolution summary
- Link to relevant commit, ADR, or Bible section if applicable
