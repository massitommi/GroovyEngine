project "Groovy"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"

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
        "Game",
        "fmod"
    }

    postbuildcommands
    {
        ("{COPYDIR} %{wks.location}/vendor/fmod/bin/" .. " %{wks.location}bin/" .. outputdir .. "/Editor/"),
        ("{COPYDIR} %{wks.location}/vendor/fmod/bin/" .. " %{wks.location}bin/" .. outputdir .. "/Sandbox/")
    }