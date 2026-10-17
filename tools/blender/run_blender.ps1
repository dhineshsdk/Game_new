# Runs a Python script in headless Blender and writes a clean result file.
# Usage: powershell -File tools\blender\run_blender.ps1 -Script tools\blender\hello_preview.py [-Args "a","b"]
param(
    [Parameter(Mandatory = $true)][string]$Script,
    [string[]]$ScriptArgs = @()
)
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$blender = 'C:\Program Files\Blender Foundation\Blender 5.2\blender.exe'
$outDir = Join-Path $root 'build\bridge'
New-Item -ItemType Directory -Force $outDir, (Join-Path $root 'build\previews') | Out-Null
$scriptPath = (Resolve-Path (Join-Path $root $Script)).Path
$raw = & $blender --background --factory-startup --python $scriptPath -- @ScriptArgs 2>&1
$code = $LASTEXITCODE
# Only lines the script marks with "CODEX:" go into the result file (keeps noisy logs out).
$lines = $raw | ForEach-Object { "$_" } | Where-Object { $_ -like 'CODEX:*' }
$result = @("exit=$code") + $lines
Set-Content -Path (Join-Path $outDir 'blender_last_result.txt') -Value $result -Encoding utf8
$result
exit $code
