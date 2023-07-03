#include "blueprint.h"
#include "class_db.h"
#include "assets/asset_serializer.h"
#include "assets/asset_loader.h"

Blueprint::Blueprint()
	: mGroovyClass(nullptr), mUUID(0), mLoaded(false)
{
}

void Blueprint::Load()
{
	AssetLoader::LoadBlueprint(this);
	mLoaded = true;
}

void Blueprint::Save()
{
	AssetSerializer::SerializeBlueprint(this);
}

void Blueprint::Serialize(DynamicBuffer& fileData)
{
	checkslow(mGroovyClass);

	fileData.push<std::string>(mGroovyClass->name);
	ObjectSerializer::SerializePropertyPack(mPropertyPack, fileData);
}

void Blueprint::Deserialize(BufferView fileData)
{
	mGroovyClass = nullptr;
	mPropertyPack.data.free();
	mPropertyPack.desc.clear();

	extern ClassDB gClassDB;
	std::string className = fileData.read<std::string>();
	GroovyClass* gClass = gClassDB[className];
	if (gClass)
	{
		mGroovyClass = gClass;
		ObjectSerializer::DeserializePropertyPack(gClass, fileData, mPropertyPack);
	}
	else
	{
		checkslowf(0, "Class not found?!?!");
	}
}

void Blueprint::SetData(GroovyObject* obj)
{
	check(obj);
	check(obj->GetClass() == mGroovyClass);

	mPropertyPack.desc.clear();
	mPropertyPack.data.free();

	ObjectSerializer::CreatePropertyPack(obj, mGroovyClass->cdo, mPropertyPack);
}

bool Blueprint::Editor_FixDependencyDeletion(AssetHandle assetToBeDeleted)
{
	bool found = false;
	size_t propDataOffset = 0;
	for (const PropertyDesc& p : mPropertyPack.desc)
	{
		bool assetTypeProp = p.classProp->type == PROPERTY_TYPE_ASSET_REF;
		bool sameAssetType = p.classProp->param1 == assetToBeDeleted.type;
		if (assetTypeProp && sameAssetType)
		{
			AssetUUID* propUUIDs = (AssetUUID*)(mPropertyPack.data.data() + propDataOffset);
			for (uint32 i = 0; i < p.arrayCount; i++)
			{
				if (propUUIDs[i] == assetToBeDeleted.uuid)
				{
					propUUIDs[i] = 0;
					found = true;
				}
			}
		}
		propDataOffset += p.sizeBytes;
	}
	return found;
}