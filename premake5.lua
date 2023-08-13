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
        debugargs "test/TestProject/TestProj.groovyproj"
    filter {}

    filter "configurations:Debug_Game or Shipping"
        debugargs "../Editor/test/TestProject/TestProj.groovyproj"
    filter {}

    filter "configurations:Debug_Editor or Development_Editor"
        defines "WITH_EDITOR"
    filter {}

    filter "configurations:Debug_Editor or Debug_Game"
        defines "BUILD_DEBUG"
    filter {}

    filter "configurations:Development_Editor"
        optimize "Speed"
    filter {}

    filter "configurations:Shipping"
        optimize "Speed"
        defines "BUILD_SHIPPING"
    filter {}

include "Groovy"
include "Editor"
include "Sandbox"
include "Game"