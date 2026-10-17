@echo off
call C:\Users\andvolodko\emsdk\emsdk_env.bat >nul
set "ROOT=c:\Work\_Projects\mu_online_game"
set "SRC=%ROOT%\reference\Mu-97k-Client-Source\src"
set "OUT=%ROOT%\emscripten_port\out\all"
set INC=-I %ROOT%\emscripten_port\browser -I %ROOT%\emscripten_port\browser\compat -I %SRC% -I %ROOT%\reference\Mu-97k-Client-Source\lib\libjpeg\jpeg-6b -I %ROOT%\reference\Mu-97k-Client-Source\lib\miniaudio
emcc -c -std=c++17 -ferror-limit=20 -Wno-macro-redefined -Wno-ignored-attributes -Wno-deprecated-non-prototype -Wno-c++11-narrowing %INC% %SRC%\globals.cpp -o %OUT%\src_globals.o
if errorlevel 1 exit /b 1
emcc -c -std=c++17 -ferror-limit=20 -Wno-macro-redefined -Wno-ignored-attributes -Wno-deprecated-non-prototype -Wno-c++11-narrowing %INC% %SRC%\Game\Game_SceneUpdate.cpp -o %OUT%\src_Game_Game_SceneUpdate.o
if errorlevel 1 exit /b 1
emcc -c -std=c++17 -ferror-limit=20 -Wno-macro-redefined -Wno-ignored-attributes -Wno-deprecated-non-prototype -Wno-c++11-narrowing %INC% %SRC%\Scene\Scene_ServerSelect_Input.cpp -o %OUT%\src_Scene_Scene_ServerSelect_Input.o
if errorlevel 1 exit /b 1
emcc -c -std=c++17 -ferror-limit=20 -Wno-macro-redefined -Wno-ignored-attributes -Wno-deprecated-non-prototype -Wno-c++11-narrowing %INC% %SRC%\Scene\Scene_Login.cpp -o %OUT%\src_Scene_Scene_Login.o
if errorlevel 1 exit /b 1
echo COMPILE_OK
call %ROOT%\emscripten_port\link-login.bat
