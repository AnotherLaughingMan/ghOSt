# ghOSt Development Bible

> **This is the canonical reference for the ghOSt operating system project.**
> It defines what ghOSt is, why it exists, how it is built, and the principles
> that govern every decision made in this codebase.
>
> This is a living document. Treat it with the same seriousness as production code.
> Every contributor is expected to read, understand, and follow it.

---

## Document Governance

| Field             | Value                                           |
| ----------------- | ----------------------------------------------- |
| Status            | **Active — Living Document**                    |
| Created           | 2026-03-07                                      |
| Last major review | 2026-03-07                                      |
| Owner             | **AnotherLaughingMan** (sole owner & developer) |
| Location          | `/docs/DEVELOPMENT_BIBLE.md`                    |

### Rules for Editing This Document

1. Changes to this document are architectural decisions. Treat them as such.
2. Do not remove content without recording why and when (append a dated note or move to a "Superseded" appendix).
3. New sections must include rationale, not just rules.
4. Keep language precise. Avoid vague aspirations; state constraints.
5. When in doubt, add context rather than remove it.

---

## Table of Contents

1. [What Is ghOSt](#1-what-is-ghost)
2. [Why ghOSt Exists](#2-why-ghost-exists)
3. [Core Philosophy](#3-core-philosophy)
4. [Design Principles](#4-design-principles)
5. [System Architecture Overview](#5-system-architecture-overview)
6. [Deployment Profiles](#6-deployment-profiles)
7. [Language & Technology Boundaries](#7-language--technology-boundaries)
8. [Memory Discipline](#8-memory-discipline)
9. [Boot Architecture](#9-boot-architecture)
10. [Security & Privacy Architecture](#10-security--privacy-architecture)
11. [Configuration Architecture](#11-configuration-architecture)
12. [UX Architecture (Future)](#12-ux-architecture-future)
13. [Application Compatibility Strategy](#13-application-compatibility-strategy)
14. [Repository Structure](#14-repository-structure)
15. [Build System & Artifacts](#15-build-system--artifacts)
16. [Versioning Architecture](#16-versioning-architecture)
17. [Development Roadmap](#17-development-roadmap)
18. [Decision Records](#18-decision-records)
19. [Contribution Standards](#19-contribution-standards)
20. [Glossary](#20-glossary)
21. [Appendices](#21-appendices)

---

## 1. What Is ghOSt

**ghOSt** (pronounced "Ghost OS") is a custom, open-source operating system built from scratch.

It ships as two distinct profiles from a single codebase:

- **Minimal Rescue Profile**: a floppy/USB-bootable CLI environment for quick system operations (formatting, diagnostics, recovery).
- **Full Install Profile**: a complete desktop/server operating system with graphical UX, broad application compatibility, and full COM support.

ghOSt is not a Linux distribution, not a fork of an existing OS, and not a wrapper. It is a ground-up operating system with its own kernel, boot path, driver model, and user experience.

**Native architecture: 64-bit (x86-64).** The kernel, drivers, and all system services run natively in 64-bit mode. A built-in 32-bit compatibility subsystem (name TBD) enables 32-bit application execution on the Full Install Profile, similar in concept to how Windows runs 32-bit apps on 64-bit — but with ghOSt's own design and branding.

---

## 2. Why ghOSt Exists

ghOSt exists because:

1. **User autonomy is under threat.** Modern operating systems increasingly serve corporate and governmental interests over user interests. Mandatory telemetry, forced updates, age verification gates, and content censorship are becoming normalized. ghOSt rejects this trajectory.

2. **Users deserve control over their own data.** Data should be owned by the user, encrypted by default where practical, and never exfiltrated without explicit informed consent. No silent telemetry. No phoning home.

3. **Censorship resistance matters.** An operating system should not be a chokepoint for restricting what users can access, run, or communicate. ghOSt is designed so that no single authority can gate access to the user's own machine.

4. **Small, understandable systems are better systems.** The industry trend toward massive, opaque OS codebases is a liability. ghOSt starts small and grows deliberately, so every component can be understood, audited, and trusted.

5. **There should be an OS that answers to its users, not to advertisers, regulators, or platform gatekeepers.**

These are not aspirations. They are requirements. Every design decision in ghOSt must be evaluated against these goals.

---

## 3. Core Philosophy

### 3.1 User Sovereignty

The user is the highest authority on their own machine. ghOSt will not:

- Enforce content restrictions on behalf of third parties.
- Require identity verification as a precondition for basic OS functionality.
- Phone home, transmit telemetry, or communicate with external services without explicit user action.
- Prevent the user from modifying, inspecting, or replacing any part of the system.

### 3.2 Privacy by Architecture

Privacy is not a feature bolted on after the fact. It is a structural property of the system:

- Local-first operation by default.
- Minimal data collection surface.
- No mandatory accounts, registrations, or identity bindings.
- Encryption and key ownership at the user level.

### 3.3 Transparency

- The system is fully open source.
- Behavior must be explainable from the code. No hidden or obfuscated logic.
- Configuration drives behavior. Magic values and undocumented side effects are defects.

### 3.4 Deliberate Growth

- Start extremely small. Validate every layer before adding the next.
- Resist the temptation to build big early.
- Every subsystem earns its place by solving a defined problem within budget (size, complexity, dependency count).

### 3.5 Resilience

- The system should be hard to break, hard to censor, and easy to recover.
- Minimal Rescue Profile exists precisely so the user always has a way back in.
- No single point of failure in update, boot, or access paths.

---

## 4. Design Principles

These principles apply to all code and architecture decisions project-wide.

### 4.1 Simplicity Over Cleverness

- Prefer explicit, readable code.
- Avoid abstraction layers that don't pay for themselves.
- If a junior contributor can't follow the logic, it's too complex.

### 4.2 Modularity Is Mandatory

- Every subsystem must have clear boundaries, inputs, and outputs.
- No subsystem may assume the existence of another unless explicitly declared as a dependency.
- Kernel internals must not leak into UX. UX must not assume kernel internals.

### 4.3 Size and Performance Are Constraints, Not Goals

- Binary size matters because it determines what media the system can boot from.
- Performance matters because it determines whether the system is usable on real hardware.
- Both are hard constraints with measurable budgets, not vague "nice to haves."

### 4.4 Fail Fast, Fail Loud

- In debug builds, catch everything. Assert aggressively.
- In release builds, fail safely with clear diagnostics where feasible.
- Never silently swallow errors in kernel-critical paths.

### 4.5 Configuration Over Hardcoding

- User-facing behavior must be driven by configuration (JSON).
- Defaults must be sane and documented.
- Hardcoded behavior is a defect unless explicitly justified and documented as an exception.

### 4.6 Security Is Structural

- Security boundaries are architectural decisions, not afterthoughts.
- Every interface that crosses a trust boundary must be validated.
- Least-privilege is the default for all access patterns.

### 4.7 Document Everything That Matters

- Architecture decisions go in `/docs/decisions/`.
- Memory layouts, hardware assumptions, and trust boundaries go near the code they affect.
- If it's not documented, it's not a decision — it's an accident.

### 4.8 No Scaffolding in Kernel Space

Kernel code is too sensitive for placeholder implementations. Every function that is committed to the kernel must be **fully implemented, tested, and correct** — not stubbed, not scaffolded, not "to be completed later."

- **No TODO stubs in kernel-space code.** If a function exists, it must work.
- **No empty function bodies** that silently return success. If a capability doesn't exist yet, the function should not exist yet.
- **No partial implementations that paper over missing logic.** A half-implemented memory allocator is worse than no memory allocator — it creates a false sense of safety.
- **Stubs in userland are acceptable** when clearly marked and gated behind feature flags. The kernel gets no such leniency.
- **If it's not done, don't ship it.** Remove unfinished code rather than committing it behind a "will finish later" comment.

**Rationale:** In kernel space, every code path can be reached by hardware interrupts, syscalls, or error conditions. A stub that returns `0` instead of actually handling a page fault will crash the system or create a security hole. The kernel is the foundation — there is no safety net beneath it.

---

## 5. System Architecture Overview

```
┌─────────────────────────────────────────────────────┐
│                    User Space                        │
│  ┌───────────┐  ┌──────────┐  ┌──────────────────┐  │
│  │  CLI Shell │  │ GUI (UX) │  │  Applications    │  │
│  │ (Rescue +  │  │ (Full    │  │  (Native + COM   │  │
│  │  Full)     │  │  only)   │  │   Full only)     │  │
│  └─────┬─────┘  └────┬─────┘  └────────┬─────────┘  │
│        │              │                 │             │
│  ┌─────┴──────────────┴─────────────────┴──────────┐ │
│  │            System Services / APIs                │ │
│  │         (COM runtime, IPC, config, etc.)         │ │
│  └──────────────────────┬──────────────────────────┘ │
├─────────────────────────┼────────────────────────────┤
│                   Kernel Space                       │
│  ┌──────────────────────┴──────────────────────────┐ │
│  │              Kernel Core (C + Assembly)          │ │
│  │  ┌──────────┐  ┌──────────┐  ┌───────────────┐ │ │
│  │  │ Memory   │  │ Scheduler│  │ IPC / Syscall  │ │ │
│  │  │ Manager  │  │          │  │ Interface      │ │ │
│  │  └──────────┘  └──────────┘  └───────────────┘ │ │
│  │  ┌──────────┐  ┌──────────┐  ┌───────────────┐ │ │
│  │  │ VFS      │  │ Security │  │ Device Manager │ │ │
│  │  │          │  │ Module   │  │                │ │ │
│  │  └──────────┘  └──────────┘  └───────────────┘ │ │
│  └─────────────────────────────────────────────────┘ │
│  ┌─────────────────────────────────────────────────┐ │
│  │        Hardware Abstraction Layer (HAL)          │ │
│  └──────────────────────┬──────────────────────────┘ │
├─────────────────────────┼────────────────────────────┤
│  ┌──────────────────────┴──────────────────────────┐ │
│  │     Bootloader / UEFI Entry (Assembly + C)      │ │
│  └─────────────────────────────────────────────────┘ │
├─────────────────────────────────────────────────────┤
│                    Hardware                          │
└─────────────────────────────────────────────────────┘
```

This diagram is aspirational and will evolve. Not all components exist yet.
The layering is intentional: each boundary is a potential security and modularity boundary.

### 5.1 Driver Model

ghOSt uses a **register-probe-attach** driver model. Every driver is a kernel-mode module that:

1. **Registers** itself with the Device Manager, declaring which bus types and device IDs it can handle.
2. **Probes** a candidate device when the bus enumerator discovers a matching device. Probe must be non-destructive — it verifies the device is actually present and compatible, then returns success or failure.
3. **Attaches** to the device on successful probe, taking ownership of that device's resources (MMIO regions, IRQs, DMA channels).

| Principle                   | Rule                                                                                                  |
| --------------------------- | ----------------------------------------------------------------------------------------------------- |
| One driver per device       | No two drivers may claim the same device simultaneously.                                              |
| Explicit resource ownership | A driver must request and receive exclusive access to MMIO, I/O ports, and IRQ lines from the kernel. |
| No blind probing            | Drivers must not touch hardware registers before the bus enumerator has matched them.                 |
| Fail-safe detach            | Every driver must implement a `detach` path that releases all resources cleanly.                      |
| No blocking in probe        | Probe must complete quickly. Lengthy initialization happens in attach.                                |
| DMA only through kernel API | Drivers never compute physical addresses themselves. All DMA buffers go through the kernel allocator. |

### 5.2 Bus Enumeration (PCI/PCIe)

PCI Express (PCIe) is the primary interconnect on all modern x86-64 hardware. Legacy parallel PCI is effectively dead — every AHCI controller, NVMe drive, USB host, GPU, and NIC on a modern system sits behind PCIe. ghOSt treats **PCIe as the native bus** and supports legacy PCI configuration only as a fallback for virtual machines and older hardware.

#### 5.2.1 PCI vs PCIe — What's Different

| Aspect           | Legacy PCI                                             | PCIe                                                               |
| ---------------- | ------------------------------------------------------ | ------------------------------------------------------------------ |
| Topology         | Shared parallel bus                                    | Point-to-point serial links with switches                          |
| Config space     | 256 bytes per function (I/O port access via CF8h/CFCh) | 4096 bytes per function (memory-mapped via ECAM)                   |
| Discovery        | Port I/O config mechanism 1                            | ECAM (Enhanced Configuration Access Mechanism) via MCFG ACPI table |
| Interrupts       | INTx (shared, level-triggered)                         | MSI / MSI-X (dedicated, edge-triggered, scalable)                  |
| Bandwidth        | 133 MB/s shared                                        | Per-lane: 250 MB/s (Gen1) to 3.9 GB/s (Gen5), per direction        |
| Error reporting  | PCI Status register (limited)                          | AER (Advanced Error Reporting) capability structure                |
| Power management | None / PME# pin                                        | PCIe PM capability + ASPM (Active State Power Management)          |
| Hot-plug         | Rare                                                   | Native support via hot-plug capability                             |

**ghOSt must support both config mechanisms** because QEMU and some older systems expose legacy PCI. But all new driver work targets PCIe capabilities.

#### 5.2.2 ECAM and Extended Config Space

On PCIe systems, the MCFG ACPI table provides the base address for the ECAM (Enhanced Configuration Access Mechanism) region — a memory-mapped window that gives direct access to the full 4096-byte config space of every function.

**ECAM address formula:** `base + (bus << 20) | (device << 15) | (function << 12) + register`

**Rules:**

- **Always check for MCFG first.** If present, use ECAM for all config access. Fall back to CF8h/CFCh only if MCFG is absent (legacy PCI or old VMs).
- **Map the ECAM region as uncacheable (UC) in the page tables.** Config space is a hardware register window — caching it causes stale reads.
- **The extended config space (bytes 256–4095) is only accessible via ECAM.** Legacy CF8h/CFCh can only reach the first 256 bytes. PCIe capability structures live in the extended range.
- **Validate MCFG entries.** Each entry specifies a bus range and base address. Do not assume a single entry covers all buses.

#### 5.2.3 PCIe Capability Structures

PCIe devices advertise their features through a linked list of capability structures starting at the offset in config byte 34h (Capabilities Pointer). Extended capabilities (PCIe-specific) start at offset 100h in the extended config space.

**Capabilities ghOSt must parse:**

| Capability             | ID    | Why it matters                                                                        |
| ---------------------- | ----- | ------------------------------------------------------------------------------------- |
| **MSI**                | 05h   | Message Signaled Interrupts — allocate vectors, prefer over INTx                      |
| **MSI-X**              | 11h   | Scalable MSI with per-vector masking — preferred for multi-queue devices (NVMe, xHCI) |
| **PCIe**               | 10h   | Device/port type, link speed/width, max payload size, max read request size           |
| **Power Management**   | 01h   | D-states (D0–D3), PME support — required for sleep/wake                               |
| **AER** (extended)     | 0001h | Advanced Error Reporting — correctable/uncorrectable error logging and masking        |
| **ASPM** (in PCIe cap) | —     | Active State Power Management — L0s/L1 link states for power savings                  |
| **SR-IOV** (extended)  | 0010h | Single Root I/O Virtualization — future, if VM support is planned                     |

**Rules:**

- **Walk the entire capability list at enumeration time.** Cache the offsets of capabilities each driver will need.
- **Never hard-code capability offsets.** They vary by device. Always follow the linked list.
- **Require MSI or MSI-X for all new drivers.** INTx is a fallback only for legacy PCI devices.
- **Read and respect Max Payload Size and Max Read Request Size** from the PCIe capability. Violations cause malformed TLP errors that are hard to diagnose.

#### 5.2.4 Enumeration Sequence

1. Parse MCFG ACPI table. If present, map ECAM regions. Otherwise, use legacy CF8h/CFCh.
2. Walk PCI configuration space (bus 0–255, device 0–31, function 0–7).
3. For each discovered function, read vendor ID, device ID, class code, subclass, prog-if, and header type.
4. Record all BARs (Base Address Registers). DO NOT write to BARs unless the kernel is performing resource allocation.
5. Walk the capability linked list (offset 34h). Record MSI/MSI-X capability offset. If ECAM is available, also walk extended capabilities at offset 100h+.
6. Identify bridge devices (header type 01h) and recursively enumerate subordinate buses.
7. Match discovered devices against registered drivers. Call probe on match.

**Hard rules:**

- **Never assume bus topology.** Enumerate; don't guess. PCIe switches create non-obvious hierarchies.
- **BAR sizing uses the standard read-write-read method.** Write all-ones, read back, mask, restore. Do this with interrupts disabled for the device.
- **MSI/MSI-X over legacy INTx.** Always prefer message-signaled interrupts when the device supports them. Allocate MSI-X vectors proportional to the device's queue count.
- **Cache enumeration results.** Walk the bus once at boot; don't re-scan unless hot-plug is involved.
- **Respect 64-bit BARs.** A BAR pair (BAR[n] + BAR[n+1]) can form a 64-bit address. Check the BAR type field before interpreting.
- **Do not ignore multi-function devices.** If header type bit 7 is set on function 0, check functions 1–7.

#### 5.2.5 PCIe Error Handling

On PCIe, errors are categorized as correctable or uncorrectable (non-fatal / fatal). AER capability exposes registers for each category.

**Minimum requirements:**

- **Enable AER if the capability is present.** Mask correctable errors in production (they're informational). Log uncorrectable errors and trigger device reset on fatal errors.
- **Check Device Status register (offset 06h in PCIe capability) for error flags.** These are set even without AER.
- **Fatal errors require link reset.** A Secondary Bus Reset (SBR) through the parent bridge, or a Function-Level Reset (FLR) if supported.
- **Log all PCIe errors with the full BDF (bus:device.function) identifier** so the failing device is immediately identifiable.
- **Completion Timeout (CTO) is the most common PCIe error in OS development.** It means the device didn't respond to a memory read. Causes: wrong BAR mapping, device in D3 power state, or device not properly initialized. Always check for this first.

### 5.3 AHCI Architecture and Known Pitfalls

AHCI (Advanced Host Controller Interface) is the standard register-level interface for SATA controllers. It is the primary storage path for spinning disks and many SSDs. The AHCI specification (Intel, publicly available) is the authoritative reference — but specs do not warn you about the real-world bugs.

**This section exists to prevent us from debugging the same AHCI pitfalls that every OS project hits.**

#### 5.3.1 AHCI Initialization Sequence

The init sequence is order-sensitive. Violating this order causes silent data corruption or controller hangs.

```
1. Discover AHCI controller via PCI enumeration (class 01h, subclass 06h, prog-if 01h).
2. Map the HBA MMIO region (BAR 5 / ABAR).
3. Read GHC.AE (AHCI Enable) — set it if not already set. Some BIOSes leave the controller in legacy IDE mode.
4. Read CAP register — determine port count, command slot count, 64-bit DMA support, NCQ support.
5. Read PI (Ports Implemented) — only touch ports whose bits are set in PI. Do NOT enumerate based on port count alone.
6. For each implemented port:
   a. Stop the port command engine (clear PxCMD.ST, wait for PxCMD.CR to clear; clear PxCMD.FRE, wait for PxCMD.FR to clear).
   b. Allocate and set PxCLB (Command List Base) — must be 1024-byte aligned.
   c. Allocate and set PxFB (FIS Base) — must be 256-byte aligned.
   d. Clear PxSERR (write all-ones to clear).
   e. Clear PxIS (write all-ones to clear pending interrupts).
   f. Enable desired interrupts in PxIE.
   g. Start the port (set PxCMD.FRE first, then PxCMD.ST).
7. Enable global interrupts: GHC.IE = 1.
8. Optionally perform COMRESET on each port to detect attached devices (check PxSSTS.DET for device presence).
```

#### 5.3.2 Known Pitfalls (MUST READ before writing AHCI code)

| Pitfall                             | What goes wrong                                                                                                                                                                                                         | How to prevent it                                                                                                                                                                                                                     |
| ----------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **IDE mode at boot**                | BIOS/firmware leaves AHCI controller in legacy IDE emulation mode. Your AHCI driver finds nothing.                                                                                                                      | Check PCI prog-if. If it's 00h or 80h (IDE), check if the controller supports AHCI mode switching via a vendor-specific PCI config register. Intel controllers use MAP register at offset 90h. **Do not assume AHCI mode is active.** |
| **Ports Implemented vs port count** | CAP.NP reports max ports, but only ports with bits set in the PI register are real. Touching phantom ports hangs the controller.                                                                                        | **Always use PI, never NP alone.** Iterate only over set bits in PI.                                                                                                                                                                  |
| **Alignment violations**            | Command list (PxCLB) must be 1024-byte aligned. Received FIS (PxFB) must be 256-byte aligned. Command tables must be 128-byte aligned. PRDTs within command tables must not cross 64 KB boundaries on some controllers. | Use the kernel's aligned DMA allocator. Verify alignment with assertions. Never allocate these structures on the stack.                                                                                                               |
| **Stopping the command engine**     | Clearing PxCMD.ST while PxCMD.CR is still 1 (commands running) causes undefined behavior on some controllers.                                                                                                           | Clear ST, then **poll CR until it reads 0** with a timeout (500ms per AHCI spec). Same for FRE/FR. If it doesn't clear, port reset is required.                                                                                       |
| **COMRESET timing**                 | Some drives need time after COMRESET before they're ready. Issuing commands too early gets garbage status.                                                                                                              | After COMRESET, poll PxSSTS.DET for value 3h (device present, PHY established). Then wait for PxTFD.BSY and PxTFD.DRQ to clear. Add a reasonable delay (spec says up to 10ms, but some drives need more).                             |
| **FIS construction errors**         | Incorrect FIS type, wrong byte ordering, missing fields in Register H2D FIS.                                                                                                                                            | Use a well-tested FIS builder function. A Register H2D FIS is exactly 20 bytes. The FIS type byte is 27h. The command/control bit is bit 7 of byte 1. Verify every field.                                                             |
| **64-bit DMA**                      | Controller advertises 64-bit support (CAP.S64A) but firmware configured it for 32-bit only, or the controller ignores upper 32 bits for certain structures.                                                             | If CAP.S64A is set, you may use 64-bit addresses. But test it — allocate a DMA buffer above 4 GB and verify a read/write works. If not, fall back to allocating all AHCI DMA buffers below 4 GB.                                      |
| **Spurious interrupts**             | Controller fires interrupts during initialization before your handler is ready, or fires interrupts for ports you haven't initialized.                                                                                  | Clear PxIS and PxSERR for all ports before global interrupt enable. Register your interrupt handler before setting GHC.IE. Ignore interrupts for ports you haven't initialized.                                                       |
| **NCQ vs non-NCQ mixing**           | AHCI supports both queued (NCQ/FPDMA) and non-queued commands, but they must not be mixed in the same command slot sequence. Mixing causes command ordering violations.                                                 | Track whether the port is in NCQ mode or non-NCQ mode. Drain one before switching to the other. Start with non-NCQ; add NCQ support later as an optimization.                                                                         |
| **Error recovery**                  | A failed command can leave the port in an error state. If you don't reset the port properly, all subsequent commands fail.                                                                                              | On error: stop command engine → clear PxSERR → clear PxIS → COMRESET → restart. Re-issue failed commands. Log the error register contents for diagnosis.                                                                              |
| **Timeout without recovery**        | A command sits in a slot forever. No interrupt fires. The driver hangs waiting.                                                                                                                                         | Every issued command must have a watchdog timeout. If the timeout expires, trigger error recovery (port reset). Never block indefinitely on command completion.                                                                       |
| **ATAPI vs ATA confusion**          | SATA port may have an ATAPI device (optical drive). The PxSIG register gives the device signature after reset — ATA is 0x00000101, ATAPI is 0xEB140101. If you send ATA commands to an ATAPI device, it will NAK.       | Check PxSIG after COMRESET and device detection. Route ATAPI devices to a separate command path (PACKET commands).                                                                                                                    |

#### 5.3.3 DMA Design Rules for AHCI

- **All AHCI DMA structures must be allocated from physically contiguous, cache-coherent memory.** This includes the command list, received FIS area, command tables, and PRDT data buffers.
- **PRDT entries may not cross a 4 MB boundary** (hardware limit per entry). Keep PRDT entries conservative — one physical page per entry is safe and simple.
- **Maximum PRDT length per command table is 65535 entries** (spec limit), but sane drivers use far fewer. A single 4 KB-aligned page per entry means one PRDT entry per page — simple and cache-friendly.
- **Zero every DMA buffer before first use.** Prevents leaking stale data if the controller reads before you fill the buffer.
- **Never reuse a command slot until the controller has finished with it.** Check PxCI (command issued) bit for the slot — if it's still set, the controller owns that memory.
- **Ensure DMA buffers remain pinned (not swappable) for the duration of the command.** Page eviction during an active DMA transfer causes data corruption.

#### 5.3.4 Testing Strategy for AHCI

- **Primary test target: QEMU with AHCI emulation.** QEMU's ICH9-AHCI is a well-tested reference. Enable it with `-device ahci,id=ahci -device ide-hd,drive=disk,bus=ahci.0`.
- **Verify with a real SATA drive** as early as possible. QEMU hides many timing and alignment bugs.
- **Test hot-plug** (connect/disconnect a SATA device while running) — even if hot-plug isn't supported initially, the driver must not crash.
- **Test with drives behind port multipliers** if port multiplier support is planned.
- **Stress test: issue thousands of I/O operations** and verify data integrity end-to-end with checksums.
- **Test error paths explicitly:** simulate drive removal mid-command, inject timeout scenarios, corrupt a FIS and verify recovery.

### 5.4 Modern Platform Standards

ghOSt targets modern x86-64 hardware — desktop and server. The following platform-level standards are required for correct operation on contemporary systems. Ignoring them means poor performance, security gaps, or outright incompatibility with server-class hardware.

#### 5.4.1 IOMMU (Intel VT-d / AMD-Vi)

An IOMMU translates device-initiated DMA addresses through page tables, providing hardware-enforced DMA isolation. Without it, any bus-mastering device can read/write any physical memory — a catastrophic security hole.

| Requirement                   | Detail                                                                                                                                                                 |
| ----------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Detection                     | Parse ACPI DMAR table (Intel VT-d) or IVRS table (AMD-Vi). If neither is present, IOMMU is unavailable.                                                                |
| Enablement                    | Enable translation by programming the IOMMU hardware registers. Must happen before any device performs DMA.                                                            |
| DMA remapping                 | Every device gets a restricted DMA address space. Only pages explicitly mapped by the kernel are accessible to that device.                                            |
| Identity mapping (early boot) | During early init before the full IOMMU driver is up, use a 1:1 identity map so UEFI-started devices continue working. Replace with restricted maps as drivers attach. |
| Interrupt remapping           | IOMMU can remap MSI/MSI-X interrupts too — prevents devices from forging interrupt vectors. Enable this.                                                               |
| Fault logging                 | IOMMU faults (device tried to access unmapped address) must be logged with the BDF of the offending device, the faulting address, and access type.                     |
| Passthrough mode              | For debugging or performance-critical paths, allow per-device passthrough (no translation) — but never as the default.                                                 |

**Why this is non-negotiable:** DMA attacks are a real threat (Thunderbolt/PCIe, compromised firmware, malicious peripherals). Any OS claiming security must enforce DMA isolation.

#### 5.4.2 APIC Architecture (APIC / x2APIC / IOAPIC)

The interrupt delivery architecture on x86-64 has three components: the Local APIC (per-core), the IOAPIC (for routing external interrupts), and optionally x2APIC (for scalability beyond 255 logical processors).

| Component  | Purpose                                                                                  | ghOSt requirement                                                                                      |
| ---------- | ---------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------ |
| Local APIC | Per-CPU interrupt receipt, IPI delivery, timer                                           | **Required.** Must detect and initialize on every core.                                                |
| IOAPIC     | Routes pin-based interrupts (legacy IRQs, HPET, etc.) to Local APICs                     | **Required.** Parse MADT for IOAPIC entries. Program redirection table entries.                        |
| x2APIC     | Extended APIC mode — MSR-based access, 32-bit APIC IDs, supports >255 logical processors | **Required on systems that support it.** Check CPUID bit 21 (ECX of leaf 01h) and MADT x2APIC entries. |

**Rules:**

- **Disable the legacy 8259 PIC.** Mask all 8259 interrupts and remap them out of the way before enabling APIC.
- **Use MADT (APIC table) for topology discovery.** Never hard-code APIC IDs or assume contiguous numbering.
- **Switch to x2APIC mode when available.** x2APIC uses MSRs instead of MMIO — it's faster and mandatory for systems with >255 cores.
- **Support IPI (Inter-Processor Interrupt)** for SMP startup, TLB shootdowns, and cross-core scheduling notifications.

#### 5.4.3 NUMA Awareness

Non-Uniform Memory Access (NUMA) is the memory architecture on all modern multi-socket and many single-socket server systems. Each CPU socket (or chiplet) has "local" memory that's fast, and "remote" memory attached to other sockets that's slower.

**Why it matters:** A NUMA-unaware allocator will randomly place memory pages, causing 2–4x latency penalties for cross-socket accesses. On a 4-socket server, this makes the system unusable.

| Requirement             | Detail                                                                                                                            |
| ----------------------- | --------------------------------------------------------------------------------------------------------------------------------- |
| SRAT table parsing      | Parse the ACPI SRAT (System Resource Affinity Table) to discover proximity domains and the CPU-to-memory mapping.                 |
| SLIT table parsing      | Parse ACPI SLIT (System Locality Information Table) for inter-node distance metrics.                                              |
| Per-node memory pools   | The physical memory allocator must maintain per-NUMA-node free lists.                                                             |
| Local-first allocation  | Default allocation policy: allocate from the NUMA node local to the requesting CPU.                                               |
| Interleave fallback     | If the local node is exhausted, fall back to the nearest node (by SLIT distance), not a random one.                               |
| Process/thread affinity | The scheduler should prefer running threads on CPUs in the same NUMA domain as their memory allocations.                          |
| Single-socket fallback  | If SRAT is absent (single-socket desktop), treat the entire system as one NUMA node. No code path may break in the non-NUMA case. |

#### 5.4.4 SMBIOS / DMI

SMBIOS (System Management BIOS) tables provide hardware inventory data: motherboard model, serial numbers, BIOS version, CPU socket count, memory DIMM configuration, chassis type, etc.

| Requirement           | Detail                                                                                                                                                             |
| --------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| Entry point           | Scan for SMBIOS 3.0 64-bit entry point (anchor `_SM3_`) in EFI system table or physical memory F0000h–FFFFFh. Fall back to SMBIOS 2.x (`_SM_`) if 3.0 isn't found. |
| Parsing               | Walk the structure table. Decode Type 0 (BIOS info), Type 1 (System info), Type 4 (Processor), Type 16/17 (Memory array/device).                                   |
| Exposure              | Expose parsed hardware info via a kernel API for userland tools (system inventory, diagnostic commands).                                                           |
| Server identification | SMBIOS chassis type (Type 3) distinguishes desktop, server, notebook, etc. This can inform auto-configuration defaults.                                            |

#### 5.4.5 ECC Memory Awareness

Server-class systems use ECC (Error-Correcting Code) memory. The memory controller can detect and correct single-bit errors and detect (but not correct) multi-bit errors. The OS must participate in monitoring this.

| Requirement                      | Detail                                                                                                                                                                          |
| -------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Detection                        | Check SMBIOS Type 16 (memory array error correction type) and Type 17 (memory device) for ECC capability.                                                                       |
| Machine Check Architecture (MCA) | Enable MCE (Machine Check Exception) and poll Machine Check banks for corrected memory errors. Log every corrected error with physical address and DIMM location (from SMBIOS). |
| Threshold monitoring             | Track corrected error counts per DIMM. Alert (via log + notification) if a DIMM exceeds a threshold — it's likely failing.                                                      |
| Uncorrectable errors             | An uncorrectable memory error triggers MCE. If the affected page is not in use, offline it. If it's in use, panic is the only safe option — data integrity is compromised.      |
| Non-ECC fallback                 | On desktop systems without ECC, skip memory error monitoring. No code path may assume ECC is present.                                                                           |

#### 5.4.6 Hardware Watchdog Timer

Servers and embedded systems require a hardware watchdog — a timer that reboots the machine if the OS stops responding. Unattended servers cannot afford indefinite hangs.

| Requirement       | Detail                                                                                                                                                                 |
| ----------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Detection         | Check ACPI WDAT (Watchdog Action Table) for a platform watchdog. Also check for iTCO watchdog on Intel chipsets (PCI device, usually on LPC/eSPI bus).                 |
| Kernel heartbeat  | Once enabled, the kernel must periodically reset (pet) the watchdog from a timer interrupt or heartbeat thread. If the kernel hangs, the watchdog fires.               |
| Timeout           | Configurable via JSON config. Default: 60 seconds. Must be long enough that normal heavy I/O doesn't trigger a false reboot.                                           |
| Disable path      | Allow explicit disable via kernel command-line or config for debugging (so developers don't get rebooted while single-stepping).                                       |
| Panic integration | On kernel panic, the kernel may choose to let the watchdog expire (triggering hardware reboot) or explicitly trigger reset. This is a policy decision (config-driven). |

#### 5.4.7 Headless / Serial Console Operation

Server installations often have no GPU, no keyboard, and no local display. ghOSt must be fully operational in headless mode.

| Requirement        | Detail                                                                                                                                                     |
| ------------------ | ---------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Serial console     | The kernel must support a serial console (COM port or UEFI serial) as the primary I/O channel for boot messages, kernel logs, login prompt, and CLI shell. |
| No GPU required    | The kernel must boot and reach a working CLI shell even if zero GPUs are detected. No code path may panic or stall on missing framebuffer.                 |
| SPCR table         | Parse ACPI SPCR (Serial Port Console Redirection) table for firmware-configured serial parameters (baud rate, port, flow control).                         |
| EFI console output | Use EFI ConOut (Simple Text Output Protocol) during early boot before the kernel's serial driver is up.                                                    |
| Remote management  | Out-of-band management (IPMI/BMC) is a future consideration. The serial console is the minimum viable server management interface.                         |

#### 5.4.8 Network Boot (PXE / HTTP Boot)

Server deployment depends on network boot. UEFI provides two mechanisms:

| Mechanism                           | How it works                                                                  | ghOSt support                                                                                        |
| ----------------------------------- | ----------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------- |
| PXE (Preboot Execution Environment) | UEFI downloads a boot image via TFTP/DHCP                                     | Support as a boot source. The ghOSt bootloader must be loadable as a PXE NBP (Network Boot Program). |
| HTTP Boot (UEFI 2.5+)               | UEFI downloads a boot image via HTTP/HTTPS from a URL provided by DHCP option | Preferred over PXE for modern deployments. Support when the UEFI firmware offers it.                 |

**Rules:**

- The ghOSt bootloader EFI binary must work when loaded by PXE or HTTP Boot — no changes to the binary itself.
- Network boot configuration (install server URL, kickstart-style configs) is handled by the PE installer, not the kernel.
- Network boot does **not** require the kernel to have its own network driver during early boot — UEFI firmware handles the network transfer.

#### 5.4.9 High-Speed Networking (Server NICs)

Desktop NICs (1 GbE) are well-served by simple ring-buffer drivers. Server NICs (10/25/40/100 GbE) require fundamentally different approaches.

| Feature              | Why it matters                                                                            | ghOSt requirement                                                                       |
| -------------------- | ----------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------- |
| Multi-queue (RSS)    | Distribute incoming packets across CPU cores via hardware hash                            | Allocate one RX/TX queue per core. Configure the RSS indirection table to match.        |
| Interrupt coalescing | Reduce interrupt overhead at high packet rates                                            | Support adaptive coalescing: batch completions, use timer-based moderation.             |
| Checksum offload     | Hardware computes TCP/UDP/IP checksums                                                    | Use hardware offload when available - do not re-compute in software.                    |
| TSO/GRO              | TCP Segmentation Offload / Generic Receive Offload — hardware handles packet segmentation | Essential for saturating 10 GbE+ links. Support when the NIC advertises the capability. |
| NAPI-style polling   | At high packet rates, switch from interrupt-driven to poll-mode to avoid interrupt storms | Design the NIC driver interface to support both interrupt and poll modes.               |
| Jumbo frames         | MTU > 1500 (up to 9000) for reduced per-packet overhead                                   | Support configurable MTU. Default remains 1500; jumbo is opt-in.                        |

**Initial target:** One well-documented 10 GbE NIC family. Intel X710/XL710 (i40e) has public datasheets. Virtio-net for VM testing.

---

## 6. Deployment Profiles

### 6.1 Minimal Rescue Profile

| Attribute       | Constraint                                              |
| --------------- | ------------------------------------------------------- |
| Target media    | Floppy disk image, USB thumb drive                      |
| Architecture    | 64-bit only (no 32-bit compat layer)                    |
| UX              | CLI-only command environment                            |
| Purpose         | Quick operations: format, diagnose, recover, inspect    |
| Feature scope   | Kernel + essential drivers + CLI shell + core utilities |
| Size budget     | As small as feasible (floppy = ~1.44 MB ceiling target) |
| Graphical stack | **Not included**                                        |
| COM runtime     | **Not included**                                        |
| Network         | Optional / minimal if included                          |

### 6.2 Full Install Profile

| Attribute       | Constraint                                         |
| --------------- | -------------------------------------------------- |
| Target media    | Hard drive, SSD, large USB                         |
| Architecture    | 64-bit native + built-in 32-bit compatibility      |
| UX              | Full graphical UX (browser-like) + CLI             |
| Purpose         | Daily-use desktop/server operating system          |
| Feature scope   | Full kernel + all drivers + GUI + app compat + COM |
| Size budget     | Practical but not unconstrained; justify bloat     |
| Graphical stack | **Required** (desktop) / **Optional** (server)     |
| COM runtime     | **Required** (long-term milestone)                 |
| Network         | Full networking stack                              |
| IOMMU           | **Enabled by default** when hardware supports it   |
| NUMA            | **Topology-aware** allocation and scheduling       |
| Headless mode   | **Supported** — serial console, no GPU required    |
| ECC monitoring  | **Active** on ECC-capable hardware                 |
| Watchdog        | **Available** — configurable via JSON config       |

**Server deployment note:** The Full Install Profile must be fully functional with no local display, no keyboard, and no mouse — controlled entirely over serial console or network. The graphical stack is a feature of the desktop sub-profile, not a hard requirement of Full Install. Server-class hardware features (IOMMU for DMA protection, NUMA for memory locality, ECC monitoring, hardware watchdog, multi-queue NICs, network boot) are first-class concerns, not afterthoughts. See Section 5.4 for the full specification.

### 6.3 Profile Rules

- Both profiles share the same kernel codebase.
- Feature selection is controlled by build-time profile gates and runtime config.
- Full Install features must never be required dependencies for Minimal Rescue.
- Every feature must declare which profile(s) it belongs to.

---

## 7. Language & Technology Boundaries

### 7.1 Kernel and Boot (both profiles)

| Layer              | Language     | Notes                                  |
| ------------------ | ------------ | -------------------------------------- |
| Bootloader / entry | Assembly + C | UEFI entry, early hardware init        |
| Kernel core        | C + Assembly | Memory, scheduling, IPC, VFS, security |
| HAL                | C            | Hardware abstraction                   |
| Early drivers      | C            | Essential device support               |

### 7.2 Userland and UX (Full Install only)

| Layer             | Language                  | Notes                        |
| ----------------- | ------------------------- | ---------------------------- |
| System services   | C / C++                   | COM runtime, IPC daemons     |
| GUI shell         | C++ and/or HTML/CSS/JS/TS | Browser-like customizable UX |
| UX customization  | HTML / CSS / JS / TS      | Themes, layouts, extensions  |
| App compatibility | C / C++                   | COM interfaces, API surfaces |
| Configuration     | JSON                      | All user-facing behavior     |

### 7.3 Boundary Rules

- Assembly and C are the **only** languages permitted in kernel space.
- C++ is permitted in userland system services and native UX components.
- HTML/CSS/JS/TS are permitted for UX rendering, customization, and web-style app surfaces.
- No language may cross the kernel/userland boundary except through defined syscall interfaces.
- JSON is the sole configuration format unless a justified exception is documented.

### 7.4 C Coding Style Standard

All C code in the ghOSt project must follow a consistent style. This is not optional — inconsistency becomes debt.

| Rule                     | Convention                                                                               |
| ------------------------ | ---------------------------------------------------------------------------------------- |
| Indentation              | 4 spaces. No tabs.                                                                       |
| Brace style              | Allman (opening brace on its own line) for functions. K&R (same line) for control flow.  |
| Naming — functions       | `snake_case` (e.g., `mem_alloc_page`, `uart_write_byte`)                                 |
| Naming — types/structs   | `PascalCase` with `_t` suffix for typedefs (e.g., `PageTable_t`, `BootInfo_t`)           |
| Naming — macros/consts   | `UPPER_SNAKE_CASE` (e.g., `PAGE_SIZE`, `MAX_CPUS`)                                       |
| Naming — globals         | Prefixed with `g_` (e.g., `g_kernel_heap`)                                               |
| Naming — statics         | Prefixed with `s_` (e.g., `s_init_done`)                                                 |
| Header guards            | `#pragma once` preferred. If include guards are needed: `GHOST_{MODULE}_{FILE}_H`        |
| Include ordering         | 1) Project headers, 2) libc/freestanding headers, 3) third-party (if any). Alphabetized. |
| Comments                 | `//` for inline comments. `/* */` for block/doc comments. Explain _why_, not _what_.     |
| Line length              | 100 columns soft limit. 120 hard limit.                                                  |
| Pointer declaration      | `type *name` (star on the name side, e.g., `void *ptr`)                                  |
| One declaration per line | No `int a, b, c;` — each gets its own line.                                              |

**Rationale:** Style consistency is not cosmetic — it reduces cognitive overhead, makes grep-based code navigation reliable, and prevents style-related merge conflicts. The conventions above are chosen for clarity in systems code, not personal preference.

### 7.5 Text Encoding

**UTF-8 is the canonical text encoding for ghOSt.** This is a foundational decision — retrofitting encoding is a nightmare.

| Rule                         | Detail                                                                                   |
| ---------------------------- | ---------------------------------------------------------------------------------------- |
| Internal string encoding     | UTF-8 everywhere. Kernel APIs, configs, filenames, log output — all UTF-8.               |
| Filesystem names             | Stored and compared as UTF-8 byte sequences.                                             |
| Console / serial output      | ASCII subset of UTF-8 in early boot. Full UTF-8 when framebuffer font supports it.       |
| Keyboard input               | Scancodes → keycodes → UTF-8 codepoints. Keyboard layout is configuration-driven (JSON). |
| No UCS-2 / UTF-16 internally | Unlike Windows, ghOSt does not use wide chars internally. UTF-8 only.                    |
| Locale / i18n                | Deferred to UX phase. Kernel is locale-agnostic. UX handles display localization.        |
| BOM                          | Never required. Never emitted. Tolerated on input if encountered.                        |

---

## 8. Memory Discipline

Memory bugs are the #1 cause of security vulnerabilities in systems software.
ghOSt treats memory discipline as a non-negotiable engineering standard.

### 8.1 General Rules

- Every buffer has a known, checked size.
- Every pointer has a documented ownership and lifetime.
- Every copy operation is length-aware and bounds-checked.
- Sensitive data is zeroed/sanitized when no longer needed.
- All external input (firmware tables, device data, disk structures, network data) is treated as untrusted.

### 8.2 Early Boot / Kernel Constraints

- No dynamic allocation in early boot unless explicitly justified and reviewed.
- Prefer static allocation or arena/region-based allocation for deterministic behavior.
- Stack frames must be small and predictable. No unbounded recursion.
- Integer narrowing in size/offset calculations is a defect.
- Debug builds must include lightweight overflow/underflow checks.

### 8.3 Documentation Requirements

- Memory layout assumptions (alignment, packing, page boundaries) must be documented near the code.
- Ownership transfer across subsystem boundaries must be explicit in function signatures or comments.
- Any exception to these rules must be documented with rationale in the code and in `/docs/decisions/`.

### 8.4 Error Handling Conventions

Error handling must be consistent across the entire codebase. Ambiguous error paths are security vulnerabilities in kernel code.

| Convention            | Rule                                                                                                     |
| --------------------- | -------------------------------------------------------------------------------------------------------- |
| Return type           | Functions that can fail return `ghost_status_t` (or equivalent typed error enum). No bare `int` returns. |
| Error propagation     | Errors propagate upward. Callers must check return values. Unchecked returns are defects.                |
| Kernel panic          | Reserved for truly unrecoverable states (corrupted page tables, double fault, etc.).                     |
| Panic severity        | Define levels: `PANIC` (halt), `OOPS` (log + attempt recovery), `WARN` (log + continue).                 |
| Assertions            | `GHOST_ASSERT()` in debug builds for invariant checking. Compiles out in release.                        |
| No silent failure     | Functions must never silently swallow errors. If recovery is attempted, it must be logged.               |
| Error context         | Error returns should carry enough context to diagnose: subsystem, operation, and error code at minimum.  |
| Cross-boundary errors | Errors crossing subsystem boundaries must be translated to the receiving subsystem's error vocabulary.   |

### 8.5 Logging & Debug Infrastructure

A project worked on in bursts needs to be self-explanatory when you come back. Logging is not optional.

| Component          | Design                                                                                    |
| ------------------ | ----------------------------------------------------------------------------------------- |
| Log levels         | `PANIC`, `ERROR`, `WARN`, `INFO`, `DEBUG`, `TRACE` — severity-ordered.                    |
| Early boot output  | Serial (COM1/UART) is the primary debug channel from first instruction to framebuffer up. |
| Kernel ring buffer | Circular in-memory log buffer for post-mortem analysis. Survives soft resets if feasible. |
| Framebuffer log    | Once framebuffer console is available, mirror log output to screen.                       |
| Timestamping       | All log entries include a monotonic timestamp (tick count until real clock is available). |
| Subsystem tags     | Every log line is tagged with its source subsystem (e.g., `[MM]`, `[SCHED]`, `[USB]`).    |
| Crash dump         | On panic: dump registers, stack trace, last N log entries to serial and screen.           |
| Debug build extras | In debug builds: verbose logging, memory poisoning, stack canaries, assertion checks.     |
| Release build      | `INFO` and above only. No `DEBUG`/`TRACE` in release binaries.                            |

---

## 9. Boot Architecture

### 9.1 Boot Path

```
Power On
  → UEFI firmware
    → ghOSt bootloader (Assembly + C)
      → Kernel early init (Assembly)
        → Kernel main init (C)
          → Profile detection (Minimal Rescue vs Full Install)
            → Service/driver init for selected profile
              → Shell (CLI) or UX (GUI) entry
```

### 9.2 Boot Requirements

- **UEFI support is mandatory.** Legacy BIOS support is not a current target.
- **Secure Boot is optional.** Design must not block future Secure Boot support.
- **TPM is optional.** Design must not block future measured-boot capabilities.
- Boot path must be deterministic and debuggable at every stage.
- Bootloader and kernel initialization are separate concerns with clean handoff.

### 9.3 Profile Selection

- The boot sequence must detect or be configured (via JSON boot config) for which profile to activate.
- Minimal Rescue Profile must reach a usable CLI prompt as fast as possible with minimal hardware init.
- Full Install Profile will initialize additional drivers, services, and UX subsystems.

---

## 10. Security & Privacy Architecture

### 10.1 Threat Model (High-Level)

ghOSt assumes the following threat landscape:

| Threat                              | ghOSt response                                         |
| ----------------------------------- | ------------------------------------------------------ |
| OS-level surveillance / telemetry   | No telemetry. No phoning home. Local-first by default. |
| Mandatory age/identity verification | No mandatory identity gates in core OS.                |
| Content censorship at OS level      | No OS-level content filtering or blocking.             |
| Data theft / unauthorized access    | Encryption, key ownership, least-privilege access.     |
| Firmware-level attacks              | Future Secure Boot / TPM path (optional).              |
| Malicious applications              | Sandboxing and permission model (future).              |

### 10.2 Privacy Principles

- **No telemetry, no exceptions.** The OS never transmits data without explicit user action.
- **No mandatory accounts.** The OS is fully functional without any online identity. **Local accounts only** — no cloud accounts required for any OS functionality.
- **No age verification.** The OS does not implement or enforce age-gating mechanisms.
- **Local-first.** All core functionality works offline.
- **User-owned encryption.** Keys are generated and stored locally. The OS has no backdoor access.
- **Local AI only by default.** Any AI features run locally on-device. Cloud AI services are never enabled by default and require explicit user setup. The OS ships no cloud AI integrations out of the box.

### 10.3 Security Design Rules

- Every trust boundary in the architecture must be explicit and documented.
- Kernel syscall interfaces must validate all inputs.
- Least-privilege is the default for all access — escalation must be explicit.
- Compliance-sensitive controls (if any) must be modular, policy-driven, and **never hardcoded into the kernel**.
- Security-critical code paths must be minimal, auditable, and separately reviewable.

### 10.4 User & Permission Model

The user/permission architecture must be defined before userland exists. Retrofitting permissions is a security disaster.

| Decision               | Current position                                                                            |
| ---------------------- | ------------------------------------------------------------------------------------------- |
| Root / superuser       | A privileged account exists for system administration. Day-to-day use is non-privileged.    |
| Permission model       | Capability-based or traditional UNIX-style — **decision needed (ADR required).**            |
| Local accounts only    | No cloud/network accounts. All accounts are local to the machine.                           |
| Account creation       | First boot creates a user account. No mandatory online registration.                        |
| Privilege escalation   | Explicit and auditable. Equivalent to `sudo` — no ambient authority.                        |
| Application sandboxing | Applications run with minimal privileges by default. Elevated access requires user consent. |
| Service accounts       | System services run under dedicated low-privilege accounts, not root.                       |

### 10.5 Disk Encryption Architecture

ghOSt's privacy promise requires encryption to be a first-class architectural concern, not a bolt-on.

| Decision                   | Current position                                                                        |
| -------------------------- | --------------------------------------------------------------------------------------- |
| Full-disk encryption (FDE) | Planned. Design must accommodate FDE from early filesystem work. **ADR required.**      |
| Partition encryption       | Supported as alternative to FDE for multi-boot scenarios.                               |
| Key storage                | Keys are user-owned, locally stored. No escrow, no cloud backup of keys by default.     |
| Key derivation             | Password-based (PBKDF2 / Argon2) with optional hardware token (TPM, USB key) in future. |
| Encryption algorithm       | AES-256 (or equivalent audited algorithm) — **decision needed when implemented.**       |
| Boot unlock flow           | Password prompt at boot (pre-kernel or early-kernel). Must work without GUI.            |
| Recovery                   | User-created recovery key. OS cannot recover data without user's key. No backdoors.     |

### 10.6 Kernel Security Hardening Baseline

The kernel must be secure by construction and hardened by default where hardware support exists. Security is not a post-release checklist item.

| Hardening area        | Current position                                                                                                                                          |
| --------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------- |
| NX / XD               | **Required.** Mark data pages non-executable whenever hardware supports it.                                                                               |
| W^X                   | **Required.** No kernel or user page may be writable and executable at the same time.                                                                     |
| SMEP / SMAP / UMIP    | **Enable when supported.** Prevent kernel execution of user pages, restrict kernel access to user memory, and block unsafe legacy user-mode instructions. |
| Stack canaries        | **Required** for C code where toolchain support is available. Prefer `-fstack-protector-strong` or equivalent.                                            |
| Guard pages           | **Required** around kernel stacks where practical. Stack overflow must fault deterministically, not corrupt adjacent memory.                              |
| User-copy boundary    | Centralize `copy_from_user` / `copy_to_user` style routines. Raw user-pointer dereference in kernel code is forbidden.                                    |
| Zeroing policy        | Zero newly allocated user-visible pages and sanitize freed sensitive buffers.                                                                             |
| KASLR                 | **Planned.** Kernel ASLR is desirable for installed systems; debug/minimal environments may disable it explicitly.                                        |
| Read-only kernel data | Mark immutable kernel sections read-only after init wherever feasible.                                                                                    |
| Panic diagnostics     | Panic path must preserve enough state for post-mortem analysis without leaking secrets unnecessarily.                                                     |

**Rules:**

- All kernel entry points validate pointers, lengths, alignment, and privilege expectations.
- Syscall handlers must fail closed on malformed input.
- Security checks belong in shared primitives, not duplicated ad hoc across subsystems.
- A hardening feature may be temporarily disabled only for a documented bring-up reason, never by silent omission.

### 10.7 Local Safety, Child Accounts, and Parental Controls

ghOSt may support parental controls, but only as a **local, owner-controlled safety feature**. They are not a censorship mechanism and must never become a remote policy channel.

| Principle                 | Rule                                                                                                           |
| ------------------------- | -------------------------------------------------------------------------------------------------------------- |
| Local-only control        | Parental controls are configured locally by the device owner or administrator. No cloud account is required.   |
| Opt-in only               | Disabled by default. No age-gating, identity verification, or jurisdiction-driven policy is baked into the OS. |
| Per-account / per-device  | Policies apply to child accounts or specific local sessions, not globally to the entire OS by default.         |
| No remote censorship      | No vendor-operated blacklist, remote kill switch, or forced content feed filtering.                            |
| UX/service layer feature  | Enforcement belongs in userland services and UX policy layers. The kernel provides primitives, not ideology.   |
| Transparent and auditable | Every restriction is visible to the local administrator and stored in editable, documented config.             |
| Emergency override        | A parent/administrator can override or disable controls locally with explicit authentication.                  |
| Privacy-preserving        | Activity summaries and policy logs stay local by default. No telemetry or cloud reporting.                     |

**Kernel responsibilities:**

- Strong account separation and privilege boundaries.
- Reliable timekeeping for schedules and time limits.
- Permission and sandbox primitives for app/session restrictions.
- Audit/event hooks for local policy services.

**UX/service responsibilities:**

- Screen-time schedules and lockout windows.
- Application allow/deny lists per child account.
- Purchase/install approval workflows.
- Web/content restrictions only through locally chosen applications or DNS/filtering services configured by the owner.

**Non-negotiable boundary:** ghOSt will not hardcode third-party moral, political, or commercial content restrictions into the OS. Parental controls are for the machine owner to manage their own household, not for outsiders to govern the device.

### 10.8 User-Space Security, Threat Detection, and Trust Model

ghOSt needs built-in security that ordinary users can actually use. That means the security model must be understandable, local-first, and technically honest about what it can detect.

#### 10.8.1 Security Goals

- Prevent untrusted applications from silently gaining dangerous access.
- Make security state visible and understandable to non-expert users.
- Detect known threats locally without requiring vendor telemetry.
- Reduce the blast radius of unknown threats through sandboxing, permissions, isolation, and exploit mitigations.
- Avoid turning "security" into a pretext for OS-level censorship or remote control.

#### 10.8.2 What Built-In Security Should Include

| Capability                    | Current position                                                                                   |
| ----------------------------- | -------------------------------------------------------------------------------------------------- |
| Application trust prompts     | **Required.** Elevated or unusual permissions require clear, human-readable consent.               |
| Package / binary verification | **Required.** Verify signatures, hashes, and provenance where available.                           |
| Quarantine / first-run policy | **Planned.** Newly downloaded or externally sourced binaries may run in a restricted mode first.   |
| Local malware scanning        | **Planned.** On-demand and scheduled scanning with local signature databases and heuristics.       |
| Sandboxed execution           | **Required direction.** Unknown or low-trust applications should be able to run with tight limits. |
| Security event logging        | **Required.** Security-relevant actions generate local audit events visible to the user/admin.     |
| Security posture dashboard    | **Planned.** UX exposes system hardening, update state, scan state, and recent alerts.             |
| Remediation workflow          | **Planned.** Quarantine, block, allow-once, trust permanently, view details, export report.        |

#### 10.8.3 Threat Intelligence: Known Threats vs Unknown Threats

There is an important distinction:

- **Known threats** can be matched against signatures, hashes, IOC feeds, CVE/KEV databases, YARA rules, package advisories, and certificate revocation data.
- **Unknown threats / true 0-days** are not in a database yet by definition. They must be mitigated through behavior-based detection, sandboxing, exploit hardening, least privilege, and fast update response.

**Rule:** ghOSt must never claim that a "0-day database" provides protection against unknown exploits. That is not technically honest.

#### 10.8.4 Detection Strategy

ghOSt's built-in protection should be layered:

| Layer                     | Purpose                                                                                                                                        |
| ------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------- |
| Hardening                 | Stop exploit classes before detection matters: NX, W^X, sandboxing, permission gates, isolation.                                               |
| Reputation / provenance   | Identify where software came from: signed package, trusted repository, local build, unknown binary.                                            |
| Signature / IOC matching  | Detect known malware, known bad hashes, known bad URLs/domains, YARA-style rules.                                                              |
| Heuristic / behavioral    | Flag suspicious actions: persistence injection, privilege escalation attempts, mass file encryption, process hollowing, unusual API sequences. |
| Containment               | Restrict what a suspicious process can touch even before a final verdict exists.                                                               |
| Human-visible remediation | Give the user/admin an actionable explanation, not just a red warning banner.                                                                  |

**Initial direction:**

- Use **signatures and open threat data** for known threats.
- Use **heuristics/behavioral rules** for suspicious activity.
- Use **sandboxing and permissions** as the primary defense against unknown threats.
- Keep any cloud-assisted analysis **disabled by default** and fully optional if ever introduced.

#### 10.8.5 Open Data Sources (Known Threats Only)

The project may consume open or openly licensed threat data for known issues and indicators, provided updates remain local-first and user-controllable.

Candidate source categories:

- **CVE / vulnerability metadata:** NVD, CVE.org, OSV.dev
- **Known exploited vulnerability lists:** CISA KEV
- **YARA / IOC feeds:** community-maintained YARA rules, Abuse.ch-style IOC feeds where licensing permits
- **Package ecosystem advisories:** GitHub Security Advisories, distro/package advisories, language ecosystem advisories
- **Certificate / signing trust data:** revocation lists and trusted publisher metadata

**Constraints:**

- Feed ingestion must be transparent, documented, and optionally user-managed.
- The system must continue functioning without live feed access.
- A stale feed must degrade detection quality, not break execution of the OS.
- Feed data is for **known threats**, not a claim of 0-day discovery.

#### 10.8.6 Heuristics and Behavioral Detection

Heuristics are necessary, but false positives destroy trust if handled badly.

| Principle                    | Rule                                                                                      |
| ---------------------------- | ----------------------------------------------------------------------------------------- |
| Explainability               | Every heuristic alert should explain what behavior triggered it in human-readable terms.  |
| Confidence tiers             | Alerts should be categorized: informational, suspicious, high risk, blocked.              |
| No silent destructive action | The system should not delete user data automatically based on a weak heuristic verdict.   |
| Quarantine over deletion     | Default remediation is containment/quarantine, not permanent destruction.                 |
| Local override               | The local administrator can review and override detections with explicit acknowledgement. |
| Auditability                 | Security decisions are logged locally with rule source, timestamp, and action taken.      |

Examples of behaviors worth flagging:

- process injecting code into another process;
- unsigned binary attempting persistence in startup/service locations;
- unexpected encryption of large numbers of user files;
- a low-trust process repeatedly requesting elevated permissions;
- an application suddenly opening raw disk, credential, or debugger interfaces without clear user intent.

### 10.9 System Update Architecture

ghOSt is not a static artifact. It needs a trustworthy servicing model for kernel fixes, security patches, feature updates, and package/application updates without turning updates into a control mechanism over the user.

#### 10.9.1 Update Principles

- **User control first.** No forced cloud-managed updates as a condition of using the OS.
- **Security updates must be easy, fast, and trustworthy.** Fast patching is part of the security model.
- **Updates must be recoverable.** A failed update must not brick the machine.
- **System updates and application updates are related but distinct.** Kernel/OS servicing must not be coupled to every application package decision.
- **Offline and local workflows must exist.** An update system that requires vendor connectivity contradicts the project mission.

#### 10.9.2 Update Classes

| Update class       | Scope                                                                | Typical reboot requirement                          |
| ------------------ | -------------------------------------------------------------------- | --------------------------------------------------- |
| Security update    | Kernel fixes, driver fixes, core libraries, signatures/IOC feeds     | Usually yes for kernel/boot; no for data-only feeds |
| Kernel update      | Kernel image, boot-critical drivers, low-level ABI-impacting changes | Yes                                                 |
| Feature update     | New OS capabilities, new system services, UX shell changes           | Sometimes                                           |
| Package/app update | Userland applications, tools, optional components                    | Usually no                                          |
| Data update        | Threat feeds, certificate bundles, time zone data, metadata          | No                                                  |

#### 10.9.3 OS Update Delivery Model

**Default direction:** ghOSt should support **atomic system updates** for the base OS and **separate package updates** for optional software.

| Component              | Direction                                                                                     |
| ---------------------- | --------------------------------------------------------------------------------------------- |
| Base OS image          | Versioned, signed system image or component set for kernel, boot artifacts, and core system   |
| Update format          | Signed manifest plus payloads (full and/or delta)                                             |
| Integrity verification | Verify signatures, hashes, version constraints, and target slot/state before applying         |
| Install strategy       | Prefer staged install into an inactive target (A/B or equivalent) over in-place mutation      |
| Activation point       | Boot manager flips to the new slot/config only after update verification succeeds             |
| Rollback               | Automatic fallback to previous known-good boot target on boot failure or health-check failure |
| Recovery path          | Minimal Rescue / recovery partition can repair or roll back a broken install                  |

**A/B-style updates are preferred** for the installed OS because they are safer than patching the live root in place.

#### 10.9.4 Kernel and Boot Updates

Kernel updates are the highest-risk update class because failure can make the whole system unbootable.

Rules:

- Kernel, initramfs/boot support files, and boot-critical drivers update as a **coordinated boot set**.
- A new kernel is written to the inactive target/slot first.
- The boot manager keeps the previous bootable kernel entry until the new one has booted successfully.
- First boot after a kernel update runs a health check and marks the slot good only after success.
- If boot fails, the boot manager must automatically fall back to the last known-good entry.
- Security-only kernel patches still follow the same rollback-safe path. "It's just a patch" is not a reason to skip safety.

#### 10.9.5 Security Updates and Response Cadence

Security updates need their own servicing expectations:

| Area                   | Requirement                                                                 |
| ---------------------- | --------------------------------------------------------------------------- |
| Out-of-band updates    | Supported for urgent security fixes.                                        |
| Feed/signature updates | May update independently of full OS updates.                                |
| Advisory metadata      | Security Center shows severity, affected components, and remediation state. |
| Reboot communication   | If a reboot is required, explain why clearly.                               |
| Deferral policy        | Users may defer, but high-risk unresolved issues must remain visible.       |

ghOSt should prefer **fast local delivery of signed security fixes** over large monolithic release trains when possible.

#### 10.9.6 Update Channels

| Channel | Purpose                                               | Risk profile |
| ------- | ----------------------------------------------------- | ------------ |
| Stable  | General users and production systems                  | Lowest       |
| Preview | Early validation of upcoming releases                 | Medium       |
| Dev     | Fast-moving builds for active development and testers | Highest      |

Rules:

- Channel selection is explicit and user-controlled.
- Stable must never silently become Preview or Dev.
- Security feed updates may remain on Stable even when feature channel is conservative.
- Minimal Rescue media should be updatable too, but with a simpler, more manual workflow.

#### 10.9.7 User Control and Policy

ghOSt rejects coercive update policy.

| Policy area            | Current position                                                                    |
| ---------------------- | ----------------------------------------------------------------------------------- |
| Automatic checks       | Allowed and configurable. Must be disableable.                                      |
| Automatic download     | Allowed and configurable. Off by default is acceptable in privacy-sensitive setups. |
| Automatic install      | Allowed only when explicitly enabled by the owner/admin.                            |
| Forced reboot          | Not allowed by default. Reboots require explicit scheduling or consent.             |
| Admin control          | Local administrator controls update policy.                                         |
| Enterprise/server mode | Longer maintenance windows, staged rollout, and manual approval must be possible.   |

#### 10.9.8 Update Provenance and Trust

- Every OS update is signed.
- Signing keys and trust roots are versioned and documented.
- Trust root rotation must be supported with overlap and recovery planning.
- The system must clearly distinguish:
  - official ghOSt updates;
  - locally built/test-signed updates;
  - third-party repositories or package sources.
- Update logs remain local by default and exportable by the user.

#### 10.9.9 Package Updates vs System Updates

The package manager is not the whole OS updater.

- **System updates** manage the base operating system: bootloader, kernel, core services, security data, and bundled system UX.
- **Package updates** manage optional software installed on top.
- The system must support updating one without forcing the other when technically possible.
- ABI and compatibility checks must prevent userland packages from being silently broken by incompatible kernel/OS updates.

#### 10.9.10 Failure and Recovery Model

Update failure is expected and must be designed for.

| Failure case                    | Required response                                                                    |
| ------------------------------- | ------------------------------------------------------------------------------------ |
| Download interrupted            | Resume or discard cleanly. Never leave partial payload mistaken as complete.         |
| Signature verification fails    | Reject update, log locally, preserve current system state.                           |
| Install interrupted             | Remain bootable on previous slot/target.                                             |
| First boot of new version fails | Automatic fallback to previous known-good version.                                   |
| Post-boot health check fails    | Mark updated slot bad and revert on next boot.                                       |
| User reports regression         | Manual rollback path must exist from recovery environment and, if possible, from UX. |

#### 10.9.11 Update Manifest and Metadata Requirements

The update manifest is the contract between the release process, the updater, and the boot path. It must be explicit enough to validate safety before any payload is applied.

| Manifest field           | Purpose                                                                                         |
| ------------------------ | ----------------------------------------------------------------------------------------------- |
| Product / profile        | Identifies whether the payload targets Minimal Rescue, Full Install, or a specific sub-profile. |
| Current version range    | Declares which installed versions may apply the update.                                         |
| Target version           | Identifies the resulting system version after successful activation.                            |
| Channel                  | Stable, Preview, or Dev. Prevents channel confusion or downgrade surprises.                     |
| Payload digests          | Hashes for each payload object and for the manifest itself.                                     |
| Signature chain          | Signing identity, trust root identifier, and any intermediate signer metadata.                  |
| Reboot class             | No reboot, userland restart, scheduled reboot, or mandatory reboot for activation.              |
| Boot-set contents        | Explicit list of kernel, bootloader, initramfs, and boot-critical drivers included together.    |
| Migration steps          | Any schema, config, or data migrations required before or after activation.                     |
| Rollback policy          | Whether rollback is supported, blocked, or gated by a data-format barrier.                      |
| Minimum recovery version | Lowest recovery environment version that can repair or roll back this update safely.            |

Rules:

- The updater must reject a manifest with missing required fields, mismatched channel, invalid signatures, or incompatible version bounds.
- Manifest evaluation must occur before writing to the inactive slot.
- A manifest may describe multiple payload classes, but it must label them separately so the UX can present them honestly.

#### 10.9.12 Installed-System Layout and Health State

Installed systems should model updates around explicit state, not guesswork.

| Element              | Direction                                                                                               |
| -------------------- | ------------------------------------------------------------------------------------------------------- |
| Slot A / Slot B      | Preferred for installed Full Install systems. One active slot, one inactive target.                     |
| Recovery environment | Separate Minimal Rescue or recovery image capable of repair, rollback, and log export.                  |
| Shared user data     | Kept outside the bootable system slot where possible to avoid clobbering user state during OS rollback. |
| Boot health marker   | Written only after successful boot, storage mount, and basic service health checks.                     |
| Attempt counter      | Limits repeated failed boots before automatic fallback.                                                 |
| Known-good pointer   | Boot manager stores the last verified boot target independently from the pending target.                |

Health checks should stay minimal and deterministic. The first implementation should verify:

- the kernel reached userland init successfully;
- required system partitions mounted read-write or read-only as expected;
- boot-critical services started far enough to declare the slot viable; and
- rollback metadata was updated without corruption.

The boot health path must not depend on the full GUI. Headless/server installs need the same safety model.

#### 10.9.13 Servicing Implementation Phases

The servicing stack should be built in layers. Attempting the final UX before the boot and trust model exists would be backward.

| Phase band | Servicing deliverable                                                                                                                                                              |
| ---------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Phase 0–1  | Build tooling emits versioned artifacts and test-signed manifests. Release process can produce deterministic payload metadata.                                                     |
| Phase 2–4  | Boot path grows slot state, boot-set version tracking, health markers, and automatic fallback rules. Recovery environment can inspect slot state.                                  |
| Phase 5–6  | Userland updater service can download or ingest signed bundles, verify manifests, stage payloads, and record local audit logs. Data/feed updates become independently refreshable. |
| Phase 7–8  | Boot Manager and ghOSt PE integrate rollback selection, offline repair, and channel-aware activation rules.                                                                        |
| Phase 10+  | GUI Update Center adds scheduling, release-note presentation, rollback entry points, and richer maintenance controls.                                                              |
| Future     | Delta payloads, key-rotation workflows, staged rollout rings, and optional fleet policy layers may be added without breaking local control.                                        |

**Initial non-goals:**

- no mandatory cloud dependency for update discovery or approval;
- no live kernel patching in the first servicing design;
- no hidden firmware updates bundled under ordinary OS patch labels.

---

## 11. Configuration Architecture

### 11.1 Format

All configuration uses **JSON**.

### 11.2 Rules

| Rule                 | Detail                                              |
| -------------------- | --------------------------------------------------- |
| Ownership            | Every config file has one clear owner (subsystem).  |
| Defaults             | All keys have documented defaults.                  |
| Versioning           | Every config file includes a `schemaVersion` field. |
| Validation           | Configs are validated early (boot/load time).       |
| Human-readability    | Configs must be readable and editable by humans.    |
| No magic values      | Undocumented behavior-altering values are defects.  |
| Schema documentation | Schemas are documented as the config surface grows. |

### 11.3 Config Scope by Profile

- **Minimal Rescue:** boot config, minimal shell config.
- **Full Install:** boot config, system services config, UX config, app compat config, security policy config.

---

## 12. UX Architecture (Future)

### 12.1 Vision

The ghOSt graphical UX is a browser-like environment:

- Fully customizable appearance and behavior.
- Driven by configuration (JSON), not compiled defaults.
- No hardcoded UX unless technically unavoidable (and then documented as an exception).

### 12.2 Technology

- Native rendering layer: **C++**
- UX content and customization: **HTML / CSS / JS / TS**
- Extension/theme surfaces: configuration-driven, documented APIs.
- **GPU acceleration:** The UX compositor and rendering pipeline must leverage GPU hardware where available for compositing, 2D/3D effects, and graphics-heavy operations (e.g., animated gradient desktop backgrounds, window transitions, transparency). Software rendering is the fallback when no supported GPU driver is loaded.

### 12.3 Theme Architecture

**Default theme: Dark Mode.** Light mode is a supported alternative, not the default.

All visual styling is driven by a theme system — no colors, fonts, spacing, or icon sets may be hardcoded in rendering code.

| Principle            | Detail                                                                        |
| -------------------- | ----------------------------------------------------------------------------- |
| Theme format         | JSON manifest + CSS variables (or equivalent token system)                    |
| Default theme        | Dark mode, ships with the OS                                                  |
| Bundled alternatives | At minimum: one dark theme (default) and one light theme                      |
| Custom themes        | Users can create, install, and share themes as standalone packages            |
| Hot-swap             | Theme changes apply without reboot or UX restart where feasible               |
| Fallback             | If a theme file is missing or corrupt, fall back to the built-in dark default |
| No hardcoded colors  | Rendering code references theme tokens, never literal color values            |
| Schema versioning    | Theme format includes a `schemaVersion` for forward compatibility             |

Theme scope covers at minimum:

- Window chrome (title bars, borders, controls)
- Shell surfaces (taskbar, launcher, menus)
- System dialogs and notifications
- Text rendering defaults (font family, size, weight)
- Icon set / icon theme
- Accent / highlight color
- Scrollbars, inputs, selection highlights

### 12.4 GPU-Accelerated Rendering

The UX layer is designed for a modern, visually rich experience. GPU acceleration is a first-class concern, not an afterthought.

| Principle               | Detail                                                                                               |
| ----------------------- | ---------------------------------------------------------------------------------------------------- |
| Compositor acceleration | Window compositing uses GPU when available — alpha blending, Z-order, damage regions                 |
| 2D effects              | Gradients, shadows, rounded corners, blur rendered via GPU pipeline                                  |
| Animated backgrounds    | Desktop backgrounds support animated gradients and shader-driven visuals (GPU-required feature)      |
| Transitions/animations  | Window open/close, workspace switch, and UI transitions are GPU-accelerated where supported          |
| Software fallback       | Every GPU-accelerated path must have a software-rendered fallback (GOP framebuffer)                  |
| Progressive enhancement | UX degrades gracefully — core usability never depends on GPU acceleration                            |
| Driver dependency       | GPU features activate dynamically based on loaded driver capabilities (no hardcoded GPU assumptions) |
| Frame budget            | Target 60 fps for composited desktop; drop effects before dropping frames                            |

### 12.5 UX Design Rules

- Separate rendering/input/runtime from policy/config.
- Keep customization surfaces intentional and documented.
- Never embed policy decisions (censorship, content filtering, access restrictions) in the UX layer.
- UX must work without network access (local-first).
- GPU-accelerated features must gracefully degrade to software rendering when GPU is unavailable.

### 12.6 Accessibility (a11y)

Accessibility cannot be retrofitted. The UX architecture must include a11y hooks from day one.

| Requirement               | Status                                                                           |
| ------------------------- | -------------------------------------------------------------------------------- |
| Keyboard-only navigation  | **Required.** Every UX function must be reachable without a mouse.               |
| High-contrast mode        | **Required.** Shipped as a bundled theme variant.                                |
| Screen reader support     | **Planned.** UX widgets must expose semantic information (role, label, state).   |
| Font scaling / large text | **Required.** System-wide font scale setting in config.                          |
| Reduced motion            | **Planned.** Config flag to suppress animations and transitions.                 |
| Color-blind safe defaults | **Planned.** Default themes must be tested for common color vision deficiencies. |

Accessibility is not a feature gate — it is a property of the UX architecture. Building a compositor without a11y hooks creates permanent debt.

### 12.7 Multi-Monitor & HiDPI

| Decision                | Current position                                                                                     |
| ----------------------- | ---------------------------------------------------------------------------------------------------- |
| Multi-monitor           | **Planned.** Compositor must model multiple outputs from the start. Per-output mode setting and DPI. |
| HiDPI / display scaling | **Planned.** Integer scaling (2x, 3x) first. Fractional scaling is deferred.                         |
| Per-monitor DPI         | Design for it now. Each monitor has its own scale factor.                                            |
| Primary display         | Configurable. No hardcoded "monitor 0 is primary" assumption.                                        |

### 12.8 Notification System

| Decision                    | Current position                                                                     |
| --------------------------- | ------------------------------------------------------------------------------------ |
| Kernel → user notifications | Events (disk full, hardware error, OOM) delivered via IPC to a notification service. |
| App → user notifications    | Applications use a notification API with user-controllable permissions.              |
| CLI profile                 | Notifications print to console/serial. No daemon required.                           |
| GUI profile                 | Toast-style notification popups. Notification center is a UX feature.                |
| Do Not Disturb              | Config-driven. User can suppress non-critical notifications.                         |

### 12.9 Font Rendering

| Decision           | Current position                                                                 |
| ------------------ | -------------------------------------------------------------------------------- |
| Font engine        | **Decision needed (ADR).** Candidates: FreeType, stb_truetype, custom.           |
| Hinting            | Must support hinting for legibility at small sizes.                              |
| Subpixel rendering | Planned for LCD displays. Must be configurable (some displays don't benefit).    |
| Default font       | Ship at least one high-quality open-source font (e.g., Noto, Inter, or similar). |
| Bitmap fallback    | Early boot uses a simple bitmap font. No TrueType dependency in kernel.          |

### 12.10 Parental Controls & Family Safety UX

Parental controls are a UX and policy-service feature built on top of the security model in Section 10.7.

| Feature              | Current position                                                                                         |
| -------------------- | -------------------------------------------------------------------------------------------------------- |
| Child account mode   | **Planned.** Child accounts have a simplified UX and policy surface.                                     |
| Time limits          | **Planned.** Per-account schedules, bedtime windows, and session duration caps.                          |
| App restrictions     | **Planned.** Allow/deny lists by app identity, package, or signer.                                       |
| Install approvals    | **Planned.** Child accounts require administrator approval for installs/updates.                         |
| Web/content controls | **Optional.** Implemented through local policy services or selected apps, never mandatory OS blacklists. |
| Activity reporting   | **Local only.** Usage summaries stay on-device unless the user explicitly exports them.                  |
| Tamper resistance    | **Required.** Child accounts cannot disable their own restrictions without admin credentials.            |

The family-safety surface must clearly distinguish:

- local parental policy chosen by the device owner;
- application-level content controls chosen by the user; and
- prohibited OS-level censorship, which ghOSt rejects.

### 12.11 Security UX & Security Center

Built-in security is only useful if users can understand what the system is doing and make sane decisions without reading a reverse-engineering manual.

| Feature              | Current position                                                                                                            |
| -------------------- | --------------------------------------------------------------------------------------------------------------------------- |
| Security dashboard   | **Planned.** One place to view update status, hardening status, scanner state, recent alerts, and trust decisions.          |
| Permission prompts   | **Required.** Prompts must describe the concrete capability being requested and why it matters.                             |
| Trust labels         | **Planned.** Apps/binaries show trust origin: signed package, trusted repo, local build, unknown download, removable media. |
| Scan controls        | **Planned.** Quick scan, full scan, custom scan, scheduled scan, and per-path exclusions with admin approval.               |
| Alert triage         | **Planned.** Users can inspect evidence, view triggered rule(s), quarantine, block, allow once, or trust permanently.       |
| Remediation guidance | **Required.** Alerts should explain next steps in plain language, not only technical jargon.                                |
| Security history     | **Planned.** Local event timeline for detections, overrides, quarantines, and important policy changes.                     |
| Quiet mode           | **Planned.** Suppress low-priority noise while never hiding high-risk events.                                               |

**Usability rules:**

- Do not train users to click through meaningless warnings.
- A security prompt that appears too often is a design failure.
- Use progressive disclosure: simple summary first, technical detail on demand.
- Security UI must distinguish between **known malicious**, **suspicious**, **untrusted**, and **merely unsigned**.
- The system should prefer clear default actions over panic-inducing messaging.

### 12.12 Update UX & Servicing Experience

Updates are a security feature, a reliability feature, and a trust feature. The UX must make them understandable and controllable.

| Feature               | Current position                                                                                                             |
| --------------------- | ---------------------------------------------------------------------------------------------------------------------------- |
| Update dashboard      | **Planned.** Show current OS version, channel, pending updates, restart requirements, and rollback state.                    |
| Update categories     | **Required.** Clearly distinguish security updates, kernel updates, feature updates, package updates, and data/feed updates. |
| Scheduling            | **Planned.** Users/admins can choose install windows and reboot windows.                                                     |
| Release notes         | **Required.** Summaries must explain what changed and whether reboot is needed.                                              |
| Rollback entry point  | **Planned.** Show whether rollback is available and how to trigger it.                                                       |
| Deferral controls     | **Required.** Users can postpone non-critical updates; deferred security risk remains visible.                               |
| Server/admin controls | **Planned.** Maintenance windows, staged rollout, and approval gates for server deployments.                                 |
| Offline updates       | **Planned.** Install signed update bundles from removable media or local network sources.                                    |

**UX rules:**

- Never hide reboot requirements.
- Never bundle a major feature change under the label of a security patch.
- Do not nag users with constant update prompts when nothing urgent is pending.
- Explain why an update matters: security, stability, compatibility, or feature addition.
- A failed update must present clear recovery instructions, not a generic error code.

#### 12.12.1 Desktop Update Flow

The desktop flow should be understandable in one glance and detailed on demand.

| Stage              | UX requirement                                                                         |
| ------------------ | -------------------------------------------------------------------------------------- |
| Status overview    | Show installed version, selected channel, last check time, and current risk state.     |
| Update list        | Group pending items by category: security, kernel, feature, package, data.             |
| Decision point     | Present install now, schedule, defer, or view details depending on policy and urgency. |
| Activation notice  | Explain whether restart is needed and what will happen at the next boot.               |
| Post-install state | Show success, pending reboot, rollback availability, or failure with next steps.       |

Desktop UX rules:

- Default view is summary-first, detail-second.
- The main call to action must be truthful: `Restart to apply kernel update` is acceptable; `Optimize system` is not.
- Security-only data updates that do not require reboot should avoid interruptive prompts.

#### 12.12.2 Server and Headless Update Flow

Full Install server deployments and headless systems cannot depend on a compositor.

| Surface             | Requirement                                                                                                           |
| ------------------- | --------------------------------------------------------------------------------------------------------------------- |
| CLI / TUI status    | A text-first update command or dashboard must show channel, pending updates, reboot state, and rollback availability. |
| Maintenance windows | Admins can define install and reboot windows separately.                                                              |
| Approval gates      | Kernel and feature updates may require explicit approval in server policy mode.                                       |
| Remote operators    | Logs and update state must be readable over serial, SSH-equivalent, or local console paths.                           |
| Failure handling    | Recovery instructions must be printable/loggable without GUI dependency.                                              |

Server UX must optimize for predictability over polish. Surprise restarts are defects.

#### 12.12.3 Minimal Rescue and Offline Update Flow

Minimal Rescue must be able to service or repair a Full Install system even when the main install is unhealthy.

| Scenario                   | Required behavior                                                                                                 |
| -------------------------- | ----------------------------------------------------------------------------------------------------------------- |
| Offline update bundle      | Validate signature, inspect target version, and apply to inactive slot or media target.                           |
| Rollback from recovery     | Present known-good targets clearly and allow explicit rollback.                                                   |
| Broken primary install     | Recovery environment can inspect boot logs, slot state, and manifest history without mounting user data unsafely. |
| Minimal Rescue self-update | Simpler, explicit workflow with checksum/signature verification and clear media replacement instructions.         |

Offline UX must assume no network, no browser, and possibly no graphics acceleration.

#### 12.12.4 Notification and Reboot Language Rules

- The system must say **what** is changing, **why** it matters, and **when** it takes effect.
- Reboot prompts must distinguish between `restart recommended` and `restart required to activate installed kernel`.
- Deferred high-severity security updates remain visible until resolved or explicitly suppressed by policy.
- If rollback is blocked by a data-format migration, the UX must say so before install begins.
- Error text must include the recovery path: retry, inspect logs, boot previous slot, or enter Minimal Rescue.

#### 12.12.5 Rollback and Failure UX

| Situation                 | UX expectation                                                                             |
| ------------------------- | ------------------------------------------------------------------------------------------ |
| Update staged, not booted | Show pending activation and give user/admin a chance to postpone reboot.                   |
| First boot fallback       | Explain that the system reverted automatically and preserve the failure reason.            |
| Manual rollback available | Show target version, age of snapshot/slot, and likely consequences before confirmation.    |
| Rollback unavailable      | Explain the exact blocker: migration barrier, missing recovery image, or overwritten slot. |

### 12.13 Deferred UX Items (Documented for Future)

These are acknowledged as real requirements but are explicitly deferred beyond initial GUI delivery:

| Item                       | Status   | Notes                                                              |
| -------------------------- | -------- | ------------------------------------------------------------------ |
| Printing support           | Deferred | Printers exist but driver complexity is enormous. Not in v1.       |
| Clipboard / drag-and-drop  | Deferred | Requires IPC + MIME type system. Phase 10+.                        |
| Backup / restore           | Deferred | Local backup tool planned. Cloud backup never by default.          |
| Branding / visual identity | Deferred | Logo, boot splash, icon set direction — not urgent.                |
| End-user documentation     | Deferred | Install guide, user manual, troubleshooting. Needed before v1.0.0. |

---

## 13. Application Compatibility Strategy

### 13.1 Native Applications

- ghOSt will define its own native application interface (syscall + userland API surface).
- Native apps are written in C, C++, or web technologies (HTML/CSS/JS/TS via the UX runtime).

### 13.2 ABI Stability Policy

The syscall interface is the contract between the kernel and userland. Breaking it silently destroys all user-space software.

| Phase    | ABI policy                                                                                 |
| -------- | ------------------------------------------------------------------------------------------ |
| `0.x.y`  | **Unstable.** Syscall numbers, signatures, and behavior may change between any release.    |
| `1.0.0`  | **Stable baseline.** Syscall ABI is frozen. Breaking changes require a major version bump. |
| Post-1.0 | New syscalls can be added (minor bump). Existing syscalls cannot change behavior.          |

- ABI stability applies to the syscall boundary only. Internal kernel APIs are always subject to change.
- The UX shell and userland libraries must version-check the kernel at startup and fail clearly on mismatch.

### 13.3 Process Lifecycle & Init System

The process model must be designed before userland exists. It shapes everything from shell to services.

| Decision           | Current position                                                                      |
| ------------------ | ------------------------------------------------------------------------------------- |
| Init / PID 1       | ghOSt will have its own init system (not systemd, not SysV). **ADR required.**        |
| Process hierarchy  | Parent-child. Orphaned processes are re-parented to init.                             |
| Service management | Config-driven service definitions (JSON). Start/stop/restart/dependency ordering.     |
| Daemon lifecycle   | Services declare dependencies and are started in dependency order by init.            |
| Zombie handling    | Kernel automatically reaps zombies whose parent has exited.                           |
| Shutdown sequence  | Init drives orderly shutdown: stop services → sync filesystems → unmount → power off. |

### 13.4 COM Support (Full Install, Long-Term)

- Full COM (Component Object Model) support is a planned major milestone for the Full Install Profile.
- This requires implementing: COM runtime, interface marshaling, class factories, registry/activation model, threading apartments, and selected OLE/ActiveX surfaces.
- COM must work for both 64-bit native and 32-bit applications (via the 32-bit compatibility subsystem).
- COM is explicitly **out of scope for Minimal Rescue Profile**.
- Approach: phased implementation — core runtime first, then interface coverage expands based on target application needs.

### 13.5 32-Bit Compatibility Subsystem (Full Install)

- ghOSt is a 64-bit OS. 32-bit application support is provided through a dedicated compatibility subsystem built into the Full Install Profile.
- The subsystem intercepts and translates 32-bit syscalls, manages separate 32-bit address spaces, and provides thunking for API/COM interop between 32-bit and 64-bit components.
- This subsystem is **not** called "WoW" or any Windows-derived name. A ghOSt-native name will be chosen (tracked as an open question).
- The 32-bit compatibility layer is **excluded from the Minimal Rescue Profile** to preserve size and simplicity.
- Design must keep the compatibility boundary explicit: 32-bit code runs in a well-defined sandbox with clear transitions to/from 64-bit services.

### 13.6 Third-Party Application Strategy

- **Phase 1:** Native ghOSt applications + web applications.
- **Phase 2:** COM runtime enables selected Windows-ecosystem applications.
- **Phase 3:** Evaluate additional compatibility layers or virtualization for remaining gaps.
- Virtualization (running a guest OS for incompatible apps) is a valid long-term option for the Full Install Profile.

---

## 14. Repository Structure

```
/boot/          Bootloader, startup assembly, UEFI entry
/kernel/        Kernel core in C and Assembly
/hal/           Hardware abstraction layer (when introduced)
/drivers/       Hardware drivers (early and optional)
/config/        JSON configs and schemas
/tools/         Build tooling, image generation scripts, utilities
/docs/          Architecture docs, this Bible, decision records
/docs/decisions/ Architecture Decision Records (ADRs)
/ux/            Future graphical UX shell
```

### 14.1 Rules

- Keep directory names short, explicit, and stable.
- Every top-level directory must have a README explaining its purpose.
- Do not nest deeply without justification.
- Test code lives adjacent to the code it tests (structure TBD as test infra matures).

---

## 15. Build System & Artifacts

### 15.1 Requirements

- Build must be reproducible from a clean checkout.
- Build must support Minimal Rescue and Full Install as separate targets.
- Incremental builds must be supported where practical.
- Warnings in kernel/boot code are treated as errors.

### 15.2 Artifacts

| Profile        | Artifact                  | Notes            |
| -------------- | ------------------------- | ---------------- |
| Minimal Rescue | Bootable floppy/USB image | Size-constrained |
| Full Install   | Installable disk image    | Full feature set |

### 15.3 Dependency Policy

- Every external dependency must be justified in writing (size, complexity, portability impact).
- Prefer no external dependencies in kernel space.
- Userland dependencies must not compromise the privacy or security posture.

### 15.4 Testing Strategy

An OS project worked on in bursts needs automated tests as a safety net. When you come back after weeks or months, tests tell you what still works.

| Layer             | Approach                                                                                           |
| ----------------- | -------------------------------------------------------------------------------------------------- |
| Unit tests        | Test individual functions (memory allocator, string ops, data structures) in a hosted environment  |
| Test framework    | Lightweight C test framework (candidates: Unity, CMocka, or custom minimal). **ADR required.**     |
| Integration tests | Test subsystem interactions (e.g., VFS + filesystem, scheduler + IPC) in QEMU                      |
| Boot smoke test   | Automated: build image → boot in QEMU → verify serial output for expected sign-of-life strings     |
| Regression tests  | Every bug fix adds a test that reproduces the bug. No fix without a test.                          |
| Test location     | Test code lives adjacent to the code it tests: `kernel/mm/tests/`, `kernel/sched/tests/`, etc.     |
| Debug vs release  | Debug builds enable assertion checks, memory poisoning, stack canaries. Release builds strip them. |
| Hardware tests    | Manual on real hardware before milestone releases. Automated testing is QEMU-only.                 |

### 15.5 CI Pipeline

Continuous integration catches regressions between work sessions. This is non-negotiable for a burst-schedule project.

| Stage                 | What it does                                                                   |
| --------------------- | ------------------------------------------------------------------------------ |
| Build (both profiles) | Cross-compile Minimal Rescue and Full Install images. Must succeed cleanly.    |
| Static analysis       | Run with `-Wall -Wextra -Werror`. Additional checks (cppcheck, etc.) optional. |
| Unit tests            | Run all unit tests in hosted environment.                                      |
| Boot smoke test       | Boot the Minimal Rescue image in QEMU, verify serial output.                   |
| Image size check      | Fail if Minimal Rescue image exceeds size budget.                              |
| Changelog check       | Warn if version was bumped without a CHANGELOG.md entry (optional gate).       |

**Platform:** GitHub Actions (free for public repos). Pipeline config lives in `/.github/workflows/`.
**Goal:** Every push to `main` must pass all CI stages. Broken main is not acceptable.

---

## 16. Versioning Architecture

ghOSt uses **OS-style versioning**, not semantic versioning (semver). The kernel/OS and the GUI/UX shell are versioned on **separate tracks** because the kernel is built first — it is the core, the foundation. The UX comes later, much later. Their version numbers will never be in lockstep.

> We build the highways first before the traffic comes.

The kernel will have many releases before the UX shell even exists. Forcing them onto a single version track would either leave the UX at an absurdly high number on day one, or artificially hold back kernel versioning while waiting for UX to catch up. Neither is acceptable.

### 16.1 Kernel / OS Version

The kernel and core OS follow a `MAJOR.MINOR.PATCH` scheme inspired by traditional OS and Linux kernel versioning:

| Component | Meaning                                                                                                        |
| --------- | -------------------------------------------------------------------------------------------------------------- |
| **MAJOR** | Fundamental architecture change, ABI break, or generational milestone (e.g., `0` → `1` = first public release) |
| **MINOR** | Feature release — new subsystem, driver, or significant capability added                                       |
| **PATCH** | Bug fix, security fix, or trivial improvement with no feature change                                           |

**Example progression:** `0.0.1` → `0.0.2` → `0.1.0` → `0.2.0` → `1.0.0`

**Rules:**

- Every kernel build **must** carry a version number. Unversioned builds are not permitted.
- The version is embedded in the kernel binary at build time (compile-time constant or build-generated header).
- The version is printed on boot (serial + framebuffer console).
- The version is queryable at runtime via syscall (for userland and UX to read).
- `CHANGELOG.md` must be updated with every version bump — no silent increments.
- Version `0.x.y` denotes pre-release. Version `1.0.0` is the first stable public release.

### 16.2 GUI / UX Shell Version

The graphical shell is versioned **independently** from the kernel because:

- **The kernel is the core and gets built first.** It will go through many versions before any UX exists.
- The UX may receive visual/feature updates without kernel changes.
- The kernel may receive driver/security patches without UX changes.
- Decoupled versioning prevents forced lockstep releases.
- The UX version will start at `0.0.1` whenever the shell is first delivered, regardless of where the kernel version stands at that point.

The UX shell uses the same `MAJOR.MINOR.PATCH` format but on its own track:

| Component | Meaning                                                                |
| --------- | ---------------------------------------------------------------------- |
| **MAJOR** | UX engine rewrite, rendering pipeline change, or breaking theme schema |
| **MINOR** | New UX feature, visual overhaul, or new default capability             |
| **PATCH** | Bug fix, theme fix, or minor visual correction                         |

**Rules:**

- UX version is stored in UX configuration and displayed in the **System → Help → About** dialog.
- The About dialog must show **both** the kernel/OS version and the UX shell version.
- UX version changes are tracked in `CHANGELOG.md` under a separate section from kernel changes.

### 16.3 Version Display (About Dialog)

When the GUI reaches maturity, the **System → Help → About** (or equivalent) panel must display:

| Field            | Source                        |
| ---------------- | ----------------------------- |
| OS name          | ghOSt                         |
| Kernel version   | Embedded in kernel binary     |
| UX shell version | From UX config / shell binary |
| Build date       | Compile-time timestamp        |
| Architecture     | x86-64 (runtime query)        |
| Profile          | Full Install / Minimal Rescue |

### 16.4 Version Tracking Discipline

Forgetting to version a build causes confusion and makes debugging impossible. These safeguards are mandatory:

| Safeguard                    | Detail                                                                                   |
| ---------------------------- | ---------------------------------------------------------------------------------------- |
| Build-time version injection | Version is injected by the build system, not manually edited in source                   |
| Version header/constant      | Single source of truth: `VERSION_MAJOR`, `VERSION_MINOR`, `VERSION_PATCH` in one file    |
| Boot banner                  | Kernel prints version on every boot (serial + console)                                   |
| Build system check           | Build fails if version has not been incremented since the last tagged release (optional) |
| Changelog enforcement        | Every version bump requires a corresponding `CHANGELOG.md` entry                         |
| Git tags                     | Every release version is tagged in git: `v0.1.0`, `vUX-0.1.0`, etc.                      |
| Pre-release label            | Builds from untagged commits append `-dev` suffix (e.g., `0.1.0-dev`)                    |

### 16.5 Git Tag Convention

| Tag format       | Meaning             | Example      |
| ---------------- | ------------------- | ------------ |
| `v{X.Y.Z}`       | Kernel / OS release | `v0.1.0`     |
| `vUX-{X.Y.Z}`    | UX shell release    | `vUX-0.1.0`  |
| `v{X.Y.Z}-rc{N}` | Release candidate   | `v0.1.0-rc1` |

---

## 17. Development Roadmap

Building an OS from scratch is one of the hardest software engineering undertakings possible. This roadmap exists to make the path honest, visible, and sequential. Each phase has hard prerequisites — skipping phases creates debt that compounds.

### 17.1 Phase 0: Toolchain & Development Environment

**Goal:** Be able to compile, link, and produce bootable artifacts before writing any OS code.

| Task               | Detail                                                                                       | Hard Problems                                                     |
| ------------------ | -------------------------------------------------------------------------------------------- | ----------------------------------------------------------------- |
| Cross-compiler     | Build or configure a GCC/Clang cross-compiler targeting x86-64 freestanding (no hosted libc) | Toolchain version pinning, reproducibility                        |
| Assembler          | NASM or GNU as for Assembly sources                                                          | Syntax choice (Intel vs AT&T) — decide once                       |
| Linker scripts     | Custom linker scripts for kernel binary layout                                               | Getting sections, alignment, and entry point right                |
| Build system       | Makefile or CMake for reproducible builds                                                    | Must support Minimal Rescue and Full Install targets from day one |
| Boot image tooling | Scripts to produce FAT32-formatted UEFI-bootable images (.img)                               | Image layout, partition tables, ESP structure                     |
| Emulator/debugger  | QEMU + OVMF (UEFI firmware) for testing without real hardware                                | GDB remote debug integration for kernel                           |
| CI pipeline        | Automated build + boot-smoke-test (future)                                                   | Reproducible across machines                                      |

**Exit criteria:** Can produce a bootable `.img` that QEMU/OVMF loads and shows a sign-of-life (e.g., colored screen or serial output).

---

### 17.2 Phase 1: UEFI Bootloader

**Goal:** A working UEFI application that gains control from firmware and prepares to hand off to the kernel.

| Task                   | Detail                                                       | Hard Problems                                                                    |
| ---------------------- | ------------------------------------------------------------ | -------------------------------------------------------------------------------- |
| UEFI application entry | Write a minimal UEFI app (`.efi` binary) loaded by firmware  | Understanding UEFI calling conventions, PE32+ format                             |
| GOP framebuffer        | Acquire Graphics Output Protocol for basic pixel output      | Mode selection, fallback if preferred mode unavailable                           |
| Memory map             | Call `GetMemoryMap()` to understand available RAM            | Memory map is volatile — must be re-read immediately before `ExitBootServices()` |
| `ExitBootServices()`   | Transition from UEFI boot services to OS-controlled runtime  | This is the point of no return — firmware memory may be reclaimed                |
| Load kernel            | Load kernel binary from filesystem on boot media into memory | Need a minimal FAT32 reader or use UEFI file protocol                            |
| Handoff structure      | Pass memory map, framebuffer info, and boot config to kernel | Define a clean boot-info struct — this is an API boundary                        |

**Key decision needed:** Use GNU-EFI, POSIX-UEFI, EDK2, or write UEFI interaction from scratch?

**Exit criteria:** UEFI app loads, gets framebuffer + memory map, loads kernel blob, jumps to kernel entry.

---

### 17.3 Phase 2: Kernel Fundamentals

**Goal:** A working 64-bit kernel that controls the CPU and memory.

| Task                    | Detail                                                                                      | Hard Problems                                                   |
| ----------------------- | ------------------------------------------------------------------------------------------- | --------------------------------------------------------------- |
| Entry point (Assembly)  | Kernel entry in long mode, set up initial stack                                             | Must already be in 64-bit mode from UEFI                        |
| GDT                     | Set up Global Descriptor Table for 64-bit flat model                                        | Segments are mostly flat in long mode but GDT is still required |
| IDT + ISRs              | Interrupt Descriptor Table, exception handlers (divide-by-zero, page fault, GP fault, etc.) | Getting ISR stubs right in Assembly, stack frame conventions    |
| PIC / APIC              | Program interrupt controller(s) for hardware interrupts                                     | Legacy PIC vs APIC/x2APIC detection and init                    |
| Physical memory manager | Parse UEFI memory map, build a physical page allocator (bitmap or free-list)                | Handling reserved/ACPI/firmware regions correctly               |
| Virtual memory manager  | Set up kernel page tables, implement `map_page()` / `unmap_page()`                          | Recursive page tables or direct-map approach; TLB management    |
| Heap allocator          | Simple kernel heap (slab, bump, or buddy) for dynamic kernel allocations                    | Fragmentation, determinism, debug-mode poisoning                |
| Serial/UART output      | Early debug output via COM1 serial port                                                     | Essential for debugging before framebuffer console works        |
| Framebuffer console     | Text rendering to GOP framebuffer (simple bitmap font)                                      | Font rendering, scrolling, cursor — keep minimal                |
| Kernel panic handler    | Catch unrecoverable errors, dump register state, halt                                       | Must work even if most subsystems are broken                    |

**Exit criteria:** Kernel boots, handles exceptions, allocates memory, prints text to screen and serial.

---

### 17.4 Phase 3: Core Kernel Services

**Goal:** Scheduling, IPC, and the syscall boundary that makes userland possible.

| Task                            | Detail                                                                 | Hard Problems                                                       |
| ------------------------------- | ---------------------------------------------------------------------- | ------------------------------------------------------------------- |
| Timer (PIT / HPET / APIC timer) | Reliable tick source for preemptive scheduling                         | Calibration differences across hardware                             |
| Thread/process model            | Kernel threads first, then user processes with separate address spaces | Context switching in Assembly, saving/restoring full CPU state      |
| Scheduler                       | Simple round-robin or priority scheduler                               | Avoiding priority inversion, idle task, timer-driven preemption     |
| Syscall interface               | `SYSCALL`/`SYSRET` (x86-64 fast syscall)                               | MSR setup, register conventions, security of user-supplied pointers |
| User-mode entry                 | Load and execute a user-mode ELF binary                                | ELF loader, user-space stack setup, ring-3 transition               |
| IPC                             | Basic message passing or shared memory                                 | Synchronization primitives (mutexes, semaphores) in kernel          |
| Signals / events                | Mechanism for kernel-to-process and process-to-process notifications   | Signal delivery during syscalls, reentrancy                         |

**Exit criteria:** Kernel can launch a userland process, preempt it, and handle syscalls from it.

---

### 17.5 Phase 4: Storage & Filesystem

**Goal:** Read and write persistent storage. This is required before any useful OS operations.

| Task            | Detail                                                                                                                                                 | Hard Problems                                                                                                                                                                                                                                                                |
| --------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------ | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| AHCI driver     | SATA controller driver for hard drives/SSDs. See Section 5.3 for architecture and pitfall catalog — read it before writing a single line of AHCI code. | PCI enumeration first, IDE-to-AHCI mode switching, HBA register init sequence (5.3.1), DMA alignment (5.3.3), COMRESET timing, port stop/start ordering, error recovery with port reset, 64-bit DMA verification, ATAPI detection. Every pitfall in 5.3.2 must be addressed. |
| NVMe driver     | NVMe controller for modern SSDs                                                                                                                        | Submission/completion queue model, PCIe config space                                                                                                                                                                                                                         |
| Partition table | GPT parsing (UEFI standard)                                                                                                                            | Protective MBR, GUID handling                                                                                                                                                                                                                                                |
| VFS layer       | Virtual Filesystem Switch — abstraction over concrete filesystems                                                                                      | Inode/dentry model, mount points, path resolution                                                                                                                                                                                                                            |
| FAT32 driver    | Required for UEFI ESP and basic interop                                                                                                                | Long filename support, cluster chaining, edge cases                                                                                                                                                                                                                          |
| Ext2/custom FS  | A real filesystem for the root partition                                                                                                               | Journal (ext3/4) adds complexity — ext2 is a simpler start                                                                                                                                                                                                                   |
| Block cache     | Cache disk blocks in memory                                                                                                                            | Cache coherency, write-back vs write-through                                                                                                                                                                                                                                 |

**Exit criteria:** Kernel can mount a root filesystem, read files, and write files.

---

### 17.6 Phase 5: Essential Hardware Drivers

**Goal:** The minimum driver set to make the system usable on real hardware.

This is one of the hardest areas. Unlike Linux (which has thousands of contributors writing drivers), ghOSt must be strategic.

| Driver class             | Initial approach                                                                 | Hard Problems                                                              |
| ------------------------ | -------------------------------------------------------------------------------- | -------------------------------------------------------------------------- |
| **USB (xHCI)**           | xHCI host controller driver — covers USB 3.x and backward compat                 | xHCI spec is ~700 pages; ring buffers, TRBs, device enumeration            |
| **USB HID**              | Keyboard + mouse via USB HID class driver                                        | HID report descriptor parsing, boot protocol fallback                      |
| **PS/2 fallback**        | PS/2 keyboard/mouse for legacy and VM support                                    | Simple but still needed for QEMU/older hardware                            |
| **PCI/PCIe enumeration** | Scan PCI bus, identify devices, allocate BARs                                    | Foundation for all PCI-based drivers (AHCI, NVMe, USB, GPU, NIC)           |
| **ACPI**                 | Parse ACPI tables (RSDP → RSDT/XSDT → MADT, FADT, etc.)                          | Required for SMP, power management, interrupt routing                      |
| **Framebuffer (GOP)**    | Use UEFI GOP framebuffer passed at boot                                          | No hardware acceleration — just pixel pushing. Sufficient for early stages |
| **Real GPU driver**      | This is the elephant in the room (see below)                                     | **Extremely hard. Deferred to later phase.**                               |
| **Audio**                | Basic HDA (High Definition Audio) controller                                     | HDA codec discovery, stream management, buffer DMA                         |
| **Network (NIC)**        | Start with one well-documented NIC family (e.g., Intel e1000/i210 or virtio-net) | Ring buffers, DMA, interrupt coalescing                                    |
| **RTC**                  | Real-time clock for timekeeping                                                  | CMOS access, BCD conversion                                                |

#### The GPU Driver Problem (Honest Assessment)

Modern GPU drivers are among the most complex software ever written. AMD's open-source Linux driver is ~3 million lines. NVIDIA's is proprietary. Intel's is ~1.5 million lines.

**ghOSt's realistic GPU strategy:**

1. **Phase 1 (now):** UEFI GOP framebuffer only. Software rendering. No acceleration.
2. **Phase 2:** Basic VESA/GOP mode switching. Simple 2D compositor for the GUI shell.
3. **Phase 3:** Target one GPU family with open documentation (likely AMD — they publish open-source register specs). Implement basic modesetting + 2D acceleration.
4. **Phase 4 (long-term):** Evaluate porting/adapting Mesa or writing a minimal Vulkan/OpenGL path.
5. **Fallback:** For GPUs we don't support, the system falls back to GOP framebuffer (functional but slow).

This is a multi-year problem. The GUI UX must be designed to work acceptably on a software-rendered framebuffer first.

#### The Audio Driver Problem

HDA (High Definition Audio) is the dominant audio interface on PC hardware. The spec is publicly available (Intel HDA specification). It's complex but achievable:

- Codec discovery and initialization
- Stream descriptor setup (output/input)
- DMA buffer management
- Mixer/volume control

A basic "play PCM audio" driver is a realistic early target. Full mixer/multi-stream support comes later.

#### Driver Strategy Summary

| Priority                | Driver                                                                   | Reason                                 |
| ----------------------- | ------------------------------------------------------------------------ | -------------------------------------- |
| P0 — Must have          | PCI enumeration, PS/2 keyboard, framebuffer console, serial, timer, AHCI | Can't function without these           |
| P1 — Boot-critical      | USB (xHCI), USB HID, GPT/FAT32, memory management                        | Required for real hardware support     |
| P2 — Core functionality | NVMe, ext2/custom FS, NIC (one family), ACPI, RTC                        | Required for Full Install to be useful |
| P3 — Full experience    | HDA audio, 2D GPU accel, additional NIC families, advanced USB classes   | Required for daily-use OS              |
| P4 — Long-term          | 3D GPU acceleration, Bluetooth, WiFi, webcam, advanced power management  | Hard, specialized, long timeline       |

---

### 17.7 Phase 6: Userland Foundation

**Goal:** A usable userland environment before any GUI work.

| Task                | Detail                                                                                     | Hard Problems                                                 |
| ------------------- | ------------------------------------------------------------------------------------------ | ------------------------------------------------------------- |
| C runtime (libc)    | Minimal freestanding libc: `string.h`, `stdio.h` (serial/framebuffer), `stdlib.h`, `errno` | Deciding scope — full POSIX or ghOSt-native subset?           |
| ELF loader          | Load and execute ELF64 binaries from filesystem                                            | Dynamic linking (deferred), relocations, `mmap`-style loading |
| Shell (CLI)         | Command-line interpreter: parse input, launch programs, built-in commands                  | Job control, piping, environment variables (phased)           |
| Core utilities      | `ls`, `cat`, `cp`, `mv`, `rm`, `mkdir`, `mount`, `format`, `echo`, `clear`, etc.           | Each one exercises different syscalls — good test surface     |
| Dynamic linker      | `ld.so` equivalent for shared libraries                                                    | Symbol resolution, lazy binding, position-independent code    |
| `/dev` device model | Device nodes for accessing drivers from userland                                           | Device registration, major/minor numbers or name-based        |

**Exit criteria:** Can boot to a CLI shell, navigate filesystem, run simple programs. This is the Minimal Rescue Profile milestone.

---

### 17.8 Phase 7: Preinstallation Environment (ghOSt PE)

**Goal:** A bootable environment that can install the Full Install Profile onto a target disk.

| Task                     | Detail                                                            | Hard Problems                                             |
| ------------------------ | ----------------------------------------------------------------- | --------------------------------------------------------- |
| Bootable PE image        | Minimal bootable image (USB/network) containing installer         | Must fit comfortably on USB; shares Minimal Rescue kernel |
| Disk partitioning        | Create GPT partition table, ESP, root partition                   | Safe handling of existing partition tables                |
| Filesystem formatting    | Format partitions (FAT32 for ESP, ext2/custom for root)           | On-disk layout correctness at format time                 |
| File copy engine         | Copy OS files from install media to target disk                   | Progress feedback, error handling, verification           |
| Boot entry setup         | Write UEFI boot entry for installed system                        | UEFI NVRAM variables or fallback boot path                |
| Configuration wizard     | Minimal text-mode setup: hostname, locale, timezone, user account | Keep it simple — no GUI needed                            |
| Network install (future) | Pull install image over network                                   | Requires network stack — deferred                         |

**Relationship to Minimal Rescue:** ghOSt PE can share most of the Minimal Rescue Profile infrastructure. The installer is essentially a specialized program running on the Rescue environment.

**Exit criteria:** Can boot PE media, partition a disk, install ghOSt, reboot into installed system.

---

### 17.9 Phase 8: Boot Manager

**Goal:** A proper boot manager that handles boot selection, recovery, and multi-boot.

| Task                             | Detail                                                              | Hard Problems                                                          |
| -------------------------------- | ------------------------------------------------------------------- | ---------------------------------------------------------------------- |
| Boot menu                        | Select between: Full Install, Minimal Rescue, recovery mode         | Timeout, keyboard input at UEFI or early-kernel stage                  |
| Boot configuration               | JSON config on ESP defining boot entries and defaults               | Config validation at boot time with minimal code                       |
| Recovery mode                    | Boot into Minimal Rescue from installed system's recovery partition | Must work even if Full Install is broken                               |
| Chainloading                     | Optionally chainload other OS bootloaders for multi-boot            | UEFI makes this easier than legacy BIOS — use `LoadImage`/`StartImage` |
| Secure Boot integration (future) | Sign bootloader and kernel for Secure Boot chains                   | Requires key enrollment, signing infrastructure                        |

**Exit criteria:** Boot manager presents menu, can boot Full Install or Rescue, recovery works from installed disk.

---

### 17.10 Phase 9: Networking Stack

**Goal:** TCP/IP networking for the Full Install Profile.

| Task             | Detail                                     | Hard Problems                                                         |
| ---------------- | ------------------------------------------ | --------------------------------------------------------------------- |
| NIC driver(s)    | At least one real NIC + virtio-net for VMs | See driver section above                                              |
| Ethernet framing | Send/receive Ethernet frames               | MAC addressing, MTU handling                                          |
| ARP              | Address Resolution Protocol                | Cache management, timeout                                             |
| IP (v4 first)    | Packet routing, fragmentation/reassembly   | Routing table, localhost, checksum                                    |
| ICMP             | Ping, error messages                       | Useful for diagnostics from day one                                   |
| UDP              | Connectionless datagrams                   | Port multiplexing, DNS depends on this                                |
| TCP              | Connection-oriented reliable stream        | State machine (SYN/ACK/FIN), retransmission, windowing — this is hard |
| DNS resolver     | Hostname resolution                        | Stub resolver is sufficient initially                                 |
| DHCP client      | Auto-configure IP address                  | Lease management, renewal                                             |
| Sockets API      | Userland interface to networking           | BSD sockets or ghOSt-native?                                          |
| TLS (future)     | Encrypted connections                      | Crypto library dependency, certificate handling — big effort          |

**Exit criteria:** Can ping, resolve DNS, make TCP connections, fetch data over HTTP.

---

### 17.11 Phase 10: GUI Compositor & Shell

**Goal:** A graphical desktop environment. This is the point where ghOSt becomes a "real desktop OS."

**Prerequisites:** Working framebuffer, mouse input, keyboard input, filesystem, userland, IPC.

| Task                       | Detail                                                            | Hard Problems                                       |
| -------------------------- | ----------------------------------------------------------------- | --------------------------------------------------- |
| Window compositor          | Manage window buffers, Z-order, compositing, damage tracking      | Performance with software rendering, tearing        |
| Input routing              | Deliver keyboard/mouse events to correct window                   | Focus management, input capture                     |
| Window manager             | Placement, resize, minimize, maximize, tiling                     | Policy-driven (not hardcoded)                       |
| Widget toolkit / rendering | Text rendering (font rasterization), basic widgets                | FreeType or custom font engine, subpixel rendering  |
| Web-style UX runtime       | HTML/CSS/JS renderer for shell UI (or embed a lightweight engine) | This is essentially building a browser — huge scope |
| Clipboard / DnD            | Clipboard and drag-and-drop between windows                       | IPC, MIME types, serialization                      |
| Theming                    | JSON-driven appearance customization                              | CSS-like or direct JSON? Define early               |
| Taskbar / launcher         | Basic application launcher and window list                        | Connects to window manager and process list         |

**Realistic assessment:** A basic tiling/stacking window manager with software rendering is achievable relatively early after Phase 9. The "browser-like UX" aspiration (HTML/CSS/JS shell) is a major investment — potentially embedding or adapting an existing engine (e.g., Servo components, WebKit subset, or a custom renderer).

**Exit criteria:** Can display multiple windows, switch between them, run graphical applications with keyboard/mouse input.

---

### 17.12 Phase 11: Application Framework & COM

**Goal:** Full Install has a real application model that third-party software can target.

This is where the application compatibility ambitions (COM, 32-bit compat) come together.

| Task                      | Detail                                                  |
| ------------------------- | ------------------------------------------------------- |
| Application packaging     | Define how apps are installed, stored, and launched     |
| Permission model          | Sandboxing, capability-based access control             |
| COM runtime (core)        | `CoCreateInstance`, class factory, interface marshaling |
| COM registry/activation   | Component registration and lookup                       |
| Threading apartments      | STA/MTA model for COM threading                         |
| 32-bit thunking layer     | Translate 32-bit syscalls and API calls to 64-bit       |
| OLE / ActiveX (selective) | Only implement what's needed for target applications    |

---

### 17.13 Phase 12: Local AI Subsystem (Future)

**Goal:** Provide on-device AI capabilities with zero cloud dependency by default.

| Task                                   | Detail                                                                                       | Hard Problems                                                                |
| -------------------------------------- | -------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------- |
| Inference runtime                      | Integrate or build a local inference engine (e.g., ONNX Runtime, llama.cpp-style, or custom) | Model format support, memory footprint, CPU vs GPU dispatch                  |
| Model management                       | Local model storage, loading, versioning                                                     | Disk space, secure model integrity verification                              |
| AI service API                         | Userland API for applications to request inference (text, image, embeddings, etc.)           | API surface design, sandboxing inference workloads                           |
| Hardware acceleration                  | Leverage GPU/NPU for inference when drivers support it                                       | Depends on GPU driver maturity (Phase 5 / long-term)                         |
| Privacy enforcement                    | All inference is local. No model phones home. No training data leaves the device.            | Auditing third-party models for telemetry, sandboxing model I/O              |
| Cloud AI opt-in (user-configured only) | Allow user to manually configure external AI endpoints if desired                            | Clear UX consent flow, no default cloud endpoints, no bundled cloud accounts |

**Rules:**

- AI features are **local-only by default**. No cloud AI services ship enabled.
- No cloud account is required, created, or suggested during setup.
- Users may optionally configure cloud AI endpoints themselves — this is a power-user feature, not a default.
- AI subsystem must run within the OS permission/sandboxing model.
- Models shipped with the OS (if any) must be open-weight / redistributable.

**Exit criteria:** A local inference engine runs on-device, applications can call it via a documented API, and no network traffic is generated unless the user explicitly configured a remote endpoint.

---

### 17.14 Phase Summary & Dependency Graph

```
Phase 0: Toolchain ──┐
                      ▼
Phase 1: UEFI Bootloader ──┐
                            ▼
Phase 2: Kernel Fundamentals ──┐
                               ▼
Phase 3: Core Kernel Services ──┬──────────────────┐
                                ▼                  ▼
Phase 4: Storage & FS    Phase 5: Essential Drivers
         │                       │
         └────────┬──────────────┘
                  ▼
Phase 6: Userland Foundation ──┐
         │                     │
         ▼                     ▼
Phase 7: ghOSt PE        Phase 8: Boot Manager
         │
         ▼
Phase 9: Networking ──┐
                      ▼
Phase 10: GUI Compositor & Shell ──┐
                                   ▼
Phase 11: App Framework & COM ──┐
                                ▼
Phase 12: Local AI Subsystem
```

````

Each phase must be stable and tested before the next begins. Phases 4/5 and 7/8 can overlap internally, but their collective output is required before Phase 6 is complete.

---

### 17.15 Known Hard Problems (Requires Ongoing Research)

| Problem             | Why it's hard                                                      | Current strategy                                                   |
| ------------------- | ------------------------------------------------------------------ | ------------------------------------------------------------------ |
| GPU drivers         | Millions of lines for modern GPUs; specs are partially proprietary | GOP framebuffer first, then target AMD (open specs)                |
| WiFi drivers        | Firmware blobs, complex state machines, per-chipset differences    | Deferred. Ethernet first. Evaluate open firmware chips later       |
| Audio drivers       | HDA spec is complex; per-codec initialization varies               | Target HDA with basic PCM playback first                           |
| USB                 | xHCI is ~700 pages of spec; per-class drivers on top               | Implement xHCI core + HID first, expand classes over time          |
| Filesystem maturity | Data corruption is unacceptable; needs exhaustive testing          | Start with FAT32 (well-understood), then build custom FS carefully |
| TCP/IP correctness  | Edge cases, congestion, fragmentation, security                    | Implement incrementally, test against real stacks                  |
| Browser-like UX     | Essentially building a web engine                                  | Evaluate embedding existing tech vs custom; phased approach        |
| Secure Boot         | Requires signing infrastructure, key management, Microsoft UEFI CA | Design-compatible now, implement later                             |
| SMP (multi-core)    | AP startup, cache coherency, lock-free data structures             | Single-core first, SMP after scheduler is stable                   |
| IOMMU               | DMA isolation, interrupt remapping — required for security on PCIe | Parse DMAR/IVRS tables; enable before any device does DMA          |
| NUMA awareness       | Memory allocation locality on multi-socket/chiplet systems         | Single-socket first; SRAT/SLIT parsing added when multi-socket targeted |

---

## 18. Decision Records

All significant architecture decisions are recorded in `/docs/decisions/` using lightweight ADR format.

### ADR Template

```markdown
# ADR-NNNN: [Title]

**Status:** proposed | accepted | superseded | deprecated
**Date:** YYYY-MM-DD
**Author(s):** [name(s)]

## Context

[Why this decision is needed]

## Decision

[What was decided]

## Alternatives Considered

[What else was evaluated and why it was rejected]

## Consequences

[What follows from this decision — positive and negative]
````

### Rules

- Decisions are **never deleted** — only superseded.
- Every decision that changes a Bible section must reference the relevant ADR.
- Trivial implementation choices do not need ADRs. Architecture, security, and compatibility choices do.

---

## 19. Contribution Standards

### 19.1 Code Quality

- All kernel/boot code: C and Assembly only. No exceptions.
- All C code must follow the memory discipline rules in this document.
- All code must build cleanly with warnings-as-errors in CI.
- Comments are technical and concise. Explain _why_, not _what_.
- **No scaffolded or stubbed code in kernel space.** Every committed kernel function must be fully implemented and correct. See Section 4.8.
- Userland stubs are permitted only when clearly marked and feature-gated.

### 19.2 Documentation Quality

- Architecture-impacting changes require updates to this Bible or a new ADR.
- Every new subsystem requires a README in its directory.
- Memory layouts, hardware assumptions, and trust boundaries are documented near the code.

### 19.3 Review Standards

- Kernel-space changes require review with explicit attention to memory safety and security boundaries.
- Changes to this Bible require review from **AnotherLaughingMan** (sole owner).
- COM runtime and security-critical changes require security-focused review.

### 19.4 Conduct

- This is a serious engineering project. Contributions are evaluated on technical merit.
- The project's values (user sovereignty, privacy, censorship resistance) are non-negotiable. Contributions that undermine them will be rejected.

---

## 20. Glossary

| Term           | Definition                                                                                            |
| -------------- | ----------------------------------------------------------------------------------------------------- |
| ghOSt          | The operating system. Stylized as **ghOSt**, pronounced "Ghost OS".                                   |
| Minimal Rescue | The floppy/USB CLI-only deployment profile.                                                           |
| Full Install   | The complete desktop/server deployment profile.                                                       |
| 32-bit compat  | The built-in subsystem that enables 32-bit apps to run on the 64-bit kernel (Full Install only).      |
| COM            | Component Object Model — a binary interface standard for interprocess communication.                  |
| HAL            | Hardware Abstraction Layer.                                                                           |
| ADR            | Architecture Decision Record.                                                                         |
| ABI            | Application Binary Interface — the syscall contract between kernel and userland.                      |
| Bible          | This document. The canonical design reference for ghOSt.                                              |
| Profile gate   | A build-time or runtime control that includes/excludes features per deployment profile.               |
| Local-first    | Functionality that works fully offline without external services.                                     |
| Trust boundary | A point in the architecture where privilege level or data trust level changes.                        |
| ghOSt PE       | Preinstallation Environment — bootable image used to install ghOSt onto a target disk.                |
| GOP            | Graphics Output Protocol — UEFI interface for framebuffer access before a GPU driver is loaded.       |
| xHCI           | Extensible Host Controller Interface — USB 3.x host controller standard.                              |
| HDA            | High Definition Audio — Intel's audio controller specification for PC audio hardware.                 |
| SMP            | Symmetric Multiprocessing — using multiple CPU cores.                                                 |
| IOMMU          | Input/Output Memory Management Unit — provides DMA address translation and device isolation.          |
| NUMA           | Non-Uniform Memory Access — memory architecture where access latency depends on CPU-memory proximity. |
| x2APIC         | Extended APIC mode — MSR-based, supports >255 logical processors. Required for many-core systems.     |
| SMBIOS         | System Management BIOS — firmware tables providing hardware inventory (board, CPU, memory info).      |
| ECC            | Error-Correcting Code memory — detects and corrects single-bit errors in server/workstation RAM.      |
| IOAPIC         | I/O APIC — routes external hardware interrupts to Local APICs on individual CPU cores.                |
| MCA            | Machine Check Architecture — CPU mechanism for reporting hardware errors (memory, bus, cache).        |
| RSS            | Receive Side Scaling — NIC feature that distributes incoming packets across multiple CPU cores.       |
| Local AI       | On-device AI inference with no cloud dependency. Cloud AI is user-opt-in only.                        |
| UTF-8          | The canonical text encoding for ghOSt. All strings, filenames, configs, and APIs use UTF-8.           |
| FDE            | Full-disk encryption — encrypting the entire disk at rest with a user-owned key.                      |
| a11y           | Accessibility — designing the UX so it is usable by people with disabilities.                         |
| HiDPI          | High dots-per-inch display support. Requires integer or fractional scaling in the compositor.         |
| CI             | Continuous Integration — automated build, test, and verification pipeline.                            |

---

## 21. Appendices

### Appendix A: Related Documents

| Document                      | Location                   | Purpose                         |
| ----------------------------- | -------------------------- | ------------------------------- |
| Copilot Instructions          | `/copilot-instructions.md` | AI assistant behavioral rules   |
| Architecture Decision Records | `/docs/decisions/`         | Individual design decisions     |
| Changelog                     | `/CHANGELOG.md`            | Version history and release log |
| Issues Log                    | `/docs/ISSUES.md`          | Issue tracking and status       |

### Appendix B: Revision History

| Date       | Change                                                                                                                                                                                                                                                                                                                                                                                          | Author             |
| ---------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------ |
| 2026-03-07 | Initial creation. Codified from copilot-instructions.md                                                                                                                                                                                                                                                                                                                                         | AnotherLaughingMan |
| 2026-03-07 | Added GPU-accelerated rendering architecture (Section 12.4), changelog and issues tracking                                                                                                                                                                                                                                                                                                      | AnotherLaughingMan |
| 2026-03-07 | Added Versioning Architecture (Section 16) — OS-style versioning with separate kernel/UX tracks                                                                                                                                                                                                                                                                                                 | AnotherLaughingMan |
| 2026-03-07 | Gap audit: added C coding style (7.4), text encoding (7.5), error handling (8.4), logging (8.5), user/permission model (10.4), disk encryption (10.5), ABI policy (13.2), process lifecycle (13.3), testing (15.4), CI (15.5), accessibility (12.6), multi-monitor/HiDPI (12.7), notifications (12.8), font rendering (12.9), deferred items (12.10). Expanded Appendix C to 24 open questions. | AnotherLaughingMan |
| 2026-03-07 | Added No Scaffolding rule (Section 4.8), Driver Architecture (5.1), PCI/PCIe bus enumeration (5.2), AHCI pitfall catalog (5.3)                                                                                                                                                                                                                                                                  | AnotherLaughingMan |
| 2026-03-07 | Expanded PCIe to full modern standard (5.2.1–5.2.5: ECAM, capability structures, enumeration, error handling). Added modern platform standards (5.4): IOMMU, APIC/x2APIC, NUMA, SMBIOS, ECC, hardware watchdog, headless/serial, network boot, high-speed NICs. Expanded Full Install Profile for server deployments. Updated Appendix C to 27 open questions.                                  | AnotherLaughingMan |
| 2026-03-07 | Added kernel security hardening baseline (10.6), local-only parental controls and child-account policy (10.7), family safety UX surface (12.10), and related tracked issues. Updated Appendix C to 29 open questions.                                                                                                                                                                           | AnotherLaughingMan |
| 2026-03-07 | Added user-space security and threat-detection model (10.8), including realistic treatment of known threats vs true 0-days, plus Security Center UX (12.11). Updated Appendix C to 31 open questions.                                                                                                                                                                                           | AnotherLaughingMan |
| 2026-03-07 | Added system update architecture and servicing model (10.9), including update classes, channels, A/B-style rollback, kernel/security update handling, and Update UX (12.12). Updated Appendix C to 33 open questions.                                                                                                                                                                           | AnotherLaughingMan |
| 2026-03-07 | Expanded update design with manifest requirements, slot/health-state model, phased servicing implementation plan, and concrete desktop/server/recovery UX flows for servicing and rollback.                                                                                                                                                                                                     | AnotherLaughingMan |

### Appendix C: Open Questions (To Be Decided)

These are known areas that need formal decisions (future ADRs):

**Architecture & Kernel:**

1. Kernel architecture: monolithic vs microkernel vs hybrid?
2. Filesystem design: custom VFS vs adopt existing?
3. Driver model: in-kernel vs userspace drivers?
4. Init system design: custom init, service dependencies, shutdown sequencing?
5. Permission model: capability-based vs traditional UNIX-style?
6. ABI freeze policy: when does the syscall interface stabilize?
7. Swap / virtual memory policy: swap-to-disk, OOM killer, overcommit behavior?
8. Power management: sleep/wake states (S0–S5), shutdown sequencing, lid/button handling?
9. Time architecture: UTC internal, wall-clock conversion, NTP, monotonic vs real-time clock?

**Security & Privacy:** 10. Disk encryption: FDE vs partition encryption, key derivation algorithm, boot unlock flow? 11. Update/package system: design, trust model, signed updates, A/B partitions? 12. Kernel hardening baseline: which mitigations are mandatory in debug, rescue, and release builds (KASLR, SMEP/SMAP, stack protector, guard pages)? 13. Built-in threat detection scope: how much should ghOSt ship natively versus relying on third-party security tools? 14. Update trust roots and signing: how are release keys rotated, revoked, and recovered if compromised?

**UX & Applications:** 15. UX runtime: embedded browser engine vs custom renderer? 16. Font rendering engine: FreeType vs stb_truetype vs custom? 17. COM implementation strategy: phased coverage plan? 18. 32-bit compatibility subsystem: naming, thunking strategy, and API coverage scope? 19. Parental controls policy: what is the minimum built-in family safety surface before crossing into unwanted censorship or complexity? 20. Security UX policy: what user-facing security decisions should be automatic, suggested, or always manual? 21. Update UX policy: when should the system auto-install, notify, defer, or require explicit approval?

**Infrastructure:** 22. Test framework selection: Unity, CMocka, or custom? 23. CI pipeline: GitHub Actions configuration, smoke test design? 24. C coding style: confirm or amend the conventions in Section 7.4?

**Networking & Services:** 25. Networking stack: scope and phasing? 26. Package management / software distribution: package format, repository model?

**Hardware & Drivers:** 27. Virtualization subsystem: hypervisor type and integration? 28. GPU driver target: AMD AMDGPU open specs as first target? 29. IOMMU policy: strict DMA isolation by default, or opt-in per device class? 30. NUMA allocation policy: local-first with interleave fallback, or topology-guided with affinities? 31. Server sub-profile: should Full Install have explicit "desktop" and "server" sub-profiles with different default configs?

**Future:** 32. Local AI subsystem: inference runtime selection, model format, API surface, GPU/NPU dispatch? 33. Printing support: scope, driver model, and timeline?

---

> **This document is the law of the land for ghOSt development.**
> If it's not in the Bible, it's not a project commitment.
> If it contradicts the Bible, the Bible wins until amended through proper process.
