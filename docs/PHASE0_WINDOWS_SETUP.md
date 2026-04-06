# Phase 0 Windows Setup

This guide defines a low-risk Windows setup path for ghOSt Phase 0 work.

The goal is simple: install the minimum tooling needed to build and boot early images without disrupting an existing Windows workstation or your current VMware setup.

---

## Principles

- Prefer portable tools over system-wide changes where practical.
- Do not enable extra Windows virtualization features unless they are actually needed.
- Keep tool versions isolated in a dedicated development tools directory.
- Treat QEMU as a developer tool, not as a replacement for VMware Workstation Pro.

---

## Recommended Windows Layout

Use a dedicated tools root outside the repository:

```text
C:\Dev\Tools\
  llvm\
  nasm\
  qemu\
  ovmf\
```

Keep ghOSt itself separate:

```text
C:\Projects\C\GhostOS
```

This keeps the repository clean and makes it easy to remove or replace tools later.

---

## Safe QEMU Install Strategy

For ghOSt on Windows, the safest initial path is:

1. Install QEMU into its own directory such as `C:\Dev\Tools\qemu`.
2. Do not overwrite VMware components or change VMware configuration.
3. Do not enable Hyper-V, WHPX, or other Windows hypervisor features for the first install.
4. Run QEMU in its default software-emulation mode first.
5. Add QEMU to `PATH` only if you decide the workflow is stable and worth keeping.

This matters because the potentially disruptive part is usually not QEMU itself. The risk comes from turning on Windows hypervisor features such as **Windows Hypervisor Platform (WHPX)** or full **Hyper-V** integration. Those features can change how other virtualization products behave.

If your current priority is preserving your existing system behavior, keep the first QEMU setup fully user-space and accelerator-free.

---

## Where to Get QEMU on Windows

The QEMU project's Windows download page points to Windows binaries provided by Stefan Weil and also documents MSYS2 packages.

For a low-risk setup, prefer one of these two paths:

### Option A: Portable Windows build

- Download the Windows 64-bit QEMU build from the source referenced by the official QEMU download page.
- Extract or install it into `C:\Dev\Tools\qemu`.
- If the package offers optional system-wide `PATH` integration, file associations, or desktop shortcuts, skip those for now.
- Launch QEMU by full path or through a repo-local script.

This is the safest option because removal is trivial: delete the directory and any repo-local wrapper script.

### Option B: MSYS2-managed install

- Install MSYS2.
- Install QEMU from the MSYS2 package set.
- Use this only if you already want MSYS2 as part of your toolchain story.

This is still reasonable, but it is less isolated than the portable path.

---

## OVMF on Windows

ghOSt's canonical no-hardware boot path is **QEMU + OVMF**.

On your current Windows setup, you do **not** need to hunt down a separate OVMF download first. The QEMU package you already installed includes the x86-64 edk2 UEFI firmware under `C:\Dev\Tools\qemu\share\`.

For your machine, use these bundled files:

```text
C:\Dev\Tools\qemu\share\edk2-x86_64-code.fd
C:\Dev\Tools\qemu\share\edk2-i386-vars.fd
```

That `edk2-i386-vars.fd` filename looks odd, but QEMU's own x86-64 firmware descriptor points to it as the NVRAM template for x86-64 UEFI boot.

If you still want a separate firmware location later, copy them into a dedicated directory such as:

```text
C:\Dev\Tools\ovmf\
  OVMF_CODE.fd
  OVMF_VARS.fd
