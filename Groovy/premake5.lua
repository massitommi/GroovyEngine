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
        "src",
        "%{wks.location}/vendor"
    }

    libdirs
    {
        "%{wks.location}/vendor/fmod/bin"
    }

    links
    {
        "fmod"
    }

    postbuildcommands
    {
        ("{COPYDIR} %{cfg.buildtarget.directory}" .. " %{wks.location}bin/" .. outputdir .. "/Editor/"),
        ("{COPYDIR} %{cfg.buildtarget.directory}" .. " %{wks.location}bin/" .. outputdir .. "/Sandbox/"),

        ("{COPYDIR} %{wks.location}/vendor/fmod/bin/" .. " %{wks.location}bin/" .. outputdir .. "/Editor/"),
        ("{COPYDIR} %{wks.location}/vendor/fmod/bin/" .. " %{wks.location}bin/" .. outputdir .. "/Sandbox/")
    }