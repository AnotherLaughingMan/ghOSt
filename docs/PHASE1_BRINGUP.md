# ghOSt Phase 1 Bring-up Guide

This document describes the exact build, boot, and debug workflow for the
Phase 1 ghOSt UEFI bring-up milestone. It covers:

- building the first UEFI sign-of-life artifact;
- booting it in QEMU with the staged-ESP fast-path;
- packaging it into a real bootable `.img`;
- attaching a debugger via LLDB and the QEMU GDB stub.

---

## Prerequisites

The Phase 0 Windows toolchain must already be installed and passing the verifier.
Run the verifier if you are unsure:

```powershell
pwsh -File .\tools\phase0\windows\Test-Phase0Toolchain.ps1
```

All scripts below are run from the repository root (`C:\Projects\C\GhostOS`).
All output goes under `out\phase1\uefi-sign-of-life\` (git-ignored).

---

## Step 1: Build the UEFI sign-of-life artifact

```powershell
pwsh -File .\tools\phase1\windows\Build-ghOStUefiSignOfLife.ps1
```

On success the script prints:

```
Building ghOSt UEFI sign-of-life artifact.
Compiler: C:\Dev\Tools\llvm\bin\clang.exe
Assembler: C:\Dev\Tools\nasm\nasm.exe
Stage0 linker: C:\Dev\Tools\llvm\bin\ld.lld.exe
Linker:   C:\Dev\Tools\llvm\bin\lld-link.exe

Build outputs:
EFI artifact:    out\phase1\uefi-sign-of-life\debug\bin\ghost-sign-of-life.efi
ESP staging root: out\phase1\uefi-sign-of-life\debug\esp
Fallback boot path: out\phase1\uefi-sign-of-life\debug\esp\EFI\BOOT\BOOTX64.EFI
Kernel payload binary: out\phase1\uefi-sign-of-life\debug\bin\kernel.bin
```

Source files involved:

| File                             | Role                                                          |
| -------------------------------- | ------------------------------------------------------------- |
| `boot\uefi\include\ghost_uefi.h` | Minimal internal UEFI type and protocol definitions           |
| `boot\uefi\src\sign_of_life.c`   | First UEFI application; emits banner via debugcon and console |

Compiler flags summary (see the script for the full list):

| Flag                            | Purpose                                                        |
| ------------------------------- | -------------------------------------------------------------- |
| `--target=x86_64-pc-win32-coff` | Emit PE/COFF for UEFI                                          |
| `-ffreestanding -fno-builtin`   | No libc, no runtime assumptions                                |
| `-mno-red-zone`                 | Required for UEFI; firmware interrupts use the stack red zone  |
| `-fshort-wchar`                 | `wchar_t` = 2 bytes, matching UEFI `CHAR16`                    |
| `-fno-stack-protector`          | No stack canary; no runtime support for it in firmware context |

Linker flags summary:

| Flag                         | Purpose                                                               |
| ---------------------------- | --------------------------------------------------------------------- |
| `/subsystem:efi_application` | Mark the PE as a UEFI application                                     |
| `/entry:efi_main`            | Set the UEFI entry point                                              |
| `/nodefaultlib`              | No CRT imports                                                        |
| `/dll`                       | Required for UEFI PE ABI; produces a DLL-style PE with a single entry |

---

## Step 2a: Quick boot via staged ESP (no image file needed)

This is the fastest iteration path. QEMU presents the staging directory as a
FAT volume; OVMF finds `EFI\BOOT\BOOTX64.EFI` and boots it automatically.
The build also emits `startup.nsh` into the ESP root so that, if OVMF drops to
the internal shell first, the shell still auto-launches `BOOTX64.EFI` from the
staged tree.

```powershell
pwsh -File .\tools\phase0\windows\Start-ghOStQemu.ps1 `
    -EspDirectoryPath .\out\phase1\uefi-sign-of-life\debug\esp `
    -DebugConPath .\out\phase1\uefi-sign-of-life\debug\qemu-debug.log `
    -NoGraphic
```

Expected terminal output from the QEMU serial console:

