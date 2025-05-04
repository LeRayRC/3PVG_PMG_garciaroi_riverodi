    project "LavaEngine"
        kind "StaticLib"
        targetdir "build/%{cfg.buildcfg}"
        includedirs {"include", "src", "deps/OpenXR/include", "include/lava/openxr" , "include/lava/openxr_common"}
        conan_config_lib()
        pchheader "stdafx.hpp"
        pchsource "src/stdafx.cpp"
        forceincludes { "stdafx.hpp" }
        setup_openxr_for_project()
        files {
                "premake5.lua",
                "src/build/conanfile.txt",
                "src/build/conan.lua",
                "src/*.cpp",
                "include/*.hpp",
                "include/*/*/*.hpp",
                "src/*/*.cpp",
                "src/*/*.hpp",
                "src/*.hpp",
                }
        