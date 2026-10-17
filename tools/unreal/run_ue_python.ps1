# Runs a Python script inside Unreal Editor (commandlet mode, no window) and writes a clean result file.
# Usage: powershell -File tools\unreal\run_ue_python.ps1 -Script tools\unreal\hello_editor.py
# The script marks the lines it wants returned with the prefix "CODEX:" (via unreal.log or print).
param(
    [Parameter(Mandatory = $true)][string]$Script,
    [switch]$WithRendering
)
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$editor = 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
$uproject = Join-Path $root 'Codex.uproject'
$outDir = Join-Path $root 'build\bridge'
New-Item -ItemType Directory -Force $outDir | Out-Null
$scriptPath = (Resolve-Path (Join-Path $root $Script)).Path
$logPath = Join-Path $root 'Saved\Logs\bridge_ue_python.log'
$args = @($uproject, '-run=pythonscript', "-script=`"$scriptPath`"", '-unattended', '-nosplash', '-nosound', "-abslog=`"$logPath`"")
if (-not $WithRendering) { $args += '-nullrhi' }
& $editor @args | Out-Null
$code = $LASTEXITCODE
# Keep only lines the script marked, so raw engine logs (which can contain machine details) stay out of the result.
$lines = Get-Content $logPath -ErrorAction SilentlyContinue | Where-Object { $_ -match 'CODEX:' } | ForEach-Object { ($_ -replace '^.*?(CODEX:)', '$1') }
$result = @("exit=$code") + $lines
Set-Content -Path (Join-Path $outDir 'ue_last_result.txt') -Value $result -Encoding utf8
$result
exit $code
