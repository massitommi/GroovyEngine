#pragma once

#include "asset.h"
#include "renderer/api/texture.h"
#include "renderer/api/shader.h"
#include "renderer/material.h"
#include "renderer/mesh.h"

class AssetLoader
{
public:
	static Texture* LoadTexture(const std::string& filePath);
	static Shader* LoadShader(const std::string& filePath);
};