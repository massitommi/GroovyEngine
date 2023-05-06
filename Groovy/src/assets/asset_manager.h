#pragma once

#include "core/core.h"
#include "asset.h"

class AssetManager
{
public:
	static void Init();

	static AssetHandle AddNew(const std::string& path);

	static size_t Find(AssetUUID id);

	static AssetUUID GenUUID();

	static const std::vector<AssetHandle>& GetAssets();
};