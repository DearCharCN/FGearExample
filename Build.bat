@echo off
setlocal EnableExtensions DisableDelayedExpansion
rem Build the editor for this project: Development / Win64.
rem Usage: Build.bat [UnrealEngineRoot] [--no-pause]
rem UE_ROOT may also specify an engine root. Otherwise use EngineAssociation.

set "FG_BUILD_PROJECT=%~dp0FGear.uproject"
set "FG_BUILD_ENGINE_ROOT=%UE_ROOT%"
set "FG_BUILD_NO_PAUSE=0"
set "FG_BUILD_EXIT=1"

:ParseArgs
if "%~1"=="" goto ResolveEngine
if /i "%~1"=="--no-pause" (
    set "FG_BUILD_NO_PAUSE=1"
) else (
    set "FG_BUILD_ENGINE_ROOT=%~1"
)
shift
goto ParseArgs

:ResolveEngine
if not exist "%FG_BUILD_PROJECT%" (
    echo ERROR: FGear.uproject is missing beside this script.
    goto Finish
)
if defined FG_BUILD_ENGINE_ROOT goto CheckEngine
for /f "usebackq delims=" %%E in (`powershell.exe -NoProfile -Command "$ErrorActionPreference='Stop'; $p=ConvertFrom-Json -InputObject ([IO.File]::ReadAllText($env:FG_BUILD_PROJECT)); $id=$p.EngineAssociation; $k=Get-Item -LiteralPath 'Registry::HKEY_CURRENT_USER\SOFTWARE\Epic Games\Unreal Engine\Builds' -ErrorAction SilentlyContinue; if ($k) { $r=$k.GetValue($id) }; if (-not $r) { $k=Get-Item -LiteralPath ('Registry::HKEY_LOCAL_MACHINE\SOFTWARE\EpicGames\Unreal Engine\'+$id) -ErrorAction SilentlyContinue; if ($k) { $r=$k.GetValue('InstalledDirectory') } }; if ($r) { [Console]::WriteLine($r) }"`) do set "FG_BUILD_ENGINE_ROOT=%%E"

:CheckEngine
if not defined FG_BUILD_ENGINE_ROOT (
    echo ERROR: Cannot locate the engine registered for this project.
    echo Specify an engine root: Build.bat "X:\UnrealEngine"
    goto Finish
)
if not exist "%FG_BUILD_ENGINE_ROOT%\Engine\Build\BatchFiles\Build.bat" (
    echo ERROR: Invalid engine root: "%FG_BUILD_ENGINE_ROOT%"
    goto Finish
)
echo Project: "%FG_BUILD_PROJECT%"
echo Engine:  "%FG_BUILD_ENGINE_ROOT%"
echo Target:  FGearEditor / Win64 / Development
echo Close the Unreal Editor before building.
echo.
call "%FG_BUILD_ENGINE_ROOT%\Engine\Build\BatchFiles\Build.bat" FGearEditor Win64 Development "-Project=%FG_BUILD_PROJECT%" -WaitMutex -NoHotReloadFromIDE
set "FG_BUILD_EXIT=%ERRORLEVEL%"

:Finish
echo.
if "%FG_BUILD_EXIT%"=="0" (
    echo Build succeeded.
) else (
    echo Build failed. Exit code: %FG_BUILD_EXIT%
)
if "%FG_BUILD_NO_PAUSE%"=="0" pause
exit /b %FG_BUILD_EXIT%
