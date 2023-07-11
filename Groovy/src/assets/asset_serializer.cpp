#include "asset_serializer.h"
#include "platform/filesystem.h"
#include "asset_manager.h"
#include "project/project.h"

#include "renderer/mesh.h"

extern GroovyProject gProj;

void AssetSerializer::SerializeGenericAsset(AssetInstance* asset, const std::string& filePath)
{
	check(asset);

	DynamicBuffer fileData;
	asset->Serialize(fileData);

	FileSystem::WriteFileBinary(filePath, fileData);
}

void AssetSerializer::SerializeGenericAsset(AssetInstance* asset)
{
	check(asset);

	AssetHandle handle = AssetManager::Get(asset->GetUUID());
	std::string absPath = (gProj.GetAssetsPath() / handle.name).string();
	SerializeGenericAsset(asset, absPath);
}

void AssetSerializer::SerializeMesh(Mesh* mesh, const std::string& filePath)
{
	check(mesh);

	DynamicBuffer fileData;
	mesh->Serialize(fileData);

	FileSystem::OverwriteFileBinary(filePath, fileData.data(), fileData.used(), mesh->GetAssetOffsetForSerialization());
}

void AssetSerializer::SerializeMesh(Mesh* mesh)
{
	check(mesh);

	AssetHandle handle = AssetManager::Get(mesh->GetUUID());
	std::string absPath = (gProj.GetAssetsPath() / handle.name).string();
	SerializeMesh(mesh, absPath);
}