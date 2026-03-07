# ghOSt Changelog

All notable changes to the ghOSt project are documented in this file.

ghOSt uses **OS-style versioning** (`MAJOR.MINOR.PATCH`), not semantic versioning.
The kernel/OS and the GUI/UX shell are versioned on **separate tracks** — the kernel is the
core and gets built first. The UX comes later, much later. We build the highways first
before the traffic comes.
See the [Development Bible](docs/DEVELOPMENT_BIBLE.md) Section 16 for the full versioning architecture.

> **Pre-release note:** ghOSt is in early development. All `0.x.y` versions are pre-release.
> Version `1.0.0` will mark the first stable public release.

---

## Kernel / OS Changelog

### [Unreleased] — Kernel

#### Added

- Project foundation: repository initialized with core documentation.
- **Development Bible** (`/docs/DEVELOPMENT_BIBLE.md`) — canonical design reference covering architecture, philosophy, and roadmap.
- **Copilot Instructions** (`/copilot-instructions.md`) — AI assistant behavioral rules (local only, not tracked in git).
- **README** (`/README.md`) — project landing page with mission, technology summary, and documentation links.
- `.gitignore` configured for build artifacts, IDE files, and local-only documents.
- Dual deployment profile architecture defined: **Minimal Rescue** (floppy/USB CLI) and **Full Install** (desktop/server GUI).
- x86-64 native architecture with planned 32-bit compatibility subsystem (Full Install only).
- UEFI boot requirement established; Secure Boot and TPM optional by design.
- Language boundaries formalized: Assembly + C (kernel), C++ / HTML / CSS / JS / TS (UX), JSON (config).
- Memory discipline rules codified for C and Assembly kernel code.
- Privacy and anti-censorship principles established: no telemetry, no mandatory accounts, local-first, local AI only by default.
- COM (Component Object Model) support planned as long-term Full Install milestone.
- 13-phase development roadmap (Phase 0–12) with dependency graph.
- Versioning architecture defined: OS-style `MAJOR.MINOR.PATCH` with separate kernel and UX tracks.
- Changelog (`/CHANGELOG.md`) and issue tracking (`/docs/ISSUES.md`) introduced.

---

## GUI / UX Shell Changelog

### [Unreleased] — UX Shell

#### Added

- Dark mode established as default theme; theme architecture defined (JSON + CSS tokens, hot-swap, schema versioning).
- GPU-accelerated UX rendering architecture defined with mandatory software fallback.
- UX version display planned for **System → Help → About** dialog (shows both kernel and UX versions).

---

## Release History

_No releases yet. Development is in the documentation and planning phase._
_This project is worked on in bursts — some long, some short. It is a ground-up OS; there are no shortcuts._

---

## Legend

| Tag            | Meaning                                          |
| -------------- | ------------------------------------------------ |
| **Added**      | New features or capabilities.                    |
| **Changed**    | Modifications to existing functionality.         |
| **Deprecated** | Features marked for future removal.              |
| **Removed**    | Features removed in this version.                |
| **Fixed**      | Bug fixes.                                       |
| **Security**   | Changes addressing vulnerabilities or hardening. |
