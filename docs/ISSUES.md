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

**Description:** Design how ghOSt updates itself safely over time. Scope includes: kernel updates, security updates, feature updates, data/feed updates, update channels (Stable/Preview/Dev), A/B or equivalent rollback-safe installation, signed manifests and payloads, trust-root management, reboot/maintenance policy, offline update bundles, and recovery fallback. The update system must be local-first, user-controlled, and capable of fast out-of-band security response without coercive forced reboots. See Bible Section 10.9 and Appendix C, item 11.

**Initial deliverables:**

- manifest schema and signature-validation rules;
- slot-state model, health-check rules, and automatic fallback behavior;
- boot-set coordination rules for kernel, bootloader, and boot-critical drivers;
- offline bundle format and recovery-environment repair flow;
- trust-root rotation and revocation procedure;
- boundary between base-system servicing and package/application updates.

---

### ISS-0037: Design update UX and servicing policy

| Field    | Value              |
| -------- | ------------------ |
| Status   | **Open**           |
| Priority | **P2**             |
| Category | `ux`               |
| Phase    | Phase 6+           |
| Created  | 2026-03-07         |
| Assignee | AnotherLaughingMan |

**Description:** Design the user/admin-facing update experience: update dashboard, category labels, release notes, restart communication, scheduling, rollback entry points, deferral controls, server maintenance windows, and offline update flow. Must clearly distinguish security updates from feature updates and avoid coercive or misleading UX. See Bible Section 12.12 and Appendix C, item 21.

**Initial deliverables:**

- desktop Update Center flow from check to install to reboot to rollback;
- headless/server servicing flow with maintenance windows and approval gates;
- Minimal Rescue/offline servicing workflow for repair and rollback;
- notification and reboot wording rules that stay technically honest;
- failure and recovery UX that surfaces exact next steps instead of generic errors.

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

### ISS-0024: Design PCI/PCIe bus enumeration subsystem

| Field    | Value              |
| -------- | ------------------ |
| Status   | **Open**           |
| Priority | **P0**             |
| Category | `drivers`          |
| Phase    | Phase 4–5          |
| Created  | 2026-03-07         |
| Assignee | AnotherLaughingMan |

**Description:** PCI/PCIe enumeration is the foundation for AHCI, NVMe, USB (xHCI), GPU, and NIC drivers — nothing works without it. PCIe is the native bus on all modern x86-64 hardware; legacy PCI is a VM/fallback path only. Design the enumeration subsystem: ECAM discovery via MCFG ACPI table, extended config space (4096 bytes per function), capability linked-list walking (MSI, MSI-X, PCIe cap, Power Management, AER), BAR sizing/allocation (including 64-bit BARs), bridge recursion, multi-function detection, and device-to-driver matching. Must also handle PCIe error reporting (AER), max payload/read request size negotiation, and legacy CF8h/CFCh fallback. Must be correct before any PCI-based driver is attempted. See Bible Section 5.2.

---

### ISS-0025: Design AHCI driver architecture and pitfall mitigation

| Field    | Value              |
| -------- | ------------------ |
| Status   | **Open**           |
| Priority | **P0**             |
| Category | `drivers`          |
| Phase    | Phase 4            |
| Created  | 2026-03-07         |
| Assignee | AnotherLaughingMan |

**Description:** AHCI is the primary SATA storage path and a notorious source of OS-dev bugs. Before writing driver code, review and plan for every pitfall cataloged in Bible Section 5.3.2: IDE mode detection, PI-based port enumeration, DMA alignment, command engine stop/start ordering, COMRESET timing, FIS construction, 64-bit DMA verification, spurious interrupts, NCQ/non-NCQ separation, error recovery via port reset, watchdog timeouts, and ATAPI detection. The driver must be tested on QEMU AHCI emulation first, then verified on real hardware. See Bible Sections 5.3.1–5.3.4.

---

### ISS-0026: Implement IOMMU support (Intel VT-d / AMD-Vi)

| Field    | Value              |
| -------- | ------------------ |
| Status   | **Open**           |
| Priority | **P1**             |
| Category | `kernel`           |
| Phase    | Phase 3–4          |
| Created  | 2026-03-07         |
| Assignee | AnotherLaughingMan |

