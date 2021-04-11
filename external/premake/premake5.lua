workspace "MindEngine"
    architecture "x64"

    configurations{
        "Debug",
        "Release",
        "Dist"
    }
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "MindEngine"
    location "MindEngine"
    kind "SharedLib"
    language "C++"

    targetdir ("bin/" ..outputdir.. "/%(prj.name)")
    targetdir ("bin_int/" ..outputdir.. "/%(prj.name)")

    files{
        "%{prj.name/src/**.h}",
        "%{prj.name/src/**.cpp}",
    }

    includedirs{
        "%{prj.name}/dependencies/spd-1.8.2/include"
    }

    filter "system:windows"
        cppdialect "C++17"
        staticruntime "On"
        systemversion "latest"

        defines{
            "MIND_PLATFORM_WINDOWS",
            "MIND_BUILD_DLL"
        }

        postbuildcommands{
            ("{COPY} %{cfg.buildtarget.relpath} ../bin/" ..outputdir.. "/Sandbox")
        }
    filter "configurations:Debug"
        defines "MIND_DEBUG"
        symbols "On"
    filter "configurations:Release"
        defines "MIND_Release"
        optimize "On"
    filter "configurations:Dist"
        defines "MIND_Dist"
        optimize "On"
project "Sandbox"
    location "Sandbox"
    kind "ConsoleApp"
    language "C++"

    targetdir ("bin/" ..outputdir.. "/%(prj.name)")
    targetdir ("bin_int/" ..outputdir.. "/%(prj.name)")

    files{
        "%{prj.name/src/**.h}",
        "%{prj.name/src/**.cpp}",
    }

    includedirs{
        "%{prj.name}/dependencies/spd-1.8.2/include",
        "Mind/src"
    }

    links{
        "MindEngine"
    }

    filter "system:windows"
        cppdialect "C++17"
        staticruntime "On"
        systemversion "latest"

        defines{
            "MIND_PLATFORM_WINDOWS"
        }
    filter "configurations:Debug"
        defines "MIND_DEBUG"
        symbols "On"
    filter "configurations:Release"
        defines "MIND_RELEASE"
        optimize "On"
    filter "configurations:Dist"
        defines "MIND_DIST"
        optimize "On"