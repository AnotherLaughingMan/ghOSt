param(
    [Parameter(Mandatory = $true, ParameterSetName = 'disk')]
    [string]$DiskImagePath,

    [Parameter(Mandatory = $true, ParameterSetName = 'esp')]
    [string]$EspDirectoryPath,

    [string]$ManifestPath = (Join-Path $PSScriptRoot 'toolchain.manifest.json'),

    [string]$StateDirectory = (Join-Path $env:LOCALAPPDATA 'ghOSt\qemu'),

    [string]$DebugConPath,

    [string]$SerialLogPath,

    [int]$MemoryMiB = 512,

    [int]$CpuCount = 2,

    [switch]$NoGraphic,

    [switch]$Gdb,

    [switch]$GdbWait,

    [switch]$ResetFirmwareState,

    [switch]$DryRun
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

if ($PSCmdlet.ParameterSetName -eq 'disk' -and -not $DryRun -and -not (Test-Path -LiteralPath $DiskImagePath)) {
    throw "Disk image not found: $DiskImagePath"
}

if ($PSCmdlet.ParameterSetName -eq 'esp' -and -not (Test-Path -LiteralPath $EspDirectoryPath)) {
    throw "ESP staging directory not found: $EspDirectoryPath"
}

$manifest = Get-Content -LiteralPath $ManifestPath -Raw | ConvertFrom-Json

$qemuPath = Get-ManifestToolPath -Manifest $manifest -ToolName 'qemuSystemX64'
$ovmfCodePath = Get-ManifestToolPath -Manifest $manifest -ToolName 'ovmfCode'
$ovmfVarsTemplatePath = Get-ManifestToolPath -Manifest $manifest -ToolName 'ovmfVarsTemplate'

New-Item -ItemType Directory -Path $StateDirectory -Force | Out-Null

$ovmfVarsPath = Join-Path $StateDirectory 'OVMF_VARS_ghOSt.fd'

if ($ResetFirmwareState) {
    Remove-Item -LiteralPath $ovmfVarsPath -Force -ErrorAction SilentlyContinue
}

if (-not (Test-Path -LiteralPath $ovmfVarsPath)) {
    Copy-Item -LiteralPath $ovmfVarsTemplatePath -Destination $ovmfVarsPath
}

$serialArgument = 'stdio'

if ($SerialLogPath) {
    $serialDirectory = Split-Path -Parent $SerialLogPath

    if ($serialDirectory) {
        New-Item -ItemType Directory -Path $serialDirectory -Force | Out-Null
    }

    $resolvedSerialLogPath = [System.IO.Path]::GetFullPath($SerialLogPath)
    Remove-Item -LiteralPath $resolvedSerialLogPath -Force -ErrorAction SilentlyContinue
    $serialArgument = ('file:' + ($resolvedSerialLogPath -replace '\\', '/'))
}

$arguments = @(
    '-machine', 'q35,accel=tcg',
    '-m', $MemoryMiB,
    '-smp', $CpuCount,
    '-serial', $serialArgument,
    '-monitor', 'none',
    '-net', 'none',
    '-boot', 'menu=on',
    '-drive', "if=pflash,format=raw,unit=0,readonly=on,file=$ovmfCodePath",
    '-drive', "if=pflash,format=raw,unit=1,file=$ovmfVarsPath"
)

if ($PSCmdlet.ParameterSetName -eq 'disk') {
    $arguments += @(
        '-device', 'ahci,id=ahci',
        '-drive', "if=none,id=bootdisk,format=raw,file=$DiskImagePath",
        '-device', 'ide-hd,drive=bootdisk,bus=ahci.0'
    )
}
else {
    $resolvedEspDirectoryPath = (Resolve-Path -LiteralPath $EspDirectoryPath).Path
    $qemuFatPath = ($resolvedEspDirectoryPath -replace '\\', '/')
    $expectedBootPath = Join-Path $resolvedEspDirectoryPath 'EFI\BOOT\BOOTX64.EFI'

    if (-not (Test-Path -LiteralPath $expectedBootPath)) {
        throw "Expected fallback UEFI boot path not found: $expectedBootPath"
    }

    $arguments += @('-drive', "format=raw,file=fat:rw:$qemuFatPath")
}

if ($DebugConPath) {
    $debugConDirectory = Split-Path -Parent $DebugConPath

    if ($debugConDirectory) {
        New-Item -ItemType Directory -Path $debugConDirectory -Force | Out-Null
    }

    $arguments += @(
        '-debugcon', "file:$DebugConPath",
        '-global', 'isa-debugcon.iobase=0x402'
    )
}

if ($Gdb -or $GdbWait) {
    $arguments += @('-gdb', 'tcp::1234')
}

if ($GdbWait) {
    $arguments += @('-S')
}

if ($NoGraphic) {
    $arguments += @('-display', 'none')
}

Write-Host 'Launching ghOSt QEMU session with software emulation only.'
Write-Host "QEMU: $qemuPath"
Write-Host "OVMF code: $ovmfCodePath"
Write-Host "OVMF vars: $ovmfVarsPath"

if ($PSCmdlet.ParameterSetName -eq 'disk') {
    Write-Host "Disk: $DiskImagePath"
}
else {
    Write-Host "ESP directory: $resolvedEspDirectoryPath"
}

if ($DebugConPath) {
    Write-Host "DebugCon: $DebugConPath"
}

if ($SerialLogPath) {
    Write-Host "Serial: $resolvedSerialLogPath"
}

if ($Gdb -or $GdbWait) {
    Write-Host 'GDB stub: tcp::1234'
}

if ($ResetFirmwareState) {
    Write-Host 'Firmware state: reset from template'
}

if ($DryRun) {
    Write-Host ''
    Write-Host 'Dry run command:'
    Write-Host "$qemuPath $($arguments -join ' ')"
    return
}

& $qemuPath @arguments