# Builds and runs the headless C++ tests for the pure modules (ADR 0005, T-0006). No UE needed.
# Usage: powershell -File tools\test_headless.ps1 [-Filter Tire] [-SelfTestFail]
# Compiles Tests/Headless/**/*.cpp plus the pure sources of CodexCore and CodexVehicleDynamics
# (every Private/*.cpp except *Module.cpp) with cl.exe, then runs the binary from the repo root.
param(
    [string]$Filter = '',
    [switch]$SelfTestFail
)
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$sw = [Diagnostics.Stopwatch]::StartNew()

$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
$vsPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vsPath) { throw 'Visual Studio with the C++ x64 toolset was not found.' }
$vcvars = Join-Path $vsPath 'VC\Auxiliary\Build\vcvars64.bat'

$pureModules = @('CodexCore', 'CodexVehicleDynamics')
$sources = @()
$includes = @("/I`"$root\Tests\Headless`"")
foreach ($m in $pureModules) {
    $includes += "/I`"$root\Source\$m\Public`""
    $sources += Get-ChildItem "$root\Source\$m\Private" -Filter *.cpp -File | Where-Object { $_.Name -notlike '*Module.cpp' } | ForEach-Object { $_.FullName }
}
$sources += Get-ChildItem "$root\Tests\Headless" -Filter *.cpp -File -Recurse | ForEach-Object { $_.FullName }

$outDir = Join-Path $root 'build\headless'
$objDir = Join-Path $outDir 'obj'
New-Item -ItemType Directory -Force $objDir | Out-Null
$exe = Join-Path $outDir 'CodexHeadlessTests.exe'

$flags = @('/nologo', '/std:c++20', '/fp:precise', '/W4', '/WX', '/permissive-', '/GR-', '/EHsc', '/O2', '/MP', '/utf-8',
    '/DCODEXCORE_API=', '/DCODEXVEHICLEDYNAMICS_API=', "/Fo`"$objDir\\`"", "/Fe`"$exe`"")
$srcArgs = $sources | ForEach-Object { "`"$_`"" }
$clLine = "cl " + (($flags + $includes + $srcArgs) -join ' ')

$rsp = Join-Path $outDir 'build.cmd'
Set-Content -Path $rsp -Encoding ascii -Value @("@echo off", "call `"$vcvars`" >nul 2>&1", $clLine)
$ErrorActionPreference = 'Continue'
$buildOut = & cmd.exe /c "`"$rsp`"" 2>&1
$ErrorActionPreference = 'Stop'
if ($LASTEXITCODE -ne 0) {
    $buildOut | ForEach-Object { "$_" } | Where-Object { $_ -match 'error|warning' }
    Write-Output "BUILD FAILED (exit $LASTEXITCODE) after $([math]::Round($sw.Elapsed.TotalSeconds, 1)) s"
    exit 2
}
$buildSeconds = $sw.Elapsed.TotalSeconds

$runArgs = @()
if ($SelfTestFail) { $runArgs += '--selftest-fail' }
if ($Filter) { $runArgs += $Filter }
Push-Location $root
try { & $exe @runArgs; $code = $LASTEXITCODE } finally { Pop-Location }

Write-Output ("HEADLESS: build {0:N1} s, total {1:N1} s, exit {2}" -f $buildSeconds, $sw.Elapsed.TotalSeconds, $code)
exit $code
