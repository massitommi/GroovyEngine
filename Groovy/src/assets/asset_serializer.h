#pragma once

#include "renderer/material.h"
#include "renderer/mesh.h"

class AssetSerializer
{
public:
	static void SerializeMaterial(MaterialAssetFile* materialAsset, const std::string& filePath);
};