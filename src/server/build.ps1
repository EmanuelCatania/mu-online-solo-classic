<#
.SYNOPSIS
    Builds the MU Online Solo Classic WebAssembly Server (Solo Engine).
    Compiles with Emscripten, packages /Data virtual filesystem, and deploys to web/.
#>

param(
    [switch]$Clean = $false
)

$ErrorActionPreference = "Stop"

# --- 1. Detect Emscripten SDK ---
if (-not (Get-Command emcc -ErrorAction SilentlyContinue)) {
    $cand = @()
    if ($env:EMSDK -and (Test-Path (Join-Path $env:EMSDK "emsdk_env.bat"))) { $cand += $env:EMSDK }
    $userEmsdk = Join-Path $env:USERPROFILE "emsdk"
    if (Test-Path (Join-Path $userEmsdk "emsdk_env.bat")) { $cand += $userEmsdk }
    if (Test-Path "C:\emsdk\emsdk_env.bat") { $cand += "C:\emsdk" }

    if ($cand.Count -gt 0) {
        $emsdkDir = $cand[0]
        Write-Host "Activating Emscripten SDK from $emsdkDir..." -ForegroundColor Cyan
        $envVars = & cmd.exe /c "call `"$emsdkDir\emsdk_env.bat`" >nul 2>&1 && set"
        foreach ($line in $envVars) {
            $idx = $line.IndexOf('=')
            if ($idx -gt 0) {
                $name = $line.Substring(0, $idx)
                $val = $line.Substring($idx + 1)
                [Environment]::SetEnvironmentVariable($name, $val, "Process")
            }
        }
    }
}

if (-not (Get-Command emcc -ErrorAction SilentlyContinue)) {
    Write-Error "emcc compiler not found in PATH, `$env:EMSDK, or `$env:USERPROFILE\emsdk. Please install/activate Emscripten SDK."
    exit 1
}

# --- 1.1 Detect CMake and Ninja (e.g. Visual Studio) ---
$vsCmakeBin = "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin"
$vsNinjaBin = "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja"

if (Test-Path $vsCmakeBin) {
    $env:PATH = "$vsCmakeBin;$env:PATH"
}
if (Test-Path $vsNinjaBin) {
    $env:PATH = "$vsNinjaBin;$env:PATH"
}

# --- 2. Resolve Paths ---
$ServerDir = $PSScriptRoot
$RepoRoot = Resolve-Path (Join-Path $ServerDir "..\..")
$BuildDir = Join-Path $ServerDir "build"
$WebDir = Join-Path $RepoRoot "web"

if ($Clean -and (Test-Path $BuildDir)) {
    Write-Host "Cleaning build directory $BuildDir..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force $BuildDir
}

New-Item -ItemType Directory -Force -Path $BuildDir, $WebDir | Out-Null

# --- 3. Configure with CMake ---
Write-Host "Configuring CMake for WebAssembly server..." -ForegroundColor Cyan
& emcmake cmake -S $ServerDir -B $BuildDir -G Ninja -DCMAKE_BUILD_TYPE=Release
if ($LASTEXITCODE -ne 0) {
    Write-Error "CMake configuration failed!"
    exit 1
}

# --- 4. Build with Ninja ---
Write-Host "Building muserver with Ninja..." -ForegroundColor Cyan
& ninja -C $BuildDir
if ($LASTEXITCODE -ne 0) {
    Write-Error "Ninja build failed!"
    exit 1
}

# --- 5. Deploy all artifacts (JS, WASM, and DATA package) to web/ ---
Write-Host "Deploying server artifacts to web/..." -ForegroundColor Cyan
Copy-Item (Join-Path $BuildDir "muserver.js") (Join-Path $WebDir "muserver.js") -Force
Copy-Item (Join-Path $BuildDir "muserver.wasm") (Join-Path $WebDir "muserver.wasm") -Force
Copy-Item (Join-Path $BuildDir "muserver.data") (Join-Path $WebDir "muserver.data") -Force

Write-Host "Server build and deployment complete! All artifacts synchronized." -ForegroundColor Green
