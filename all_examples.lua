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

project"ECS"
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
        files "examples/ecs_render.cpp"
        files "src/shaders/*"
        files "src/shaders/*/*"
        files "src/shaders/*/*/*"
        files "src/stdafx.cpp"
        files "examples/assets/*"
        common_settings()
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
project"Gaussian"
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
	files "examples/gaussian_splatting_demostrator.cpp"
	files "src/shaders/*"
    files "src/shaders/*/*"
	files "src/stdafx.cpp"
	files "examples/assets/*"
	common_settings()