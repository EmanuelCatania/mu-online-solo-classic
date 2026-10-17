@echo off
call C:\Users\andvolodko\emsdk\emsdk_env.bat >nul
set "ROOT=c:\Work\_Projects\mu_online_game"
set "SRC=%ROOT%\reference\Mu-97k-Client-Source\src"
set "OUT=%ROOT%\emscripten_port\out"
set INC=-I %ROOT%\emscripten_port\browser -I %ROOT%\emscripten_port\browser\compat -I %SRC% -I %ROOT%\reference\Mu-97k-Client-Source\lib\libjpeg\jpeg-6b -I %ROOT%\reference\Mu-97k-Client-Source\lib\miniaudio

emcc -std=c++17 -sDEFAULT_TO_CXX -Wno-macro-redefined -Wno-ignored-attributes -Wno-deprecated-non-prototype -Wno-c++11-narrowing %INC% %ROOT%\emscripten_port\browser\shell_m2.cpp %OUT%\Game_MainLoop.o %OUT%\FrameLimiter.o %OUT%\Window.o %OUT%\Timer.o %OUT%\Misc.o %OUT%\Sound_DS3D.o %OUT%\Sound_Queue.o %OUT%\Reconnect.o %OUT%\Camera.o %OUT%\Camera3D.o %OUT%\Input_WndProc.o %OUT%\Input.o %OUT%\UI_InGameMenu.o %OUT%\Chat_Log.o %OUT%\System_Legacy.o %OUT%\Game_SceneUpdate.o %OUT%\Game_EnterWorldTick.o %OUT%\Game_CharSelectTick.o %OUT%\Entity_Init.o %OUT%\Scene_CharSelect.o %OUT%\Crypto.o %OUT%\Scene_Login.o %OUT%\Render_Frame.o %OUT%\MapManager.o %OUT%\Math_3D.o %OUT%\PacketFrame.o %OUT%\GL_State.o %OUT%\BMD_DrawMesh.o %OUT%\BMD_Load.o %OUT%\GL_LegacyState.o %OUT%\Render_LegacyTransforms.o %OUT%\BMD_LegacyDraw.o %OUT%\globals.o -o %OUT%\shell_m2.html --shell-file %ROOT%\emscripten_port\browser\shell.html -sLEGACY_GL_EMULATION -sGL_FFP_ONLY -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sINITIAL_MEMORY=134217728 -sSTACK_SIZE=1048576 -sFORCE_FILESYSTEM=1 -Wl,--error-limit=0 > %ROOT%\emscripten_port\proof\m2-link.txt 2>&1
if errorlevel 1 (
  echo LINK_FAIL
  exit /b 1
)
echo LINK_OK
exit /b 0
