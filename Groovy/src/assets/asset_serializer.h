#pragma once

#include "renderer/material.h"
#include "renderer/mesh.h"

class AssetSerializer
{
public:
	static void SerializeMaterial(Material* mat, const std::string& filePath);
	static void SerializeMesh(Mesh* mesh, const std::string& filePath);
};
