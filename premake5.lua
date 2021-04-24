workspace "GameboyEmulator"
    architecture "x64"

    configurations{
        "Debug",
        "Release"
    }
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "GameboyEmulator/dependencies/imgui"
include "GameboyEmulator/dependencies/GLAD"

project "GameboyEmulator"
location "GameboyEmulator"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "Off"

    targetdir ("bin/" ..outputdir.. "/%{prj.name}")
    objdir ("bin_int/" ..outputdir.. "/%{prj.name}")

    files{
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/dependencies/GLM/**.hpp",
        "%{prj.name}/dependencies/GLM/**.inl",
    }

    defines{
        "_CRT_SECURE_NO_WARNINGS",
    }

    includedirs{
        "%{prj.name}/src",
        "%{prj.name}/dependencies/GLM",
        "%{prj.name}/dependencies/imgui",
        "%{prj.name}/dependencies/GLAD/include",
        "%{prj.name}/dependencies/SDL2/include"
    }

    libdirs{
        "%{prj.name}/dependencies/SDL2/lib"
    }

    links{
        "opengl32.lib",
        "winmm.lib",
        "Setupapi.lib",
        "Imm32.lib",
        "Version.lib",
        "ImGui",
        "Glad",
        "SDL2-static.lib"
    }

    filter "configurations:Debug"
        defines {
            "MIND_DEBUG",
            "MIND_ENABLE_ASSERT"
        }

        runtime "Debug"
        symbols "On"
    filter "configurations:Release"
        defines "MIND_RELEASE"

        runtime "Release"
        optimize "On"