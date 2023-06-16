#pragma once

#include "core/core.h"
#include <filesystem>

struct Project
{
	std::filesystem::path projFile;
	std::filesystem::path assetRegistry;
	std::filesystem::path assets;
	std::string name;
};
