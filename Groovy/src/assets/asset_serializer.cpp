#include "asset_serializer.h"
#include "platform/filesystem.h"

void AssetSerializer::SerializeMaterial(Material* mat, const std::string& filePath)
{
	check(mat);
	
	DynamicBuffer fileData;
	mat->Serialize(fileData);

	FileSystem::WriteFileBinary(filePath, fileData);
}
