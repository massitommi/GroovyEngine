project "Groovy"
    kind "SharedLib"
    language "C++"
    cppdialect "C++17"

    defines "BUILD_GROOVY_CORE"

    files
    {
        "src/**.h",
        "src/**.cpp"
    }

    includedirs
    {
        "src"
    }

    postbuildcommands
    {
        "{COPYDIR} %{cfg.buildtarget.directory}" .. " %{wks.location}bin/" .. outputdir .. "/Editor/"
    }