@echo off
call C:\Users\andvolodko\emsdk\emsdk_env.bat >nul
set "ROOT=c:\Work\_Projects\mu_online_game"
set "SRC=%ROOT%\reference\Mu-97k-Client-Source\src"
set "OUT=%ROOT%\emscripten_port\out"
set "PROOF=%ROOT%\emscripten_port\proof"
set INC=-I %ROOT%\emscripten_port\browser -I %ROOT%\emscripten_port\browser\compat -I %SRC% -I %ROOT%\reference\Mu-97k-Client-Source\lib\libjpeg\jpeg-6b -I %ROOT%\reference\Mu-97k-Client-Source\lib\miniaudio
set FLAGS=-c -std=c++17 -ferror-limit=15 -Wno-macro-redefined -Wno-ignored-attributes -Wno-deprecated-non-prototype -Wno-c++11-narrowing

call :one PacketFrame %SRC%\Net\PacketFrame.cpp %OUT%\PacketFrame.o %PROOF%\m1-packet.txt
if errorlevel 1 exit /b 1
call :one GL_State %SRC%\Render\GL_State.cpp %OUT%\GL_State.o %PROOF%\m1-gl.txt
if errorlevel 1 exit /b 1
call :one BMD_DrawMesh %SRC%\Render\BMD_DrawMesh.cpp %OUT%\BMD_DrawMesh.o %PROOF%\m1-draw.txt
if errorlevel 1 exit /b 1
call :one BMD_Load %SRC%\Render\BMD_Load.cpp %OUT%\BMD_Load.o %PROOF%\m1-load.txt
if errorlevel 1 exit /b 1
call :one GL_LegacyState %SRC%\Render\GL_LegacyState.cpp %OUT%\GL_LegacyState.o %PROOF%\m1-gllegacy.txt
if errorlevel 1 exit /b 1
call :one LegacyTransforms %SRC%\Render\Render_LegacyTransforms.cpp %OUT%\Render_LegacyTransforms.o %PROOF%\m1-xform.txt
if errorlevel 1 exit /b 1
call :one BMD_LegacyDraw %SRC%\Render\BMD_LegacyDraw.cpp %OUT%\BMD_LegacyDraw.o %PROOF%\m1-legacydraw.txt
if errorlevel 1 exit /b 1
call :one globals %SRC%\globals.cpp %OUT%\globals.o %PROOF%\m1-globals.txt
if errorlevel 1 exit /b 1
echo COMPILE_OK
exit /b 0

:one
echo COMPILING %~1
emcc %FLAGS% %INC% %~2 -o %~3 > %~4 2>&1
exit /b %ERRORLEVEL%
