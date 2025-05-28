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
    pchheader "stdafx.hpp"
    pchsource "src/stdafx.cpp"
    forceincludes { "stdafx.hpp" }
    files "src/stdafx.cpp"
    files "examples/assets/*"
    files "examples/hellotrianglewithinput.cpp"
    common_settings()
project "GLTFLoad"
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
    files "examples/gltf_loader.cpp"
    files "src/shaders/*"
    files "src/shaders/*/*"
    files "src/shaders/*/*/*"
    files "src/stdafx.cpp"
    files "examples/assets/*"
    common_settings()
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
project"JobSystem"
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
    files "src/stdafx.cpp"
    files "examples/assets/*"
    files "examples/job_system_demostrator.cpp"
    common_settings()
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
    pchheader "stdafx.hpp"
    pchsource "src/stdafx.cpp"
    forceincludes { "stdafx.hpp" }
    files "src/stdafx.cpp"
    files "examples/assets/*"
    files "examples/scripting_demostrator.cpp"
    common_settings()
project"Lights"
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
    files "examples/lights_demostrator.cpp"
    files "src/shaders/*"
    files "src/shaders/*/*"
    files "src/shaders/*/*/*"
    files "src/stdafx.cpp"
    files "examples/assets/*"
    common_settings()
project"PBR"
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
    files "examples/pbr_demostrator.cpp"
    files "src/shaders/*"
    files "src/shaders/*/*"
    files "src/stdafx.cpp"
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
project"CartoonRender"
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
    files "examples/cartoon_demostrator.cpp"
    files "src/shaders/*"
    files "src/shaders/*/*"
    files "src/stdafx.cpp"
    files "examples/assets/*"
    common_settings()