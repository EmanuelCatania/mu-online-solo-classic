@echo off
setlocal EnableDelayedExpansion
call C:\Users\andvolodko\emsdk\emsdk_env.bat >nul
set "ROOT=c:\Work\_Projects\mu_online_game"
set "TREE=%ROOT%\reference\Mu-97k-Client-Source"
set "OUT=%ROOT%\emscripten_port\out"
set "DATA=%ROOT%\game_097k\Client\Data"
set INC=-I %ROOT%\emscripten_port\browser -I %ROOT%\emscripten_port\browser\compat -I %TREE%\src -I %TREE%\lib\libjpeg\jpeg-6b -I %TREE%\lib\miniaudio
set CXXFLAGS=-c -std=c++17 -ferror-limit=20 -Wno-macro-redefined -Wno-ignored-attributes -Wno-deprecated-non-prototype -Wno-c++11-narrowing
for %%S in (Sound\Sound.cpp Sound\SoundManager.cpp Sound\Sound_DS3D.cpp) do (
  set "REL=src\%%S"
  set "FLAT=src_%%S"
  set "FLAT=!FLAT:\=_!"
  set "FLAT=!FLAT:.cpp=.o!"
  del "%OUT%\all\!FLAT!" 2>nul
  emcc %CXXFLAGS% %INC% "%TREE%\!REL!" -o "%OUT%\all\!FLAT!" > "%ROOT%\emscripten_port\proof\m5-!FLAT!.txt" 2>&1
  if errorlevel 1 (
    echo COMPILE_FAIL %%S
    exit /b 1
  )
)
del "%ROOT%\emscripten_port\proof\m5-objs.txt" 2>nul
for %%F in ("%OUT%\all\*.o") do (
  set "P=%%~fF"
  set "P=!P:\=/!"
  echo !P!>> "%ROOT%\emscripten_port\proof\m5-objs.txt"
)
emcc -std=c++17 -ferror-limit=30 -sDEFAULT_TO_CXX -Wno-macro-redefined -Wno-ignored-attributes -Wno-deprecated-non-prototype -Wno-c++11-narrowing %INC% %ROOT%\emscripten_port\browser\shell_m5.cpp %ROOT%\emscripten_port\browser\net_ws.cpp @"%ROOT%\emscripten_port\proof\m5-objs.txt" -o %OUT%\shell_m5.html --shell-file %ROOT%\emscripten_port\browser\shell.html -sLEGACY_GL_EMULATION -sGL_FFP_ONLY -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sINITIAL_MEMORY=536870912 -sSTACK_SIZE=1048576 -sFORCE_FILESYSTEM=1 -lwebsocket.js --preload-file "%DATA%\World1@/Data/World1" --preload-file "%DATA%\Player\ArmorClass01.bmd@/Data/Player/ArmorClass01.bmd" --preload-file "%DATA%\Music\Lorencia.mp3@/Data/Music/Lorencia.mp3" --preload-file "%DATA%\Sound\aMouse.WAV@/Data/Sound/aMouse.WAV" --preload-file "%DATA%\Enc1.dat@/Data/Enc1.dat" --preload-file "%DATA%\Dec2.dat@/Data/Dec2.dat" -Wl,--error-limit=0 > %ROOT%\emscripten_port\proof\m5-link.txt 2>&1
if errorlevel 1 (
  echo LINK_FAIL
  exit /b 1
)
echo LINK_OK
exit /b 0
