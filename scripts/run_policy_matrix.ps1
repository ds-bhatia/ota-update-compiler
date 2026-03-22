param(
    [string]$ClangExe = "clang",
    [string]$OptExe = "opt",
    [string]$TestsDir = "tests",
    [string]$PluginPath = ""
)

$ErrorActionPreference = "Stop"

function Resolve-PluginPath {
    param([string]$ExplicitPath)

    if ($ExplicitPath -and (Test-Path $ExplicitPath)) {
        return (Resolve-Path $ExplicitPath).Path
    }

    $candidates = @(
        "llvm-pass/build/TraversalPass.dll",
        "llvm-pass/build/Release/TraversalPass.dll",
        "llvm-pass/build/Debug/TraversalPass.dll",
        "llvm-pass/build/TraversalPass.so",
        "llvm-pass/build/libTraversalPass.so"
    )

    foreach ($candidate in $candidates) {
        if (Test-Path $candidate) {
            return (Resolve-Path $candidate).Path
        }
    }

    throw "Unable to find pass plugin. Build the plugin first or pass -PluginPath."
}

function Run-PolicyCheck {
    param(
        [string]$CFile,
        [string]$LlFile,
        [string]$Expected,
        [string]$Clang,
        [string]$Opt,
        [string]$Plugin
    )

    & $Clang -S -emit-llvm $CFile -o $LlFile
    if ($LASTEXITCODE -ne 0) {
        return @{ success = $false; detail = "clang failed" }
    }

    & $Opt -load-pass-plugin $Plugin -passes=traversal-pass -disable-output $LlFile
    $passSucceeded = ($LASTEXITCODE -eq 0)

    if ($Expected -eq "pass" -and $passSucceeded) {
        return @{ success = $true; detail = "expected pass, got pass" }
    }

    if ($Expected -eq "fail" -and -not $passSucceeded) {
        return @{ success = $true; detail = "expected fail, got fail" }
    }

    return @{ success = $false; detail = "expected $Expected, got " + ($(if ($passSucceeded) { "pass" } else { "fail" })) }
}

$plugin = Resolve-PluginPath -ExplicitPath $PluginPath

$testFiles = Get-ChildItem -Path $TestsDir -Filter "*.c" | Sort-Object Name
if ($testFiles.Count -eq 0) {
    throw "No .c files found in $TestsDir"
}

$failures = 0

Write-Host "Using plugin: $plugin"
Write-Host ""

foreach ($file in $testFiles) {
    $base = [System.IO.Path]::GetFileNameWithoutExtension($file.Name)
    $ll = Join-Path $TestsDir ($base + ".ll")

    $expected = "skip"
    if ($base.StartsWith("secure")) { $expected = "pass" }
    if ($base.StartsWith("insecure")) { $expected = "fail" }

    if ($expected -eq "skip") {
        Write-Host "[SKIP] $($file.Name) (no secure/insecure prefix)"
        continue
    }

    $result = Run-PolicyCheck -CFile $file.FullName -LlFile $ll -Expected $expected -Clang $ClangExe -Opt $OptExe -Plugin $plugin
    if ($result.success) {
        Write-Host "[OK]   $($file.Name): $($result.detail)"
    }
    else {
        Write-Host "[FAIL] $($file.Name): $($result.detail)"
        $failures += 1
    }
}

Write-Host ""
if ($failures -gt 0) {
    Write-Host "Matrix result: FAILED ($failures mismatches)"
    exit 1
}

Write-Host "Matrix result: PASSED"
exit 0
