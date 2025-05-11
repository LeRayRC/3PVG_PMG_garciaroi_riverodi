cmake_minimum_required(VERSION 3.22.1)
project(openxr-build)

# Para FetchContent_Declare() y FetchContent_MakeAvailable()
include(FetchContent)

# Configuración de OpenXR - Desactiva tests y activa API Layers
set(BUILD_TESTS OFF CACHE INTERNAL "Build tests")
set(BUILD_API_LAYERS ON CACHE INTERNAL "Use OpenXR layers")

# Descarga el SDK de OpenXR
FetchContent_Declare(
    OpenXR
    URL_HASH MD5=924a94a2da0b5ef8e82154c623d88644
    URL https://github.com/KhronosGroup/OpenXR-SDK-Source/archive/refs/tags/release-1.0.34.zip
    SOURCE_DIR openxr
)
FetchContent_MakeAvailable(OpenXR)

# Opcional: imprime información sobre la ubicación del SDK
message(STATUS "OpenXR SDK ubicado en: ${openxr_SOURCE_DIR}")