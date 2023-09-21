    architecture "x86_64"
    
    configurations
    {
        "Debug",
        "Development",
        "Shipping"
    }

    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    rtti "Off"

    defines ("LINKER_OUTPUT_DIR=\"" .. outputdir .. "\"")

    disablewarnings "4251"

    filter "configurations:Debug"
        optimize "Off"
        defines { "BUILD_DEBUG", "WITH_EDITOR" }
    filter {}

    filter "configurations:Development"
        optimize "On"
        defines { "BUILD_DEVELOPMENT", "WITH_EDITOR" }
    filter {}

    filter "configurations:Shipping"
        optimize "On"
        defines { "BUILD_SHIPPING", "BUILD_MONOLITHIC" }
    filter {}

    filter "system:windows"
        defines "PLATFORM_WIN32"
    filter {}