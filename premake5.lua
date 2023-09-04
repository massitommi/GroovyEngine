workspace "GroovyEngine"
    architecture "x86_64"

    configurations
    {
        "Debug_Editor",
        "Development_Editor",
        "Debug_Game",
        "Shipping"
    }

    startproject "Editor"
    debugargs "../DemoProject/DemoProject.groovyproj"

    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    rtti "Off"

    filter "system:windows"
        defines "PLATFORM_WIN32"
    filter {}

    filter "configurations:Debug_Editor or Development_Editor"
        defines "WITH_EDITOR"
    filter {}

    filter "configurations:Debug_Editor or Debug_Game"
        defines "BUILD_DEBUG"
    filter {}

    filter "configurations:Development_Editor"
        optimize "On"
    filter {}

    filter "configurations:Shipping"
        optimize "On"
        defines "BUILD_SHIPPING"
    filter {}

include "Groovy"
include "Editor"
include "Sandbox"
include "Game"