#include "asset_loader.h"
#include "platform/filesystem.h"
#include "asset_manager.h"
#include "project/project.h"

#include "renderer/api/texture.h"
#include "renderer/api/shader.h"

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

void AssetLoader::LoadGenericAsset(AssetInstance* asset)
{
	extern GroovyProject gProj;
	AssetHandle handle = AssetManager::Get(asset->GetUUID());
	std::string absPath = (gProj.GetAssetsPath() / handle.name).string();
	Buffer fileData;
	FileSystem::ReadFileBinary(absPath, fileData);
	asset->Deserialize(fileData);
}