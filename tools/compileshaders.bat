@echo off
set originaldir=%cd%
set shadersdir=..\src\shaders\
set compiler=glslc.exe

:: Cambia al directorio de shaders
cd /d %shadersdir%

:: Recorre todos los archivos .vert, .frag, .comp, etc., en el directorio
for %%f in (*.vert *.frag *.comp *.geom *.tesc *.tese) do (
    echo Compiling %%f
    %compiler% %%f -o %%~nf%%~xf.spv
)

echo Compilation complete!

cd /d %originaldir%
pause