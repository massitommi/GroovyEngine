project "Game"
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
        "%{wks.location}/Groovy/src"
    }