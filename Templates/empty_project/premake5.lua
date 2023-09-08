include "premake_config.lua"

workspace (PROJECT_NAME)
	include (ENGINE_LOCATION .. "/premake_core.lua")

	project (PROJECT_NAME)
    location (PROJECT_NAME)
    kind "SharedLib"
    language "C++"
    cppdialect "C++17"

    files
    {
        (PROJECT_NAME .. "/src/**.cpp"),
		(PROJECT_NAME .. "/src/**.h"),
    }

	includedirs
	{
		(PROJECT_NAME .. "/src"),
		(ENGINE_LOCATION .. "/Groovy/src")
	}

	links
	{
		(ENGINE_LOCATION .. "/bin/" .. outputdir .. "/Groovy/Groovy")
	}