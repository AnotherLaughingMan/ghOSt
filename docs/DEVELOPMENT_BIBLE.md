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
16. [Decision Records](#16-decision-records)
17. [Contribution Standards](#17-contribution-standards)
18. [Glossary](#18-glossary)
19. [Appendices](#19-appendices)

---

## 1. What Is ghOSt

**ghOSt** (pronounced "Ghost OS") is a custom, open-source operating system built from scratch.

It ships as two distinct profiles from a single codebase:

- **Minimal Rescue Profile**: a floppy/USB-bootable CLI environment for quick system operations (formatting, diagnostics, recovery).
- **Full Install Profile**: a complete desktop/server operating system with graphical UX, broad application compatibility, and full COM support.

ghOSt is not a Linux distribution, not a fork of an existing OS, and not a wrapper. It is a ground-up operating system with its own kernel, boot path, driver model, and user experience.

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

---

## 6. Deployment Profiles

### 6.1 Minimal Rescue Profile

| Attribute       | Constraint                                              |
| --------------- | ------------------------------------------------------- |
| Target media    | Floppy disk image, USB thumb drive                      |
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
| UX              | Full graphical UX (browser-like) + CLI             |
| Purpose         | Daily-use desktop/server operating system          |
| Feature scope   | Full kernel + all drivers + GUI + app compat + COM |
| Size budget     | Practical but not unconstrained; justify bloat     |
| Graphical stack | **Required**                                       |
| COM runtime     | **Required** (long-term milestone)                 |
| Network         | Full networking stack                              |

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
- **No mandatory accounts.** The OS is fully functional without any online identity.
- **No age verification.** The OS does not implement or enforce age-gating mechanisms.
- **Local-first.** All core functionality works offline.
- **User-owned encryption.** Keys are generated and stored locally. The OS has no backdoor access.

### 10.3 Security Design Rules

- Every trust boundary in the architecture must be explicit and documented.
- Kernel syscall interfaces must validate all inputs.
- Least-privilege is the default for all access — escalation must be explicit.
- Compliance-sensitive controls (if any) must be modular, policy-driven, and **never hardcoded into the kernel**.
- Security-critical code paths must be minimal, auditable, and separately reviewable.

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

### 12.3 UX Design Rules

- Separate rendering/input/runtime from policy/config.
- Keep customization surfaces intentional and documented.
- Never embed policy decisions (censorship, content filtering, access restrictions) in the UX layer.
- UX must work without network access (local-first).

---

## 13. Application Compatibility Strategy

### 13.1 Native Applications

- ghOSt will define its own native application interface (syscall + userland API surface).
- Native apps are written in C, C++, or web technologies (HTML/CSS/JS/TS via the UX runtime).

### 13.2 COM Support (Full Install, Long-Term)

- Full COM (Component Object Model) support is a planned major milestone for the Full Install Profile.
- This requires implementing: COM runtime, interface marshaling, class factories, registry/activation model, threading apartments, and selected OLE/ActiveX surfaces.
- COM is explicitly **out of scope for Minimal Rescue Profile**.
- Approach: phased implementation — core runtime first, then interface coverage expands based on target application needs.

### 13.3 Third-Party Application Strategy

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

---

## 16. Decision Records

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
```

### Rules

- Decisions are **never deleted** — only superseded.
- Every decision that changes a Bible section must reference the relevant ADR.
- Trivial implementation choices do not need ADRs. Architecture, security, and compatibility choices do.

---

## 17. Contribution Standards

### 17.1 Code Quality

- All kernel/boot code: C and Assembly only. No exceptions.
- All C code must follow the memory discipline rules in this document.
- All code must build cleanly with warnings-as-errors in CI.
- Comments are technical and concise. Explain _why_, not _what_.

### 17.2 Documentation Quality

- Architecture-impacting changes require updates to this Bible or a new ADR.
- Every new subsystem requires a README in its directory.
- Memory layouts, hardware assumptions, and trust boundaries are documented near the code.

### 17.3 Review Standards

- Kernel-space changes require review with explicit attention to memory safety and security boundaries.
- Changes to this Bible require review from **AnotherLaughingMan** (sole owner).
- COM runtime and security-critical changes require security-focused review.

### 17.4 Conduct

- This is a serious engineering project. Contributions are evaluated on technical merit.
- The project's values (user sovereignty, privacy, censorship resistance) are non-negotiable. Contributions that undermine them will be rejected.

---

## 18. Glossary

| Term           | Definition                                                                              |
| -------------- | --------------------------------------------------------------------------------------- |
| ghOSt          | The operating system. Stylized as **ghOSt**, pronounced "Ghost OS".                     |
| Minimal Rescue | The floppy/USB CLI-only deployment profile.                                             |
| Full Install   | The complete desktop/server deployment profile.                                         |
| COM            | Component Object Model — a binary interface standard for interprocess communication.    |
| HAL            | Hardware Abstraction Layer.                                                             |
| ADR            | Architecture Decision Record.                                                           |
| Bible          | This document. The canonical design reference for ghOSt.                                |
| Profile gate   | A build-time or runtime control that includes/excludes features per deployment profile. |
| Local-first    | Functionality that works fully offline without external services.                       |
| Trust boundary | A point in the architecture where privilege level or data trust level changes.          |

---

## 19. Appendices

### Appendix A: Related Documents

| Document                      | Location                   | Purpose                       |
| ----------------------------- | -------------------------- | ----------------------------- |
| Copilot Instructions          | `/copilot-instructions.md` | AI assistant behavioral rules |
| Architecture Decision Records | `/docs/decisions/`         | Individual design decisions   |

### Appendix B: Revision History

| Date       | Change                                                  | Author             |
| ---------- | ------------------------------------------------------- | ------------------ |
| 2026-03-07 | Initial creation. Codified from copilot-instructions.md | AnotherLaughingMan |

### Appendix C: Open Questions (To Be Decided)

These are known areas that need formal decisions (future ADRs):

1. Kernel architecture: monolithic vs microkernel vs hybrid?
2. Filesystem design: custom VFS vs adopt existing?
3. UX runtime: embedded browser engine vs custom renderer?
4. COM implementation strategy: phased coverage plan?
5. Driver model: in-kernel vs userspace drivers?
6. Networking stack: scope and phasing?
7. Update/package system: design and trust model?
8. Virtualization subsystem: hypervisor type and integration?

---

> **This document is the law of the land for ghOSt development.**
> If it's not in the Bible, it's not a project commitment.
> If it contradicts the Bible, the Bible wins until amended through proper process.
