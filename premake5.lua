workspace "GameboyEmulator"
    architecture "x64"

    configurations{
        "Debug",
        "Release"
    }
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "GameboyEmulator/dependencies/imgui"
include "GameboyEmulator/dependencies/GLAD"
include "GameboyEmulator/dependencies/GLFW"

project "GameboyEmulator"
location "GameboyEmulator"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "On"

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
        "GLFW_INCLUDE_NONE"
    }

    includedirs{
        "%{prj.name}/src",
        "%{prj.name}/dependencies/GLM",
        "%{prj.name}/dependencies/imgui",
        "%{prj.name}/dependencies/GLAD/include",
        "%{prj.name}/dependencies/GLFW/include"
    }

    links{
        "opengl32.lib",
        "ImGui",
        "Glad",
        "GLFW"
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