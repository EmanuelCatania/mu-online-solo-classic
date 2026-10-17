@echo off
call C:\Users\andvolodko\emsdk\emsdk_env.bat >nul
set "ROOT=c:\Work\_Projects\mu_online_game"
set "SRC=%ROOT%\reference\Mu-97k-Client-Source\src"
set "OUT=%ROOT%\emscripten_port\out"
set INC=-I %ROOT%\emscripten_port\browser -I %ROOT%\emscripten_port\browser\compat -I %SRC% -I %ROOT%\reference\Mu-97k-Client-Source\lib\libjpeg\jpeg-6b -I %ROOT%\reference\Mu-97k-Client-Source\lib\miniaudio
set BMD=%ROOT%\game_097k\Client\Data\Item\Axe01.bmd

emcc -std=c++17 -sDEFAULT_TO_CXX -Wno-macro-redefined -Wno-ignored-attributes -Wno-deprecated-non-prototype -Wno-c++11-narrowing %INC% %ROOT%\emscripten_port\browser\mesh_one.cpp %OUT%\Math_3D.o %OUT%\PacketFrame.o %OUT%\GL_State.o %OUT%\BMD_DrawMesh.o %OUT%\BMD_Load.o %OUT%\GL_LegacyState.o %OUT%\Render_LegacyTransforms.o %OUT%\BMD_LegacyDraw.o %OUT%\globals.o -o %OUT%\mesh_one.html --shell-file %ROOT%\emscripten_port\browser\shell.html -sLEGACY_GL_EMULATION -sGL_FFP_ONLY -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sINITIAL_MEMORY=134217728 -sSTACK_SIZE=1048576 -sFORCE_FILESYSTEM=1 --preload-file "%BMD%@/Data/Item/Axe01.bmd" > %ROOT%\emscripten_port\proof\m1-link.txt 2>&1
if errorlevel 1 (
  echo LINK_FAIL
  exit /b 1
)
echo LINK_OK
exit /b 0
