@echo off
setlocal enabledelayedexpansion

set originaldir=%cd%
set shadersdir=..\src\shaders\
set compiler=glslc.exe

echo Buscando y compilando shaders en %shadersdir% y subdirectorios...

:: Cambia al directorio de shaders
cd /d %shadersdir%

:: Recorre recursivamente todos los archivos .vert, .frag, etc.
for /r %%f in (*.vert *.frag *.comp *.geom *.tesc *.tese) do (
    echo Compilando %%f
    %compiler% "%%f" -o "%%~dpnf%%~xf.spv"
    if errorlevel 1 (
        echo Error al compilar %%f
    ) else (
        echo Compilado exitosamente: %%~nxf.spv
    )
)

echo Compilacion completada!

cd /d %originaldir%
pause