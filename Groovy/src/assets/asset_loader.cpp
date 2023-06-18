#include "asset_loader.h"
#include "platform/filesystem.h"

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