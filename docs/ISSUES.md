# ghOSt Issue Tracker

> **Project:** ghOSt — [https://github.com/AnotherLaughingMan/ghOSt](https://github.com/AnotherLaughingMan/ghOSt)
> **Owner:** AnotherLaughingMan
> **Last updated:** 2026-03-08

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

### ISS-0003: Select and configure cross-compiler toolchain

| Field    | Value              |
| -------- | ------------------ |
| Status   | **In Progress**    |
| Priority | **P0**             |
| Category | `toolchain`        |
| Phase    | Phase 0            |
| Created  | 2026-03-07         |
| Assignee | AnotherLaughingMan |

**Description:** The primary toolchain direction is now chosen: **Clang + LLD + NASM (Intel syntax)**. Windows Phase 0 tooling is now installed and validated at the expected explicit paths, and the repo contains helper scripts plus a manifest for QEMU/OVMF launch and tool verification. Remaining work is to pin exact versions as policy, define canonical target triples and flags, document Linux setup, and prove the stack by producing a bootable sign-of-life image in QEMU/OVMF. This remains the Phase 0 exit gate — no OS code can be written safely until the chosen toolchain is actually configured and reproducible. See Bible Section 17.1 and ADR-0003.

**Initial deliverables:**

- exact pinned versions for Clang/LLVM, LLD, NASM, QEMU, and OVMF;
- documented install/bootstrap steps for Windows and Linux;
- canonical compile/link flags for freestanding kernel and boot artifacts;
- proof build that generates a bootable image and reaches sign-of-life in QEMU/OVMF.

**Current progress:**

- Windows Phase 0 setup documented in `docs/PHASE0_WINDOWS_SETUP.md`.
- Explicit-path manifest and PowerShell helpers added under `tools/phase0/windows/`.
- Clang, LLD, NASM, QEMU, `qemu-img`, and bundled edk2 firmware verified on the current Windows workstation.
- QEMU helper path validated with a scratch-image dry run using software emulation (`accel=tcg`).
- Clang + `lld-link` now produce a bootable x86-64 UEFI `.efi` artifact from repo-local source.
- QEMU/OVMF now reaches the ghOSt UEFI sign-of-life app and emits the expected debug banner.
- Remaining blockers are now boot-artifact generation, canonical compile/link flags, Linux setup documentation, and version pinning policy.

---

### ISS-0004: Choose UEFI development approach

| Field    | Value              |
| -------- | ------------------ |
| Status   | **Resolved**       |
| Priority | **P1**             |
| Category | `boot`             |
| Phase    | Phase 1            |
| Created  | 2026-03-07         |
| Assignee | AnotherLaughingMan |

**Description:** Resolved by ADR-0004. ghOSt will use a minimal from-scratch UEFI
application and loader path for early bring-up and the first sign-of-life boot milestone.
The default path remains aligned with Clang + LLD + NASM and avoids taking on EDK2 as a
required build framework for the first `.efi` artifact. EDK2 remains a reference and
optional later validation path.

**Resolution notes:**

- Adopt a repo-owned minimal UEFI interface layer instead of GNU-EFI, POSIX-UEFI, or EDK2.
- Build the first x86-64 `.efi` directly as PE/COFF with the primary ghOSt toolchain.
- Continue immediately into `ISS-0041` using this decision as the implementation baseline.

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
| Status   | **In Progress**    |
| Priority | **P0**             |
| Category | `toolchain`        |
| Phase    | Phase 0            |
| Created  | 2026-03-07         |
| Assignee | AnotherLaughingMan |

**Description:** Configure QEMU with OVMF (UEFI firmware) as the canonical no-hardware development and debugging environment. The Windows host-side setup is now working with QEMU's bundled edk2 x86-64 firmware, explicit-path helper scripts, scratch-image generation, staged-ESP boot, and TCG-based launch flow. The launcher now supports staged ESP directories, debugcon capture, and GDB stub flags, and QEMU/OVMF successfully boots the repo-local ghOSt UEFI sign-of-life artifact. VMware Workstation Pro 17 is acceptable as a secondary/manual validation path when it is the only VM platform available, but it does not replace QEMU/OVMF for automation, reproducibility, or preferred debug workflow. Essential for iterating on boot and kernel code.

**Current progress:**

- QEMU installed under `C:\Dev\Tools\qemu` and verified.
- Bundled edk2 firmware path verified for x86-64 UEFI boot.
- Repo-local helpers added for verification, launch, and scratch-image creation.
- Dry-run launch path verified with `q35,accel=tcg` and serial console wiring.
- `Start-ghOStQemu.ps1` now accepts `-EspDirectoryPath`, `-DebugConPath`, `-Gdb`, and `-GdbWait`.
- QEMU/OVMF now successfully boots the staged ghOSt UEFI sign-of-life app from a repo-local ESP directory and captures its debug output.
- Remaining work is packaging and validating the real bootable `.img` path in addition to the staged-ESP fast path.

---

### ISS-0041: Produce first UEFI sign-of-life boot artifact

| Field    | Value              |
| -------- | ------------------ |
| Status   | **In Progress**    |
| Priority | **P0**             |
| Category | `boot`             |
| Phase    | Phase 0            |
| Created  | 2026-03-08         |
| Assignee | AnotherLaughingMan |

**Description:** Build the first minimal ghOSt UEFI sign-of-life target and package it into a QEMU/OVMF-bootable image. This is the immediate next executable step now that the Windows Phase 0 toolchain and QEMU firmware paths are working. Scope includes the smallest viable EFI application or equivalent proof artifact, image layout, fallback boot path naming, and serial or framebuffer output that proves firmware handoff succeeded. The first repo-local UEFI source, build script, ESP staging layout, packaged-image path, and QEMU/OVMF validation path now exist; the loader now queries the UEFI memory map, discovers GOP framebuffer details, validates the first ghOSt kernel image header plus x86-64 boot-info entry ABI, reaches a C-backed early kernel initialization stub after `ExitBootServices()`, and now encodes explicit ownership/lifetime rules for the handed-off payload, final memory map, framebuffer descriptor, and early kernel stack. See Bible Sections 17.1 and 17.2.

**Current progress:**

- Minimal repo-owned UEFI header layer added under `boot/uefi/include/`.
- First sign-of-life application added under `boot/uefi/src/`.
- `tools/phase1/windows/Build-ghOStUefiSignOfLife.ps1` now emits `ghost-sign-of-life.efi` and stages `EFI/BOOT/BOOTX64.EFI`.
- `tools/phase0/windows/Start-ghOStQemu.ps1` now supports booting directly from a staged ESP directory, capturing QEMU debugcon output, and exposing the QEMU GDB stub.
- QEMU/OVMF validation succeeded with the expected banner: `ghOSt Phase 1 UEFI sign-of-life`.
- The staged ESP now emits `startup.nsh` so the shell fallback still launches `EFI\BOOT\BOOTX64.EFI` reliably when OVMF lands in the internal shell first.
- The UEFI sign-of-life app now successfully calls `GetMemoryMap()` and logs usable memory regions plus a total usable-after-exit summary in QEMU/OVMF.
- The UEFI sign-of-life app now successfully locates GOP, logs framebuffer base/size plus resolution/pixel format, and writes a tiny test pattern when the framebuffer layout is directly usable.
- The UEFI sign-of-life app now successfully opens `\ghOSt\kernel.bin` from the staged ESP, reads it into loader-owned memory, and validates the first ghOSt kernel image header plus explicit x86-64 boot-info entry ABI in QEMU/OVMF.
- The Phase 1 loader now prepares a ghOSt-owned boot-info handoff struct containing payload address/size, framebuffer details, and the latest memory-map buffer plus map key, and logs that summary in QEMU/OVMF.
- The Phase 1 loader now exits boot services, transfers control into the staged payload, and provides a dedicated kernel stack in the boot-info handoff.
- The staged `kernel.bin` payload is now built from an assembly entry shim plus a freestanding C initialization stub, and it validates boot info while remaining headless-safe when no framebuffer is present.
- The boot-info ABI now makes ownership explicit: the kernel owns the loaded payload buffer, final memory-map buffer, and early stack after the jump, while framebuffer metadata remains a borrowed MMIO description.
- `tools/phase1/windows/New-ghOStUefiImage.ps1` now stages packaging through a fixed temporary VHD because Windows `diskpart` cannot attach the raw `.img` directly and rejects sparse VHDs before conversion.
- The elevated packaged-image path has been hardened to boot deterministically under QEMU/OVMF: the headless launcher now resets stale OVMF firmware state, writes serial output to a log file instead of binding it to interactive `stdio`, and `ghost-uefi.img` reaches `exit_boot_services ok`, `kernel_entry jumping`, and `kernel_stage0 halted` under `debugcon`.

**Remaining work:**

- move the ownership-aware handoff buffers toward a deliberate page-aligned physical allocation policy instead of convenient loader-pool placement.

**Also added:**

- `tools/phase1/windows/New-ghOStUefiImage.ps1` — packages staged ESP into a real GPT/FAT32 `.img` disk image.
- `tools/phase1/windows/Start-ghOStGdb.ps1` — attaches LLDB to the QEMU GDB stub for source-level debugging.
- `tools/phase0/windows/Start-ghOStQemu.ps1` — now accepts `-Gdb` and `-GdbWait` switches.
- `docs/PHASE1_BRINGUP.md` — exact build, boot, and debug workflow documented.

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

**Description:** PCI/PCIe enumeration is the foundation for AHCI, NVMe, USB (xHCI), GPU, and NIC drivers — nothing works without it. PCIe is the native bus on all modern x86-64 hardware; legacy PCI is a VM/fallback path only. Design the enumeration subsystem: ECAM discovery via MCFG ACPI table, extended config space (4096 bytes per function), capability linked-list walking (MSI, MSI-X, PCIe cap, Power Management, AER), BAR sizing/allocation (including 64-bit BARs), bridge recursion, multi-function detection, and device-to-driver matching. Must also handle PCIe error reporting (AER), max payload/read request size negotiation, and legacy CF8h/CFCh fallback. Must be correct before any PCI-based driver is attempted. This is also the path that underpins **M.2 NVMe** and **U.2 NVMe** devices, since those are form factors built on PCIe + NVMe rather than separate OS-level protocols. See Bible Section 5.2.

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

**Description:** AHCI is the primary SATA storage path and a notorious source of OS-dev bugs. Before writing driver code, review and plan for every pitfall cataloged in Bible Section 5.3.2: IDE mode detection, PI-based port enumeration, DMA alignment, command engine stop/start ordering, COMRESET timing, FIS construction, 64-bit DMA verification, spurious interrupts, NCQ/non-NCQ separation, error recovery via port reset, watchdog timeouts, and ATAPI detection. The driver must be tested on QEMU AHCI emulation first, then verified on real hardware. This also covers **M.2 SATA** devices, since M.2 in that configuration is a SATA/AHCI transport choice rather than a separate storage protocol. See Bible Sections 5.3.1–5.3.4.

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

### ISS-0038: Design microkernel IPC semantics and capability transfer

| Field    | Value              |
| -------- | ------------------ |
| Status   | **Open**           |
| Priority | **P0**             |
| Category | `kernel`           |
| Phase    | Phase 2–3          |
| Created  | 2026-03-08         |
| Assignee | AnotherLaughingMan |

**Description:** Define the core IPC model for the microkernel: synchronous vs asynchronous message passing, reply semantics, blocking rules, timeout behavior, handle/capability transfer, ownership rules, and security validation at the syscall boundary. This is now a critical-path design because the microkernel architecture depends on IPC for driver servers, filesystem servers, and system services. See Bible Sections 5.0 and 17.4, plus ADR-0002.

**Initial deliverables:**

- syscall surface for send/receive/reply and handle transfer;
- message buffer ownership and memory-mapping rules;
- blocking, timeout, cancellation, and deadlock-avoidance policy;
- capability/handle validation and revocation model;
- debugging and tracing requirements for IPC failures.

---

### ISS-0039: Design user-space driver server lifecycle and restart policy

| Field    | Value              |
| -------- | ------------------ |
| Status   | **Open**           |
| Priority | **P1**             |
| Category | `drivers`          |
| Phase    | Phase 4–5          |
| Created  | 2026-03-08         |
| Assignee | AnotherLaughingMan |

**Description:** Define how user-space driver servers are launched, supervised, restarted, detached, and recovered after failure. Scope includes: probe/attach sequencing, device ownership handoff, crash detection, restart eligibility, state cleanup, DMA/IRQ resource revocation, and the line between automatic recovery and manual intervention. See Bible Sections 5.0, 5.1, and 17.6, plus ADR-0002 and Appendix C, item 2.

**Initial deliverables:**

- driver server state machine from registration through detach;
- supervisor responsibilities and restart policy classes;
- resource cleanup rules for MMIO, IRQs, DMA buffers, and outstanding I/O;
- policy for when a failed driver server may be restarted automatically;
- observability requirements: logs, crash reports, and operator-visible device state.

---

### ISS-0040: Design filesystem server and VFS boundary

| Field    | Value              |
| -------- | ------------------ |
| Status   | **Open**           |
| Priority | **P1**             |
| Category | `filesystem`       |
| Phase    | Phase 4–5          |
| Created  | 2026-03-08         |
| Assignee | AnotherLaughingMan |

**Description:** Define the boundary between the Virtual Filesystem service and concrete filesystem/storage servers in the microkernel architecture. Scope includes: namespace ownership, mount routing, path resolution across IPC boundaries, file-handle representation, caching boundaries, permission checks, and the split between VFS policy and filesystem implementation. See Bible Sections 5.0, 17.5, and ADR-0002, plus Appendix C, item 1.

**Initial deliverables:**

- VFS service responsibilities versus filesystem server responsibilities;
- mount, unmount, and namespace routing model;
- file-handle and descriptor model across IPC boundaries;
- caching and coherency strategy between VFS, block cache, and filesystem servers;
- failure behavior when a filesystem or storage server hangs, exits, or is restarted.

---

## Resolved Issues

### ISS-0002: Decide kernel architecture (monolithic vs micro vs hybrid)

| Field           | Value                                                        |
| --------------- | ------------------------------------------------------------ |
| Status          | **Resolved**                                                 |
| Priority        | **P0**                                                       |
| Category        | `architecture`                                               |
| Phase           | Phase 0                                                      |
| Created         | 2026-03-07                                                   |
| Resolved        | 2026-03-08                                                   |
| Assignee        | AnotherLaughingMan                                           |
| Resolution Link | `docs/decisions/ADR-0002-kernel-architecture-microkernel.md` |

**Resolution:** ghOSt will use a **microkernel architecture**. The privileged kernel is intentionally minimal and owns low-level CPU, memory, scheduling, IPC, and security-boundary enforcement. Drivers, filesystems, and higher-level OS services run as user-space servers by default. The rationale and consequences are recorded in ADR-0002, and the Development Bible architecture sections were updated to match.

---

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
