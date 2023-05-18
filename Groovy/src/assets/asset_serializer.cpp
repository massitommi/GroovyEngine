#include "asset_serializer.h"
#include "platform/filesystem.h"

void AssetSerializer::SerializeMaterial(Material* mat, const std::string& filePath)
{
	check(mat);

	Buffer matData;
	matData.resize
	(
		sizeof(MaterialAssetHeader) + // header
		mat->mConstBuffersData.size() + // const buffer data
		mat->mTextures.size() * sizeof(AssetUUID) // texture UUIDs
	);

	MaterialAssetHeader header;
	header.shaderID = mat->mShader->GetUUID();
	header.constBuffersSize = mat->mConstBuffersData.size();
	header.numTextures = mat->mTextures.size();

	byte* filePtr = matData.data();
	memcpy(filePtr, &header, sizeof(MaterialAssetHeader));
	filePtr += sizeof(MaterialAssetHeader);
	memcpy(filePtr, mat->mConstBuffersData.data(), mat->mConstBuffersData.size());
	filePtr += mat->mConstBuffersData.size();

	AssetUUID* texturePtr = (AssetUUID*)filePtr;
	for (uint32 i = 0; i < mat->mTextures.size(); i++)
		texturePtr[i] = mat->mTextures[i] ? mat->mTextures[i]->GetUUID() : 0;
	
	auto res = FileSystem::WriteFileBinary(filePath, matData);
	check(res == FILE_OPEN_RESULT_OK);
}

void AssetSerializer::SerializeMesh(Mesh* mesh, const std::string& filePath)
{
	check(mesh);

	Buffer meshData;
	meshData.resize(mesh->GetMaterials().size() * sizeof(AssetUUID));

	for (uint32 i = 0; i < mesh->GetMaterials().size(); i++)
		meshData.as<AssetUUID>()[i] = mesh->GetMaterials()[i]->GetUUID();

	size_t materialsOffset =
		sizeof(MaterialAssetHeader) +
		mesh->GetVertexBufferSize() +
		mesh->GetIndexBufferSize() +
		mesh->GetSubmeshes().size() * sizeof(SubmeshData);

	auto res = FileSystem::AppendFileBinary(filePath, meshData, materialsOffset);
	check(res == FILE_OPEN_RESULT_OK);
}
