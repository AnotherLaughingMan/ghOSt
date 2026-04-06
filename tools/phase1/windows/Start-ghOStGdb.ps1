param(
    [string]$EfiPath = ([System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..\..\out\phase1\uefi-sign-of-life\debug\bin\ghost-sign-of-life.efi'))),

    [string]$ManifestPath = ([System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..\phase0\windows\toolchain.manifest.json'))),

    [string]$Host = 'localhost',

    [int]$Port = 1234
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# ---------------------------------------------------------------------------
# Locate lldb from the LLVM bin directory next to clang
# ---------------------------------------------------------------------------

if (-not (Test-Path -LiteralPath $ManifestPath)) {
    throw "Manifest not found: $ManifestPath"
}

$manifest   = Get-Content -LiteralPath $ManifestPath -Raw | ConvertFrom-Json
$clangEntry = $manifest.tools.clang

if ($null -eq $clangEntry) {
    throw "Tool 'clang' missing from manifest $ManifestPath"
}

if (-not (Test-Path -LiteralPath $clangEntry.path)) {
    throw "Clang path not found: $($clangEntry.path)"
}

$llvmBin  = Split-Path -Parent $clangEntry.path
$lldbPath = Join-Path $llvmBin 'lldb.exe'

if (-not (Test-Path -LiteralPath $lldbPath)) {
    throw "lldb.exe not found at: $lldbPath"
}

if (-not (Test-Path -LiteralPath $EfiPath)) {
    throw "EFI binary not found: $EfiPath`nRun Build-ghOStUefiSignOfLife.ps1 first."
}

# ---------------------------------------------------------------------------
# Build the LLDB startup command sequence
# ---------------------------------------------------------------------------
#
# LLDB command sequence:
#   target create    -- loads the binary so symbols are available
#   gdb-remote       -- attaches to the QEMU GDB stub
#
# We use --one-line-before-file so the target is ready before the REPL opens,
# and --no-use-colors to keep output readable in a plain PowerShell terminal.
#
# UEFI runs in long mode (64-bit) so no explicit architecture override is
# needed when connecting to a QEMU q35 target.
# ---------------------------------------------------------------------------

$lldbArgs = @(
    '--no-use-colors',
    '--one-line-before-file', "target create `"$EfiPath`"",
    '--one-line', "gdb-remote ${Host}:${Port}"
)

Write-Host 'Starting LLDB and attaching to QEMU GDB stub.'
Write-Host "LLDB: $lldbPath"
Write-Host "Symbol file: $EfiPath"
Write-Host "Target: ${Host}:${Port}"
Write-Host ''
Write-Host 'Make sure QEMU was launched with -Gdb (and optionally -GdbWait) first.'
Write-Host 'You can set breakpoints with: breakpoint set --name efi_main'
Write-Host ''

& $lldbPath @lldbArgs
