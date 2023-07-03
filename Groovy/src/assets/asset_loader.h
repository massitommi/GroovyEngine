#pragma once

#include "asset.h"

// raw asset loader
class AssetLoader
{
public:

	static class Texture* LoadTexture(const std::string& filePath);
	static class Shader* LoadShader(const std::string& filePath);

	static void LoadMaterial(class Material* material);
	static void LoadMesh(class Mesh* mesh);
	static void LoadBlueprint(class Blueprint* bp);
};