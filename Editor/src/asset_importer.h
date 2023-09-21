#pragma once

#include "core/core.h"
#include "assets/asset.h"
#include "renderer/api/texture.h"
#include "platform/filedialog.h"

struct SupportedImport
{
	ExtensionFilter extensions;
	EAssetType type;
};

class AssetImporter
{
public:
	static EAssetType GetTypeFromFilename(const std::string& filename);
	static bool ImportTexture(const std::string& originalFile, const std::string& newFile);
	static bool ImportMesh(const std::string& originalFile, const std::string& newFile);
	static bool ImportAudio(const std::string& originalFile, const std::string& newFile);

	static bool GetRawTexture(const std::string& compressedFile, Buffer& outBuffer, TextureSpec& outSpec);

	static void TryImportAsset(const std::string& file);

	static const std::vector<SupportedImport>& GetSupportedImports();
};