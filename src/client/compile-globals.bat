@echo off
call C:\Users\andvolodko\emsdk\emsdk_env.bat >nul
set "ROOT=c:\Work\_Projects\mu_online_game"
set "SRC=%ROOT%\reference\Mu-97k-Client-Source\src"
set INC=-I %ROOT%\emscripten_port\browser -I %ROOT%\emscripten_port\browser\compat -I %SRC% -I %ROOT%\reference\Mu-97k-Client-Source\lib\libjpeg\jpeg-6b -I %ROOT%\reference\Mu-97k-Client-Source\lib\miniaudio
emcc -c -std=c++17 -ferror-limit=20 -Wno-macro-redefined -Wno-ignored-attributes -Wno-deprecated-non-prototype -Wno-c++11-narrowing %INC% %SRC%\globals.cpp -o %ROOT%\emscripten_port\out\globals.o > %ROOT%\emscripten_port\proof\m1-globals.txt 2>&1
if errorlevel 1 (
  echo FAIL
  exit /b 1
)
echo COMPILE_OK
exit /b 0