```
BdsDxe: loading Boot0001 "UEFI QEMU QEMU USB HARDDRIVE ..."
BdsDxe: starting Boot0001 "UEFI QEMU QEMU USB HARDDRIVE ..."
ghOSt Phase 1 UEFI sign-of-life
toolchain=clang+lld-link target=x86_64-pc-win32-coff
kernel_payload loaded size=996 bytes
kernel_payload entry_point=0x...
kernel_payload header_ok
kernel_payload abi_ok
kernel_payload ownership=kernel
gop mode=0 resolution=1280x800 format=BGRR8 ppsl=1280
gop framebuffer_base=0x80000000 framebuffer_size=0x3E8000
memory_map descriptors=... descriptor_size=48 version=1
usable type=Conventional start=0x... pages=... bytes=0x...
usable_after_exit_total=0x... bytes
boot_info revision=1 size=160 flags=0xF
boot_info kernel_base=0x... kernel_size=0x... kernel_flags=0x1F
boot_info framebuffer_base=0x... framebuffer_size=0x... framebuffer_flags=0x3
boot_info memory_map_base=0x... memory_map_size=0x... map_key=0x... memory_map_flags=0x5
boot_info kernel_stack_base=0x... kernel_stack_size=0x10000 kernel_stack_top=0x... kernel_stack_flags=0xF
exit_boot_services map_ready key=0x... flags=0x7
exit_boot_services ok
kernel_entry jumping
kernel_stage0 entered
kernel_stage0 boot_info_ok
kernel_stage0 framebuffer_ok
kernel_stage0 halted
```

The same banner, kernel-payload summary, GOP summary, and memory-map summary also appear in
`qemu-debug.log` via the QEMU `isa-debugcon` device wired to port `0x402`.
Descriptor counts, framebuffer addresses, and resolution values are
machine-specific; do not expect an exact byte-for-byte match with the sample
above. The current loader also opens `\ghOSt\kernel.bin` from the ESP,
copies it into loader-owned pool memory, validates its first real ghOSt kernel
image header, resolves the payload entry address, validates the loader-to-kernel
entry ABI, and prepares a boot-info handoff
structure that captures the loaded payload address, entry point, framebuffer
details, the latest UEFI memory map buffer plus map key, and a dedicated
loader-owned kernel stack. The handoff now makes ownership explicit: after the
jump, the kernel owns the loaded payload buffer, the final memory-map buffer,
and the early kernel stack, while the framebuffer descriptor is only a borrowed
MMIO description and must not be freed or repurposed as normal RAM. The loader
then writes a tiny checkerboard test
pattern into the top-left corner of the GOP framebuffer when the firmware
exposes a direct 32-bit RGB/BGR layout. After a final memory-map refresh, the
loader marks that map as the final exit-ready snapshot, successfully calls
`ExitBootServices()`, and transfers control into
`kernel.bin`, the first ghOSt-owned kernel image format built from an assembly
entry shim plus a
freestanding C initialization stub. The payload switches to the provided kernel
stack immediately, validates the boot-info contract, remains headless-safe when
no framebuffer is present, paints a framebuffer block when GOP data exists, and
then halts.

---

## Step 2b: Package into a real bootable `.img`

Use this when you need a proper disk image file (e.g., for VMware or future
hardware testing).

> **Requires elevation.** `diskpart` needs administrator access to attach a
> virtual disk. Run the terminal as Administrator.

```powershell
pwsh -File .\tools\phase1\windows\New-ghOStUefiImage.ps1
```

The script:

1. Creates a temporary fixed-size 64 MiB VHD staging disk via `diskpart`.
2. Attaches that VHD as a virtual disk with `diskpart`.
3. Initializes GPT, creates a 60 MiB EFI System Partition (FAT32), and
   assigns it a temporary drive letter `Z:`.
4. Copies the full staged ESP tree, including `EFI\BOOT\BOOTX64.EFI`,
   `ghOSt\kernel.bin`, and `startup.nsh`.
5. Removes the drive letter, detaches the staging VHD, and converts it into
   the final raw `.img`.

Output:

```
out\phase1\uefi-sign-of-life\debug\ghost-uefi.img
```

Boot the resulting image:

```powershell
pwsh -File .\tools\phase0\windows\Start-ghOStQemu.ps1 `
    -DiskImagePath .\out\phase1\uefi-sign-of-life\debug\ghost-uefi.img `
    -DebugConPath .\out\phase1\uefi-sign-of-life\debug\qemu-debug.log `
    -NoGraphic
