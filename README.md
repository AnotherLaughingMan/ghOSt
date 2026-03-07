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

TBD — open-source license to be selected.
