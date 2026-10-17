@echo off
setlocal EnableDelayedExpansion

REM --- Detect emcc compiler in PATH, EMSDK, or %USERPROFILE%\emsdk ---
where emcc >nul 2>nul
if errorlevel 1 (
  if defined EMSDK (
    if exist "%EMSDK%\emsdk_env.bat" call "%EMSDK%\emsdk_env.bat" >nul 2>nul
  )
)
where emcc >nul 2>nul
if errorlevel 1 (
  if exist "%USERPROFILE%\emsdk\emsdk_env.bat" (
    call "%USERPROFILE%\emsdk\emsdk_env.bat" >nul 2>nul
  ) else if exist "C:\emsdk\emsdk_env.bat" (
    call "C:\emsdk\emsdk_env.bat" >nul 2>nul
  )
)
where emcc >nul 2>nul
if errorlevel 1 (
  echo Error: emcc compiler not found in PATH or EMSDK.
  echo Please install and activate Emscripten SDK: https://emscripten.org
  exit /b 1
)

set "CLIENT_DIR=%~dp0"
if "%CLIENT_DIR:~-1%"=="\" set "CLIENT_DIR=%CLIENT_DIR:~0,-1%"

set "OUT=%CLIENT_DIR%\out\all"
set "PROOF=%CLIENT_DIR%\proof\all"
if not exist "%OUT%" mkdir "%OUT%"
if not exist "%PROOF%" mkdir "%PROOF%"

set INC=-I "%CLIENT_DIR%\browser" -I "%CLIENT_DIR%\browser\compat" -I "%CLIENT_DIR%\src" -I "%CLIENT_DIR%\lib\libjpeg\jpeg-6b" -I "%CLIENT_DIR%\lib\miniaudio"
set CXXFLAGS=-c -std=c++17 -ferror-limit=5 -Wno-macro-redefined -Wno-ignored-attributes -Wno-deprecated-non-prototype -Wno-c++11-narrowing
set CFLAGS=-c -std=c11 -ferror-limit=5 -Wno-deprecated-non-prototype -I "%CLIENT_DIR%\lib\libjpeg\jpeg-6b"

set OK=0
set BAD=0
set N=0

for /f "usebackq delims=" %%R in ("%CLIENT_DIR%\m2-sources.txt") do (
  set /a N+=1
  call :one %%R
)
echo DONE ok=!OK! bad=!BAD! total=!N!
exit /b 0

:one
set "REL=%~1"
set "FLAT=%REL:\=_%"
set "FLAT=%FLAT:/=_%"
set "FLAT=%FLAT:.cpp=.o%"
set "FLAT=%FLAT:.c=.o%"
if exist "%OUT%\%FLAT%" (
  set /a OK+=1
  exit /b 0
)
echo [!N!] %REL%
if /I "%~x1"==".c" (
  emcc %CFLAGS% "%CLIENT_DIR%\%REL%" -o "%OUT%\%FLAT%" > "%PROOF%\%FLAT%.txt" 2>&1
) else (
  emcc %CXXFLAGS% %INC% "%CLIENT_DIR%\%REL%" -o "%OUT%\%FLAT%" > "%PROOF%\%FLAT%.txt" 2>&1
)
if errorlevel 1 (
  set /a BAD+=1
  echo FAIL %REL%>> "%CLIENT_DIR%\proof\compile-fail.txt"
) else (
  set /a OK+=1
)
exit /b 0
