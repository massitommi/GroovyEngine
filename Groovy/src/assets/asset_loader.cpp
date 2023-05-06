#include "asset_loader.h"
#include "platform/filesystem.h"
#include <map>

Texture* AssetLoader::LoadTexture(const std::string& filePath)
{
	Buffer data;
	FileSystem::ReadFileBinary(filePath, data);
	GroovyAssetHeader_Texture* header = (GroovyAssetHeader_Texture*)data.data();
	
	TextureSpec spec;
	spec.width = header->width;
	spec.height = header->height;
	spec.format = header->format;
	
	return Texture::Create(spec, data.data() + sizeof(GroovyAssetHeader_Texture), data.size() - sizeof(GroovyAssetHeader_Texture));
}

Shader* AssetLoader::LoadShader(const std::string& filePath)
{
	Buffer data;
	FileSystem::ReadFileBinary(filePath, data);

	std::string_view tmpFakeStr((char*)data.data(), data.size());
	size_t vertexStart = tmpFakeStr.find(GROOVY_SHADER_VERTEX_SEGMENT, 0) + strlen(GROOVY_SHADER_VERTEX_SEGMENT);
	size_t pixelStart = tmpFakeStr.find(GROOVY_SHADER_PIXEL_SEGMENT, vertexStart) + strlen(GROOVY_SHADER_PIXEL_SEGMENT);

	size_t vertexSize = pixelStart - strlen(GROOVY_SHADER_PIXEL_SEGMENT) - vertexStart;
	size_t pixelSize = data.size() - pixelStart;
	
	return Shader::Create(data.data() + vertexStart, vertexSize, data.data() + pixelStart, pixelSize);
}

Mesh* AssetLoader::LoadMesh(const std::string& filePath)
{
	Buffer fileData;
	FileSystem::ReadFileBinary(filePath, fileData);
	GroovyAssetHeader_Mesh* header = fileData.as<GroovyAssetHeader_Mesh>();

	size_t vertexBufferSize = header->vertexBufferSize;
	size_t indexBufferSize = header->indexBufferSize;
	byte* vertexBufferData = fileData.data() + sizeof(GroovyAssetHeader_Mesh);
	byte* indexBufferData = vertexBufferData + vertexBufferSize;

	VertexBuffer* vertexBuffer = VertexBuffer::Create(vertexBufferSize, vertexBufferData, sizeof(MeshVertex));
	IndexBuffer* indexBuffer = IndexBuffer::Create(indexBufferSize, indexBufferData);

	SubmeshData* submeshData = (SubmeshData*)(indexBufferData + indexBufferSize);
	std::vector<SubmeshData> submeshes;
	submeshes.resize(header->submeshCount);
	memcpy(submeshes.data(), submeshData, header->submeshCount * sizeof(SubmeshData));

	return new Mesh(vertexBuffer, indexBuffer, submeshes);
}

//void AssetLoader::LoadMaterialAsset(const std::string& filePath, MaterialAsset& outAsset)
//{
//	Buffer fileData;
//	FileSystem::ReadFileBinary(filePath, fileData);
//
//	GroovyAssetHeader_Material* header = fileData.as<GroovyAssetHeader_Material>();
//	byte* constBuffersData = fileData.data() + sizeof(GroovyAssetHeader_Material);
//
//	// material uuid
//	outAsset.uuid = header->uuid;
//
//	// shader uuid
//	outAsset.shaderID = header->shaderID;
//	
//	// const buffer data
//	outAsset.constBuffersData.resize(header->constBuffersSize);
//	memcpy(outAsset.constBuffersData.data(), constBuffersData, header->constBuffersSize);
//	
//	// texture uuids
//	outAsset.textureRes.resize(header->numTextures);
//	AssetUUID* texturesResData = (AssetUUID*)(constBuffersData + header->constBuffersSize);
//	for (uint32 i = 0; i < header->numTextures; i++)
//	{
//		outAsset.textureRes[i] = texturesResData[i];
//	}
//}
//
//void AssetLoader::SaveMaterialAsset(const std::string& filePath, const MaterialAsset& inAsset)
//{
//	GroovyAssetHeader_Material header;
//	header.uuid = inAsset.uuid;
//	header.shaderID = inAsset.shaderID;
//	header.constBuffersSize = inAsset.constBuffersData.size();
//	header.numTextures = inAsset.textureRes.size();
//	header.type = ASSET_TYPE_MATERIAL;
//	memcpy(header.magic, GROOVY_ASSET_MAGIC, GROOVY_ASSET_MAGIC_SIZE);
//
//	Buffer data;
//	data.resize
//	(
//		sizeof(GroovyAssetHeader_Material) + // header
//		inAsset.constBuffersData.size() +  // buffers data
//		inAsset.textureRes.size() * sizeof(AssetUUID) // textures res
//	);
//
//	byte* dataPtr = data.data();
//	memcpy(dataPtr, &header, sizeof(GroovyAssetHeader_Material));
//	dataPtr += sizeof(GroovyAssetHeader_Material);
//	memcpy(dataPtr, inAsset.constBuffersData.data(), inAsset.constBuffersData.size());
//	dataPtr += inAsset.constBuffersData.size();
//	memcpy(dataPtr, inAsset.textureRes.data(), inAsset.textureRes.size() * sizeof(AssetUUID));
//
//	FileSystem::WriteFileBinary(filePath, data);
//}