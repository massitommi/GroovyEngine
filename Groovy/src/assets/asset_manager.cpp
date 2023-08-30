#include "asset_manager.h"
#include "platform/filesystem.h"
#include "project/project.h"
#include "asset_loader.h"
#include <random>
#include "renderer/api/renderer_api.h"
#include "renderer/api/texture.h"
#include "renderer/api/shader.h"
#include "renderer/material.h"
#include "renderer/mesh.h"
#include "gameframework/blueprint.h"
#include "gameframework/scene.h"
#include "utils/string/string_utils.h"

static std::map<AssetUUID, AssetHandle> sAssetRegistry;
static std::vector<AssetHandle> sAssets;

// random stuff
static std::random_device sRandomDevice;
static std::mt19937_64 sRandomEngine(sRandomDevice());
static std::uniform_int_distribution<uint64> sRandomDistributor;
// random stuff end

static AssetUUID GenUUID()
{
	return sRandomDistributor(sRandomEngine);
}

extern GroovyProject gProj;
CORE_API Texture* DEFAULT_TEXTURE = nullptr;
CORE_API Shader* DEFAULT_SHADER = nullptr;
CORE_API Material* DEFAULT_MATERIAL = nullptr;
CORE_API Mesh* DEFAULT_CUBE = nullptr;

