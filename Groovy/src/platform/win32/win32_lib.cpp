#if PLATFORM_WIN32

#include "win32_globals.h"
#include "platform/lib.h"

void* Lib::LoadDll(const std::string& path)
{
	HMODULE lib = LoadLibraryA(path.c_str());
	if (!lib)
	{
		GROOVY_LOG_ERR("Lib::LoadDll Unable to load dll");
	}
	return lib;
}

void Lib::UnloadDll(void* dll)
{
	check(dll);

	if (!FreeLibrary((HMODULE)dll))
	{
		GROOVY_LOG_ERR("Lib::UnloadDll Unable to unload dll");
	}
}

void* Lib::GetSymbol(void* program, const std::string& symbolName)
{
	return GetProcAddress((HMODULE)program, symbolName.c_str());
}

#endif