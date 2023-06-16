#include "asset_loader.h"
#include "platform/filesystem.h"
#include "asset_manager.h"
#include "asset_manager.h"
#include "project/project.h"
#include "classes/object_serializer.h"

extern Texture* DEFAULT_TEXTURE;
extern Shader* DEFAULT_SHADER;
extern Material* DEFAULT_MATERIAL;

extern Project gProj;

Texture* AssetLoader::LoadTexture(const std::string& filePath)
{
	Buffer data;
	FileSystem::ReadFileBinary(filePath, data);
	TextureAssetHeader* header = data.as<TextureAssetHeader>();
	
	TextureSpec spec;
	spec.width = header->width;
	spec.height = header->height;
	spec.format = header->format;
	
	return Texture::Create(spec, data.data() + sizeof(TextureAssetHeader), data.size() - sizeof(TextureAssetHeader));
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

void AssetLoader::LoadMaterial(const std::string& filePath, Material* mat)
{
	checkslow(mat);
	
	Buffer fileData;
	FileSystem::ReadFileBinary(filePath, fileData);

	MaterialAssetFile materialAsset;
	ObjectSerializer::DeserializeSimpleObject(&materialAsset, fileData);

	materialAsset.DeserializeOntoMaterial(mat);
}

void AssetLoader::LoadMaterial(Material* mat)
{
	checkslow(mat);

	AssetHandle handle = AssetManager::Get(mat->GetUUID());
	std::string filePath = (gProj.assets / handle.name).string();

	LoadMaterial(filePath, mat);
}














//
//Material* AssetLoader::LoadMaterial(const std::string& filePath)
//{
//	Buffer fileData;
//	FileSystem::ReadFileBinary(filePath, fileData);
//
//	const MaterialAssetHeader* header = fileData.as<MaterialAssetHeader>();
//
//	const byte* dataPtr = fileData.data() + sizeof(MaterialAssetHeader);
//
//	// shader
//	Shader* shaderInstance = DEFAULT_SHADER;
//	if(header->shaderID)
//		shaderInstance = AssetManager::Get<Shader>(header->shaderID);
//	
//	check(shaderInstance); // not loaded yet???
//	
//	Material* mat = new Material(shaderInstance);
//
//	// const buffers data
//	memcpy(mat->mConstBuffersData.data(), dataPtr, header->constBuffersSize);
//	dataPtr += header->constBuffersSize;
//
//	// textures
//	AssetUUID* textureIDptr = (AssetUUID*)dataPtr;
//	for (Texture*& tex : mat->mTextures)
//	{
//		Texture* texture = DEFAULT_TEXTURE;
//		if (*textureIDptr)
//			texture = AssetManager::Get<Texture>(*textureIDptr);
//		
//		check(texture);
//		
//		tex = texture;
//
//		textureIDptr++;
//	}
//
//	return mat;
//}
//
//Mesh* AssetLoader::LoadMesh(const std::string& filePath)
//{
//	Buffer fileData;
//	FileSystem::ReadFileBinary(filePath, fileData);
//	MeshAssetHeader* header = fileData.as<MeshAssetHeader>();
//
//	size_t vertexBufferSize = header->vertexBufferSize;
//	size_t indexBufferSize = header->indexBufferSize;
//	byte* vertexBufferData = fileData.data() + sizeof(MeshAssetHeader);
//	byte* indexBufferData = vertexBufferData + vertexBufferSize;
//	byte* submeshData = indexBufferData + indexBufferSize;
//	byte* materialData = submeshData + header->submeshCount * sizeof(SubmeshData);
//
//	VertexBuffer* vertexBuffer = VertexBuffer::Create(vertexBufferSize, vertexBufferData, sizeof(MeshVertex));
//	IndexBuffer* indexBuffer = IndexBuffer::Create(indexBufferSize, indexBufferData);
//
//	std::vector<SubmeshData> submeshes;
//	submeshes.resize(header->submeshCount);
//	memcpy(submeshes.data(), submeshData, header->submeshCount * sizeof(SubmeshData));
//
//	std::vector<Material*> materials;
//	materials.resize(header->submeshCount);
//	AssetUUID* materialsID = (AssetUUID*)materialData;
//	for (Material*& mat : materials)
//	{
//		Material* material = DEFAULT_MATERIAL;
//		if (*materialsID)
//			material = AssetManager::Get<Material>(*materialsID);
//		
//		check(material);
//
//		mat = material;
//
//		materialsID++;
//	}
//
//	return new Mesh(vertexBuffer, indexBuffer, submeshes, materials);
//}