**Description:** IOMMU provides hardware-enforced DMA isolation — without it, any bus-mastering PCIe device can read/write any physical memory. Implement: ACPI DMAR table parsing (Intel VT-d) and IVRS table parsing (AMD-Vi), DMA remapping with per-device page tables, interrupt remapping, identity mapping during early boot, fault logging with BDF identification. Must be enabled before any device performs DMA on production systems. See Bible Section 5.4.1.

---

### ISS-0027: Implement APIC architecture (Local APIC / IOAPIC / x2APIC)

| Field    | Value              |
| -------- | ------------------ |
| Status   | **Open**           |
| Priority | **P0**             |
| Category | `kernel`           |
| Phase    | Phase 2            |
| Created  | 2026-03-07         |
| Assignee | AnotherLaughingMan |

**Description:** The interrupt delivery subsystem is foundational for everything: timer, scheduler, device interrupts, SMP. Implement: legacy 8259 PIC disable, Local APIC initialization per core, IOAPIC redirection table programming, MADT-based topology discovery, x2APIC detection and MSR-based initialization when available, IPI support for SMP startup and TLB shootdowns. See Bible Section 5.4.2.

---

### ISS-0028: Design NUMA-aware memory allocator

| Field    | Value              |
| -------- | ------------------ |
| Status   | **Open**           |
| Priority | **P2**             |
| Category | `kernel`           |
| Phase    | Phase 3+           |
| Created  | 2026-03-07         |
| Assignee | AnotherLaughingMan |

**Description:** Modern multi-socket and chiplet-based systems use NUMA topology. Memory allocator must parse ACPI SRAT/SLIT tables, maintain per-node free lists, default to local-first allocation, and fall back by proximity distance. Scheduler should prefer keeping threads on CPUs near their memory. Must degrade gracefully to single-node behavior when SRAT is absent. See Bible Section 5.4.3.

---

### ISS-0029: Implement SMBIOS/DMI hardware inventory parsing

| Field    | Value              |
| -------- | ------------------ |
| Status   | **Open**           |
| Priority | **P2**             |
| Category | `kernel`           |
| Phase    | Phase 5            |
| Created  | 2026-03-07         |
| Assignee | AnotherLaughingMan |

**Description:** Parse SMBIOS tables for hardware inventory: Type 0 (BIOS info), Type 1 (system info), Type 3 (chassis — desktop vs server detection), Type 4 (processor), Type 16/17 (memory arrays/devices, ECC capability). Expose via kernel API for userland tools. See Bible Section 5.4.4.

---

### ISS-0030: Implement ECC memory error monitoring

| Field    | Value              |
| -------- | ------------------ |
| Status   | **Open**           |
| Priority | **P2**             |
| Category | `kernel`           |
| Phase    | Phase 5            |
| Created  | 2026-03-07         |
| Assignee | AnotherLaughingMan |

**Description:** On ECC-capable systems: enable Machine Check Architecture (MCA), poll MC banks for corrected memory errors, log with physical address and DIMM location (from SMBIOS), threshold-alert on failing DIMMs, offline poisoned pages for uncorrectable errors. Must be a no-op on non-ECC systems. See Bible Section 5.4.5.

---

### ISS-0031: Implement headless / serial console support

| Field    | Value              |
| -------- | ------------------ |
| Status   | **Open**           |
| Priority | **P1**             |
| Category | `kernel`           |
| Phase    | Phase 2            |
| Created  | 2026-03-07         |
| Assignee | AnotherLaughingMan |

**Description:** The kernel must boot to a working CLI shell with no GPU, no keyboard, and no display — serial console only. Parse ACPI SPCR table for firmware-configured serial parameters. Ensure no code path panics or stalls on missing framebuffer. Server deployments depend entirely on this. See Bible Section 5.4.7.

---

### ISS-0032: Support network boot (PXE / HTTP Boot)

| Field    | Value              |
| -------- | ------------------ |
| Status   | **Open**           |
| Priority | **P2**             |
| Category | `boot`             |
| Phase    | Phase 7–8          |
| Created  | 2026-03-07         |
| Assignee | AnotherLaughingMan |

