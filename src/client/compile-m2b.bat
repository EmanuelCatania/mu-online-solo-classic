@echo off
call C:\Users\andvolodko\emsdk\emsdk_env.bat >nul
set "ROOT=c:\Work\_Projects\mu_online_game"
set "SRC=%ROOT%\reference\Mu-97k-Client-Source\src"
set "OUT=%ROOT%\emscripten_port\out"
set "PROOF=%ROOT%\emscripten_port\proof"
set INC=-I %ROOT%\emscripten_port\browser -I %ROOT%\emscripten_port\browser\compat -I %SRC% -I %ROOT%\reference\Mu-97k-Client-Source\lib\libjpeg\jpeg-6b -I %ROOT%\reference\Mu-97k-Client-Source\lib\miniaudio
set FLAGS=-c -std=c++17 -ferror-limit=8 -Wno-macro-redefined -Wno-ignored-attributes -Wno-deprecated-non-prototype -Wno-c++11-narrowing
set FAIL=0

call :one SystemLegacy %SRC%\Core\System_Legacy.cpp %OUT%\System_Legacy.o %PROOF%\m2-syslegacy.txt
call :one SceneUpdate %SRC%\Game\Game_SceneUpdate.cpp %OUT%\Game_SceneUpdate.o %PROOF%\m2-sceneupdate.txt
call :one EnterWorld %SRC%\Game\Game_EnterWorldTick.cpp %OUT%\Game_EnterWorldTick.o %PROOF%\m2-enterworld.txt
call :one CharTick %SRC%\Game\Game_CharSelectTick.cpp %OUT%\Game_CharSelectTick.o %PROOF%\m2-chartick.txt
call :one EntityInit %SRC%\Entity\Entity_Init.cpp %OUT%\Entity_Init.o %PROOF%\m2-entityinit.txt
call :one SceneChar %SRC%\Scene\Scene_CharSelect.cpp %OUT%\Scene_CharSelect.o %PROOF%\m2-scenechar.txt
call :one Crypto %SRC%\Net\Crypto.cpp %OUT%\Crypto.o %PROOF%\m2-crypto.txt
call :one SceneLogin %SRC%\Scene\Scene_Login.cpp %OUT%\Scene_Login.o %PROOF%\m2-scenelogin.txt
call :one RenderFrame %SRC%\Render\Render_Frame.cpp %OUT%\Render_Frame.o %PROOF%\m2-renderframe.txt
call :one MapManager %SRC%\Game\MapManager.cpp %OUT%\MapManager.o %PROOF%\m2-map.txt
if %FAIL%==0 echo COMPILE_OK
exit /b %FAIL%

:one
echo COMPILING %~1
emcc %FLAGS% %INC% %~2 -o %~3 > %~4 2>&1
if errorlevel 1 (
  echo FAIL %~1
  set FAIL=1
)
exit /b 0
