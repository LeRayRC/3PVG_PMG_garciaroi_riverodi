@echo off
SETLOCAL EnableDelayedExpansion

echo =======================================================
echo Compilando OpenXR SDK para Debug y Release
echo =======================================================

REM Crea directorios para construir OpenXR
IF NOT EXIST build_openxr mkdir build_openxr
cd build_openxr

REM Configura con CMake
echo Configurando CMake para OpenXR...
cmake -S .. -B . -DCMAKE_CONFIGURATION_TYPES="Debug;Release;RelWithDebInfo"

REM Compila Debug
echo.
echo Compilando OpenXR (Debug)...
cmake --build . --config Debug

REM Compila Release
echo.
echo Compilando OpenXR (Release)...
cmake --build . --config Release

REM Compila RelWithDebInfo
echo.
echo Compilando OpenXR (RelWithDebInfo)...
cmake --build . --config RelWithDebInfo

REM Vuelve al directorio principal
cd ..

REM Crea directorios de destino para el SDK
IF NOT EXIST deps\OpenXR mkdir deps\OpenXR
IF NOT EXIST deps\OpenXR\include mkdir deps\OpenXR\include
IF NOT EXIST deps\OpenXR\include\openxr mkdir deps\OpenXR\include\openxr
IF NOT EXIST deps\OpenXR\lib mkdir deps\OpenXR\lib
IF NOT EXIST deps\OpenXR\lib\Debug mkdir deps\OpenXR\lib\Debug
IF NOT EXIST deps\OpenXR\lib\Release mkdir deps\OpenXR\lib\Release
IF NOT EXIST deps\OpenXR\lib\RelWithDebInfo mkdir deps\OpenXR\lib\RelWithDebInfo

REM Copia las cabeceras
echo.
echo Copiando cabeceras de OpenXR...
xcopy /s /y build_openxr\openxr\include\openxr\*.* deps\OpenXR\include\openxr\

REM Copia las librerías
echo.
echo Copiando librerías de OpenXR...
copy build_openxr\_deps\openxr-build\src\loader\Debug\*.lib deps\OpenXR\lib\Debug\
copy build_openxr\_deps\openxr-build\src\loader\Release\*.lib deps\OpenXR\lib\Release\
copy build_openxr\_deps\openxr-build\src\loader\RelWithDebInfo\*.lib deps\OpenXR\lib\RelWithDebInfo\

build_openxr\_deps\openxr-build\src\loader

REM Copia los DLLs
copy build_openxr\openxr\loader\Debug\*.dll deps\OpenXR\lib\Debug\
copy build_openxr\openxr\loader\Release\*.dll deps\OpenXR\lib\Release\
copy build_openxr\openxr\loader\RelWithDebInfo\*.dll deps\OpenXR\lib\RelWithDebInfo\

echo.
echo =======================================================
echo Compilación y configuración de OpenXR completada
echo Las cabeceras y librerías están en deps\OpenXR
echo =======================================================

ENDLOCAL
