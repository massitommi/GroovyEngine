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

    filter "configurations:Debug_Editor or Development_Editor"
        postbuildcommands ("{COPYDIR} %{cfg.buildtarget.directory}" .. " %{wks.location}bin/" .. outputdir .. "/Editor/")
    filter {}

    filter "configurations:Debug_Game or Shipping"
        postbuildcommands ("{COPYDIR} %{cfg.buildtarget.directory}" .. " %{wks.location}bin/" .. outputdir .. "/Sandbox/")
    filter {}