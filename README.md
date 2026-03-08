# ghOSt

> Pronounced **"Ghost OS"**

A custom, open-source operating system built from scratch — designed for **user sovereignty**, **censorship resistance**, and **privacy by architecture**.

## What Is ghOSt?

ghOSt is a ground-up OS with two deployment profiles:

- **Minimal Rescue** — a floppy/USB-bootable CLI environment for quick system operations (formatting, diagnostics, recovery).
- **Full Install** — a complete desktop/server OS with a browser-like graphical UX, broad application compatibility, and full COM support.

## Core Values

- **User autonomy first.** No telemetry, no mandatory accounts, no age-gating, no content censorship at the OS level.
- **Privacy by design.** Local-first operation, user-owned encryption, minimal data collection.
- **Small, auditable, deliberate.** Every component earns its place.

## Technology

| Layer           | Language                   |
| --------------- | -------------------------- |
| Kernel / Boot   | Assembly + C               |
| System services | C / C++                    |
| GUI shell       | C++ / HTML / CSS / JS / TS |
| Configuration   | JSON                       |

UEFI boot required. Secure Boot and TPM support are optional (design does not block them).

## Documentation

- [Development Bible](docs/DEVELOPMENT_BIBLE.md) — the canonical design reference
- [Changelog](CHANGELOG.md) — version history and release log
- [Issue Tracker](docs/ISSUES.md) — known issues, planned work, and open questions

## Owner

**AnotherLaughingMan**

## License

Licensed under **GNU GPL v3.0 or later** (`GPL-3.0-or-later`).

See [LICENSE](LICENSE) for the full license text and [ADR-0001](docs/decisions/ADR-0001-license-selection.md) for the rationale.

## Contributing

By submitting a contribution to ghOSt, you agree that your contribution will be licensed under **GPL-3.0-or-later**.

Read the [Development Bible](docs/DEVELOPMENT_BIBLE.md) before making architecture-impacting changes.
