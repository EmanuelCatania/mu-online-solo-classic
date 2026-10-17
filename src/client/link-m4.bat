@echo off
setlocal EnableDelayedExpansion
call C:\Users\andvolodko\emsdk\emsdk_env.bat >nul
set "ROOT=c:\Work\_Projects\mu_online_game"
set "TREE=%ROOT%\reference\Mu-97k-Client-Source"
set "OUT=%ROOT%\emscripten_port\out"
set "DATA=%ROOT%\game_097k\Client\Data"
set INC=-I %ROOT%\emscripten_port\browser -I %ROOT%\emscripten_port\browser\compat -I %TREE%\src -I %TREE%\lib\libjpeg\jpeg-6b -I %TREE%\lib\miniaudio
set CXXFLAGS=-c -std=c++17 -ferror-limit=20 -Wno-macro-redefined -Wno-ignored-attributes -Wno-deprecated-non-prototype -Wno-c++11-narrowing
for %%S in (Net\Net_Process.cpp Net\HWID.cpp Net\Network.cpp Net\MuEmu.cpp Net\Net_Connect.cpp Net\Net_Disconnect.cpp Net\Crypto.cpp Render\SMD_Parser.cpp Scene\Scene_CharSelect_Nav.cpp) do (
  set "REL=src\%%S"
  set "FLAT=src_%%S"
  set "FLAT=!FLAT:\=_!"
  set "FLAT=!FLAT:.cpp=.o!"
  del "%OUT%\all\!FLAT!" 2>nul
  emcc %CXXFLAGS% %INC% "%TREE%\!REL!" -o "%OUT%\all\!FLAT!" > "%ROOT%\emscripten_port\proof\m4-!FLAT!.txt" 2>&1
  if errorlevel 1 (
    echo COMPILE_FAIL %%S
    exit /b 1
  )
)
del "%ROOT%\emscripten_port\proof\m4-objs.txt" 2>nul
for %%F in ("%OUT%\all\*.o") do (
  set "P=%%~fF"
  set "P=!P:\=/!"
  echo !P!>> "%ROOT%\emscripten_port\proof\m4-objs.txt"
)
emcc -std=c++17 -ferror-limit=30 -sDEFAULT_TO_CXX -Wno-macro-redefined -Wno-ignored-attributes -Wno-deprecated-non-prototype -Wno-c++11-narrowing %INC% %ROOT%\emscripten_port\browser\shell_m4.cpp %ROOT%\emscripten_port\browser\net_ws.cpp @"%ROOT%\emscripten_port\proof\m4-objs.txt" -o %OUT%\shell_m4.html --shell-file %ROOT%\emscripten_port\browser\shell.html -sLEGACY_GL_EMULATION -sGL_FFP_ONLY -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sINITIAL_MEMORY=536870912 -sSTACK_SIZE=1048576 -sFORCE_FILESYSTEM=1 -lwebsocket.js --preload-file "%DATA%\Enc1.dat@/Data/Enc1.dat" --preload-file "%DATA%\Dec2.dat@/Data/Dec2.dat" -Wl,--error-limit=0 > %ROOT%\emscripten_port\proof\m4-link.txt 2>&1
if errorlevel 1 (
  echo LINK_FAIL
  exit /b 1
)
echo LINK_OK
exit /b 0
