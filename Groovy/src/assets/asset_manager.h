#pragma once

#include "asset.h"
#include <map>

class AssetManager
{
public:
	static void Init();

	static AssetHandle Get(AssetUUID uuid);

	template<typename T>
	static T* Get(AssetUUID uuid)
	{
		return (T*)Get(uuid).instance;
	}

	static void SaveRegistry();

#if WITH_EDITOR

	static const std::map<AssetUUID, AssetHandle>& GetRegistry();

#endif
};