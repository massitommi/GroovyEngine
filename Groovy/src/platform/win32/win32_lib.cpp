#if PLATFORM_WIN32

#include "win32_globals.h"
#include "platform/lib.h"

void* Lib::LoadDll(const std::string& path)
{
	return LoadLibraryA(path.c_str());
}

void Lib::UnloadDll(void* dll)
{
	check(FreeLibrary((HMODULE)dll));
}

void* Lib::GetSymbol(void* program, const std::string& symbolName)
{
	return GetProcAddress((HMODULE)program, symbolName.c_str());
}

#endif