```

Optionally, add `-Boot` to `New-ghOStUefiImage.ps1` to build and boot in one
command. If you want direct post-`ExitBootServices()` validation from the
packaged image path, pass `-DebugConPath` through that same command:

```powershell
pwsh -File .\tools\phase1\windows\New-ghOStUefiImage.ps1 `
  -Boot `
  -NoGraphic `
  -DebugConPath .\out\phase1\uefi-sign-of-life\debug\logs\qemu-packaged-debugcon.log
```

In this headless packaged-image path, the launcher now resets the OVMF vars
store from the bundled template before booting the freshly generated disk image
and redirects serial output to `out\phase1\uefi-sign-of-life\debug\ghost-uefi.serial.log`.
That avoids stale NVRAM boot entries and removes terminal input from the UEFI
shell path, making packaged-image validation deterministic instead of depending
on `startup.nsh` running inside an interactive `stdio` session.

Treat the `debugcon` log as the authoritative source for post-handoff
validation; a successful packaged-image run will continue with
`exit_boot_services ok`, `kernel_entry jumping`, and the `kernel_stage0 ...`
markers. The serial log should now begin with a direct fixed-disk boot such as
`BdsDxe: loading Boot0001 "UEFI QEMU HARDDISK ..."` followed by the ghOSt
loader banner, rather than dropping to the internal shell first.

Dry-run the full sequence first to preview what will happen:

```powershell
pwsh -File .\tools\phase1\windows\New-ghOStUefiImage.ps1 -DryRun
```

---

## Step 3: Attach LLDB for source-level debugging

Launch QEMU with the GDB stub open and execution halted at reset:

```powershell
pwsh -File .\tools\phase0\windows\Start-ghOStQemu.ps1 `
    -EspDirectoryPath .\out\phase1\uefi-sign-of-life\debug\esp `
    -NoGraphic `
    -Gdb `
    -GdbWait
```

The `-GdbWait` flag adds `-S` so QEMU pauses before executing the first
firmware instruction. QEMU will print nothing and wait.

In a second terminal, attach LLDB:

```powershell
pwsh -File .\tools\phase1\windows\Start-ghOStGdb.ps1
```

LLDB loads the `.efi` symbol file and connects to `localhost:1234`. Useful
LLDB commands once attached:

```
# Set a breakpoint on the ghOSt entry point
breakpoint set --name efi_main

# Resume execution
continue

# Inspect a variable
frame variable system_table

# Print the next few instructions
disassemble --count 10
```

> **Note on symbol addressing:** UEFI loads `.efi` images at a runtime
> base address chosen by firmware, not the link-time base. The first time you
> hit `efi_main`, LLDB may show the correct source line if ASLR happens to
> match, but rebasing the module manually is necessary for fully accurate
> symbol resolution. A helper for this will be added in a later milestone.

---

## Expected sign-of-life output

The `qemu-debug.log` file should begin with:

```
ghOSt Phase 1 UEFI sign-of-life
toolchain=clang+lld-link target=x86_64-pc-win32-coff
kernel_payload loaded size=
kernel_payload entry_point=
kernel_payload header_ok
kernel_payload abi_ok
gop mode=
gop framebuffer_base=
memory_map descriptors=
usable_after_exit_total=
boot_info revision=
boot_info kernel_base=
boot_info framebuffer_base=
boot_info memory_map_base=
boot_info kernel_stack_base=
exit_boot_services ok
kernel_entry jumping
kernel_stage0 entered
kernel_stage0 boot_info_ok
kernel_stage0 pmm_init
kernel_stage0 pmm_ok pages_total=131072 pages_usable=<N> pages_free=<N> bitmap_base=0x<addr>
kernel_stage0 framebuffer_ok
kernel_stage0 halted
```

`pages_total` equals the highest physical RAM address divided by 4096. On the
512 MiB QEMU machine this is 131072. `pages_free` is slightly less than
`pages_usable` because the bitmap itself occupies a few pages of
EfiConventionalMemory at `bitmap_base`.

If the log file exists but is empty, OVMF did not reach `efi_main`. Check:

