#include "asset_loader.h"

Texture* AssetLoader::LoadTexture(const Buffer& textureFile)
{
	if (!assetUtils::IsGroovyAsset_Texture(textureFile))
		return nullptr;
	TextureHeader* header = (TextureHeader*)textureFile.data();
	checkf(header->channels == 4, "Only RGBA textures are currently supported");
	TextureSpec spec;
	spec.width = header->width;
	spec.height = header->height;
	spec.format = COLOR_FORMAT_R8G8B8A8_UNORM;
	return Texture::Create(spec, textureFile.data() + sizeof(TextureHeader), textureFile.size() - sizeof(TextureHeader));
}
