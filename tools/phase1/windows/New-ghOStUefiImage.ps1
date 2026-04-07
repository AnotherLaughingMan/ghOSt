param(
    [string]$ManifestPath = ([System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..\phase0\windows\toolchain.manifest.json'))),

    [string]$EspPath = ([System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..\..\out\phase1\uefi-sign-of-life\debug\esp'))),

    [string]$ImagePath = ([System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..\..\out\phase1\uefi-sign-of-life\debug\ghost-uefi.img'))),

    [int]$ImageSizeMiB = 64,

    [string]$DebugConPath,

    [switch]$Boot,

    [switch]$NoGraphic,

    [switch]$DryRun
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

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

function Invoke-DiskPart {
    param([string[]]$Commands)

    $script = $Commands -join [System.Environment]::NewLine
    $tempScript = [System.IO.Path]::GetTempFileName()

    try {
        Set-Content -LiteralPath $tempScript -Value $script -Encoding Ascii
        $result = & diskpart /s $tempScript 2>&1
        return $result
    }
    finally {
        Remove-Item -LiteralPath $tempScript -Force -ErrorAction SilentlyContinue
    }
}

function Test-IsAdministrator {
    $currentIdentity = [Security.Principal.WindowsIdentity]::GetCurrent()
    $currentPrincipal = [Security.Principal.WindowsPrincipal]::new($currentIdentity)

    return $currentPrincipal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

# ---------------------------------------------------------------------------
# Validation
# ---------------------------------------------------------------------------

if (-not (Test-Path -LiteralPath $ManifestPath)) {
    throw "Manifest not found: $ManifestPath"
}

if (-not (Test-Path -LiteralPath $EspPath)) {
    throw "ESP staging directory not found: $EspPath"
}

$ManifestPath = [System.IO.Path]::GetFullPath($ManifestPath)
$EspPath = [System.IO.Path]::GetFullPath($EspPath)
$ImagePath = [System.IO.Path]::GetFullPath($ImagePath)

$bootx64Path = Join-Path $EspPath 'EFI\BOOT\BOOTX64.EFI'
$kernelPayloadPath = Join-Path $EspPath 'ghOSt\kernel.bin'

if (-not (Test-Path -LiteralPath $bootx64Path)) {
    throw "BOOTX64.EFI not found in ESP at: $bootx64Path"
}

if (-not (Test-Path -LiteralPath $kernelPayloadPath)) {
    throw "Kernel payload not found in ESP at: $kernelPayloadPath"
}

$manifest = Get-Content -LiteralPath $ManifestPath -Raw | ConvertFrom-Json
$qemuImgPath = Get-ManifestToolPath -Manifest $manifest -ToolName 'qemuImg'

$launcherPath = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..\phase0\windows\Start-ghOStQemu.ps1'))

if (-not (Test-Path -LiteralPath $launcherPath)) {
    throw "QEMU launcher not found: $launcherPath"
}

Write-Host "ESP source: $EspPath"
Write-Host "Image target: $ImagePath"
Write-Host "Image size: $ImageSizeMiB MiB"
Write-Host ''

# ---------------------------------------------------------------------------
# Step 1: allocate an attachable fixed-size staging VHD via diskpart
# ---------------------------------------------------------------------------

$imageDirectory = Split-Path -Parent $ImagePath
$imageNameWithoutExtension = [System.IO.Path]::GetFileNameWithoutExtension($ImagePath)
$stagingVhdPath = Join-Path $imageDirectory ($imageNameWithoutExtension + '.staging.vhd')
$defaultSerialLogPath = Join-Path $imageDirectory ($imageNameWithoutExtension + '.serial.log')

if ($imageDirectory) {
    New-Item -ItemType Directory -Path $imageDirectory -Force | Out-Null
}

Write-Host "Step 1: Creating fixed staging VHD (${ImageSizeMiB} MiB) ..."

if ($DryRun) {
    Write-Host "  [dry-run] diskpart create vdisk file=`"$stagingVhdPath`" maximum=$ImageSizeMiB type=fixed"
}
else {
    Remove-Item -LiteralPath $stagingVhdPath -Force -ErrorAction SilentlyContinue
    Remove-Item -LiteralPath $ImagePath -Force -ErrorAction SilentlyContinue

    $createVhdCommands = @(
        "create vdisk file=`"$stagingVhdPath`" maximum=$ImageSizeMiB type=fixed",
        'exit'
    )

    $createVhdResult = Invoke-DiskPart -Commands $createVhdCommands

    if (-not (Test-Path -LiteralPath $stagingVhdPath)) {
        $createTranscript = ($createVhdResult | Out-String).Trim()

        if (-not $createTranscript) {
            $createTranscript = '<no diskpart output captured>'
        }

        throw "Failed to create staging VHD at $stagingVhdPath. DiskPart output:`n$createTranscript"
    }
}

# ---------------------------------------------------------------------------
# Step 2: partition as GPT with a single EFI System Partition using diskpart
# ---------------------------------------------------------------------------
#
# diskpart cannot work on relative paths or PowerShell PSDrive roots, so we
# resolve to a proper Win32 path first.
#
# The partition sequence:
#   - convert gpt          → initialize as GPT (no MBR)
#   - create partition efi → UEFI system partition (type EF00)
#   - format fs=fat32 ...  → FAT32 with a short label
#   - assign               → temporarily mount for file copy
#
# After file copy we immediately remove the drive letter so the image stays
# self-contained and does not float into Windows explorer.
# ---------------------------------------------------------------------------

$resolvedImagePath = $stagingVhdPath

Write-Host 'Step 2: Attaching image and writing GPT/ESP via diskpart ...'

if ($DryRun) {
    Write-Host '  [dry-run] Would attach the staging VHD, partition it as GPT + FAT32 EFI System Partition via diskpart,'
    Write-Host '  [dry-run] and copy the full staged ESP tree into the mounted partition.'
    Write-Host '  [dry-run] Would detach the staging VHD and convert it to the final raw .img.'
}
else {
    if (-not (Test-IsAdministrator)) {
        throw 'Administrator privileges are required for the diskpart-based image packaging step. Re-run this script from an elevated terminal.'
    }

    $mountedEspRoot = 'Z:\'
    $isAttached = $false

    try {
        # Attach the staging VHD so diskpart can partition it.
        $attachCommands = @(
            "select vdisk file=`"$resolvedImagePath`"",
            'attach vdisk',
            'convert gpt',
            'create partition efi size=60',
            'format fs=fat32 label=GHOST_ESP quick',
            'assign letter=Z',
            'exit'
        )

        $attachResult = Invoke-DiskPart -Commands $attachCommands
        $isAttached = $true

        # Give Windows a moment to finish mounting the volume.
        Start-Sleep -Milliseconds 1500

        # Copy the ESP directory tree into the mounted partition.
        if (-not (Test-Path -LiteralPath $mountedEspRoot)) {
            $attachTranscript = ($attachResult | Out-String).Trim()

            if (-not $attachTranscript) {
                $attachTranscript = '<no diskpart output captured>'
            }

            throw "Mounted ESP volume at Z:\ not found after diskpart assign. DiskPart output:`n$attachTranscript"
        }

        Get-ChildItem -LiteralPath $EspPath -Force | ForEach-Object {
            Copy-Item -LiteralPath $_.FullName -Destination $mountedEspRoot -Recurse -Force
        }
    }
    finally {
        if ($isAttached) {
            $detachCommands = @(
                'select volume Z',
                'remove letter=Z',
                "select vdisk file=`"$resolvedImagePath`"",
                'detach vdisk',
                'exit'
            )

            $detachResult = Invoke-DiskPart -Commands $detachCommands
            $detachTranscript = ($detachResult | Out-String)

            if ($detachTranscript -match 'error') {
                throw "Failed to detach staging VHD cleanly. DiskPart output:`n$detachTranscript"
            }
        }
    }

    Write-Host 'Step 3: Converting staged image to final raw image ...'
    & $qemuImgPath convert -f raw -O raw $stagingVhdPath $ImagePath | Out-Null

    if ($LASTEXITCODE -ne 0 -or -not (Test-Path -LiteralPath $ImagePath)) {
        throw "qemu-img failed to produce the final raw image at $ImagePath (exit code $LASTEXITCODE)."
    }

    Remove-Item -LiteralPath $stagingVhdPath -Force -ErrorAction SilentlyContinue
}

Write-Host ''
Write-Host 'Image ready.'
Write-Host "Path: $ImagePath"
Write-Host ''

# ---------------------------------------------------------------------------
# Step 4 (optional): launch QEMU to validate the image
# ---------------------------------------------------------------------------

if ($Boot) {
    Write-Host 'Booting image in QEMU ...'

    $launchArgs = @{
        DiskImagePath = $ImagePath
        ManifestPath  = $ManifestPath
    }

    if ($DebugConPath) {
        $launchArgs.DebugConPath = $DebugConPath
    }

    if ($NoGraphic) {
        $launchArgs.SerialLogPath = $defaultSerialLogPath
        $launchArgs.ResetFirmwareState = $true
    }

    if ($NoGraphic) {
        $launchArgs.NoGraphic = $true
    }

    if ($DryRun) {
        $launchArgs.DryRun = $true
    }

    & $launcherPath @launchArgs
}
