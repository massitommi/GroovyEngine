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

    startproject "Groovy"

    defines { "PLATFORM_WIN32" }

    filter "configurations:Debug"
        defines { "BUILD_DEBUG" }
    filter{}

include "Groovy"
include "Editor"
include "Sandbox"