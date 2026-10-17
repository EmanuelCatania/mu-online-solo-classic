@echo off
setlocal EnableDelayedExpansion
call C:\Users\andvolodko\emsdk\emsdk_env.bat >nul
set "ROOT=c:\Work\_Projects\mu_online_game"
set "TREE=%ROOT%\reference\Mu-97k-Client-Source"
set "OUT=%ROOT%\emscripten_port\out"
set "DATA=%ROOT%\game_097k\Client\Data"
set "STAGE=%OUT%\login-data"
set INC=-I %ROOT%\emscripten_port\browser -I %ROOT%\emscripten_port\browser\compat -I %TREE%\src -I %TREE%\lib\libjpeg\jpeg-6b -I %TREE%\lib\miniaudio
if exist "%STAGE%" rmdir /s /q "%STAGE%"
powershell -NoProfile -Command ^
  "$data='%DATA%'; $stage='%STAGE%';" ^
  "Get-ChildItem $data -Recurse -File | ForEach-Object {" ^
  "  $rel=$_.FullName.Substring($data.Length).TrimStart('\');" ^
  "  $n=$_.Name;" ^
  "  if ($n -match '(?i)\.ozj$') { $n=$n.Substring(0,$n.Length-4)+'.OZJ' }" ^
  "  elseif ($n -match '(?i)\.ozt$') { $n=$n.Substring(0,$n.Length-4)+'.OZT' }" ^
  "  $destDir=Join-Path $stage ([IO.Path]::GetDirectoryName($rel));" ^
  "  if (-not $destDir) { $destDir=$stage };" ^
  "  New-Item -ItemType Directory -Force -Path $destDir | Out-Null;" ^
  "  Copy-Item -LiteralPath $_.FullName -Destination (Join-Path $destDir $n) -Force" ^
  "};" ^
  "Copy-Item (Join-Path $data 'Music\Lorencia.mp3') (Join-Path $stage 'Music\MuTheme.mp3') -Force;" ^
  "function Exact([string]$dir,[string]$want) {" ^
  "  $hit = Get-ChildItem $dir -File | Where-Object { $_.Name -ieq $want -and $_.Name -cne $want } | Select-Object -First 1;" ^
  "  if (-not $hit) { return }" ^
  "  $tmp = Join-Path $dir ($want + '.renaming');" ^
  "  Move-Item -LiteralPath $hit.FullName -Destination $tmp -Force;" ^
  "  Move-Item -LiteralPath $tmp -Destination (Join-Path $dir $want) -Force" ^
  "}" ^
  "Exact (Join-Path $stage 'Local') 'Item.bmd';" ^
  "Exact (Join-Path $stage 'Local') 'Skill.bmd';" ^
  "Exact (Join-Path $stage 'Local') 'Quest.bmd'"
del "%ROOT%\emscripten_port\proof\login-objs.txt" 2>nul
for %%F in ("%OUT%\all\*.o") do (
  set "P=%%~fF"
  set "P=!P:\=/!"
  echo !P!>> "%ROOT%\emscripten_port\proof\login-objs.txt"
)
emcc -std=c++17 -ferror-limit=30 -sDEFAULT_TO_CXX -Wno-macro-redefined -Wno-ignored-attributes -Wno-deprecated-non-prototype -Wno-c++11-narrowing %INC% %ROOT%\emscripten_port\browser\shell_login.cpp %ROOT%\emscripten_port\browser\net_ws.cpp @"%ROOT%\emscripten_port\proof\login-objs.txt" -o %OUT%\shell_login.html --shell-file %ROOT%\emscripten_port\browser\shell.html --profiling-funcs -sLEGACY_GL_EMULATION -sGL_FFP_ONLY -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sINITIAL_MEMORY=536870912 -sSTACK_SIZE=8388608 -sFORCE_FILESYSTEM=1 -sALLOW_MEMORY_GROWTH=0 -lwebsocket.js --preload-file "%STAGE%@/Data" --preload-file "%ROOT%\game_097k\Client\Config.ini@/config.ini" -Wl,--error-limit=0 > %ROOT%\emscripten_port\proof\login-link.txt 2>&1
if errorlevel 1 (
  echo LINK_FAIL
  exit /b 1
)
echo LINK_OK
powershell -NoProfile -File "%ROOT%\emscripten_port\patch-login-page.ps1" -Token login100
if errorlevel 1 (
  echo PATCH_FAIL
  exit /b 1
)
exit /b 0
