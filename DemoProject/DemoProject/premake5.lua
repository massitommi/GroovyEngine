project (PROJECT_NAME)
    location (PROJECT_NAME)
    kind "SharedLib"
    language "C++"
    cppdialect "C++17"

    files
    {
        "src/**.cpp",
        "src/**.h"
    }

    