workspace "GroovyEngine"
    configurations
    {
        "Debug",
        "Release"
    }

    platforms
    {
        "Win64"
    }

    startproject "Editor"

    defines { "PLATFORM_WIN32" }
    defines { "WITH_EDITOR" }

    filter "configurations:Debug"
        defines { "BUILD_DEBUG" }
    filter{}

include "Groovy"
include "Editor"
include "Sandbox"