# Browser build and compile probes. Leaves the native MSVC project untouched.
# Requires Emscripten SDK 6.0.12 (or newer 6.x) activated in this process.
$ErrorActionPreference = "Continue"
if (Get-Variable -Name PSNativeCommandUseErrorActionPreference -ErrorAction SilentlyContinue) {
    $PSNativeCommandUseErrorActionPreference = $false
}

$Port = $PSScriptRoot
$Client = Join-Path (Split-Path -Parent $Port) "reference\Mu-97k-Client-Source"
$Out = Join-Path $Port "out"
$Proof = Join-Path $Port "proof"
New-Item -ItemType Directory -Force -Path $Out, $Proof | Out-Null

function Invoke-Emcc {
    param([string[]]$EmccArgs, [string]$Log)
    Write-Host "emcc $($EmccArgs[0])"
    $output = & emcc @EmccArgs 2>&1
    $code = $LASTEXITCODE
    $output | Out-File -FilePath $Log -Encoding utf8
    if ($code -ne 0) {
        $output | Select-Object -Last 30 | ForEach-Object { Write-Host $_ }
    }
    return $code
}

# --- ABI ---
$code = Invoke-Emcc -Log (Join-Path $Proof "abi_probe-build.txt") -EmccArgs @(
    (Join-Path $Port "browser\abi_probe.cpp"),
    "-o", (Join-Path $Out "abi_probe.js"),
    "-sENVIRONMENT=node"
)
if ($code -ne 0) { throw "abi_probe failed: $code" }
node (Join-Path $Out "abi_probe.js") | Tee-Object -FilePath (Join-Path $Proof "abi_probe-run.txt")

# --- Protocol headers (existing headers, new driver) ---
$code = Invoke-Emcc -Log (Join-Path $Proof "protocol_check-build.txt") -EmccArgs @(
    (Join-Path $Port "browser\protocol_check.cpp"),
    "-I", (Join-Path $Client "src"),
    "-o", (Join-Path $Out "protocol_check.js"),
    "-sENVIRONMENT=node"
)
if ($code -ne 0) { throw "protocol_check failed: $code" }
node (Join-Path $Out "protocol_check.js")
"protocol_check exit=$LASTEXITCODE" | Tee-Object -FilePath (Join-Path $Proof "protocol_check-run.txt")

# --- libjpeg 6b, the smallest existing C subset ---
$jpegDir = Join-Path $Client "lib\libjpeg\jpeg-6b"
$jpegSources = @(
    "jcapimin.c","jcapistd.c","jccoefct.c","jccolor.c","jcdctmgr.c","jchuff.c",
    "jcinit.c","jcmainct.c","jcmarker.c","jcmaster.c","jcomapi.c","jcparam.c",
    "jcphuff.c","jcprepct.c","jcsample.c","jctrans.c","jdapimin.c","jdapistd.c",
    "jdatadst.c","jdatasrc.c","jdcoefct.c","jdcolor.c","jddctmgr.c","jdhuff.c",
    "jdinput.c","jdmainct.c","jdmarker.c","jdmaster.c","jdmerge.c","jdphuff.c",
    "jdpostct.c","jdsample.c","jdtrans.c","jerror.c","jfdctflt.c","jfdctfst.c",
    "jfdctint.c","jidctflt.c","jidctfst.c","jidctint.c","jidctred.c","jquant1.c",
    "jquant2.c","jutils.c","jmemmgr.c","jmemansi.c"
) | ForEach-Object { Join-Path $jpegDir $_ }

$code = Invoke-Emcc -Log (Join-Path $Proof "jpeg_roundtrip-build.txt") -EmccArgs (
    @((Join-Path $Port "browser\jpeg_roundtrip.cpp")) + $jpegSources + @(
        "-I", $jpegDir,
        "-o", (Join-Path $Out "jpeg_roundtrip.js"),
        "-sENVIRONMENT=node",
        "-Wno-deprecated-non-prototype",
        "-Wno-shift-negative-value"
    )
)
if ($code -ne 0) { throw "jpeg_roundtrip failed: $code" }
node (Join-Path $Out "jpeg_roundtrip.js") | Tee-Object -FilePath (Join-Path $Proof "jpeg_roundtrip-run.txt")
if ($LASTEXITCODE -ne 0) { throw "jpeg_roundtrip run failed: $LASTEXITCODE" }

# --- WebGL 2 canvas ---
$code = Invoke-Emcc -Log (Join-Path $Proof "webgl2_smoke-build.txt") -EmccArgs @(
    (Join-Path $Port "browser\webgl2_smoke.cpp"),
    "-o", (Join-Path $Out "webgl2_smoke.html"),
    "--shell-file", (Join-Path $Port "browser\shell.html"),
    "-sMIN_WEBGL_VERSION=2",
    "-sMAX_WEBGL_VERSION=2",
    "-sENVIRONMENT=web"
)
if ($code -ne 0) { throw "webgl2_smoke failed: $code" }

# --- SEH is expected to fail ---
Invoke-Emcc -Log (Join-Path $Proof "seh_probe-build.txt") -EmccArgs @(
    (Join-Path $Port "browser\seh_probe.cpp"),
    "-o", (Join-Path $Out "seh_probe.js"),
    "-sENVIRONMENT=node"
) | Out-Null

# --- Existing client translation units. These are expected to fail. ---
$includes = @(
    "-I", (Join-Path $Client "src"),
    "-I", $jpegDir,
    "-I", (Join-Path $Client "lib\miniaudio")
)
$units = @(
    "src\Math\Math_3D.cpp",
    "src\Render\GL_State.cpp",
    "src\Render\BMD_DrawMesh.cpp",
    "src\Net\PacketFrame.cpp",
    "src\Net\Protocol\Protocol_Check.cpp",
    "src\Core\Window.cpp"
)
foreach ($unit in $units) {
    $name = [IO.Path]::GetFileNameWithoutExtension($unit)
    $src = Join-Path $Client $unit
    Invoke-Emcc -Log (Join-Path $Proof "$name-build.txt") -EmccArgs (
        @("-c", $src, "-std=c++17") + $includes + @("-o", (Join-Path $Out "$name.o"))
    ) | Out-Null
}

Write-Host "BUILD_OK out=$Out"
