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
        "GLFW_INCLUDE_NONE",
        "SFML_STATIC"
    }

    includedirs{
        "%{prj.name}/src",
        "%{prj.name}/dependencies/GLM",
        "%{prj.name}/dependencies/imgui",
        "%{prj.name}/dependencies/GLAD/include",
        "%{prj.name}/dependencies/GLFW/include",
        "%{prj.name}/dependencies/SFML/include"
    }

    libdirs{
        "%{prj.name}/dependencies/SFML/lib"
    }

    links{
        "opengl32.lib",
        "winmm.lib",
        "ImGui",
        "Glad",
        "GLFW"
    }

    filter "configurations:Debug"
        defines {
            "MIND_DEBUG",
            "MIND_ENABLE_ASSERT"
        }

        links{
            "sfml-system-s-d.lib",
            "sfml-window-s-d.lib"
        }

        runtime "Debug"
        symbols "On"
    filter "configurations:Release"
        defines "MIND_RELEASE"

        links{
            "sfml-system-s.lib",
            "sfml-window-s.lib"
        }

        runtime "Release"
        optimize "On"