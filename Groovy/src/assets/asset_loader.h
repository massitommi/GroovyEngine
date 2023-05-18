#pragma once

#include "asset.h"
#include "renderer/api/texture.h"
#include "renderer/api/shader.h"
#include "renderer/material.h"
#include "renderer/mesh.h"

// Raw asset loader
class AssetLoader
{
public:
	static Texture* LoadTexture(const std::string& filePath);
	static Shader* LoadShader(const std::string& filePath);
	static Material* LoadMaterial(const std::string& filePath);
	static Mesh* LoadMesh(const std::string& filePath);
};