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

workspace "Motor"
    configurations { "Debug", "Release", "RelWithDebInfo" }
    architecture "x64"
    location "build"
    cppdialect "C++23"
    startproject "Window"

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
    project "Motor"
        kind "StaticLib"
        targetdir "build/%{cfg.buildcfg}"
        includedirs "include"
        conan_config_lib()
        pchheader "stdafx.hpp"
        pchsource "src/stdafx.cpp"
        forceincludes { "stdafx.hpp" }
        files {
                "premake5.lua",
                "src/build/conanfile.txt",
                "src/build/conan.lua",
                "src/*.cpp",
                "include/*.hpp",
                "include/*/*.hpp",
                "src/*/*.cpp",
                }
    project"Window"
        kind "ConsoleApp" -- This was WindowedApp
        language "C++"
        targetdir "build/%{prj.name}/%{cfg.buildcfg}"
        includedirs "include"
        links "Motor"
        conan_config_exec("Debug")
        conan_config_exec("Release")
        conan_config_exec("RelWithDebInfo")
        debugargs { _MAIN_SCRIPT_DIR .. "/examples/data" }
        files "examples/window.cpp"
    project"HelloTriangle"
        kind "ConsoleApp" -- This was WindowedApp
        language "C++"
        targetdir "build/%{prj.name}/%{cfg.buildcfg}"
        includedirs "include"
        links "Motor"
        conan_config_exec("Debug")
        conan_config_exec("Release")
        conan_config_exec("RelWithDebInfo")
        debugargs { _MAIN_SCRIPT_DIR .. "/examples/data" }
        files "examples/hellotriangle.cpp"
        files "include/examples/hellotriangle.hpp"
        files "include/custom_vulkan_helpers.hpp"
        files "src/shaders/*"
    project"HelloTriangleWithInput"
        kind "ConsoleApp" -- This was WindowedApp
        language "C++"
        targetdir "build/%{prj.name}/%{cfg.buildcfg}"
        includedirs "include"
        links "Motor"
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
        links "Motor"
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
        links "Motor"
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
        links "Motor"
        conan_config_exec("Debug")
        conan_config_exec("Release")
        conan_config_exec("RelWithDebInfo")
        debugargs { _MAIN_SCRIPT_DIR .. "/examples/data" }
        files "examples/job_system_demostrator.cpp"
        files "src/shaders/*"
        files "examples/assets/*"