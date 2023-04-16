project "Sandbox"
    kind "StaticLib"
    language "C++"

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