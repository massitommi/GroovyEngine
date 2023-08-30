#include "core/core.h"

class CORE_API AssetSerializer
{
public:
	static void SerializeGenericAsset(class AssetInstance* asset, const std::string& filePath);
	static void SerializeGenericAsset(class AssetInstance* asset);

	static void SerializeMesh(class Mesh* mesh, const std::string& filePath);
	static void SerializeMesh(class Mesh* mesh);
};