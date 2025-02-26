    project "LavaEngine"
        kind "StaticLib"
        targetdir "build/%{cfg.buildcfg}"
        includedirs {"include", "src"}
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
                "include/*/*/*.hpp",
                "src/*/*.cpp",
                "src/*/*.hpp",
                "src/*.hpp",
                }