<div align="center">

# 👻 ghOSt

### _Pronounced "Ghost OS"_

**A custom, open-source operating system built from scratch**
_Designed for user sovereignty, censorship resistance, and privacy by architecture._

---

> [!CAUTION]
>
> ### 🚫 CALIFORNIA RESTRICTION NOTICE
>
> **The ghOSt operating system, its kernel, and all associated source code are strictly NOT authorized for distribution, use, compilation, modification, or deployment within the State of California.** Any use of this software by individuals or entities located in or operating from California is expressly prohibited. By accessing this repository, you acknowledge and agree to this restriction. See the full [Legal Disclaimer](LEGAL_DISCLAIMER.md) for details.

---

</div>

## About

ghOSt is a ground-up operating system with two deployment profiles:

| Profile            | Description                                                                                                           |
| ------------------ | --------------------------------------------------------------------------------------------------------------------- |
| **Minimal Rescue** | A floppy/USB-bootable CLI environment for quick system operations — formatting, diagnostics, and recovery.            |
| **Full Install**   | A complete desktop/server OS with a browser-like graphical UX, broad application compatibility, and full COM support. |

## Core Values

| Principle                        | Details                                                                                                                                                |
| -------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------ |
| **User Autonomy First**          | No telemetry, no mandatory online or cloud accounts, no age-gating, no content censorship at the OS level. Local user and administrator accounts only. |
| **Privacy by Design**            | Local-first operation, user-owned encryption, minimal data collection.                                                                                 |
| **Small, Auditable, Deliberate** | Every component earns its place.                                                                                                                       |

## Technology Stack

| Layer           | Language                   |
| --------------- | -------------------------- |
| Kernel / Boot   | Assembly + C               |
| System Services | C / C++                    |
| GUI Shell       | C++ / HTML / CSS / JS / TS |
| Configuration   | JSON                       |

- **Boot:** UEFI required. Secure Boot and TPM support are optional (design does not block them).
- **Storage:** Protocol-based rather than connector-name based — **M.2 NVMe** and **U.2 NVMe** fall under PCIe + NVMe support, while **M.2 SATA** falls under SATA/AHCI support.

## Documentation

| Resource                                       | Description                                    |
| ---------------------------------------------- | ---------------------------------------------- |
| [Development Bible](docs/DEVELOPMENT_BIBLE.md) | The canonical design reference                 |
| [Changelog](CHANGELOG.md)                      | Version history and release log                |
| [Issue Tracker](docs/ISSUES.md)                | Known issues, planned work, and open questions |
| [Legal Disclaimer](LEGAL_DISCLAIMER.md)        | Warranty, liability, and jurisdictional terms  |

## License

Licensed under **GNU GPL v3.0 or later** (`GPL-3.0-or-later`).

See [LICENSE](LICENSE) for the full license text and [ADR-0001](docs/decisions/ADR-0001-license-selection.md) for the rationale.

## Contributing

By submitting a contribution to ghOSt, you agree that your contribution will be licensed under **GPL-3.0-or-later**.

Read the [Development Bible](docs/DEVELOPMENT_BIBLE.md) before making architecture-impacting changes.

## Author

**AnotherLaughingMan**

---

<div align="center">

_ghOSt — Your machine. Your rules._

</div>
