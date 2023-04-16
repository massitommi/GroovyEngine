project "Groovy"
    kind "WindowedApp"
    language "C++"

    files
    {
        "src/**.h",
        "src/**.cpp"
    }

    includedirs
    {
        "src"
    }

    links
    {
        "Editor"
    }