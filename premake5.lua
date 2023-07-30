workspace "GroovyEngine"
    configurations
    {
        "Debug_Editor",
        "Development_Editor",
        "Debug_Game",
        "Shipping"
    }

    platforms
    {
        "Win64"
    }

    startproject "Editor"

    defines "PLATFORM_WIN32"

    rtti "Off"

    filter "configurations:Debug_Editor or Development_Editor"
        defines "WITH_EDITOR"
    filter {}

    filter "configurations:Debug_Editor or Debug_Game"
        defines "BUILD_DEBUG"
    filter {}

    filter "configurations:Shipping"
        defines "BUILD_SHIPPING"
    filter {}

include "Groovy"
include "Editor"
include "Sandbox"
include "Game"