```

Rules:

- Treat the code firmware file as read-only.
- Copy the vars template per test VM so NVRAM state is disposable.
- Do not keep mutable firmware-variable files in the repo.

---

## First-Pass Install Sequence

1. Create `C:\Dev\Tools` if it does not exist.
2. Place LLVM/Clang under `C:\Dev\Tools\llvm`.
3. Place NASM under `C:\Dev\Tools\nasm`.
4. Place QEMU under `C:\Dev\Tools\qemu`.
5. Use the bundled QEMU edk2 firmware files first; only create `C:\Dev\Tools\ovmf` if you later want a separate firmware copy.
6. Verify each tool by full path before adding anything to `PATH`.

Example verification commands in PowerShell:

```powershell
& 'C:\Dev\Tools\qemu\qemu-system-x86_64.exe' --version
& 'C:\Dev\Tools\llvm\bin\clang.exe' --version
& 'C:\Dev\Tools\nasm\nasm.exe' -v
```

If those work by full path, the install is already usable.

---

## LLVM and NASM on Windows

Use the same strategy as QEMU: keep them isolated under `C:\Dev\Tools`, avoid global `PATH` changes at first, and verify them by full path.

### LLVM / Clang

Low-risk options:

- Use an official LLVM Windows binary distribution and place it under `C:\Dev\Tools\llvm`.
- If you already use Visual Studio heavily, LLVM's Windows documentation also supports working from the Visual Studio ecosystem, but ghOSt does not need a full Visual Studio-based LLVM source build just to begin Phase 0.

For ghOSt's current needs, the practical target is simply:

```text
C:\Dev\Tools\llvm\bin\clang.exe
C:\Dev\Tools\llvm\bin\ld.lld.exe
```

Verify with:

```powershell
& 'C:\Dev\Tools\llvm\bin\clang.exe' --version
& 'C:\Dev\Tools\llvm\bin\ld.lld.exe' --version
```

### NASM

NASM provides Windows builds from its official release area. Place the extracted binary under:

```text
C:\Dev\Tools\nasm\nasm.exe
```

Verify with:

```powershell
& 'C:\Dev\Tools\nasm\nasm.exe' -v
```

The key rule is the same across all three tools: make them work by explicit path first, then decide later whether any `PATH` integration is worth it.

---

## Repo-Local Helpers

The repository now includes PowerShell helpers under `tools\phase0\windows\`:

- `Test-Phase0Toolchain.ps1` checks for Clang, LLD, NASM, QEMU, and OVMF using explicit paths from the manifest.
- `Start-ghOStQemu.ps1` launches QEMU with `accel=tcg` and OVMF using explicit paths, so no global `PATH` change is required.
- `New-ghOStScratchImageAndBoot.ps1` creates a raw scratch disk image with `qemu-img` and then launches QEMU through the same safe helper flow.
- `toolchain.manifest.json` is the first executable draft of the Phase 0 Windows tool manifest.

The default manifest now points at the firmware already bundled with `C:\Dev\Tools\qemu\share`.

Example usage:

```powershell
pwsh -File .\tools\phase0\windows\Test-Phase0Toolchain.ps1
pwsh -File .\tools\phase0\windows\Start-ghOStQemu.ps1 -DiskImagePath C:\path\to\ghost.img -DryRun
pwsh -File .\tools\phase0\windows\Start-ghOStQemu.ps1 -DiskImagePath C:\path\to\ghost.img
pwsh -File .\tools\phase0\windows\New-ghOStScratchImageAndBoot.ps1 -DryRun
pwsh -File .\tools\phase0\windows\New-ghOStScratchImageAndBoot.ps1
```

The launcher intentionally forces `accel=tcg` right now. That keeps the first QEMU workflow independent of Hyper-V, WHPX, and other host-level hypervisor changes.

---

## Why Not Add Everything to PATH Immediately

System-wide `PATH` changes are not catastrophic, but they are unnecessary early on.

Avoiding immediate `PATH` edits gives you:

- no accidental tool shadowing;
- no conflict with other compilers or emulators already installed;
- easy rollback;
- clearer reproducibility while the toolchain is still being pinned.

Prefer small wrapper scripts in the repository later, for example under `tools\` or `scripts\`, that call the chosen binaries by explicit path.

---

## VMware Coexistence Guidance

QEMU itself does not replace or overwrite VMware Workstation Pro.

What can affect coexistence is enabling Windows hypervisor features for acceleration. To minimize risk:

- keep VMware as-is;
- install QEMU separately;
- use QEMU with default software emulation first;
- postpone WHPX evaluation until you explicitly want faster QEMU execution.

If you later test WHPX, do it as a deliberate second step and expect to re-check VMware performance and behavior afterward.

---

## Recommended ghOSt Position

For this project's current state, the recommended Windows approach is:

- **Manual validation today:** VMware Workstation Pro 17 is acceptable.
- **Canonical boot/debug target:** QEMU + OVMF.
- **Lowest-risk host setup:** portable QEMU install, no hypervisor feature changes, full-path invocation.
- **Later optimization:** evaluate WHPX only after the first reproducible boot path exists.

This keeps Phase 0 moving without taking unnecessary host-level risks.

---

## Practical Answer

If your real concern is, "How do I get QEMU onto my system without disrupting my current system?", the practical answer is:

1. Install a portable 64-bit Windows QEMU build into `C:\Dev\Tools\qemu`.
2. Keep it out of global `PATH` for now.
3. Do not enable Hyper-V or WHPX yet.
4. Use the edk2 firmware that ships with your QEMU install, or copy it out later if you want a separate firmware folder.
5. Run QEMU by explicit path until the workflow is proven.

That gives you a reversible, low-risk QEMU setup with minimal host impact.
