#include "core/core.h"

class AssetSerializer
{
public:
	static void SerializeMaterial(class Material* material, const std::string& filePath);
	static void SerializeMaterial(class Material* material);
	static void SerializeMesh(class Mesh* mesh, const std::string& filePath);
	static void SerializeMesh(class Mesh* mesh);
	static void SerializeBlueprint(class Blueprint* bp, const std::string& filePath);
	static void SerializeBlueprint(class Blueprint* bp);
};