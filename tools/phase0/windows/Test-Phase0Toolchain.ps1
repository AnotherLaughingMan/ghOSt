param(
    [string]$ManifestPath = (Join-Path $PSScriptRoot 'toolchain.manifest.json')
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Get-ToolVersionLine {
    param(
        [string]$ExecutablePath,
        [string[]]$VersionArgs
    )

    if (-not $VersionArgs -or $VersionArgs.Count -eq 0) {
        return ''
    }

    $output = & $ExecutablePath @VersionArgs 2>&1
    return ($output | Select-Object -First 1)
}

if (-not (Test-Path -LiteralPath $ManifestPath)) {
    throw "Manifest not found: $ManifestPath"
}

$manifest = Get-Content -LiteralPath $ManifestPath -Raw | ConvertFrom-Json
$results = @()
$missingRequiredTool = $false

foreach ($property in $manifest.tools.PSObject.Properties) {
    $tool = $property.Value
    $exists = Test-Path -LiteralPath $tool.path
    $versionLine = ''
    $versionCheck = 'n/a'

    if ($exists -and $tool.PSObject.Properties.Name -contains 'versionArgs') {
        $versionLine = Get-ToolVersionLine -ExecutablePath $tool.path -VersionArgs $tool.versionArgs

        if ($tool.PSObject.Properties.Name -contains 'versionRegex' -and $tool.versionRegex) {
            if ($versionLine -match $tool.versionRegex) {
                $versionCheck = 'ok'
            }
            else {
                $versionCheck = 'unexpected'
            }
        }
    }

    if (-not $exists -and $tool.required) {
        $missingRequiredTool = $true
    }

    $results += [PSCustomObject]@{
        Tool         = $tool.displayName
        Required     = [bool]$tool.required
        Exists       = $exists
        VersionCheck = $versionCheck
        Path         = $tool.path
        VersionLine  = $versionLine
    }
}

$results | Format-Table -AutoSize | Out-String | Write-Host

if ($missingRequiredTool) {
    Write-Error 'One or more required Phase 0 tools are missing. Install them to the paths in the manifest or update the manifest.'
}

Write-Host 'Phase 0 toolchain check completed.'