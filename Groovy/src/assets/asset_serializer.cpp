#include "asset_serializer.h"
#include "classes/object_serializer.h"
#include "platform/filesystem.h"


void AssetSerializer::SerializeMaterial(MaterialAssetFile* materialAsset, const std::string& filePath)
{
	checkslow(materialAsset);

	DynamicBuffer fileData;

	/*ObjectSerializer::SerializeSimpleObject
	(
		materialAsset,
		(GroovyObject*)MaterialAssetFile::GetStaticClass()->cdo,
		fileData
	);*/

	FileSystem::WriteFileBinary(filePath, fileData);
}
