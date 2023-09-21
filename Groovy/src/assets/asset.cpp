#include "asset.h"
#include "asset_manager.h"

std::string AssetInstance::GetAssetName() const
{
	return AssetManager::Get(GetUUID()).name;
}
