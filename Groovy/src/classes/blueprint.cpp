#include "blueprint.h"

Blueprint::Blueprint(GroovyClass* inClass)
	: mGroovyClass(inClass), mUUID(0), mLoaded(false)
{
}

void Blueprint::Load()
{
}

void Blueprint::Save()
{
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