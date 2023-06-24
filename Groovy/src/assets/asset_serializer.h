#include "core/core.h"

class AssetSerializer
{
public:
	static void SerializeMaterial(class Material* mat, const std::string& filePath);
	static void SerializeMesh(class Mesh* mesh, const std::string& filePath);
};