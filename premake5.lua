newoption {
    trigger = "build-engine",
    description = "Builds the engine instead of using a prebuilt one"
}

newoption {
    trigger = "include-examples",
    description = "Includes all examples"
}
conan = {}
configs = { 'Debug', 'Release', 'RelWithDebInfo' }

for i = 1, 3 do
    include("build/deps/" .. configs[i] .. "/conanbuildinfo.premake.lua")
    conan[configs[i]] = {}
    local cfg = conan[configs[i]]
    cfg["build_type"] = conan_build_type
    cfg["arch"] = conan_arch
    cfg["includedirs"] = conan_includedirs
    cfg["libdirs"] = conan_libdirs
    cfg["bindirs"] = conan_bindirs
    cfg["libs"] = conan_libs
    cfg["system_libs"] = conan_system_libs
    cfg["defines"] = conan_defines
    cfg["cxxflags"] = conan_cxxflags
    cfg["cflags"] = conan_cflags
    cfg["sharedlinkflags"] = conan_sharedlinkflags
    cfg["exelinkflags"] = conan_exelinkflags
    cfg["frameworks"] = conan_frameworks
    cfg["imgui_bindings_path"] = conan_rootpath_imgui .. "/res/bindings/"
end

function conan_config_exec()
    configs = { 'Debug', 'Release', 'RelWithDebInfo' }
    for i = 1, 3 do
        local cfg = conan[configs[i]]
        filter("configurations:" .. configs[i])

        linkoptions { cfg["exelinkflags"] }
        includedirs { cfg["includedirs"] }
        libdirs { cfg["libdirs"] }
        links { cfg["libs"] }
        links { cfg["system_libs"] }
        links { cfg["frameworks"] }
        defines { cfg["defines"] }
        libdirs { cfg["libdirs"] }
        if not _OPTIONS["build-engine"] then 
            libdirs { "deps/LavaEngine/" .. configs[i] }
        end
        filter {}
    end
end

function conan_config_lib()
    configs = { 'Debug', 'Release', 'RelWithDebInfo' }
    for i = 1, 3 do
        local cfg = conan[configs[i]]
        filter("configurations:" .. configs[i])

        linkoptions { cfg["sharedlinkflags"] }
        includedirs { cfg["includedirs"], cfg["imgui_bindings_path"]}
        defines { cfg["defines"] }
        files {
            cfg["imgui_bindings_path"] .. "imgui_impl_vulkan.cpp",
            cfg["imgui_bindings_path"] .. "imgui_impl_glfw.cpp",
        }

        filter {}
    end
end

