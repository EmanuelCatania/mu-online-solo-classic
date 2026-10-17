@echo off
setlocal EnableDelayedExpansion
call C:\Users\andvolodko\emsdk\emsdk_env.bat >nul
set "ROOT=c:\Work\_Projects\mu_online_game"
set "TREE=%ROOT%\reference\Mu-97k-Client-Source"
set "OUT=%ROOT%\emscripten_port\out\all"
set INC=-I %ROOT%\emscripten_port\browser -I %ROOT%\emscripten_port\browser\compat -I %TREE%\src -I %TREE%\lib\libjpeg\jpeg-6b -I %TREE%\lib\miniaudio
set CXXFLAGS=-c -std=c++17 -ferror-limit=20 -Wno-macro-redefined -Wno-ignored-attributes -Wno-deprecated-non-prototype -Wno-c++11-narrowing
del "%OUT%\src_UI_UI_LegacyExterns.o" "%OUT%\src_Scene_Scene_Intro.o" "%OUT%\src_Local_Text_Data.o" "%OUT%\src_Core_Window.o"
emcc %CXXFLAGS% %INC% "%TREE%\src\UI\UI_LegacyExterns.cpp" -o "%OUT%\src_UI_UI_LegacyExterns.o"
if errorlevel 1 exit /b 1
emcc %CXXFLAGS% %INC% "%TREE%\src\Scene\Scene_Intro.cpp" -o "%OUT%\src_Scene_Scene_Intro.o"
if errorlevel 1 exit /b 1
emcc %CXXFLAGS% %INC% "%TREE%\src\Render\Font_LegacyFactory.cpp" -o "%OUT%\src_Render_Font_LegacyFactory.o"
if errorlevel 1 exit /b 1
emcc %CXXFLAGS% %INC% "%TREE%\src\Item\Item_Data.cpp" -o "%OUT%\src_Item_Item_Data.o"
if errorlevel 1 exit /b 1
emcc %CXXFLAGS% %INC% "%TREE%\src\Item\Skill_Data.cpp" -o "%OUT%\src_Item_Skill_Data.o"
if errorlevel 1 exit /b 1
emcc %CXXFLAGS% %INC% "%TREE%\src\Item\Quest_Data.cpp" -o "%OUT%\src_Item_Quest_Data.o"
if errorlevel 1 exit /b 1
emcc %CXXFLAGS% %INC% "%TREE%\src\Item\Gate_Data.cpp" -o "%OUT%\src_Item_Gate_Data.o"
if errorlevel 1 exit /b 1
emcc %CXXFLAGS% %INC% "%TREE%\src\Item\Dialog_Data.cpp" -o "%OUT%\src_Item_Dialog_Data.o"
if errorlevel 1 exit /b 1
emcc %CXXFLAGS% %INC% "%TREE%\src\Local\Text_Data.cpp" -o "%OUT%\src_Local_Text_Data.o"
if errorlevel 1 exit /b 1
emcc %CXXFLAGS% %INC% "%TREE%\src\Core\Window.cpp" -o "%OUT%\src_Core_Window.o"
if errorlevel 1 exit /b 1
echo COMPILE_OK
exit /b 0
