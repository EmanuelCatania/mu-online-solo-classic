@echo off
setlocal EnableDelayedExpansion
call C:\Users\andvolodko\emsdk\emsdk_env.bat >nul
set "ROOT=c:\Work\_Projects\mu_online_game"
set "TREE=%ROOT%\reference\Mu-97k-Client-Source"
set "OUT=%ROOT%\emscripten_port\out"
set INC=-I %ROOT%\emscripten_port\browser -I %ROOT%\emscripten_port\browser\compat -I %TREE%\src -I %TREE%\lib\libjpeg\jpeg-6b -I %TREE%\lib\miniaudio
del "%ROOT%\emscripten_port\proof\m2-objs.txt" 2>nul
for %%F in ("%OUT%\all\*.o") do (
  set "P=%%~fF"
  set "P=!P:\=/!"
  echo !P!>> "%ROOT%\emscripten_port\proof\m2-objs.txt"
)
emcc -std=c++17 -sDEFAULT_TO_CXX -Wno-macro-redefined -Wno-ignored-attributes -Wno-deprecated-non-prototype -Wno-c++11-narrowing %INC% %ROOT%\emscripten_port\browser\shell_m2.cpp @"%ROOT%\emscripten_port\proof\m2-objs.txt" -o %OUT%\shell_m2.html --shell-file %ROOT%\emscripten_port\browser\shell.html -sLEGACY_GL_EMULATION -sGL_FFP_ONLY -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sINITIAL_MEMORY=536870912 -sSTACK_SIZE=1048576 -sFORCE_FILESYSTEM=1 -Wl,--error-limit=0 > %ROOT%\emscripten_port\proof\m2-link.txt 2>&1
if errorlevel 1 (
  echo LINK_FAIL
  exit /b 1
)
echo LINK_OK
exit /b 0
