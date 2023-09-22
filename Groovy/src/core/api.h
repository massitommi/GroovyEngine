#pragma once

#define DLL_EXPORT __declspec(dllexport)
#define DLL_IMPORT __declspec(dllimport)

#if !BUILD_MONOLITHIC
	#if BUILD_GROOVY_CORE
		#define CORE_API DLL_EXPORT
	#else
		#define CORE_API DLL_IMPORT
	#endif

	#define GAME_API DLL_EXPORT
#else
	#define CORE_API
	#define GAME_API
#endif