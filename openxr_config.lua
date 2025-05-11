-- OpenXR Configuration
-- Este script configura las rutas a las cabeceras y librerías de OpenXR precompiladas

-- Opciones para OpenXR
newoption {
    trigger = "openxr-sdk-path",
    description = "Ruta al SDK de OpenXR precompilado",
    default = "deps/OpenXR"
}

newoption {
    trigger = "xr-runtime-json",
    description = "Ubicación opcional de un archivo de configuración específico del runtime de OpenXR",
    default = ""
}

-- Configuración de OpenXR para el proyecto
function setup_openxr()
    -- Define rutas de OpenXR basadas en la ruta del SDK proporcionada
    local openxr_sdk_path = _OPTIONS["openxr-sdk-path"]
    
    -- Verifica si el SDK de OpenXR existe
    if not os.isdir(openxr_sdk_path) then
        print("Warning: El SDK de OpenXR no se encuentra en " .. openxr_sdk_path)
        print("Ejecuta primero build_openxr.bat para compilar OpenXR")
    end
    
    -- Ruta de las cabeceras
    local openxr_include_path = path.join(openxr_sdk_path, "include")
    
    -- Rutas de bibliotecas para diferentes configuraciones
    local openxr_lib_paths = {
        Debug = path.join(openxr_sdk_path, "lib/Debug"),
        Release = path.join(openxr_sdk_path, "lib/Release"),
        RelWithDebInfo = path.join(openxr_sdk_path, "lib/RelWithDebInfo")
    }
    
    -- Añade directorios de inclusión para OpenXR
    includedirs { openxr_include_path }
    
    -- Configura rutas de bibliotecas y enlaces basados en la configuración
    filter "configurations:Debug"
        libdirs { openxr_lib_paths.Debug }
        links { "openxr_loaderd" }
        
    filter "configurations:Release"
        libdirs { openxr_lib_paths.Release }
        links { "openxr_loader" }
        
    filter "configurations:RelWithDebInfo"
        libdirs { openxr_lib_paths.RelWithDebInfo }
        links { "openxr_loader" }
        
    filter {}
    
    -- Define definiciones específicas de OpenXR
    defines { "XR_USE_GRAPHICS_API_VULKAN" }
    
    -- Añade la ruta del runtime JSON si se especifica
    if _OPTIONS["xr-runtime-json"] ~= "" then
        defines { "XR_RUNTIME_JSON=\"" .. _OPTIONS["xr-runtime-json"] .. "\"" }
        debugenvs { "XR_RUNTIME_JSON=" .. _OPTIONS["xr-runtime-json"] }
    end
end

function setup_openxr_for_project()
    -- Aplica la configuración de OpenXR para el proyecto actual
    setup_openxr()
end