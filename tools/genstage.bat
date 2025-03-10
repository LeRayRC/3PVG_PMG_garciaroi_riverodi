@echo off
setlocal

::generate dependencies
echo Generate dependencies...
call gendeps.bat

:: Move to upper folder
cd /d "%~dp0.."


::generate solution
.\tools\premake5.exe vs2022 --build-engine

::Compiling library
echo Compiling library...
call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild" build\Lava.sln /t:LavaEngine /p:Configuration=Debug /p:Platform=x64
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild" build\Lava.sln /t:LavaEngine /p:Configuration=Release /p:Platform=x64
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild" build\Lava.sln /t:LavaEngine /p:Configuration=RelWithDebInfo /p:Platform=x64

:: Define local variables
set STAGE_DIR=stage
set DEST_ZIP=LavaEngine.zip
set BUILD_DIR=build
set DEPS_DIR=%STAGE_DIR%\deps\LavaEngine
set TOOLS_DIR=tools
set INCLUDE_DIR=include
set SRC_DIR=src
set EXAMPLES_DIR=examples
set DOC_DIR=doc

:: 1. Crear la carpeta stage
echo Create folder %STAGE_DIR%...
mkdir %STAGE_DIR% 2>nul

:: 2. Create folder deps and copy library files
echo Copy .lib files...
mkdir %DEPS_DIR%\Debug 2>nul
mkdir %DEPS_DIR%\Release 2>nul
mkdir %DEPS_DIR%\RelWithDebInfo 2>nul
copy /Y %BUILD_DIR%\Debug\LavaEngine.lib %DEPS_DIR%\Debug\
copy /Y %BUILD_DIR%\Release\LavaEngine.lib %DEPS_DIR%\Release\
copy /Y %BUILD_DIR%\RelWithDebInfo\LavaEngine.lib %DEPS_DIR%\RelWithDebInfo\

:: 3. Copy tools folder
echo Copying tools...
xcopy /E /I /Y %TOOLS_DIR% %STAGE_DIR%\tools
del /F /Q %STAGE_DIR%\tools\genstage.bat

:: 4. Copy .lua files
echo Copy .lua files...
copy /Y *.lua %STAGE_DIR%\
del /F /Q %STAGE_DIR%\build_engine.lua
del /F /Q %STAGE_DIR%\all_examples.lua

:: 5. Copy header files
echo Copy include...
xcopy /E /I /Y %INCLUDE_DIR% %STAGE_DIR%\include

mkdir %STAGE_DIR%\%SRC_DIR% 2>nul
xcopy /E /I /Y %SRC_DIR%\build %STAGE_DIR%\%SRC_DIR%\build
xcopy /E /I /Y %SRC_DIR%\shaders %STAGE_DIR%\%SRC_DIR%\shaders
xcopy  /I /Y %SRC_DIR%\stdafx.cpp %STAGE_DIR%\%SRC_DIR%\
xcopy /E /I /Y %EXAMPLES_DIR%\assets %STAGE_DIR%\%EXAMPLES_DIR%\assets
xcopy  /I /Y %EXAMPLES_DIR%\flycam_demostrator.cpp %STAGE_DIR%\%EXAMPLES_DIR%\
xcopy  /I /Y %DOC_DIR%\LavaEngineDocumentation.url %STAGE_DIR%\


powershell -Command "Compress-Archive -Path %STAGE_DIR%\* -DestinationPath %DEST_ZIP% -Force"
rmdir /s /q %STAGE_DIR%


echo Process completed
endlocal
