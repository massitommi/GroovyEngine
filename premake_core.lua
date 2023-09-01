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

    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    rtti "Off"

    defines "PLATFORM_WIN32"
    defines ("LINKER_OUTPUT_DIR=\"" .. outputdir .. "\"")

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