**Description:** The ghOSt bootloader EFI binary must be loadable via PXE (TFTP/DHCP) and UEFI HTTP Boot. No binary changes required — the same EFI binary works for disk, PXE, and HTTP Boot. PE installer handles network-based install configuration. See Bible Section 5.4.8.

---

### ISS-0033: Define kernel security hardening baseline

| Field    | Value              |
| -------- | ------------------ |
| Status   | **Open**           |
| Priority | **P0**             |
| Category | `security`         |
| Phase    | Phase 2–3          |
| Created  | 2026-03-07         |
| Assignee | AnotherLaughingMan |

**Description:** Define and implement the minimum kernel hardening baseline: NX/XD, W^X, centralized user-copy validation, stack canaries, guard pages, read-only kernel data after init, and hardware features such as SMEP/SMAP/UMIP when supported. Decide which mitigations are mandatory in debug, Minimal Rescue, and Full Install builds. See Bible Section 10.6 and Appendix C, item 12.

---

### ISS-0034: Design local parental controls and child-account policy

| Field    | Value              |
| -------- | ------------------ |
| Status   | **Open**           |
| Priority | **P2**             |
| Category | `ux`               |
| Phase    | Phase 6+           |
| Created  | 2026-03-07         |
| Assignee | AnotherLaughingMan |

**Description:** Design a local-only parental-controls system that aligns with ghOSt's anti-censorship values. Scope: child accounts, time limits, app restrictions, install approvals, tamper resistance, and optional local web/content controls through user-selected services or apps. Must not require cloud accounts, telemetry, age verification, or vendor-operated blacklists. See Bible Sections 10.7 and 12.10, plus Appendix C, item 17.

---

### ISS-0035: Design built-in threat detection and trust model

| Field    | Value              |
| -------- | ------------------ |
| Status   | **Open**           |
| Priority | **P1**             |
| Category | `security`         |
| Phase    | Phase 5–6          |
| Created  | 2026-03-07         |
| Assignee | AnotherLaughingMan |

**Description:** Define ghOSt's built-in user-space security model: application trust prompts, binary/package verification, quarantine behavior, local malware scanning, heuristic/behavioral detection, and integration of open threat feeds for known threats. The design must explicitly distinguish known-threat intelligence from true 0-day detection, and must remain local-first with no mandatory cloud dependency. See Bible Section 10.8 and Appendix C, item 13.

---

### ISS-0036: Design Security Center UX and remediation flow

| Field    | Value              |
| -------- | ------------------ |
| Status   | **Open**           |
| Priority | **P2**             |
| Category | `ux`               |
| Phase    | Phase 6+           |
| Created  | 2026-03-07         |
| Assignee | AnotherLaughingMan |

**Description:** Design the graphical security experience: dashboard, permission prompts, trust labels, scan controls, alert triage, remediation actions, security history, and quiet mode. Prompts and alerts must be understandable to normal users, avoid constant warning fatigue, and clearly distinguish known malicious, suspicious, untrusted, and unsigned. See Bible Section 12.11 and Appendix C, item 19.

---

## Resolved Issues

### ISS-0001: Select open-source license

| Field           | Value                                          |
| --------------- | ---------------------------------------------- |
| Status          | **Resolved**                                   |
| Priority        | **P1**                                         |
| Category        | `meta`                                         |
| Phase           | Pre-Phase 0                                    |
| Created         | 2026-03-07                                     |
| Resolved        | 2026-03-07                                     |
| Assignee        | AnotherLaughingMan                             |
| Resolution Link | `docs/decisions/ADR-0001-license-selection.md` |

**Resolution:** ghOSt is licensed under **GPL-3.0-or-later**. GPL v3 was selected over MIT, Apache 2.0, BSD, and MPL 2.0 because strong copyleft and anti-tivoization protections best align with the project's user-sovereignty, privacy, and anti-censorship goals. The official license text is now in `/LICENSE`, README no longer shows `TBD`, and the rationale is recorded in ADR-0001.

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
