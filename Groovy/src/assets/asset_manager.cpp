#include "asset_manager.h"
#include "platform/platform.h"
#include "project/project.h"
#include "asset_loader.h"
#include <random>
#include "renderer/api/renderer_api.h"

static std::map<AssetUUID, AssetHandle> sAssetRegistry;

// random stuff
static std::random_device sRandomDevice;
static std::mt19937_64 sRandomEngine(sRandomDevice());
static std::uniform_int_distribution<uint64> sRandomDistributor;
// random stuff end

static AssetUUID GenUUID()
{
	return sRandomDistributor(sRandomEngine);
}

extern Project gProj;
Texture* DEFAULT_TEXTURE = nullptr;
Shader* DEFAULT_SHADER = nullptr;
Material* DEFAULT_MATERIAL = nullptr;

static AssetInstance* InstantiateAsset(const AssetHandle& handle)
{
	std::string absFilePath = (gProj.assets / handle.name).string();

	switch (handle.type)
	{
		case ASSET_TYPE_TEXTURE:
			return AssetLoader::LoadTexture(absFilePath);

		case ASSET_TYPE_SHADER:
			return AssetLoader::LoadShader(absFilePath);

		case ASSET_TYPE_MATERIAL:
			return new Material();

		case ASSET_TYPE_MESH:
			return new Mesh();
	}

	checkslowf(0, "Trying to instantiate an unknown type asset??!?!?");
}

void AssetManager::Init()
{
	// default assets
	{
		// default texture
		{
			TextureSpec spec;
			spec.width = spec.height = 1;
			spec.format = COLOR_FORMAT_R8G8B8A8_UNORM;
			uint32 data = 0xffffffff;

			DEFAULT_TEXTURE = Texture::Create(spec, &data, 4);
		}
		// default shader
		{
			std::string shaderFile = "default_shader";
			switch (RendererAPI::GetAPI())
			{
				case RENDERER_API_D3D11:
					shaderFile += ".hlsl";
					break;
			}

			DEFAULT_SHADER = AssetLoader::LoadShader((gProj.assets / "default" / shaderFile).string());
		}
		// default material
		{
			DEFAULT_MATERIAL = new Material();
			DEFAULT_MATERIAL->SetShader(DEFAULT_SHADER);
			DEFAULT_MATERIAL->SetResources(DEFAULT_TEXTURE);
		}

		AssetHandle tmpHandle;

		tmpHandle.name = "DEFAULT_TEXTURE";
		tmpHandle.type = ASSET_TYPE_TEXTURE;
		tmpHandle.uuid = 1;
		tmpHandle.instance = DEFAULT_TEXTURE;
		tmpHandle.instance->__internal_SetUUID(1);

		sAssetRegistry[tmpHandle.uuid] = tmpHandle;

		tmpHandle.name = "DEFAULT_SHADER";
		tmpHandle.type = ASSET_TYPE_SHADER;
		tmpHandle.uuid = 2;
		tmpHandle.instance = DEFAULT_SHADER;
		tmpHandle.instance->__internal_SetUUID(2);

		sAssetRegistry[tmpHandle.uuid] = tmpHandle;

		tmpHandle.name = "DEFAULT_MATERIAL";
		tmpHandle.type = ASSET_TYPE_MATERIAL;
		tmpHandle.uuid = 3;
		tmpHandle.instance = DEFAULT_MATERIAL;
		tmpHandle.instance->__internal_SetUUID(3);

		sAssetRegistry[tmpHandle.uuid] = tmpHandle;
	}

	Buffer registryFile;
	FileSystem::ReadFileBinary(gProj.assetRegistry.string(), registryFile);

	if (!registryFile.size())
		return;

	BufferView registryView(registryFile);

	uint32 assetsCount = registryView.read<uint32>();

	std::vector<AssetHandle> fileAssets;
	fileAssets.reserve(assetsCount);

	// load registry
	for (uint32 i = 0; i < assetsCount; i++)
	{
		AssetHandle& assetHandle = fileAssets.emplace_back();
		assetHandle.name = registryView.read<std::string>();
		assetHandle.uuid = registryView.read<AssetUUID>();
		assetHandle.type = registryView.read<EAssetType>();
		assetHandle.instance = InstantiateAsset(assetHandle);

		assetHandle.instance->__internal_SetUUID(assetHandle.uuid);

		sAssetRegistry[assetHandle.uuid] = assetHandle;
	}

	// load assets
	for (const AssetHandle& handle : fileAssets)
	{
		if (!handle.instance->IsLoaded())
			handle.instance->Load();
	}
}

void AssetManager::Shutdown()
{
	for (const auto [uuid, handle] : sAssetRegistry)
		delete handle.instance;
}

AssetHandle AssetManager::Get(AssetUUID uuid)
{
	if(sAssetRegistry.find(uuid) != sAssetRegistry.end())
		return sAssetRegistry[uuid];
	return {};
}

void AssetManager::SaveRegistry()
{
	DynamicBuffer registryFile;

	registryFile.push<uint32>(sAssetRegistry.size() - 3);

	for (const auto& [uuid, handle] : sAssetRegistry)
	{
		if (handle.instance == DEFAULT_TEXTURE || handle.instance == DEFAULT_SHADER || handle.instance == DEFAULT_MATERIAL)
			continue;

		registryFile.push(handle.name);
		registryFile.push(handle.uuid);
		registryFile.push(handle.type);
	}

	FileSystem::WriteFileBinary(gProj.assetRegistry.string(), registryFile);
}

#if WITH_EDITOR

const std::map<AssetUUID, AssetHandle>& AssetManager::Editor_GetRegistry()
{
	return sAssetRegistry;
}

std::vector<AssetHandle> AssetManager::Editor_GetAssets(EAssetType filter)
{
	std::vector<AssetHandle> res;

	if (filter == ASSET_TYPE_NONE)
	{
		for (const auto& [uuid, handle] : sAssetRegistry)
			res.push_back(handle);
	}
	else
	{
		for (const auto& [uuid, handle] : sAssetRegistry)
			if (handle.type == filter)
				res.push_back(handle);
	}

	return res;
}

AssetHandle AssetManager::Editor_OnImport(const std::string& fileName, EAssetType type)
{
	AssetUUID uuid = GenUUID();

	AssetHandle& handle = sAssetRegistry[uuid];
	handle.name = fileName;
	handle.type = type;
	handle.uuid = uuid;
	handle.instance = InstantiateAsset(handle);
	handle.instance->__internal_SetUUID(uuid);

	extern bool gEditorPendingSave;
	gEditorPendingSave = true;

	return handle;
}

void AssetManager::Editor_Delete(AssetUUID uuid)
{
	delete sAssetRegistry[uuid].instance;
	sAssetRegistry.erase(uuid);

	extern bool gEditorPendingSave;
	gEditorPendingSave = true;
}

#endif