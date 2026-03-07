# ghOSt Copilot Instructions

## 1) Project Mission

ghOSt is a custom open-source operating system project with two target profiles:

- Name usage: write the project name as **ghOSt**.
- Pronunciation: say it as **"Ghost OS"**.

- **Minimal Rescue Profile** (floppy/USB): tiny bootable environment for quick CLI operations.
- **Full Install Profile** (full PC): complete OS experience for normal desktop/server usage.

Initial focus:

- Build a reliable, modular kernel core that supports both profiles.
- Prioritize bootability and size constraints for the Minimal Rescue Profile (floppy-capable image where feasible, and USB thumb drive boot support).
- For the Minimal Rescue Profile, load only a minimal bootable environment with a CLI-like command interface for quick operations.
- Keep architecture modular so advanced features can be added later without rewriting core systems.

Long-term direction:

- Add a fully customizable graphical UX (browser-like shell experience).
- Avoid hardcoded UX decisions unless absolutely necessary.
- Deliver a Full Install Profile capable of broad application compatibility, including full COM support as a long-term target.
- Prioritize user autonomy, censorship resistance, and privacy-preserving access patterns.

---

## 2) Language & Component Boundaries

Use the following language boundaries unless explicitly changed in project decisions.

### Kernel / Low-level boot-critical code

- Primary languages: **Assembly** and **C**.
- Keep low-level routines minimal, explicit, and performance-aware.
- Prefer static allocation patterns in early boot and kernel-critical paths.
- Treat memory management as a first-class concern in all C/Assembly code.

### UX / Higher-level shell (future)

- Supported technologies: **C++**, **TypeScript**, **JavaScript**, **HTML**, and **CSS** (depending on selected runtime/tooling path).
- UX must be customizable and driven by configuration.

### Configuration

- Configuration format: **JSON**.
- Store system, boot, and UX behavior in JSON where practical.
- Define schemas/validation as soon as config surface expands.

---

## 3) Platform & Boot Requirements

Current targets:

- **UEFI support is required**.
- **Secure Boot is optional** (design should not block future support).
- **TPM integration is optional** (design should not block future support).

Storage/boot media goals:

- Support boot from small media profiles:
  - Floppy-sized image target (where practical for minimal build artifacts).
  - USB thumb drive boot support (important).
  - Minimal runtime target: command-line UX only (no graphical stack on minimal media profiles).

Deployment profile intent:

- **Minimal Rescue Profile**
  - Purpose: quick operations (for example, formatting drives, diagnostics, recovery tasks).
  - UX scope: CLI-like command environment only.
  - Feature scope: essential services only; avoid heavy subsystems.

- **Full Install Profile**
  - Purpose: daily-use operating system on full PC hardware.
  - UX scope: full graphical UX and richer runtime capabilities.
  - Compatibility scope: target broad app support, including COM support as a planned major milestone.

Implementation guidance:

- Keep boot path deterministic and debuggable.
- Separate bootloader concerns from kernel initialization concerns.
- Keep early hardware assumptions explicit and documented.
- Keep minimal-media boot payload focused on shell/command execution and essential system services only.
- Keep profile-specific feature gates explicit so Minimal Rescue remains lean while Full Install can expand.

---

## 4) Kernel Engineering Principles

1. **Small first, then scale**
   - Favor tiny, testable subsystems.
   - Do not introduce heavy abstractions early.

2. **Performance and size awareness**
   - Measure binary size impact for kernel/boot changes.
   - Keep hot paths simple and predictable.

3. **Deterministic behavior**
   - Prefer explicit initialization over implicit defaults.
   - Fail fast with clear diagnostics in debug builds.

4. **Modularity for future growth**
   - Keep interfaces narrow and clean.
   - Avoid coupling kernel internals to UX implementation details.

5. **No unnecessary hardcoding**
   - Especially for UX and user-facing behavior.
   - Use JSON-driven configuration whenever practical.

6. **Memory discipline by default**
   - Avoid unbounded stack usage; keep stack frames small and predictable.
   - Use fixed-size buffers only with explicit bounds checks.
   - Validate pointer assumptions before dereference in C paths.
   - Keep ownership/lifetime rules simple and documented for each subsystem.
   - Zero or sanitize sensitive memory when no longer needed.

### Memory Rules for C and Assembly (Required)

- No dynamic allocation in early boot unless explicitly justified and reviewed.
- Prefer static or region/arena-based allocation for deterministic behavior.
- Every copy/move operation must be length-aware and bounds-checked.
- Avoid implicit integer narrowing in size/offset calculations.
- Treat all external input (firmware tables, disk structures, device data) as untrusted.
- Add lightweight debug checks for overflows/underflows in debug builds where feasible.
- Document memory layout assumptions (alignment, packing, page boundaries) near the code.

---

## 5) Repository Organization (Planned)

