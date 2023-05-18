#include "asset_manager.h"
#include "platform/filesystem.h"
#include "platform/messagebox.h"
#include "project/project.h"

#include <random>

#include "asset_loader.h"

#include "renderer/material.h"

static std::string sAssetRegistryPath;
static std::string sAssetsPath;

static std::vector<AssetHandle> sAssetRegistry;

// random stuff
static std::random_device sRandomDevice;
static std::mt19937_64 sRandomEngine(sRandomDevice());
static std::uniform_int_distribution<uint64> sRandomDistributor;
// random stuff end

Texture* DEFAULT_TEXTURE;
Shader* DEFAULT_SHADER;
Material* DEFAULT_MATERIAL;

static AssetUUID GenUUID()
{
	return sRandomDistributor(sRandomEngine);
}

static std::vector <size_t> GetFiltered(EAssetType type)
{
	std::vector<size_t> res;
	for (size_t i = 0; i < sAssetRegistry.size(); i++)
	{
		if (sAssetRegistry[i].type == type)
			res.push_back(i);
	}
	return res;
}

/*
		
	asset registry

	uuid
	type
	filenamelen

*/

extern Project gProj;

void AssetManager::Init()
{
	sAssetRegistryPath = gProj.registryPath;
	sAssetsPath = gProj.assetsPath;

	// default assets
	{
		// texture
		{
			TextureSpec tex;
			tex.format = COLOR_FORMAT_R8G8B8A8_UNORM;
			tex.width = tex.height = 1;
			uint32 data = 0xffffffff; // white
			DEFAULT_TEXTURE = Texture::Create(tex, &data, 1 * 1 * 4);
		}

		// shader
		{
			DEFAULT_SHADER = AssetLoader::LoadShader(sAssetsPath + "default/default_shader.hlsl");
		}

		// material
		{
			DEFAULT_MATERIAL = new Material(DEFAULT_SHADER);
			DEFAULT_MATERIAL->SetTextures(DEFAULT_TEXTURE);
		}
	}

	// load registry
	{
		Buffer registryFile;
		FileSystem::ReadFileBinary(sAssetRegistryPath, registryFile);

		if (!registryFile.size())
		{
			return;
		}

		size_t numAssets = *(registryFile.as<size_t>());

		byte* registryPtr = registryFile.data() + sizeof(size_t);
		for (size_t i = 0; i < numAssets; i++)
		{
			AssetUUID uuid = *((AssetUUID*)registryPtr);
			registryPtr += sizeof(AssetUUID);

			EAssetType type = *((EAssetType*)registryPtr);
			registryPtr += sizeof(EAssetType);

			size_t fileNameLen = *((size_t*)registryPtr);
			registryPtr += sizeof(size_t);

			std::string fileName;
			fileName.resize(fileNameLen);
			memcpy(fileName.data(), registryPtr, fileNameLen);

			AssetHandle asset;
			asset.path = sAssetsPath + fileName;
			asset.name = fileName;
			asset.uuid = uuid;
			asset.type = type;
			asset.instance = nullptr;

			sAssetRegistry.push_back(asset);

			registryPtr += fileNameLen;
		}
	}

	// instantiate assets
	{
		// textures
		{
			std::vector<size_t> filteredAssets = GetFiltered(ASSET_TYPE_TEXTURE);
			for (size_t i : filteredAssets)
			{
				AssetHandle& handle = sAssetRegistry[i];
				AssetInstance* ins = AssetLoader::LoadTexture(handle.path);
				ins->__internal_SetUUID(handle.uuid);
				handle.instance = ins;
			}
		}
		// shaders
		{
			std::vector<size_t> filteredAssets = GetFiltered(ASSET_TYPE_SHADER);
			for (size_t i : filteredAssets)
			{
				AssetHandle& handle = sAssetRegistry[i];
				AssetInstance* ins = AssetLoader::LoadShader(handle.path);
				ins->__internal_SetUUID(handle.uuid);
				handle.instance = ins;
			}
		}
		// materials
		{
			std::vector<size_t> filteredAssets = GetFiltered(ASSET_TYPE_MATERIAL);
			for (size_t i : filteredAssets)
			{
				AssetHandle& handle = sAssetRegistry[i];
				AssetInstance* ins = AssetLoader::LoadMaterial(handle.path);
				ins->__internal_SetUUID(handle.uuid);
				handle.instance = ins;
			}
		}
		// mesh
		{
			std::vector<size_t> filteredAssets = GetFiltered(ASSET_TYPE_MESH);
			for (size_t i : filteredAssets)
			{
				AssetHandle& handle = sAssetRegistry[i];
				AssetInstance* ins = AssetLoader::LoadMesh(handle.path);
				ins->__internal_SetUUID(handle.uuid);
				handle.instance = ins;
			}
		}
	}
}

