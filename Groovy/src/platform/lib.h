#pragma once

#include "core/core.h"

class CORE_API Lib
{
public:
	static void* LoadDll(const std::string& path);
	static void UnloadDll(void* dll);

	static void* GetSymbol(void* program, const std::string& symbolName);
};