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

	static const std::map<AssetUUID, AssetHandle>& GetRegistry();
	static const std::vector<AssetHandle>& GetAssets();
	static std::vector<AssetHandle> GetAssets(EAssetType filter);

	static AssetHandle FindByPath(const std::string& filePath);

#if WITH_EDITOR

	static AssetHandle Editor_OnImport(const std::string& fileName, EAssetType type);
	static AssetHandle Editor_OnAdd(const std::string& fileName, EAssetType type, AssetInstance* instance);
	static void Editor_Delete(AssetUUID uuid);

#endif
};