- The ESP directory has `EFI\BOOT\BOOTX64.EFI`.
- The OVMF vars file at `%LOCALAPPDATA%\ghOSt\qemu\OVMF_VARS_ghOSt.fd` is
  not corrupted. Delete it to have the launcher recreate it from the template.
- QEMU output on the serial console for prior boot-manager errors.

---

## File map

```
boot/
  kernel/
    mm/
      ghost_pmm.c             Early physical memory manager (bitmap allocator)
    stage0/
      kernel_stage0.asm       Assembly entry shim plus ghOSt kernel image header
      kernel_stage0.c         Freestanding C early kernel initialization stub
      kernel_stage0.ld        Linker script for early ghOSt kernel image layout
  uefi/
    include/
      ghost_boot_info.h       ghOSt-owned firmware-to-kernel handoff structs
      ghost_kernel_image.h    ghOSt kernel image header and entry ABI constants
      ghost_log.h             Shared debugcon, console, and formatting helpers
      ghost_pmm.h             Early PMM interface (init, alloc, free, stats)
      ghost_loader.h          Loader entry shared by the tiny UEFI application
      ghost_uefi.h            Minimal UEFI type and protocol definitions
    src/
      ghost_log.c             Shared debugcon, console, and formatting helpers
      ghost_loader.c          Payload, GOP, memory-map, boot-info, and kernel jump
      sign_of_life.c          Tiny UEFI entry point that calls the loader

tools/
  phase0/
    windows/
      toolchain.manifest.json Explicit tool paths
      Test-Phase0Toolchain.ps1
      Start-ghOStQemu.ps1     QEMU launcher (disk image or ESP directory)
      New-ghOStScratchImageAndBoot.ps1
  phase1/
    windows/
      Build-ghOStUefiSignOfLife.ps1  Compile, assemble, link, and stage BOOTX64.EFI plus kernel payload
      New-ghOStUefiImage.ps1         Package staged ESP into a real .img
      Start-ghOStGdb.ps1             Attach LLDB to the QEMU GDB stub

out/                          (git-ignored)
  phase1/
    uefi-sign-of-life/
      debug/
        bin/
          ghost-sign-of-life.efi     Compiled UEFI application
          ghost-sign-of-life.pdb     Symbol/debug info
          kernel.bin                 Assembled early ghOSt kernel image
        esp/
          ghOSt/
            kernel.bin               Staged kernel payload
        obj/
          sign_of_life.obj           Intermediate object
        esp/
          EFI/BOOT/BOOTX64.EFI      Fallback boot path copy
        ghost-uefi.img              Real packaged disk image (after New-ghOStUefiImage.ps1)
        qemu-debug.log              Debugcon output from last QEMU run
```

---

## Next milestones

The sign-of-life proof now includes the first real post-`ExitBootServices()`
kernel transfer, a freestanding C early kernel initialization stub, the first
ghOSt-owned kernel image header, an explicit ownership-aware firmware-to-
kernel handoff contract, page-backed payload/map/stack placement, and fresh raw
packaged-image validation under QEMU/OVMF.
The sign-of-life proof now also includes an early physical memory manager that
consumes the final UEFI memory map, classifies reclaimable regions
(EfiConventionalMemory, EfiBootServicesCode, EfiBootServicesData), builds a
bitmap allocator over the physical address space of the RAM regions seen in the
map, and exposes `ghost_pmm_alloc_page()` / `ghost_pmm_free_page()` to the
early kernel. On a 512 MiB QEMU machine the PMM reports approximately
`pages_total=131072`, `pages_usable≈129000`, `pages_free≈129000` (minus the few
bytes for the bitmap itself) via the `kernel_stage0 pmm_ok` log line.

The next follow-on work moves up-stack into virtual memory management:

1. **GDT setup** — install a 64-bit flat-model Global Descriptor Table before
   any interrupt or protection work can proceed.
2. **IDT + basic exception handlers** — set up the Interrupt Descriptor Table
   with at minimum page-fault, GP-fault, and double-fault handlers so crashes
   diagnose cleanly rather than resetting the machine.
3. **Virtual memory manager** — implement `map_page()` / `unmap_page()` using
   the PMM for page-table page allocations, and remap the kernel to a
   canonical higher-half virtual address.

See `docs/DEVELOPMENT_BIBLE.md` Section 17.3 for the full Phase 2 task table.
