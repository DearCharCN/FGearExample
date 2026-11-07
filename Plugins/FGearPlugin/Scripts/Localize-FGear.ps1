<#
.SYNOPSIS
Maintains FGear's editor translations without a project localization target.
.EXAMPLE
.\Localize-FGear.ps1 -Action Gather,Export
.EXAMPLE
.\Localize-FGear.ps1 -Action Import,Compile,Report
.EXAMPLE
.\Localize-FGear.ps1 -Project C:\Projects\Host\Host.uproject -EngineRoot E:\UE -Action Compile
#>
[CmdletBinding()]
param(
    [ValidateSet('Gather', 'Export', 'Import', 'Compile', 'Report')]
    [string[]] $Action = @('Compile'),
    [string] $Project,
    [string] $EngineRoot
)

$ErrorActionPreference = 'Stop'
$PluginRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$Descriptor = Join-Path $PluginRoot 'FGearPlugin.uplugin'
if (-not (Test-Path -LiteralPath $Descriptor -PathType Leaf)) {
    throw "FGearPlugin.uplugin was not found beside the Scripts directory."
}

# Locate the owning project when the plugin is installed in a project's Plugins tree.
# Engine-installed plugins can specify a host project explicitly.
if (-not $Project) {
    $Candidate = [System.IO.DirectoryInfo]::new($PluginRoot)
    while ($null -ne $Candidate) {
        $FoundProjects = @(Get-ChildItem -LiteralPath $Candidate.FullName -Filter '*.uproject' -File)
        if ($FoundProjects.Count -eq 1) {
            $Project = $FoundProjects[0].FullName
            break
        }
        if ($FoundProjects.Count -gt 1) {
            throw "Multiple projects found in $($Candidate.FullName). Specify -Project."
        }
        $Candidate = $Candidate.Parent
    }
}
if (-not $Project) { throw 'Specify -Project with a host .uproject that enables FGearPlugin.' }
$Project = (Resolve-Path -LiteralPath $Project).Path
if ([System.IO.Path]::GetExtension($Project) -ne '.uproject') {
    throw '-Project must point to a .uproject file.'
}

if (-not $EngineRoot) {
    $ProjectDescriptor = Get-Content -LiteralPath $Project -Raw | ConvertFrom-Json
    $Association = $ProjectDescriptor.EngineAssociation
    $BuildsKey = 'HKCU:\Software\Epic Games\Unreal Engine\Builds'
    if ($Association -and (Test-Path -LiteralPath $BuildsKey)) {
        $Builds = Get-ItemProperty -LiteralPath $BuildsKey
        $BuildProperty = $Builds.PSObject.Properties[$Association]
        if ($null -ne $BuildProperty) { $EngineRoot = [string]$BuildProperty.Value }
    }
    if (-not $EngineRoot -and $Association) {
        $LauncherKey = "HKLM:\SOFTWARE\EpicGames\Unreal Engine\$Association"
        if (Test-Path -LiteralPath $LauncherKey) {
            $EngineRoot = (Get-ItemProperty -LiteralPath $LauncherKey).InstalledDirectory
        }
    }
}
if (-not $EngineRoot) { throw 'Could not resolve the engine association. Specify -EngineRoot.' }
$EngineRoot = (Resolve-Path -LiteralPath $EngineRoot).Path
$EditorCommandlet = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
if (-not (Test-Path -LiteralPath $EditorCommandlet -PathType Leaf)) {
    throw "UnrealEditor-Cmd.exe was not found under $EngineRoot."
}

$ResourceRoot = Join-Path $PluginRoot 'Content\Localization\FGearEditor'
$TemplateRoot = Join-Path $PluginRoot 'Config\Localization'
$RunId = (Get-Date -Format 'yyyyMMdd_HHmmss') + '_' + [guid]::NewGuid().ToString('N').Substring(0, 8)
$GeneratedRoot = Join-Path $PluginRoot "Intermediate\Localization\FGearEditor\$RunId"
$RunRoot = Join-Path $PluginRoot "Saved\Localization\FGearEditor\$RunId"
$ReportRoot = Join-Path $PluginRoot 'Saved\Localization\FGearEditor\Reports'
foreach ($Directory in @($GeneratedRoot, $RunRoot, $ReportRoot)) {
    New-Item -ItemType Directory -Path $Directory -Force | Out-Null
}

# Gather/import can update translation source files. Preserve a local rollback copy.
if (($Action -contains 'Gather' -or $Action -contains 'Import') -and
    (Test-Path -LiteralPath $ResourceRoot)) {
    Copy-Item -LiteralPath $ResourceRoot -Destination (Join-Path $RunRoot 'Backup') -Recurse
}

$Utf8Bom = [System.Text.UTF8Encoding]::new($true)
$ConfigFiles = @()
foreach ($Step in $Action) {
    $TemplatePath = Join-Path $TemplateRoot "FGearEditor_$Step.ini.in"
    if (-not (Test-Path -LiteralPath $TemplatePath -PathType Leaf)) {
        throw "Missing localization template: $TemplatePath"
    }
    $ConfigText = [System.IO.File]::ReadAllText($TemplatePath)
    $ConfigText = $ConfigText.Replace('@PLUGIN_DIR@', $PluginRoot.Replace('\', '/'))
    $ConfigText = $ConfigText.Replace('@REPORT_DIR@', $ReportRoot.Replace('\', '/'))
    $ConfigPath = Join-Path $GeneratedRoot "FGearEditor_$Step.ini"
    [System.IO.File]::WriteAllText($ConfigPath, $ConfigText, $Utf8Bom)
    $ConfigFiles += $ConfigPath.Replace('\', '/')
}

$Arguments = @(
    $Project,
    '-run=GatherText',
    ('-config=' + ($ConfigFiles -join ';')),
    '-unattended', '-nop4', '-NullRHI', '-nosound', '-nosplash',
    ('-abslog=' + (Join-Path $RunRoot 'Localize.log'))
)
Write-Host "FGear localization: $($Action -join ', ')"
Write-Host "Plugin: $PluginRoot"
Write-Host "Project: $Project"
& $EditorCommandlet @Arguments 2>&1 | Tee-Object -FilePath (Join-Path $RunRoot 'Console.log')
$CommandletExitCode = $LASTEXITCODE
if ($CommandletExitCode -ne 0) {
    throw "Localization failed with exit code $CommandletExitCode. See $RunRoot."
}
Write-Host "Localization completed. Resources: $ResourceRoot"
Write-Host "Logs: $RunRoot"
