#pragma once
 
#include "assets.h"
#include "renderer/api/texture.h"

class AssetLoader
{
public:
	static Texture* LoadTexture(const Buffer& textureFile);
};