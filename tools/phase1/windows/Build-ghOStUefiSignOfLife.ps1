param(
    [string]$ManifestPath = ([System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..\phase0\windows\toolchain.manifest.json'))),

    [string]$OutputRoot = ([System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..\..\out\phase1\uefi-sign-of-life'))),

    [string]$Configuration = 'debug'
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Get-ManifestToolPath {
    param(
        [object]$Manifest,
        [string]$ToolName
    )

    $tool = $Manifest.tools.$ToolName

    if ($null -eq $tool) {
        throw "Tool '$ToolName' is missing from manifest $ManifestPath"
    }

    if (-not (Test-Path -LiteralPath $tool.path)) {
        throw "Required tool path not found for '$ToolName': $($tool.path)"
    }

    return $tool.path
}

if (-not (Test-Path -LiteralPath $ManifestPath)) {
    throw "Manifest not found: $ManifestPath"
}

$repositoryRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..\..'))
$manifest = Get-Content -LiteralPath $ManifestPath -Raw | ConvertFrom-Json

$clangPath = Get-ManifestToolPath -Manifest $manifest -ToolName 'clang'
$nasmPath = Get-ManifestToolPath -Manifest $manifest -ToolName 'nasm'
$llvmBinPath = Split-Path -Parent $clangPath
$ldLldPath = Join-Path $llvmBinPath 'ld.lld.exe'
$lldLinkPath = Join-Path $llvmBinPath 'lld-link.exe'
$llvmObjcopyPath = Join-Path $llvmBinPath 'llvm-objcopy.exe'

if (-not (Test-Path -LiteralPath $lldLinkPath)) {
    throw "Required linker path not found: $lldLinkPath"
}

if (-not (Test-Path -LiteralPath $ldLldPath)) {
    throw "Required stage0 linker path not found: $ldLldPath"
}

if (-not (Test-Path -LiteralPath $llvmObjcopyPath)) {
    throw "Required objcopy path not found: $llvmObjcopyPath"
}

$includeDirectory = Join-Path $repositoryRoot 'boot\uefi\include'
$kernelIncludeDirectory = Join-Path $repositoryRoot 'boot\kernel\include'
$kernelStage0Directory = Join-Path $repositoryRoot 'boot\kernel\stage0'
$kernelStage0AssemblyPath = Join-Path $kernelStage0Directory 'kernel_stage0.asm'
$kernelStage0CPath = Join-Path $kernelStage0Directory 'kernel_stage0.c'
$kernelStage0LinkerScriptPath = Join-Path $kernelStage0Directory 'kernel_stage0.ld'
$ghostPmmDirectory = Join-Path $repositoryRoot 'boot\kernel\mm'
$ghostPmmCPath = Join-Path $ghostPmmDirectory 'ghost_pmm.c'
$kernelVersionHeaderPath = Join-Path $kernelIncludeDirectory 'ghost_version.h'
$versionConsistencyScriptPath = Join-Path $repositoryRoot 'tools\phase1\windows\Test-ghOStVersionConsistency.ps1'
$sourceDirectory = Join-Path $repositoryRoot 'boot\uefi\src'

if (-not (Test-Path -LiteralPath $sourceDirectory)) {
    throw "UEFI source directory not found: $sourceDirectory"
}

if (-not (Test-Path -LiteralPath $kernelStage0AssemblyPath)) {
    throw "Kernel stage0 assembly source not found: $kernelStage0AssemblyPath"
}

if (-not (Test-Path -LiteralPath $kernelStage0CPath)) {
    throw "Kernel stage0 C source not found: $kernelStage0CPath"
}

if (-not (Test-Path -LiteralPath $kernelStage0LinkerScriptPath)) {
    throw "Kernel stage0 linker script not found: $kernelStage0LinkerScriptPath"
}

if (-not (Test-Path -LiteralPath $ghostPmmCPath)) {
    throw "Ghost PMM C source not found: $ghostPmmCPath"
}

if (-not (Test-Path -LiteralPath $kernelVersionHeaderPath)) {
    throw "Kernel version header not found: $kernelVersionHeaderPath"
}

if (-not (Test-Path -LiteralPath $versionConsistencyScriptPath)) {
    throw "Version consistency script not found: $versionConsistencyScriptPath"
}

$sourcePaths = @(Get-ChildItem -LiteralPath $sourceDirectory -Filter '*.c' | Sort-Object Name | Select-Object -ExpandProperty FullName)

if ($sourcePaths.Count -eq 0) {
    throw "No UEFI source files found under: $sourceDirectory"
}

$configurationRoot = Join-Path $OutputRoot $Configuration
$objectDirectory = Join-Path $configurationRoot 'obj'
$binaryDirectory = Join-Path $configurationRoot 'bin'
$espRoot = Join-Path $configurationRoot 'esp'
$efiBootDirectory = Join-Path $espRoot 'EFI\BOOT'
$kernelPayloadEspDirectory = Join-Path $espRoot 'ghOSt'

New-Item -ItemType Directory -Path $objectDirectory -Force | Out-Null
New-Item -ItemType Directory -Path $binaryDirectory -Force | Out-Null
New-Item -ItemType Directory -Path $efiBootDirectory -Force | Out-Null
New-Item -ItemType Directory -Path $kernelPayloadEspDirectory -Force | Out-Null

$efiPath = Join-Path $binaryDirectory 'ghost-sign-of-life.efi'
$pdbPath = Join-Path $binaryDirectory 'ghost-sign-of-life.pdb'
$kernelPayloadElfPath = Join-Path $binaryDirectory 'kernel.elf'
$kernelPayloadBinaryPath = Join-Path $binaryDirectory 'kernel.bin'
$legacyKernelPayloadBinaryPath = Join-Path $binaryDirectory 'kernel-stage0.bin'
$kernelStage0AssemblyObjectPath = Join-Path $objectDirectory 'kernel_stage0_asm.o'
$kernelStage0CObjectPath = Join-Path $objectDirectory 'kernel_stage0_c.o'
$ghostPmmCObjectPath = Join-Path $objectDirectory 'ghost_pmm_c.o'
$espBootPath = Join-Path $efiBootDirectory 'BOOTX64.EFI'
$kernelPayloadEspPath = Join-Path $kernelPayloadEspDirectory 'kernel.bin'
$legacyKernelPayloadEspPath = Join-Path $kernelPayloadEspDirectory 'kernel-stage0.bin'
$startupScriptPath = Join-Path $espRoot 'startup.nsh'

$objectPaths = @()

Write-Host 'Building ghOSt UEFI sign-of-life artifact.'
Write-Host "Compiler: $clangPath"
Write-Host "Assembler: $nasmPath"
Write-Host "Stage0 linker: $ldLldPath"
Write-Host "Linker: $lldLinkPath"
Write-Host "Sources: $($sourcePaths.Count)"

& $versionConsistencyScriptPath -RepositoryRoot $repositoryRoot

Remove-Item -LiteralPath $legacyKernelPayloadBinaryPath -Force -ErrorAction SilentlyContinue
Remove-Item -LiteralPath $legacyKernelPayloadEspPath -Force -ErrorAction SilentlyContinue

$stage0AssemblyArgs = @(
    '-f', 'elf64',
    $kernelStage0AssemblyPath,
    '-o', $kernelStage0AssemblyObjectPath
)

& $nasmPath @stage0AssemblyArgs

$stage0CompileArgs = @(
    '--target=x86_64-unknown-none-elf',
    '-ffreestanding',
    '-fno-builtin',
    '-fno-pic',
    '-fno-stack-protector',
    '-mno-red-zone',
    '-std=c11',
    '-Wall',
    '-Wextra',
    '-Werror',
    '-O2',
    '-g',
    '-I', $includeDirectory,
    '-I', $kernelIncludeDirectory,
    '-c', $kernelStage0CPath,
    '-o', $kernelStage0CObjectPath
)

& $clangPath @stage0CompileArgs

$ghostPmmCompileArgs = @(
    '--target=x86_64-unknown-none-elf',
    '-ffreestanding',
    '-fno-builtin',
    '-fno-pic',
    '-fno-stack-protector',
    '-mno-red-zone',
    '-std=c11',
    '-Wall',
    '-Wextra',
    '-Werror',
    '-O2',
    '-g',
    '-I', $includeDirectory,
    '-I', $kernelIncludeDirectory,
    '-c', $ghostPmmCPath,
    '-o', $ghostPmmCObjectPath
)

& $clangPath @ghostPmmCompileArgs

$stage0LinkArgs = @(
    '-T', $kernelStage0LinkerScriptPath,
    '-nostdlib',
    '-static',
    '--gc-sections',
    '-o', $kernelPayloadElfPath,
    $kernelStage0AssemblyObjectPath,
    $kernelStage0CObjectPath,
    $ghostPmmCObjectPath
)

& $ldLldPath @stage0LinkArgs

$stage0ObjcopyArgs = @(
    '-O', 'binary',
    $kernelPayloadElfPath,
    $kernelPayloadBinaryPath
)

& $llvmObjcopyPath @stage0ObjcopyArgs

foreach ($sourcePath in $sourcePaths) {
    $objectPath = Join-Path $objectDirectory ([System.IO.Path]::GetFileNameWithoutExtension($sourcePath) + '.obj')
    $compileArgs = @(
        '--target=x86_64-pc-win32-coff',
        '-ffreestanding',
        '-fno-builtin',
        '-fno-stack-protector',
        '-fshort-wchar',
        '-mno-red-zone',
        '-std=c11',
        '-Wall',
        '-Wextra',
        '-Werror',
        '-O2',
        '-g',
        '-I', $includeDirectory,
        '-c', $sourcePath,
        '-o', $objectPath
    )

    & $clangPath @compileArgs
    $objectPaths += $objectPath
}

$linkArgs = @(
    '/machine:x64',
    '/subsystem:efi_application',
    '/dll',
    '/entry:efi_main',
    '/nodefaultlib',
    '/incremental:no',
    '/opt:ref',
    '/opt:icf',
    '/debug:full',
    "/out:$efiPath",
    "/pdb:$pdbPath"
) + $objectPaths

& $lldLinkPath @linkArgs

Copy-Item -LiteralPath $efiPath -Destination $espBootPath -Force
Copy-Item -LiteralPath $kernelPayloadBinaryPath -Destination $kernelPayloadEspPath -Force

$startupScriptContent = @(
    'echo -off',
    'map -r',
    'if exist fs0:\EFI\BOOT\BOOTX64.EFI then',
    '  echo ghOSt startup.nsh launching fs0:\EFI\BOOT\BOOTX64.EFI',
    '  fs0:\EFI\BOOT\BOOTX64.EFI',
    '  goto done',
    'endif',
    'if exist fs1:\EFI\BOOT\BOOTX64.EFI then',
    '  echo ghOSt startup.nsh launching fs1:\EFI\BOOT\BOOTX64.EFI',
    '  fs1:\EFI\BOOT\BOOTX64.EFI',
    '  goto done',
    'endif',
    'echo ghOSt startup.nsh could not find EFI\BOOT\BOOTX64.EFI',
    'map -r',
    'goto done',
    ':done',
    'exit'
) -join "`r`n"

Set-Content -LiteralPath $startupScriptPath -Value $startupScriptContent -NoNewline

Write-Host ''
Write-Host 'Build outputs:'
Write-Host "EFI artifact: $efiPath"
Write-Host "ESP staging root: $espRoot"
Write-Host "Fallback boot path: $espBootPath"
Write-Host "Kernel payload binary: $kernelPayloadBinaryPath"
Write-Host "Kernel payload: $kernelPayloadEspPath"
Write-Host "Startup script: $startupScriptPath"