void AssetManager::Shutdown()
{
	for (const AssetHandle& handle : sAssetRegistry)
	{
		delete handle.instance;
	}

	delete DEFAULT_MATERIAL;
	delete DEFAULT_SHADER;
	delete DEFAULT_TEXTURE;
}

void AssetManager::SaveRegistry()
{
	Buffer fileData;

	size_t totalFileSize =
		sizeof(AssetUUID) * sAssetRegistry.size() 
		+
		sizeof(EAssetType) * sAssetRegistry.size();
	
	for (size_t i = 0; i < sAssetRegistry.size(); i++)
	{
		totalFileSize += sAssetRegistry[i].path.length();
	}

	if (totalFileSize)
	{
		fileData.resize(totalFileSize + sizeof(size_t));
		*fileData.as<size_t>() = sAssetRegistry.size();

		byte* assetRecordPtr = fileData.data() + sizeof(size_t);

		for (const AssetHandle& handle : sAssetRegistry)
		{
			*((AssetUUID*)assetRecordPtr) = handle.uuid; // UUID
			assetRecordPtr += sizeof(AssetUUID);

			*((EAssetType*)assetRecordPtr) = handle.type; // EAssetType
			assetRecordPtr += sizeof(EAssetType);

			*((size_t*)assetRecordPtr) = handle.name.length(); // Name length
			assetRecordPtr += sizeof(size_t);

			memcpy(assetRecordPtr, handle.name.data(), handle.name.length()); // Actual path string
			assetRecordPtr += handle.name.length();
		}
	}

	FileSystem::WriteFileBinary(sAssetRegistryPath, fileData);
}

const std::vector<AssetHandle>& AssetManager::GetRegistry()
{
	return sAssetRegistry;
}

std::vector<AssetHandle> AssetManager::GetRegistryFiltered(EAssetType filter)
{
	std::vector<AssetHandle> result;

	for (const AssetHandle& handle : sAssetRegistry)
		if (handle.type == filter)
			result.push_back(handle);

	return result;
}

size_t AssetManager::Find(AssetUUID uuid)
{
	size_t i;
	for (i = 0; i < sAssetRegistry.size(); i++)
	{
		if (sAssetRegistry[i].uuid == uuid)
		{
			return i;
		}
	}
	return i;
}

AssetHandle& AssetManager::Get(AssetUUID uuid)
{
	for (size_t i = 0; i < sAssetRegistry.size(); i++)
	{
		if (sAssetRegistry[i].uuid == uuid)
		{
			return sAssetRegistry[i];
		}
	}
	check(0);
}

AssetHandle AssetManager::AddEditorNew(const std::string& path, EAssetType type, AssetInstance* instance)
{
	AssetHandle handle;
	handle.path = path;
	handle.name = path.substr(path.find(gProj.assetsPath) + gProj.assetsPath.length());
	handle.uuid = GenUUID();
	handle.type = type;
	handle.instance = instance;

	sAssetRegistry.push_back(handle);

	return handle;
}

void AssetManager::EditorDelete(const AssetHandle& handle)
{
	size_t assetIndex = Find(handle.uuid);
	check(assetIndex < sAssetRegistry.size());

	// todo: resolve 

	FileSystem::DeleteFile(handle.path);
	sAssetRegistry.erase(sAssetRegistry.begin() + assetIndex);
}