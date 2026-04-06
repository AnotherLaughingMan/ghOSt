# ADR-0002: Kernel Architecture — Microkernel

**Status:** accepted
**Date:** 2026-03-08
**Author(s):** AnotherLaughingMan

## Context

ghOSt needed a formal kernel architecture decision before Phase 2 kernel work begins. The main options were monolithic kernel, microkernel, and hybrid kernel.

This decision matters because it determines:

- what code is trusted inside kernel space;
- where drivers, filesystems, and system services live;
- how failure isolation works;
- how IPC must be designed; and
- how the project balances simplicity, resilience, modularity, and long-term maintainability.

ghOSt's stated values already push in a particular direction:

- small, understandable subsystems;
- explicit trust boundaries;
- resilience and recoverability;
- strong security boundaries; and
- modular growth without rewriting the core.

## Decision

ghOSt will use a **microkernel architecture**.

The kernel will be kept intentionally small and will own only the primitives that truly require privileged kernel execution:

- low-level CPU and interrupt handling;
- address-space and virtual-memory management;
- thread scheduling and context switching;
- IPC primitives and handle/capability transfer;
- minimal kernel object and security boundary enforcement; and
- the smallest possible hardware-management surface needed to bootstrap user-space servers.

Drivers, filesystems, device stacks, and higher-level operating-system services will run outside the kernel as isolated user-space servers or trusted system services.

## Alternatives Considered

### Monolithic kernel

- **Pros:** simpler early bring-up, fewer context switches, easier direct-call integration between subsystems.
- **Cons:** pushes large amounts of code into kernel space, weakens fault isolation, and makes every driver bug a kernel bug.
- **Verdict:** rejected. It conflicts with ghOSt's emphasis on narrow trust boundaries and long-term resilience.

### Hybrid kernel

- **Pros:** can keep some performance-sensitive subsystems close to the kernel while moving others out.
- **Cons:** often becomes a monolithic kernel with better marketing unless the boundary is enforced rigorously.
- **Verdict:** rejected as the initial architecture. It leaves too much room for boundary erosion during early bring-up.

## Rationale

ghOSt is not trying to be the fastest route to a barely working kernel. It is trying to be a durable OS architecture aligned with user sovereignty, debuggability, and security.

Microkernel architecture best matches those goals:

- **Fault isolation:** a driver or filesystem server crash should not automatically crash the whole kernel.
- **Security boundaries:** smaller kernel code means less privileged attack surface.
- **Auditability:** fewer subsystems in kernel space makes review more realistic for a small project.
- **Modularity:** user-space servers can evolve independently with clearer interfaces.
- **Recovery:** restarting a failed service is easier than recovering from a kernel panic.

The cost is complexity in IPC, server orchestration, and early bring-up. ghOSt accepts that cost explicitly.

## Consequences

### Architectural consequences

- The Development Bible must describe the kernel as a microkernel, not a monolithic core.
- Driver architecture changes from kernel-mode modules to predominantly user-space driver servers.
- Filesystem logic belongs in user-space filesystem servers, with the kernel exposing the primitives needed for IPC, memory mapping, and device access.
- The VFS becomes a service boundary design problem, not just an in-kernel abstraction.

### Implementation consequences

- IPC becomes a Phase 2–3 critical path, not a later convenience feature.
- The device manager becomes a user-space or split-control service layered on top of small kernel primitives.
- Bootstrapping must include a minimal path from kernel bring-up to first trusted user-space server.
- Some tightly constrained early-boot or interrupt-path helpers may remain inside the kernel, but only by explicit exception and documentation.

### Non-goals

- ghOSt will not pretend a microkernel is free of performance trade-offs.
- ghOSt will not quietly drift into a hybrid/monolithic design without a superseding ADR.
- "Performance-sensitive" is not a blanket excuse to move large subsystems into kernel space.

## Follow-up Work

- Update the Development Bible architecture diagram and driver model.
- Resolve ISS-0002.
- Create follow-up design work for:
  - IPC semantics and capability/handle passing;
  - user-space driver server lifecycle and restart policy;
  - filesystem server model and VFS boundary;
  - bootstrapping the first trusted user-space services.
