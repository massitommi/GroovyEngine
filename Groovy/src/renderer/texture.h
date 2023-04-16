#pragma once

#include "core/core.h"

enum ETextureFormat
{
	TEXTURE_FORMAT_RGBA
};

struct TextureSpec
{
	uint32 width, height;
	ETextureFormat format;
};

class Texture
{
public:
	virtual ~Texture() = default;
	
	virtual void Bind(uint32 slot) = 0;
	virtual uint64 GetRendererID() const = 0;
	virtual void SetData(void* data, size_t size) = 0;
	virtual TextureSpec GetSpecs() const = 0;

	static Texture* Create(TextureSpec specs, void* data, size_t size);
};

