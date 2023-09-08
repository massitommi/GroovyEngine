#pragma once

#include "core/core.h"
#include "assets/asset.h"
#include "renderer/api/texture.h"

class AssetImporter
{
public:
	static EAssetType GetTypeFromFilename(const std::string& filename);
	static bool ImportTexture(const std::string& originalFile, const std::string& newFile);
	static bool ImportMesh(const std::string& originalFile, const std::string& newFile);
	static bool ImportAudio(const std::string& originalFile, const std::string& newFile);

	static bool GetRawTexture(const std::string& compressedFile, Buffer& outBuffer, TextureSpec& outSpec);
};