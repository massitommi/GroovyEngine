#pragma once

#include "core/core.h"
#include "assets/assets.h"
#include "renderer/api/texture.h"

class AssetImporter
{
public:
	static EAssetType GetTypeFromFilename(const String& filename);


	static bool GetRawTexture(const String& compressedFile, Buffer& outBuffer, TextureSpec& outSpec);
	static bool ImportTexture(const String& originalFile, const String& outFolder);
};