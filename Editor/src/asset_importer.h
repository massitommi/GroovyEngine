#pragma once

#include "core/core.h"

class AssetImporter
{
public:
	static bool ImportTexture(const String& originalFile, const String& outFilePath);
};