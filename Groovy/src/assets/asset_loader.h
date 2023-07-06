#pragma once

#include "asset.h"

// raw asset loader
class AssetLoader
{
public:

	static class Texture* LoadTexture(const std::string& filePath);
	static class Shader* LoadShader(const std::string& filePath);

	static void LoadGenericAsset(class AssetInstance* asset);
};