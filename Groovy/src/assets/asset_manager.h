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

	static AssetHandle Editor_Add(const std::string& name, EAssetType type, AssetInstance* instance = nullptr);
	static void Editor_Remove(AssetHandle handle);
	static void Editor_Rename(AssetHandle handle, const std::string& newName);

#endif
};