static AssetInstance* InstantiateAsset(const AssetHandle& handle)
{
	std::string absFilePath = (gProj.GetAssetsPath() / handle.name).string();

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

		case ASSET_TYPE_BLUEPRINT:
			return new ObjectBlueprint();

		case ASSET_TYPE_ACTOR_BLUEPRINT:
			return new ActorBlueprint();

		case ASSET_TYPE_SCENE:
			return new Scene();
	}

	checkslowf(0, "Trying to instantiate an unknown type asset??!?!?");
	return nullptr;
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
			std::string shaderFile;
			switch (RendererAPI::GetAPI())
			{
				case RENDERER_API_D3D11:
					shaderFile = "default_shader.hlsl";
					break;
			}

			DEFAULT_SHADER = AssetLoader::LoadShader((std::filesystem::path("shaders") / shaderFile).string());
		}
		// default material
		{
			DEFAULT_MATERIAL = new Material();
			DEFAULT_MATERIAL->SetShader(DEFAULT_SHADER);
			DEFAULT_MATERIAL->SetResources(DEFAULT_TEXTURE);
		}
		// default cube (mesh)
		{
			MeshVertex cubeVertexBuffer[] =
			{
				// front face
				{
					{ -0.5f, -0.5f, -0.5f, 1.0f },
					{ 1.0f, 1.0f, 1.0f, 1.0f },
					{ 0.0f, 1.0f }
				},
				{
					{ -0.5f, +0.5f, -0.5f, 1.0f },
					{ 1.0f, 1.0f, 1.0f, 1.0f },
					{ 0.0f, 0.0f }
				},
				{
					{ +0.5f, +0.5f, -0.5f, 1.0f },
					{ 1.0f, 1.0f, 1.0f, 1.0f },
					{ 1.0f, 0.0f }
				},
				{
					{ +0.5f, -0.5f, -0.5f, 1.0f },
					{ 1.0f, 1.0f, 1.0f, 1.0f },
					{ 1.0f, 1.0f }
				},
				// right face
				{
					{ +0.5f, -0.5f, -0.5f, 1.0f },
					{ 1.0f, 1.0f, 1.0f, 1.0f },
					{ 0.0f, 1.0f }
				},
				{
					{ +0.5f, +0.5f, -0.5f, 1.0f },
					{ 1.0f, 1.0f, 1.0f, 1.0f },
					{ 0.0f, 0.0f }
				},
				{
					{ +0.5f, +0.5f, +0.5f, 1.0f },
					{ 1.0f, 1.0f, 1.0f, 1.0f },
					{ 1.0f, 0.0f }
				},
				{
					{ +0.5f, -0.5f, +0.5f, 1.0f },
					{ 1.0f, 1.0f, 1.0f, 1.0f },
					{ 1.0f, 1.0f }
				},
				// left face
				{
					{ -0.5f, -0.5f, +0.5f, 1.0f },
					{ 1.0f, 1.0f, 1.0f, 1.0f },
					{ 0.0f, 1.0f }
				},
				{
					{ -0.5f, +0.5f, +0.5f, 1.0f },
					{ 1.0f, 1.0f, 1.0f, 1.0f },
					{ 0.0f, 0.0f }
				},
				{
					{ -0.5f, +0.5f, -0.5f, 1.0f },
					{ 1.0f, 1.0f, 1.0f, 1.0f },
					{ 1.0f, 0.0f }
				},
				{
					{ -0.5f, -0.5f, -0.5f, 1.0f },
					{ 1.0f, 1.0f, 1.0f, 1.0f },
					{ 1.0f, 1.0f }
				},
				// top face
				{
					{ -0.5f, +0.5f, -0.5f, 1.0f },
					{ 1.0f, 1.0f, 1.0f, 1.0f },
					{ 0.0f, 1.0f }
				},
				{
					{ -0.5f, +0.5f, +0.5f, 1.0f },
					{ 1.0f, 1.0f, 1.0f, 1.0f },
					{ 0.0f, 0.0f }
				},
				{
					{ +0.5f, +0.5f, +0.5f, 1.0f },
					{ 1.0f, 1.0f, 1.0f, 1.0f },
					{ 1.0f, 0.0f }
				},
				{
					{ +0.5f, +0.5f, -0.5f, 1.0f },
					{ 1.0f, 1.0f, 1.0f, 1.0f },
					{ 1.0f, 1.0f }
				},
				// bottom face
				{
					{ -0.5f, -0.5f, +0.5f, 1.0f },
					{ 1.0f, 1.0f, 1.0f, 1.0f },
					{ 0.0f, 1.0f }
				},
				{
					{ -0.5f, -0.5f, -0.5f, 1.0f },
					{ 1.0f, 1.0f, 1.0f, 1.0f },
					{ 0.0f, 0.0f }
				},
				{
					{ +0.5f, -0.5f, -0.5f, 1.0f },
					{ 1.0f, 1.0f, 1.0f, 1.0f },
					{ 1.0f, 0.0f }
				},
				{
					{ +0.5f, -0.5f, +0.5f, 1.0f },
					{ 1.0f, 1.0f, 1.0f, 1.0f },
					{ 1.0f, 1.0f }
				},
				// back face
				{
					{ +0.5f, -0.5f, +0.5f, 1.0f },
					{ 1.0f, 1.0f, 1.0f, 1.0f },
					{ 0.0f, 1.0f }
				},
				{
					{ +0.5f, +0.5f, +0.5f, 1.0f },
					{ 1.0f, 1.0f, 1.0f, 1.0f },
					{ 0.0f, 0.0f }
				},
				{
					{ -0.5f, +0.5f, +0.5f, 1.0f },
					{ 1.0f, 1.0f, 1.0f, 1.0f },
					{ 1.0f, 0.0f }
				},
				{
					{ -0.5f, -0.5f, +0.5f, 1.0f },
					{ 1.0f, 1.0f, 1.0f, 1.0f },
					{ 1.0f, 1.0f }
				}
			};

			MeshIndex cubeIndexBuffer[6 * 6];

			for (uint32 i = 0; i < 6; i++)
			{
				cubeIndexBuffer[i * 6 + 0] = 0 + 4 * i;
				cubeIndexBuffer[i * 6 + 1] = 1 + 4 * i;
				cubeIndexBuffer[i * 6 + 2] = 2 + 4 * i;
				cubeIndexBuffer[i * 6 + 3] = 2 + 4 * i;
				cubeIndexBuffer[i * 6 + 4] = 3 + 4 * i;
				cubeIndexBuffer[i * 6 + 5] = 0 + 4 * i;
			}

			VertexBuffer* vBuffer = VertexBuffer::Create(sizeof(cubeVertexBuffer), cubeVertexBuffer, sizeof(MeshVertex));
			IndexBuffer* iBuffer = IndexBuffer::Create(sizeof(cubeIndexBuffer), cubeIndexBuffer);
			std::vector<SubmeshData> submeshes;
			submeshes.push_back
			({
				sizeof(cubeVertexBuffer) / sizeof(MeshVertex),
				sizeof(cubeIndexBuffer) / sizeof(MeshIndex)
			});
			std::vector<Material*> mats;
			mats.push_back(DEFAULT_MATERIAL);

			DEFAULT_CUBE = new Mesh(vBuffer, iBuffer, submeshes, mats);
		}
