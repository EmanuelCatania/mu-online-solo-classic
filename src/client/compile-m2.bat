@echo off
call C:\Users\andvolodko\emsdk\emsdk_env.bat >nul
set "ROOT=c:\Work\_Projects\mu_online_game"
set "SRC=%ROOT%\reference\Mu-97k-Client-Source\src"
set "OUT=%ROOT%\emscripten_port\out"
set "PROOF=%ROOT%\emscripten_port\proof"
set INC=-I %ROOT%\emscripten_port\browser -I %ROOT%\emscripten_port\browser\compat -I %SRC% -I %ROOT%\reference\Mu-97k-Client-Source\lib\libjpeg\jpeg-6b -I %ROOT%\reference\Mu-97k-Client-Source\lib\miniaudio
set FLAGS=-c -std=c++17 -ferror-limit=20 -Wno-macro-redefined -Wno-ignored-attributes -Wno-deprecated-non-prototype -Wno-c++11-narrowing

call :one Game_MainLoop %SRC%\Game\Game_MainLoop.cpp %OUT%\Game_MainLoop.o %PROOF%\m2-loop.txt
if errorlevel 1 exit /b 1
call :one FrameLimiter %SRC%\Game\FrameLimiter.cpp %OUT%\FrameLimiter.o %PROOF%\m2-limiter.txt
if errorlevel 1 exit /b 1
call :one Window %SRC%\Core\Window.cpp %OUT%\Window.o %PROOF%\m2-window.txt
if errorlevel 1 exit /b 1
call :one Timer %SRC%\Game\Timer.cpp %OUT%\Timer.o %PROOF%\m2-timer.txt
if errorlevel 1 exit /b 1
call :one Misc %SRC%\Util\Misc.cpp %OUT%\Misc.o %PROOF%\m2-misc.txt
if errorlevel 1 exit /b 1
call :one SoundDS3D %SRC%\Sound\Sound_DS3D.cpp %OUT%\Sound_DS3D.o %PROOF%\m2-sound.txt
if errorlevel 1 exit /b 1
call :one SoundQueue %SRC%\Sound\Sound_Queue.cpp %OUT%\Sound_Queue.o %PROOF%\m2-queue.txt
if errorlevel 1 exit /b 1
call :one Reconnect %SRC%\Net\Reconnect.cpp %OUT%\Reconnect.o %PROOF%\m2-reconnect.txt
if errorlevel 1 exit /b 1
call :one Camera %SRC%\Render\Camera.cpp %OUT%\Camera.o %PROOF%\m2-camera.txt
if errorlevel 1 exit /b 1
call :one Camera3D %SRC%\Render\Camera3D.cpp %OUT%\Camera3D.o %PROOF%\m2-camera3d.txt
if errorlevel 1 exit /b 1
call :one InputWnd %SRC%\Input\Input_WndProc.cpp %OUT%\Input_WndProc.o %PROOF%\m2-inputwnd.txt
if errorlevel 1 exit /b 1
call :one Input %SRC%\Input\Input.cpp %OUT%\Input.o %PROOF%\m2-input.txt
if errorlevel 1 exit /b 1
call :one Menu %SRC%\UI\UI_InGameMenu.cpp %OUT%\UI_InGameMenu.o %PROOF%\m2-menu.txt
if errorlevel 1 exit /b 1
call :one ChatLog %SRC%\UI\Chat_Log.cpp %OUT%\Chat_Log.o %PROOF%\m2-chatlog.txt
if errorlevel 1 exit /b 1
echo COMPILE_OK
exit /b 0

:one
echo COMPILING %~1
emcc %FLAGS% %INC% %~2 -o %~3 > %~4 2>&1
if errorlevel 1 (
  echo FAIL %~1
  exit /b 1
)
exit /b 0
