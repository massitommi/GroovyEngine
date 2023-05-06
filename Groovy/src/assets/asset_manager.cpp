#include "asset_manager.h"
#include "platform/filesystem.h"
#include "platform/messagebox.h"
#include "project/Project.h"

#include <random>

static std::vector<AssetHandle> sAssetRegistry;

// random stuff
static std::random_device sRandomDevice;
static std::mt19937_64 sRandomEngine(sRandomDevice());
static std::uniform_int_distribution<uint64> sRandomDistributor;
// random stuff end

void AssetManager::Init()
{
	const auto& groovyAssets = FileSystem::GetFilesInDir(Project::GetMain()->GetAssetPath(), { GROOVY_ASSET_EXT });
	const auto& groovyShaders = FileSystem::GetFilesInDir(Project::GetMain()->GetAssetPath(), { ".hlsl" });
	const std::string assetPath = Project::GetMain()->GetAssetPath();

	for (const std::string& asset : groovyAssets)
	{
		AddNew(assetPath + asset);
	}

	for (const std::string& shader : groovyShaders)
	{
		AssetHandle handle;
		handle.path = assetPath + shader;
		handle.type = ASSET_TYPE_SHADER;
		handle.uuid = 1; // todo: fix

		sAssetRegistry.push_back(handle);
	}
}

AssetHandle AssetManager::AddNew(const std::string& path)
{
	GroovyAssetHeader assetHeader = {};
	size_t bytesRead = 0;
	FileSystem::ReadFileBinary(path, &assetHeader, sizeof(GroovyAssetHeader), bytesRead);
	check(bytesRead == sizeof(GroovyAssetHeader) && strncmp(assetHeader.magic, GROOVY_ASSET_MAGIC, GROOVY_ASSET_MAGIC_SIZE) == 0);
	
	AssetHandle newAssetHandle;
	newAssetHandle.path = path;
	newAssetHandle.uuid = assetHeader.uuid;
	newAssetHandle.type = assetHeader.type;
	
	sAssetRegistry.push_back(newAssetHandle);

	return newAssetHandle;
}

size_t AssetManager::Find(AssetUUID id)
{
	size_t i;
	for (i = 0; i < sAssetRegistry.size(); i++)
	{
		if (sAssetRegistry[i].uuid == id)
		{
			break;
		}
	}
	return i;
}

AssetUUID AssetManager::GenUUID()
{
	return sRandomDistributor(sRandomEngine);
}

const std::vector<AssetHandle>& AssetManager::GetAssets()
{
	return sAssetRegistry;
}
