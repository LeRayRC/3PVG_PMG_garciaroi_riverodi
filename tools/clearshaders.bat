@echo off

:: Guardar el directorio actual
set originaldir=%cd%

:: Establecer el directorio donde están los shaders
set shadersdir=..\src\shaders\

:: Cambia al directorio de shaders
cd /d %shadersdir%

:: Eliminar todos los archivos .spv en el directorio actual
del /q *.spv

:: Volver al directorio original
cd /d %originaldir%

echo All .spv files have been deleted and returned to the original directory.
pause
