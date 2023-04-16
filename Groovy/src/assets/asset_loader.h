#pragma once

#include "assets.h"
#include "core/core.h"

#include "platform/filesystem.h"
#include "renderer/texture.h"

class AssetLoader
{
public:
	template<typename T>
	static T* Load(const String& path)
	{
		static_assert(0, "Unsupported type!");
		return nullptr;
	}

	template<>
	static Texture* Load(const String& path)
	{
		Buffer data;
		if (FileSystem::ReadFileBinary(path, data) != FILE_OPEN_RESULT_OK)
		{
			// todo return default texture
			checkslowf(0, "Groovy texture file not found! file: %s", *path);
			return nullptr;
		}
		// magic check
		if (!assetUtils::IsGroovyAsset<Texture>(data.data(), data.size()))
		{
			// todo return default texture
			checkslowf(0, "Not a groovy texture!");
			return nullptr;
		}
		GroovyTextureHeader* header = (GroovyTextureHeader*)data.data();
		TextureSpec spec;
		spec.width = header->width;
		spec.height = header->height;
		spec.format = TEXTURE_FORMAT_RGBA;
		if (header->channels != 4)
		{
			// todo return default texture
			return nullptr;
		}
		return Texture::Create(spec, data.data() + sizeof(GroovyTextureHeader), spec.width * spec.height * 4);
	}
};