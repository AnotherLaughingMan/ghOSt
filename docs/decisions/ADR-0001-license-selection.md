# ADR-0001: License Selection — GPL v3

**Status:** accepted
**Date:** 2026-03-07
**Author(s):** AnotherLaughingMan

## Context

ghOSt needs a license before accepting external contributions or distributing artifacts. The project's core values are user sovereignty, censorship resistance, privacy by design, and open-source transparency. The license must reinforce — not undermine — these values.

Candidates evaluated: GPL v3, MIT, Apache 2.0, BSD (2-clause), and MPL 2.0.

## Decision

ghOSt is licensed under the **GNU General Public License v3.0 (GPL-3.0-or-later)**.

## Evaluation

### GPL v3

- **Strong copyleft:** all derivative works must be distributed under the same license with full source. No proprietary forks.
- **Anti-tivoization (Section 6):** hardware distributors must provide the means to install modified versions. Prevents locked-down devices running ghOSt that users cannot modify.
- **Patent grant:** contributors grant users a patent license, protecting against patent aggression.
- **Proven at OS scale:** the Linux kernel uses GPL v2; v3 adds the anti-tivoization protections ghOSt's mission demands.
- **Verdict: best fit for ghOSt's values.**

### MIT

- Permissive. No copyleft. No patent grant.
- Allows proprietary forks that strip privacy features, add telemetry, or impose censorship.
- **Verdict: directly contradicts the project mission.**

### Apache 2.0

- Permissive with patent grant.
- Allows proprietary forks — same problem as MIT.
- **Verdict: patent grant is good, but permissiveness undermines the mission.**

### BSD (2-clause)

- Effectively identical to MIT in practice. No copyleft. No patent grant.
- **Verdict: same rejection as MIT.**

### MPL 2.0

- Weak copyleft at file level. Modified files must stay open, but proprietary modules can be added alongside.
- A company could wrap ghOSt's kernel with proprietary telemetry modules legally.
- **Verdict: insufficient protection. File-level copyleft is too narrow.**

## Rationale

The fundamental question is: **can someone take ghOSt, add surveillance or censorship, and distribute it as a closed product?**

- Under MIT/BSD/Apache: **yes.**
- Under MPL 2.0: **partially — proprietary additions are legal.**
- Under GPL v3: **no.** All derivatives must remain open, with source available, and hardware must allow user modification.

ghOSt exists specifically because modern OS vendors abuse user trust. A permissive license would allow the exact behavior ghOSt was created to prevent. GPL v3's copyleft is the enforcement mechanism for ghOSt's values.

The anti-tivoization clause is particularly important for an OS project. Without it, a hardware vendor could ship ghOSt on locked-down devices where users cannot install modified versions — defeating the entire purpose.

## Trade-offs

- GPL v3 may deter some corporate contributors who want proprietary integration. **For ghOSt, this is a feature, not a bug.**
- Code cannot be relicensed to permissive later without consent from all contributors.
- Some libraries (e.g., LLVM/Clang components) use Apache 2.0, which is GPL v3-compatible. MIT and BSD are also compatible. MPL 2.0 has a compatibility provision for GPL.

## Consequences

- All source code in the ghOSt repository is licensed under GPL-3.0-or-later.
- A `LICENSE` file containing the full GPL v3 text is placed in the repository root.
- The README is updated to reflect the license.
- External contributions implicitly grant the same license (standard GPL contribution model).
- Third-party dependencies must be GPL v3-compatible (GPL, LGPL, MIT, BSD, Apache 2.0, MPL 2.0 are all compatible).
- Documentation may optionally use a separate license (e.g., CC BY-SA 4.0) in the future, but defaults to GPL v3 for now.
