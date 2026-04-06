param(
    [string]$ManifestPath = (Join-Path $PSScriptRoot 'toolchain.manifest.json'),

    [string]$OutputDirectory = (Join-Path $env:LOCALAPPDATA 'ghOSt\images'),

    [string]$ImageName = 'ghost-scratch.img',

    [string]$ImageSize = '64M',

    [int]$MemoryMiB = 512,

    [int]$CpuCount = 2,

    [switch]$NoGraphic,

    [switch]$DryRun
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Get-ManifestToolPath
{
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

$manifest = Get-Content -LiteralPath $ManifestPath -Raw | ConvertFrom-Json
$qemuImgPath = Get-ManifestToolPath -Manifest $manifest -ToolName 'qemuImg'
$launcherPath = Join-Path $PSScriptRoot 'Start-ghOStQemu.ps1'

if (-not (Test-Path -LiteralPath $launcherPath)) {
    throw "Launcher script not found: $launcherPath"
}

New-Item -ItemType Directory -Path $OutputDirectory -Force | Out-Null

$diskImagePath = Join-Path $OutputDirectory $ImageName

if (-not (Test-Path -LiteralPath $diskImagePath)) {
    Write-Host "Creating raw scratch image: $diskImagePath ($ImageSize)"
    & $qemuImgPath create -f raw $diskImagePath $ImageSize
}
else {
    Write-Host "Scratch image already exists: $diskImagePath"
}

$launchArgs = @{
    DiskImagePath = $diskImagePath
    ManifestPath  = $ManifestPath
    MemoryMiB     = $MemoryMiB
    CpuCount      = $CpuCount
}

if ($NoGraphic) {
    $launchArgs.NoGraphic = $true
}

if ($DryRun) {
    $launchArgs.DryRun = $true
}

& $launcherPath @launchArgs