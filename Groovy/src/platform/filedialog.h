#pragma once

#include "core/core.h"

#define MAKE_EXT(ext) "*." #ext

struct ExtensionFilter
{
	std::string description; // whatever you want
	std::vector<std::string> extensions; // *.png allows every png file, specific.png allows only specifig.png
};

typedef std::vector<ExtensionFilter> ExtensionFilters;

class CORE_API FileDialog
{
public:
	static std::string OpenFileDialog(const std::string& titleBar, const ExtensionFilters& filters);
	static std::string SaveFileDialog(const std::string& titleBar);
};