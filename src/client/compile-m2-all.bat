@echo off
setlocal EnableDelayedExpansion
call C:\Users\andvolodko\emsdk\emsdk_env.bat >nul
set "ROOT=c:\Work\_Projects\mu_online_game"
set "TREE=%ROOT%\reference\Mu-97k-Client-Source"
set "OUT=%ROOT%\emscripten_port\out\all"
set "PROOF=%ROOT%\emscripten_port\proof\all"
if not exist "%OUT%" mkdir "%OUT%"
if not exist "%PROOF%" mkdir "%PROOF%"
set INC=-I %ROOT%\emscripten_port\browser -I %ROOT%\emscripten_port\browser\compat -I %TREE%\src -I %TREE%\lib\libjpeg\jpeg-6b -I %TREE%\lib\miniaudio
set CXXFLAGS=-c -std=c++17 -ferror-limit=5 -Wno-macro-redefined -Wno-ignored-attributes -Wno-deprecated-non-prototype -Wno-c++11-narrowing
set CFLAGS=-c -std=c11 -ferror-limit=5 -Wno-deprecated-non-prototype -I %TREE%\lib\libjpeg\jpeg-6b
set OK=0
set BAD=0
set N=0
for /f "usebackq delims=" %%R in ("%ROOT%\emscripten_port\proof\m2-sources.txt") do (
  set /a N+=1
  call :one %%R
)
echo DONE ok=!OK! bad=!BAD! total=!N!
echo DONE ok=!OK! bad=!BAD! total=!N! > "%ROOT%\emscripten_port\proof\m2-all-status.txt"
exit /b 0

:one
set "REL=%~1"
set "FLAT=%REL:\=_%"
set "FLAT=%FLAT:/=_%"
set "FLAT=%FLAT:.cpp=.o%"
set "FLAT=%FLAT:.c=.o%"
echo [!N!] %REL%
if exist "%OUT%\%FLAT%" (
  set /a OK+=1
  exit /b 0
)
if /I "%~x1"==".c" (
  emcc %CFLAGS% "%TREE%\%REL%" -o "%OUT%\%FLAT%" > "%PROOF%\%FLAT%.txt" 2>&1
) else (
  emcc %CXXFLAGS% %INC% "%TREE%\%REL%" -o "%OUT%\%FLAT%" > "%PROOF%\%FLAT%.txt" 2>&1
)
if errorlevel 1 (
  set /a BAD+=1
  echo FAIL %REL%>> "%ROOT%\emscripten_port\proof\m2-all-fail.txt"
) else (
  set /a OK+=1
)
exit /b 0
