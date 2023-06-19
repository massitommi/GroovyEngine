#pragma once

#include "asset.h"
#include <map>

class AssetManager
{
public:
	static void Init();
	static void Shutdown();

	static AssetHandle Get(AssetUUID uuid);

	template<typename T>
	static T* Get(AssetUUID uuid)
	{
		return (T*)Get(uuid).instance;
	}

	static void SaveRegistry();

#if WITH_EDITOR

	static const std::map<AssetUUID, AssetHandle>& Editor_GetRegistry();
	static std::vector<AssetHandle> Editor_GetAssets(EAssetType filter = ASSET_TYPE_NONE);
	static AssetHandle Editor_OnImport(const std::string& fileName, EAssetType type);
	static void Editor_Delete(AssetUUID uuid);

#endif
};