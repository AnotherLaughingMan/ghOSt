param(
    [string]$RepositoryRoot = ([System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..\..')))
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$versionHeaderPath = Join-Path $RepositoryRoot 'boot\kernel\include\ghost_version.h'
$changelogPath = Join-Path $RepositoryRoot 'CHANGELOG.md'

if (-not (Test-Path -LiteralPath $versionHeaderPath)) {
    throw "Kernel version header not found: $versionHeaderPath"
}

if (-not (Test-Path -LiteralPath $changelogPath)) {
    throw "Changelog not found: $changelogPath"
}

$versionHeader = Get-Content -LiteralPath $versionHeaderPath -Raw
$changelog = Get-Content -LiteralPath $changelogPath -Raw

$majorMatch = [regex]::Match($versionHeader, '#define\s+GHOST_VERSION_MAJOR\s+(\d+)U')
$minorMatch = [regex]::Match($versionHeader, '#define\s+GHOST_VERSION_MINOR\s+(\d+)U')
$patchMatch = [regex]::Match($versionHeader, '#define\s+GHOST_VERSION_PATCH\s+(\d+)U')
$stringMatch = [regex]::Match($versionHeader, '#define\s+GHOST_VERSION_STRING\s+"(\d+\.\d+\.\d+)"')

if (-not $majorMatch.Success -or -not $minorMatch.Success -or -not $patchMatch.Success -or -not $stringMatch.Success) {
    throw "Unable to parse version macros from: $versionHeaderPath"
}

$headerMajor = $majorMatch.Groups[1].Value
$headerMinor = $minorMatch.Groups[1].Value
$headerPatch = $patchMatch.Groups[1].Value
$headerString = $stringMatch.Groups[1].Value
$composedHeaderVersion = "$headerMajor.$headerMinor.$headerPatch"

if ($headerString -ne $composedHeaderVersion) {
    throw "Kernel version macro mismatch in $versionHeaderPath. GHOST_VERSION_STRING='$headerString' but numeric macros compose to '$composedHeaderVersion'."
}

$kernelReleaseMatch = [regex]::Match($changelog, '(?m)^###\s+\[(\d+\.\d+\.\d+)\]\s+—\s+Kernel\b')

if (-not $kernelReleaseMatch.Success) {
    throw "Unable to find a versioned Kernel release heading in $changelogPath (expected e.g. '### [0.1.0] — Kernel')."
}

$changelogKernelVersion = $kernelReleaseMatch.Groups[1].Value

if ($headerString -ne $changelogKernelVersion) {
    throw "Kernel version mismatch. ghost_version.h reports '$headerString' but latest Kernel release entry in CHANGELOG.md is '$changelogKernelVersion'."
}

Write-Host "Version consistency check passed: kernel=$headerString changelog=$changelogKernelVersion"
