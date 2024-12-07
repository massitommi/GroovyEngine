project "DemoProject"
    kind "SharedLib"
    language "C++"
    cppdialect "C++17"

    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
    targetdir ("%{prj.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{prj.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "src/**.h",
        "src/**.cpp"
    }

    includedirs
    {
        "src",
        "%{wks.location}/Groovy/src"
    }

    links
    {
        "Groovy"
    }