#pragma once

#include "core/core.h"
#include "asset.h"

class AssetManager
{
public:
	static void Init();
	static void Shutdown();

	static void SaveRegistry();
	static const std::vector<AssetHandle>& GetRegistry();
	static std::vector<AssetHandle> GetRegistryFiltered(EAssetType filter);
	static size_t Find(AssetUUID uuid);
	static AssetHandle& Get(AssetUUID uuid);
	
	template<typename T>
	static T* GetInstance(AssetUUID uuid)
	{
		return (T*)(Get(uuid).instance);
	}

#if WITH_EDITOR
	static AssetHandle AddEditorNew(const std::string& path, EAssetType type, AssetInstance* instance = nullptr);
	static void EditorDelete(const AssetHandle& handle);
#endif
};