workspace "Lava"
    configurations { "Debug", "Release", "RelWithDebInfo" }
    architecture "x64"
    location "build"
    cppdialect "C++23"
    startproject "Window"

    function common_settings()
        removefiles { 
            "src/shaders/*.spv"
        }
        prebuildcommands {
            "..\\tools\\compileshaders.bat"
        }
    end

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
        runtime "Debug"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
        runtime "Release"

    filter "configurations:RelWithDebInfo"
        defines { "NDEBUG" }
        optimize "On"
        runtime "Release"
        symbols "On"
    filter {}

    if _OPTIONS["build-engine"] then 
        if os.isfile("build_engine.lua") then 
            include("build_engine.lua")
        end
    end

    project"Flycam"
        kind "ConsoleApp" -- This was WindowedApp
        language "C++"
        targetdir "build/%{prj.name}/%{cfg.buildcfg}"
        links { "LavaEngine" }
        includedirs "include"
        conan_config_exec("Debug")
        conan_config_exec("Release")
        conan_config_exec("RelWithDebInfo")
        pchheader "stdafx.hpp"
        pchsource "src/stdafx.cpp"
        forceincludes { "stdafx.hpp" }
        debugargs { _MAIN_SCRIPT_DIR .. "/examples/data" }
        files "examples/flycam_demostrator.cpp"
        files "src/shaders/*"
        files "src/stdafx.cpp"
        files "examples/assets/*"
        common_settings()

    if _OPTIONS["include-examples"] then
        project"Window"
            kind "ConsoleApp" -- This was WindowedApp
            language "C++"
            targetdir "build/%{prj.name}/%{cfg.buildcfg}"
            includedirs "include"
            links "LavaEngine"
            conan_config_exec("Debug")
            conan_config_exec("Release")
            conan_config_exec("RelWithDebInfo")
            debugargs { _MAIN_SCRIPT_DIR .. "/examples/data" }
            pchheader "stdafx.hpp"
            pchsource "src/stdafx.cpp"
            forceincludes { "stdafx.hpp" }
            debugargs { _MAIN_SCRIPT_DIR .. "/examples/data" }
            files "examples/window_demostrator.cpp"
            files "src/stdafx.cpp"
            common_settings()
        project"HelloTriangle"
            kind "ConsoleApp" -- This was WindowedApp
            language "C++"
            targetdir "build/%{prj.name}/%{cfg.buildcfg}"
            includedirs "include"
            links "LavaEngine"
            conan_config_exec("Debug")
            conan_config_exec("Release")
            conan_config_exec("RelWithDebInfo")
            debugargs { _MAIN_SCRIPT_DIR .. "/examples/data" }
            pchheader "stdafx.hpp"
            pchsource "src/stdafx.cpp"
            forceincludes { "stdafx.hpp" }
            debugargs { _MAIN_SCRIPT_DIR .. "/examples/data" }
            files "src/stdafx.cpp"
            files "examples/assets/*"
            files "examples/hellotriangle.cpp"
            common_settings()
        project"HelloTriangleWithInput"
            kind "ConsoleApp" -- This was WindowedApp
            language "C++"
            targetdir "build/%{prj.name}/%{cfg.buildcfg}"
            includedirs "include"
            links "LavaEngine"
            conan_config_exec("Debug")
            conan_config_exec("Release")
            conan_config_exec("RelWithDebInfo")
            debugargs { _MAIN_SCRIPT_DIR .. "/examples/data" }
            files "examples/hellotrianglewithinput.cpp"
            files "include/custom_vulkan_helpers.hpp"
            files "src/shaders/*"
        project"GLTFLoad"
            kind "ConsoleApp" -- This was WindowedApp
            language "C++"
            targetdir "build/%{prj.name}/%{cfg.buildcfg}"
            includedirs "include"
            links "LavaEngine"
            conan_config_exec("Debug")
            conan_config_exec("Release")
            conan_config_exec("RelWithDebInfo")
            debugargs { _MAIN_SCRIPT_DIR .. "/examples/data" }
            files "examples/gltf_loader.cpp"
            files "include/examples/gltf_loader.hpp"
            files "src/shaders/*"
            files "examples/assets/*"
        project"ECSRender"
            kind "ConsoleApp" -- This was WindowedApp
            language "C++"
            targetdir "build/%{prj.name}/%{cfg.buildcfg}"
            includedirs "include"
            links "LavaEngine"
            conan_config_exec("Debug")
            conan_config_exec("Release")
            conan_config_exec("RelWithDebInfo")
            debugargs { _MAIN_SCRIPT_DIR .. "/examples/data" }
            files "examples/ecs_render.cpp"
            files "include/examples/ecs_render.hpp"
            files "src/shaders/*"
            files "examples/assets/*"
            -- files "src/custom_vulkan_helpers.cpp"
        project"JobSystemDemostrator"
            kind "ConsoleApp" -- This was WindowedApp
            language "C++"
            targetdir "build/%{prj.name}/%{cfg.buildcfg}"
            includedirs "include"
            links "LavaEngine"
            conan_config_exec("Debug")
            conan_config_exec("Release")
            conan_config_exec("RelWithDebInfo")
            debugargs { _MAIN_SCRIPT_DIR .. "/examples/data" }
            files "examples/job_system_demostrator.cpp"
            files "src/shaders/*"
            files "examples/assets/*"
        project"Scripting"
            kind "ConsoleApp" -- This was WindowedApp
            language "C++"
            targetdir "build/%{prj.name}/%{cfg.buildcfg}"
            includedirs "include"
            links "LavaEngine"
            conan_config_exec("Debug")
            conan_config_exec("Release")
            conan_config_exec("RelWithDebInfo")
            debugargs { _MAIN_SCRIPT_DIR .. "/examples/data" }
            files "examples/scripting_demostrator.cpp"
            files "examples/scripts/*"
            files "src/shaders/*"
            files "examples/assets/*"
        project"PBR"
            kind "ConsoleApp" -- This was WindowedApp
            language "C++"
            targetdir "build/%{prj.name}/%{cfg.buildcfg}"
            includedirs "include"
            links "LavaEngine"
            conan_config_exec("Debug")
            conan_config_exec("Release")
            conan_config_exec("RelWithDebInfo")
            debugargs { _MAIN_SCRIPT_DIR .. "/examples/data" }
            files "examples/pbr_demostrator.cpp"
            files "src/shaders/*"
            files "examples/assets/*"
            common_settings()
        project"Shadows"
            kind "ConsoleApp" -- This was WindowedApp
            language "C++"
            targetdir "build/%{prj.name}/%{cfg.buildcfg}"
            includedirs "include"
            links "LavaEngine"
            conan_config_exec("Debug")
            conan_config_exec("Release")
            conan_config_exec("RelWithDebInfo")
            pchheader "stdafx.hpp"
            pchsource "src/stdafx.cpp"
            forceincludes { "stdafx.hpp" }
            debugargs { _MAIN_SCRIPT_DIR .. "/examples/data" }
            files "examples/shadows_demostrator.cpp"
            files "src/shaders/*"
            files "src/stdafx.cpp"
            files "examples/assets/*"
            common_settings()
    end