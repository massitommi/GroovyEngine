#include "project.h"
#include "classes/object_serializer.h"
#include "platform/filesystem.h"

GROOVY_CLASS_IMPL(GroovyProject)
	GROOVY_REFLECT(mStartupScene)
GROOVY_CLASS_END()

void GroovyProject::BuildPaths(const char* projectFilePath)
{
	mProjFilePath = projectFilePath;
	mProjName = mProjFilePath.filename().replace_extension().string();
	mAssetsPath = mProjFilePath.parent_path() / "assets";
	mAssetRegistryPath = mAssetsPath / "assetregistry";
}

void GroovyProject::Load()
{
	Buffer fileData;
	FileSystem::ReadFileBinary(mProjFilePath.string(), fileData);

	if (fileData.size())
	{
		PropertyPack pack;
		BufferView fileDataView(fileData);
		ObjectSerializer::DeserializePropertyPack(GroovyProject::StaticClass(), fileDataView, pack);
		ObjectSerializer::DeserializePropertyPackData(pack, this);
	}
}

void GroovyProject::Save()
{
	PropertyPack pack;
	ObjectSerializer::CreatePropertyPack(this, GroovyProject::StaticCDO(), pack);
	DynamicBuffer serializedPack;
	ObjectSerializer::SerializePropertyPack(pack, serializedPack);
	FileSystem::WriteFileBinary(mProjFilePath.string(), serializedPack);
}
