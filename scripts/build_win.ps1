<#
.SYNOPSIS
    DivTG Local Build Script (Windows MSVC 2022)

.DESCRIPTION
    Configures and compiles DivTG.exe locally using Visual Studio 2022.
#>

[CmdletBinding()]
param(
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Debug",
    [switch]$ConfigureOnly
)

$ErrorActionPreference = "Stop"

Write-Host "=== DivTG Windows Build Script ===" -ForegroundColor Cyan
Write-Host "Configuration: $Configuration" -ForegroundColor Yellow

# 1. Check Python
try {
    $pyVer = python --version
    Write-Host "[OK] Python: $pyVer" -ForegroundColor Green
} catch {
    Write-Error "Python 3 is required. Please install Python and add it to PATH."
}

# 2. Check CMake
try {
    $cmakeVer = cmake --version | Select-Object -First 1
    Write-Host "[OK] CMake: $cmakeVer" -ForegroundColor Green
} catch {
    Write-Warning "cmake.exe not found directly in current PATH."
    Write-Warning "Make sure you run this script inside 'x64 Native Tools Command Prompt for VS 2022'!"
}

# 3. Check dependencies directory
$repoRoot = Resolve-Path "$PSScriptRoot\.."
$depsPath = "$repoRoot\..\win64\Libraries"
if (Test-Path $depsPath) {
    Write-Host "[OK] Dependencies found at: $depsPath" -ForegroundColor Green
} else {
    Write-Warning "Dependencies directory '$depsPath' not found."
    Write-Warning "For a full local build, Telegram dependencies must be placed in ../win64/Libraries."
    Write-Warning "Alternatively, you can use GitHub Actions to build automatically in the cloud without local setup!"
}

# 4. Generate icons if missing
$icoPath = "$repoRoot\Telegram\Resources\art\icon256.ico"
if (-not (Test-Path $icoPath)) {
    Write-Host "[+] Generating DivTG icons..." -ForegroundColor Yellow
    python "$PSScriptRoot\generate_icons.py"
}

# 5. CMake configure
$buildDir = "$repoRoot\out"
Write-Host "`n[+] Configuring CMake in $buildDir..." -ForegroundColor Cyan

cmake -B "$buildDir" `
    -S "$repoRoot" `
    -G "Visual Studio 17 2022" `
    -A x64 `
    -DTDESKTOP_API_ID=17349 `
    -DTDESKTOP_API_HASH=344583e45741c457fe1862106095a5eb

if ($ConfigureOnly) {
    Write-Host "`n[DONE] Project configured. You can open $buildDir\Telegram.sln in Visual Studio." -ForegroundColor Green
    return
}

# 6. Build target
Write-Host "`n[+] Compiling DivTG (target Telegram)..." -ForegroundColor Cyan
cmake --build "$buildDir" --config $Configuration --target Telegram

Write-Host "`n[SUCCESS] Build finished! Executable: $buildDir\$Configuration\DivTG.exe" -ForegroundColor Green
