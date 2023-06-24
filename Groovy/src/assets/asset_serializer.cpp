#include "asset_serializer.h"
#include "platform/filesystem.h"

#include "renderer/material.h"
#include "renderer/mesh.h"

void AssetSerializer::SerializeMaterial(Material* mat, const std::string& filePath)
{
	check(mat);
	
	DynamicBuffer fileData;
	mat->Serialize(fileData);

	FileSystem::WriteFileBinary(filePath, fileData);
}

void AssetSerializer::SerializeMesh(Mesh* mesh, const std::string& filePath)
{
	check(mesh);

	DynamicBuffer fileData;
	mesh->Serialize(fileData);

	FileSystem::OverwriteFileBinary(filePath, fileData.data(), fileData.used(), mesh->GetAssetOffsetForSerialization());
}