;
		AssetHandle tmpHandle;

		tmpHandle.name = "DEFAULT_TEXTURE";
		tmpHandle.type = ASSET_TYPE_TEXTURE;
		tmpHandle.uuid = 1;
		tmpHandle.instance = DEFAULT_TEXTURE;
		tmpHandle.instance->__internal_SetUUID(1);

		sAssets.push_back(tmpHandle);
		sAssetRegistry[tmpHandle.uuid] = tmpHandle;

		tmpHandle.name = "DEFAULT_SHADER";
		tmpHandle.type = ASSET_TYPE_SHADER;
		tmpHandle.uuid = 2;
		tmpHandle.instance = DEFAULT_SHADER;
		tmpHandle.instance->__internal_SetUUID(2);

		sAssets.push_back(tmpHandle);
		sAssetRegistry[tmpHandle.uuid] = tmpHandle;

		tmpHandle.name = "DEFAULT_MATERIAL";
		tmpHandle.type = ASSET_TYPE_MATERIAL;
		tmpHandle.uuid = 3;
		tmpHandle.instance = DEFAULT_MATERIAL;
		tmpHandle.instance->__internal_SetUUID(3);

		sAssets.push_back(tmpHandle);
		sAssetRegistry[tmpHandle.uuid] = tmpHandle;

		tmpHandle.name = "DEFAULT_CUBE";
		tmpHandle.type = ASSET_TYPE_MESH;
		tmpHandle.uuid = 4;
		tmpHandle.instance = DEFAULT_CUBE;
		tmpHandle.instance->__internal_SetUUID(4);

		sAssets.push_back(tmpHandle);
		sAssetRegistry[tmpHandle.uuid] = tmpHandle;
	}

	Buffer registryFile;
	FileSystem::ReadFileBinary(gProj.GetAssetRegistryPath().string(), registryFile);

	if (!registryFile.size())
		return;

	BufferView registryView(registryFile);

	uint32 assetsCount = registryView.read<uint32>();

	std::vector<AssetHandle> registry;
	// load registry
	for (uint32 i = 0; i < assetsCount; i++)
	{
		AssetHandle& asset = registry.emplace_back();
		asset.name = registryView.read<std::string>();
		asset.uuid = registryView.read<AssetUUID>();
		asset.type = registryView.read<EAssetType>();
	}
	// validate registry
	for (uint32 i = 0; i < registry.size(); i++)
	{
		if (!FileSystem::FileExists((gProj.GetAssetsPath() / registry[i].name).string()))
		{
			registry.erase(registry.begin() + i);
			i--;
		}
		else
		{
			AssetHandle& asset = sAssets.emplace_back(registry[i]);
			asset.instance = InstantiateAsset(asset);
			asset.instance->__internal_SetUUID(asset.uuid);

			sAssetRegistry[asset.uuid] = asset;
		}
	}

	// load assets
	for(uint32 i = 4; i < sAssets.size(); i++)
		if (sAssets[i].type != ASSET_TYPE_SCENE && !sAssets[i].instance->IsLoaded())
			sAssets[i].instance->Load();
}

void AssetManager::Shutdown()
{
	for (AssetHandle& handle : sAssets)
		delete handle.instance;
}

AssetHandle AssetManager::Get(AssetUUID uuid)
{
	if (uuid == 0)
		return {};

	if(sAssetRegistry.find(uuid) != sAssetRegistry.end())
		return sAssetRegistry[uuid];
	
	return {};
}

void AssetManager::SaveRegistry()
{
	DynamicBuffer registryFile;

	registryFile.push<uint32>(sAssets.size() - 4);

	for (uint32 i = 4; i < sAssets.size(); i++)
	{
		registryFile.push(sAssets[i].name);
		registryFile.push(sAssets[i].uuid);
		registryFile.push(sAssets[i].type);
	}

	FileSystem::WriteFileBinary(gProj.GetAssetRegistryPath().string(), registryFile);
}

const std::map<AssetUUID, AssetHandle>& AssetManager::GetRegistry()
{
	return sAssetRegistry;
}

const std::vector<AssetHandle>& AssetManager::GetAssets()
{
	return sAssets;
}

std::vector<AssetHandle> AssetManager::GetAssets(EAssetType filter)
{
	std::vector<AssetHandle> assets;
	
	for (AssetHandle& handle : sAssets)
		if (handle.type == filter)
			assets.push_back(handle);
	
	return assets;
}

AssetHandle AssetManager::FindByPath(const std::string& filePath)
{
	for (AssetHandle& handle : sAssets)
	{
		if (stringUtils::EqualsCaseInsensitive(filePath, handle.name))
			return handle;
	}
	return {};
}

#if WITH_EDITOR

AssetHandle AssetManager::Editor_Add(const std::string& name, EAssetType type, AssetInstance* instance)
{
	AssetUUID newUUID = GenUUID();

	AssetHandle newHandle;
	newHandle.name = name;
	newHandle.type = type;
	newHandle.uuid = newUUID;
	newHandle.instance = instance ? instance : InstantiateAsset(newHandle);
	newHandle.instance->__internal_SetUUID(newUUID);

	sAssets.push_back(newHandle);
	sAssetRegistry[newUUID] = newHandle;

	SaveRegistry();

	return newHandle;
}

void AssetManager::Editor_Remove(AssetHandle handle)
{
	checkslow(handle.uuid);

	sAssetRegistry.erase(handle.uuid);

	sAssets.erase(std::find_if(sAssets.begin(), sAssets.end(), [=](AssetHandle& h) { return h.uuid == handle.uuid; }));

	for (const AssetHandle& a : sAssets)
		a.instance->Editor_FixDependencyDeletion(handle);

	delete handle.instance;

	SaveRegistry();
}

void AssetManager::Editor_Rename(AssetHandle handle, const std::string& newName)
{
	checkslow(handle.uuid);

	sAssetRegistry[handle.uuid].name = newName;

	auto it = std::find_if(sAssets.begin(), sAssets.end(), [=](AssetHandle& h) { return h.uuid == handle.uuid; });
	it->name = newName;

	SaveRegistry();
}

#endif