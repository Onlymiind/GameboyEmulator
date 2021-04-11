workspace "GameboyEmulator"
    architecture "x86"

    configurations{
        "Debug",
        "Release"
    }
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

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
        "&{prj.name}/dependencies/GLM/**.hpp",
        "&{prj.name}/dependencies/GLM/**.inl"
    }

    defines{
        "_CRT_SECURE_NO_WARNINGS",
        "SFML_STATIC"
    }

    includedirs{
        "%{prj.name}/src",
        "%{prj.name}/dependencies/SFML/include",
        "%{prj.name}/dependencies/GLM"
    }

    libdirs{
        "%{prj.name}/dependencies/SFML/lib"
    }

    links{
        "opengl32.lib",
        "winmm.lib",
        "gdi32.lib",
        "flac.lib",
        "ogg.lib",
        "vorbis.lib",
        "vorbisfile.lib",
        "vorbisenc.lib",
        "freetype.lib",
        "ws2_32.lib"
    }

    filter "configurations:Debug"
        defines {
            "MIND_DEBUG",
            "MIND_ENABLE_ASSERT"
        }

        links{
            "sfml-graphics-s-d.lib",
            "sfml-window-s-d.lib",
            "sfml-system-s-d.lib",
            "sfml-audio-s-d.lib"
        }

        runtime "Debug"
        symbols "On"
    filter "configurations:Release"
        defines "MIND_RELEASE"
        
        links{
            "sfml-graphics-s-d.lib",
            "sfml-window-s-d.lib",
            "sfml-system-s-d.lib",
            "sfml-audio-s-d.lib"
        }

        runtime "Release"
        optimize "On"