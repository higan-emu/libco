-- https://github.com/higan-emu/libco
-- License: https://github.com/higan-emu/libco/blob/master/LICENSE
-- author: beaumanvienna (JC), 2021
-- premake script to build examples for gcc/clang/MSVC
--
-- for gcc/clang run
--      premake5.exe gmake2 && make verbose=1 config=debug|release
-- for VS run
--      premake5.exe vs2019

workspace "examples"
    architecture "x86_64"
    startproject "timing-test"
    configurations 
    { 
        "Debug", 
        "Release" 
    }

project "timing-test"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    targetdir "bin/%{cfg.buildcfg}"

    files 
    { 
        "../../libco.c", 
        "test_timing.cpp"
    }

    includedirs 
    { 
        "../.."
    }

    flags
    {
        "MultiProcessorCompile"
    }

    filter { "action:gmake*" }
        buildoptions { "-fomit-frame-pointer"}
    
    filter { "system:windows", "action:gmake*" }
        buildoptions { "-fdiagnostics-color=always" }

    filter { "system:windows", "action:vs*" }
        defines
        {
            "LIBCO_MPROTECT"
        }
        
    filter { "configurations:Debug" }
        defines { "DEBUG" }
        symbols "On"

    filter { "configurations:Release" }
        defines { "NDEBUG" }
        optimize "On"

project "argument-test"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    targetdir "bin/%{cfg.buildcfg}"

    files 
    { 
        "../../libco.c", 
        "test_args.cpp"
    }

    includedirs 
    { 
        "../.."
    }

    flags
    {
        "MultiProcessorCompile"
    }

    filter { "action:gmake*" }
        buildoptions { "-fomit-frame-pointer"}

    filter { "system:windows", "action:gmake*" }
        buildoptions { "-fdiagnostics-color=always" }

    filter { "system:windows", "action:vs*" }
        defines
        {
            "LIBCO_MPROTECT"
        }
        
    filter { "configurations:Debug" }
        defines { "DEBUG" }
        symbols "On"

    filter { "configurations:Release" }
        defines { "NDEBUG" }
        optimize "On"

project "serialization-test"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    targetdir "bin/%{cfg.buildcfg}"

    files 
    { 
        "../../libco.c", 
        "test_serialization.cpp"
    }

    includedirs 
    { 
        "../.."
    }

    flags
    {
        "MultiProcessorCompile"
    }

    filter { "action:gmake*" }
        buildoptions { "-fomit-frame-pointer"}
    
    filter { "system:windows", "action:gmake*" }
        buildoptions { "-fdiagnostics-color=always" }

    filter { "system:windows", "action:vs*" }
        defines
        {
            "LIBCO_MPROTECT"
        }
        
    filter { "configurations:Debug" }
        defines { "DEBUG" }
        symbols "On"

    filter { "configurations:Release" }
        defines { "NDEBUG" }
        optimize "On"
