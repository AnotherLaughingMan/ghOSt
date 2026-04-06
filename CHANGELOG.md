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
- **LICENSE** (`/LICENSE`) — project licensed under GPL-3.0-or-later.
- **ADR-0001** (`/docs/decisions/ADR-0001-license-selection.md`) — license decision and rationale.
- **ADR-0002** (`/docs/decisions/ADR-0002-kernel-architecture-microkernel.md`) — kernel architecture decision and rationale.
- **ADR-0003** (`/docs/decisions/ADR-0003-toolchain-selection.md`) — toolchain selection and rationale.
- **ADR-0004** (`/docs/decisions/ADR-0004-uefi-boot-approach.md`) — early UEFI boot approach decision and rationale.
- **Phase 0 Windows Setup** (`/docs/PHASE0_WINDOWS_SETUP.md`) — low-risk Windows bootstrap guidance for Clang/LLD, NASM, QEMU, and OVMF.
- **Phase 0 Windows Scripts** (`/tools/phase0/windows/`) — PowerShell helpers and a manifest for explicit-path toolchain validation and QEMU/OVMF launch.
- **Scratch Image Boot Helper** (`/tools/phase0/windows/New-ghOStScratchImageAndBoot.ps1`) — one-command raw image creation plus safe QEMU/OVMF launch for early boot testing.
- **Minimal UEFI Sign-of-Life Source** (`/boot/uefi/`) — first repo-local x86-64 UEFI application and minimal internal header layer for early bring-up.
- **Phase 1 UEFI Build Script** (`/tools/phase1/windows/Build-ghOStUefiSignOfLife.ps1`) — emits a PE/COFF `.efi` artifact with Clang + `lld-link` and stages `EFI/BOOT/BOOTX64.EFI`.
- **UEFI Image Packager** (`/tools/phase1/windows/New-ghOStUefiImage.ps1`) — creates a real GPT/FAT32 bootable `.img` from the staged ESP directory using `diskpart` and `qemu-img`.
- **LLDB Debug Launcher** (`/tools/phase1/windows/Start-ghOStGdb.ps1`) — attaches LLDB to the QEMU GDB stub with the `.efi` symbol file pre-loaded.
- **Phase 1 Bring-up Guide** (`/docs/PHASE1_BRINGUP.md`) — exact build, boot, debug, and image-packaging workflow for Phase 1.
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

#### Changed

- License selected: **GPL-3.0-or-later**. The project now uses strong copyleft to protect user sovereignty, privacy, and anti-censorship goals.
- System servicing design expanded with manifest requirements, slot health-state rules, phased implementation milestones, and concrete desktop/server/recovery update UX flows.
- Kernel architecture decision made: ghOSt now explicitly targets a microkernel, with drivers and filesystem logic expected to live in user-space servers by default.
- Phase 0 QEMU launcher now matches the documented Phase 1 workflow: it can boot either a raw disk image or a staged ESP directory, capture debugcon output, and expose the QEMU GDB stub.
- Phase 1 staged-ESP boot flow now emits `startup.nsh` so the ghOSt sign-of-life app still launches when OVMF drops to the internal shell before selecting the fallback boot path.
- Phase 1 UEFI sign-of-life loader now acquires the UEFI memory map and logs usable RAM regions plus a usable-after-exit summary during QEMU/OVMF bring-up.
- Phase 1 UEFI sign-of-life loader now locates GOP, logs framebuffer resolution and base/size, and writes a small framebuffer test pattern when the pixel layout is directly usable.
- Phase 1 UEFI sign-of-life loader now opens a staged ESP payload, reads `ghOSt\kernel-stage0.bin` into loader-owned memory, and validates its `GHOSTKRN-STAGE0` marker under QEMU/OVMF.
- Phase 1 UEFI bring-up now uses a split loader layout with a small `efi_main` entry file plus a dedicated loader module that prepares a ghOSt-owned boot-info handoff struct from the loaded payload, GOP state, and latest UEFI memory map.
- Phase 1 UEFI bring-up now assembles a repo-owned raw `kernel-stage0.bin` payload with NASM, resolves its entry address from a tiny stage-0 header, exits boot services cleanly, and jumps into the payload under QEMU/OVMF.
- Phase 1 UEFI bring-up now builds `kernel-stage0.bin` through an ELF-linked stage-0 pipeline with an assembly entry shim plus a freestanding C initialization stub, keeping the raw payload contract while making early kernel entry headless-safe.
- Phase 1 UEFI image packaging on Windows now stages the ESP through a fixed temporary VHD created by `diskpart` and converts that VHD into the final raw `.img`, fixing both the broken raw-image attach path and the sparse-VHD attachment limitation.
- Phase 1 packaged-image validation can now pass a `DebugConPath` directly through `New-ghOStUefiImage.ps1 -Boot`, making post-`ExitBootServices()` verification available without a separate manual QEMU launch.
- Phase 1 UEFI bring-up now replaces the temporary `GHOSTKRN-STAGE0` header with the first ghOSt kernel image header, stages the payload as `ghOSt\kernel.bin`, and validates an explicit x86-64 boot-info entry ABI before jumping into the kernel image.
- Phase 1 UEFI bring-up now makes loader-to-kernel ownership explicit in `GhostBootInfo_t`: the kernel takes ownership of the payload buffer, final memory-map snapshot, and early stack after the jump, while framebuffer metadata is marked as borrowed MMIO and the final exit-ready map state is logged before `ExitBootServices()`.
- Phase 1 packaged-image validation is now deterministic in headless QEMU runs: the launcher can reset OVMF vars from template, redirect serial output to a file, and boot the freshly generated raw disk image without depending on interactive shell behavior.
- Toolchain direction decided: ghOSt will standardize on Clang + LLD + NASM (Intel syntax) as the primary Phase 0 development stack.
- UEFI approach decided: ghOSt will use a minimal from-scratch UEFI application and loader path for early bring-up, keeping EDK2 as an optional later reference rather than the default boot framework.
- QEMU/OVMF bring-up now reaches the first ghOSt UEFI sign-of-life application through a repo-local staged ESP path, with debug output captured via QEMU debugcon.

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