As the repo grows, prefer this high-level layout:

- `/boot/` - bootloader, startup assembly, UEFI entry
- `/kernel/` - kernel core in C/Assembly
- `/hal/` - hardware abstraction layer (if/when introduced)
- `/drivers/` - early and optional hardware drivers
- `/config/` - JSON configs and schemas
- `/tools/` - build tooling, image scripts, utility scripts
- `/docs/` - architecture notes, Development Bible, decisions, specs
- `/docs/decisions/` - Architecture Decision Records (ADRs)
- `/ux/` - future customizable UX shell (C++ and/or web stack path: HTML/CSS/JS/TS)

Keep names short, explicit, and stable to reduce churn.

---

## 6) Build & Compatibility Guidance

- Keep the default build path simple and reproducible.
- Support incremental builds where possible.
- Treat warnings as issues in core/boot paths when feasible.
- Preserve compatibility goals for tiny image profiles when adding features.
- When adding dependencies, justify impact on size, complexity, and portability.
- Maintain separate build targets/artifacts for Minimal Rescue and Full Install profiles.
- Do not require Full Install compatibility layers (including COM-related subsystems) in Minimal Rescue builds.

---

## 7) Configuration Rules (JSON)

When introducing JSON configs:

- Define clear ownership for each config file.
- Document defaults and required keys.
- Add versioning strategy for config evolution (e.g., `schemaVersion`).
- Validate configs early during load/boot where applicable.
- Keep config human-readable and minimally verbose.

Do not bury critical behavior in undocumented magic values.

---

## 8) Security Posture (Current + Future)

Current:

- UEFI baseline support.

Future-ready (optional now):

- Secure Boot compatibility path.
- TPM-aware capabilities.

Guidance:

- Keep security-sensitive boundaries explicit.
- Avoid introducing patterns that would block signing, trust chains, or measured boot in later phases.

### Privacy, Censorship Resistance, and Identity Principles

- Design for user control and freedom of expression by minimizing centralized control points.
- Prefer privacy-preserving defaults and local-first operation where practical.
- Minimize mandatory identity collection and avoid unnecessary age/identity verification dependencies in core OS design.
- Keep compliance-sensitive controls modular and policy-driven so deployments can adapt to jurisdictional requirements without hardcoding intrusive behavior into the kernel.
- Treat user data protection as a core objective: secure storage, clear key ownership boundaries, and least-privilege access patterns.

---

## 9) UX Vision (Future)

Target UX direction:

- Browser-like graphical environment.
- Highly customizable behavior and appearance.
- Support web-style UI technologies (HTML/CSS/JS/TS) where practical.
- Configuration-first design (JSON), minimal hardcoded UX logic.

When implementing UX later:

- Separate rendering/input/runtime from policy/config.
- Keep extension/customization surfaces intentional and documented.

---

## 10) Copilot Working Rules for This Repo

When generating or editing code:

- Use the canonical project stylization **ghOSt** in docs and user-facing text.
- Treat the spoken/read-aloud name as **"Ghost OS"**.
- Respect language boundaries (Assembly/C for core, C++/TS for UX).
- For UX, allow C++, TypeScript, JavaScript, HTML, and CSS based on architecture decisions.
- Prefer minimal, focused changes over broad refactors.
- Do not add unnecessary dependencies.
- Do not hardcode user-facing UX behavior unless required.
- For C/Assembly changes, call out memory impact (stack, heap/arena, and buffer boundaries).
- Include/update documentation for architecture-impacting changes.
- Keep comments technical and concise; avoid noise.
- For risky changes, propose a small phased path first.

When requirements are ambiguous:

- Choose the simplest design that preserves future extensibility.
- Document assumptions in `/docs/`.
- Consult the Development Bible (`/docs/DEVELOPMENT_BIBLE.md`) for authoritative design guidance. The Bible takes precedence over ad-hoc decisions.

---

## 11) Architecture Decision Tracking

As ghOSt grows, capture major decisions in lightweight ADR-style notes under `/docs/decisions/`.

Each decision should include:

- Context
- Decision
- Alternatives considered
- Consequences
- Status (proposed/accepted/superseded)

This keeps long-term evolution clear as contributors join.

---

## 12) Living Document Policy

This file is a living baseline. The **Development Bible** (`/docs/DEVELOPMENT_BIBLE.md`) is the canonical, comprehensive design authority for ghOSt. This copilot-instructions file provides working rules for AI assistance; the Bible provides the full project philosophy, architecture, and contribution standards.

Rules for updates:

- Keep sections concise and practical.
- Add new constraints before adding implementation-specific details.
- Prefer explicit requirements over vague preferences.
- Preserve backward context when changing direction (note date/reason).

Suggested cadence:

- Update this document whenever architecture, boot targets, language boundaries, or UX strategy changes.
