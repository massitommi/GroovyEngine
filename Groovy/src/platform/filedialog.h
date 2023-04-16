#pragma once

#include "core/core.h"

#define MAKE_EXT(ext) "*." #ext

struct ExtensionFilter
{
	String description; // whatever you want
	std::vector<String> extensions; // *.png allows every png file, specific.png allows only specifig.png
};

typedef std::vector<ExtensionFilter> ExtensionFilters;

class FileDialog
{
public:
	static String OpenFileDialog(const String& titleBar, const ExtensionFilters& filters);
	static String SaveFileDialog(const String& titleBar);
};