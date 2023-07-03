#include "asset_serializer.h"
#include "platform/filesystem.h"
#include "asset_manager.h"
#include "project/project.h"

#include "renderer/material.h"
#include "renderer/mesh.h"
#include "classes/blueprint.h"

extern Project gProj;

void AssetSerializer::SerializeMaterial(Material* material, const std::string& filePath)
{
	check(material);
	
	DynamicBuffer fileData;
	material->Serialize(fileData);

	FileSystem::WriteFileBinary(filePath, fileData);
}

void AssetSerializer::SerializeMaterial(Material* material)
{
	check(material);

	AssetHandle handle = AssetManager::Get(material->GetUUID());
	std::string absPath = (gProj.assets / handle.name).string();
	SerializeMaterial(material, absPath);
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
	std::string absPath = (gProj.assets / handle.name).string();
	SerializeMesh(mesh, absPath);
}

void AssetSerializer::SerializeBlueprint(Blueprint* bp, const std::string& filePath)
{
	DynamicBuffer fileData;
	bp->Serialize(fileData);

	FileSystem::WriteFileBinary(filePath, fileData);
}

void AssetSerializer::SerializeBlueprint(Blueprint* bp)
{
	check(bp);

	AssetHandle handle = AssetManager::Get(bp->GetUUID());
	std::string absPath = (gProj.assets / handle.name).string();
	SerializeBlueprint(bp, absPath);
}
