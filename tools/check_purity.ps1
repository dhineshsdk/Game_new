# Enforces ADR 0005: CodexCore and CodexVehicleDynamics are standard C++20 only.
# Usage: powershell -File tools\check_purity.ps1   (exit 0 = pure, 1 = violations listed)
# Exempt: the one module-glue file per module (Private/*Module.cpp) and the Build.cs itself.
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot

$allowedStd = @('cmath', 'cstdint', 'cstddef', 'array', 'type_traits', 'limits', 'bit')
$banned = @(
    'CoreMinimal', '\bUSTRUCT\b', '\bUCLASS\b', '\bUPROPERTY\b', '\bUFUNCTION\b', '\bGENERATED_BODY\b', '\.generated\.h',
    '\bUE_LOG\b', '\bFVector\w*', '\bFQuat\b', '\bFQuat4\w*', '\bFMath\b', '\bTArray\b', '\bFString\b', '\bTEXT\(',
    '\bcheck\(', '\bensure\(', '\bthrow\b', '\btry\b', '\bcatch\b', '\bdynamic_cast\b', '\btypeid\b', '\bUObject\b'
)
$allowedDeps = @{
    'CodexCore'            = @('Core')
    'CodexVehicleDynamics' = @('Core', 'CodexCore')
}

$violations = @()
foreach ($module in $allowedDeps.Keys) {
    $dir = Join-Path $root "Source\$module"
    $files = Get-ChildItem $dir -Recurse -File -Include *.h, *.cpp | Where-Object { $_.Name -notlike '*Module.cpp' }
    foreach ($f in $files) {
        $rel = $f.FullName.Substring($root.Length + 1)
        $n = 0
        foreach ($line in Get-Content $f.FullName) {
            $n++
            $code = ($line -replace '//.*$', '')
            if ($code -match '^\s*#\s*include\s*[<"]([^>"]+)[>"]') {
                $inc = $Matches[1]
                $ok = ($allowedStd -contains $inc) -or ($inc -match '^Codex\w*\.h$')
                if (-not $ok) { $violations += "${rel}:${n}: include not allowed: $inc" }
            }
            foreach ($b in $banned) {
                if ($code -match $b) { $violations += "${rel}:${n}: banned token '$($Matches[0])'" }
            }
        }
    }

    $buildCs = Get-Content (Join-Path $dir "$module.Build.cs") -Raw
    $deps = @()
    foreach ($m in [regex]::Matches($buildCs, 'DependencyModuleNames\.AddRange\(\s*new\s+string\[\]\s*\{([^}]*)\}')) {
        $deps += [regex]::Matches($m.Groups[1].Value, '"([^"]+)"') | ForEach-Object { $_.Groups[1].Value }
    }
    if ($buildCs -match 'DependencyModuleNames\.Add\(') { $violations += "$module.Build.cs: use AddRange only (single Add not checked)" }
    $extra = $deps | Where-Object { $allowedDeps[$module] -notcontains $_ }
    if ($extra) { $violations += "$module.Build.cs: dependency not allowed: $($extra -join ', ')" }
    if ($buildCs -notmatch 'FPSemantics\s*=\s*FPSemanticsMode\.Precise') { $violations += "$module.Build.cs: FPSemantics must be Precise" }
    if ($buildCs -notmatch 'PCHUsage\s*=\s*PCHUsageMode\.NoPCHs') { $violations += "$module.Build.cs: PCHUsage must be NoPCHs" }
}

if ($violations.Count -gt 0) {
    $violations | ForEach-Object { Write-Output "PURITY: $_" }
    Write-Output "PURITY FAILED: $($violations.Count) violation(s)"
    exit 1
}
Write-Output 'PURITY OK: CodexCore, CodexVehicleDynamics'
exit 0
