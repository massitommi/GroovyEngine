#include "blueprint.h"
#include "classes/class_db.h"
#include "assets/asset_serializer.h"
#include "assets/asset_loader.h"

ObjectBlueprint::ObjectBlueprint()
	: mGroovyClass(nullptr), mUUID(0), mLoaded(false)
{
}

void ObjectBlueprint::Load()
{
	AssetLoader::LoadGenericAsset(this);
	mLoaded = true;
}

void ObjectBlueprint::Save()
{
	AssetSerializer::SerializeGenericAsset(this);
}

void ObjectBlueprint::Serialize(DynamicBuffer& fileData) const
{
	if (!mGroovyClass)
	{
		// TODO: Warning: This blueprint cannot be serialized because the class is not set
		return;
	}

	fileData.push<std::string>(mGroovyClass->name);
	ObjectSerializer::SerializePropertyPack(mPropertyPack, fileData);
}

void ObjectBlueprint::Deserialize(BufferView fileData)
{
	if (!fileData.remaining())
	{
		// TODO: Warning: Empty blueprint file
		return;
	}

	extern ClassDB gClassDB;
	std::string className = fileData.read<std::string>();
	mGroovyClass = gClassDB[className];
	if (mGroovyClass)
	{
		ObjectSerializer::DeserializePropertyPack(mGroovyClass, fileData, mPropertyPack);
	}
	else
	{
		// TODO: Warning: This Class does not exist anymore
	}
}

void ObjectBlueprint::Clear()
{
	mGroovyClass = nullptr;
	mPropertyPack.desc.clear();
	mPropertyPack.data.free();
}

void ObjectBlueprint::SetData(GroovyObject* obj)
{
	checkslow(obj);

	Clear();
	mGroovyClass = obj->GetClass();

	ObjectSerializer::CreatePropertyPack(obj, mGroovyClass->cdo, mPropertyPack);
}

void ObjectBlueprint::CopyProperties(GroovyObject* obj)
{
	if (!mGroovyClass)
		return;

	check(obj);
	check(obj->GetClass() == mGroovyClass);

	ObjectSerializer::DeserializePropertyPackData(mPropertyPack, obj);
}

uint32 DepencyDeletionFix(const AssetHandle& assetToBeDeleted, PropertyPack& packToSanitize)
{
	uint32 fixed = 0;
	size_t propDataOffset = 0;
	for (const PropertyDesc& p : packToSanitize.desc)
	{
		bool assetTypeProp = p.classProp->type == PROPERTY_TYPE_ASSET_REF;
		bool sameAssetType = p.classProp->param1 == assetToBeDeleted.type;
		if (assetTypeProp && sameAssetType)
		{
			AssetUUID* propUUIDs = (AssetUUID*)(packToSanitize.data.data() + propDataOffset);
			for (uint32 i = 0; i < p.arrayCount; i++)
			{
				if (propUUIDs[i] == assetToBeDeleted.uuid)
				{
					propUUIDs[i] = 0;
					fixed++;
				}
			}
		}
		propDataOffset += p.sizeBytes;
	}
	return fixed;
}

bool ObjectBlueprint::Editor_FixDependencyDeletion(AssetHandle assetToBeDeleted)
{
	return DepencyDeletionFix(assetToBeDeleted, mPropertyPack);
}

#include "actor.h"
#include "actorcomponent.h"
#include "actor_serializer.h"

ActorBlueprint::ActorBlueprint()
	: mUUID(0), mLoaded(false)
{
	mActorPack.actorClass = nullptr;
}

void ActorBlueprint::Load()
{
	AssetLoader::LoadGenericAsset(this);
	mLoaded = true;
}

void ActorBlueprint::Save()
{
	AssetSerializer::SerializeGenericAsset(this);
}

void ActorBlueprint::Serialize(DynamicBuffer& fileData) const
{
	if (!mActorPack.actorClass)
	{
		// TODO: Warning: This blueprint cannot be serialized because the Actor class is not set
		return;
	}

	ActorSerializer::SerializeActorPack(mActorPack, fileData);
}

void ActorBlueprint::Deserialize(BufferView fileData)
{
	if (!fileData.remaining())
	{
		// TODO: Warning: Empty blueprint file
		return;
	}

	ActorSerializer::DeserializeActorPack(fileData, mActorPack);
}

void ActorBlueprint::Clear()
{
	mActorPack.actorClass = nullptr;
	mActorPack.actorProperties.desc.clear();
	mActorPack.actorProperties.data.free();

	mActorPack.nativeComponents.clear();
	mActorPack.editorComponents.clear();
}

void ActorBlueprint::SetData(Actor* actor)
{
	checkslow(actor);

	Clear();
	mActorPack.actorClass = actor->GetClass();

	ActorSerializer::CreateActorPack(actor, (Actor*)actor->GetCDO(), mActorPack);
}

void ActorBlueprint::CopyProperties(Actor* actor)
{
	check(actor);
	
	if (mActorPack.actorClass)
	{
		ActorSerializer::DeserializeActorPackData(mActorPack, actor);
	}
}

bool ActorBlueprint::Editor_FixDependencyDeletion(AssetHandle assetToBeDeleted)
{
	bool actorFixed = false;
	bool nativeComponentsFixed = false;
	bool editorComponentsFixed = false;

	actorFixed = DepencyDeletionFix(assetToBeDeleted, mActorPack.actorProperties);

	for (ComponentPack& pack : mActorPack.nativeComponents)
	{
		if (DepencyDeletionFix(assetToBeDeleted, pack.componentProperties))
		{
			nativeComponentsFixed = true;
		}
	}

	for (ComponentPack& pack : mActorPack.editorComponents)
	{
		if (DepencyDeletionFix(assetToBeDeleted, pack.componentProperties))
		{
			editorComponentsFixed = true;
		}
	}

	return actorFixed || nativeComponentsFixed || editorComponentsFixed;
}
