#include "blueprint.h"
#include "classes/class_db.h"
#include "runtime/object_allocator.h"
#include "assets/asset_serializer.h"
#include "assets/asset_loader.h"

ObjectBlueprint::ObjectBlueprint()
	: mGroovyClass(nullptr), mDefaultObject(nullptr), mUUID(0), mLoaded(false)
{
}

ObjectBlueprint::~ObjectBlueprint()
{
	if (mDefaultObject)
		ObjectAllocator::Destroy(mDefaultObject);
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
		GROOVY_LOG_WARN("%s Serialization skipped, class is NULL", GetAssetName().c_str());
		return;
	}

	fileData.push<std::string>(mGroovyClass->name);
	ObjectSerializer::SerializePropertyPack(mPropertyPack, fileData);
}

void ObjectBlueprint::Deserialize(BufferView fileData)
{
	if (!fileData.remaining())
	{
		GROOVY_LOG_WARN("%s file is empty, deserialization skipped", GetAssetName().c_str());
		return;
	}

	extern ClassDB gClassDB;
	std::string className = fileData.read<std::string>();
	mGroovyClass = gClassDB[className];
	if (mGroovyClass)
	{
		ObjectSerializer::DeserializePropertyPack(mGroovyClass, fileData, mPropertyPack);
		mDefaultObject = ObjectAllocator::Instantiate(mGroovyClass);
		ObjectSerializer::DeserializePropertyPackData(mPropertyPack, mDefaultObject);
	}
	else
	{
		GROOVY_LOG_WARN("%s class '%s' not found in class DB", GetAssetName().c_str(), className.c_str());
	}
}

void ObjectBlueprint::CopyProperties(GroovyObject* obj)
{
	if (!mGroovyClass)
	{
		GROOVY_LOG_WARN("%s CopyProperties skipped, class is NULL", GetAssetName().c_str());
		return;
	}

	check(obj);
	check(obj->GetClass() == mGroovyClass);

	ObjectSerializer::DeserializePropertyPackData(mPropertyPack, obj);
}

void ObjectBlueprint::SetupEmpty(GroovyClass* objClass)
{
	mPropertyPack.desc.clear();
	mPropertyPack.data.free();

	mGroovyClass = objClass;

	mDefaultObject = ObjectAllocator::Instantiate(objClass);
}

void ObjectBlueprint::RebuildPack(GroovyObject* basedOn)
{
	checkslow(basedOn);
	checkslow(basedOn->GetClass() == mGroovyClass);

	// cleanup
	mPropertyPack.desc.clear();
	mPropertyPack.data.free();
	ObjectAllocator::Destroy(mDefaultObject);

	// rebuild pack
	ObjectSerializer::CreatePropertyPack(basedOn, basedOn->GetCDO(), mPropertyPack);

	// create new default object and copy properties
	mDefaultObject = ObjectAllocator::Instantiate(mGroovyClass);
	ObjectSerializer::DeserializePropertyPackData(mPropertyPack, mDefaultObject);
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

#if WITH_EDITOR

bool ObjectBlueprint::Editor_FixDependencyDeletion(AssetHandle assetToBeDeleted)
{
	return DepencyDeletionFix(assetToBeDeleted, mPropertyPack);
}

#endif

ActorBlueprint::ActorBlueprint()
	: mUUID(0), mLoaded(false)
{
	mActorPack.actorClass = nullptr;
	mDefaultActor = nullptr;
}

ActorBlueprint::~ActorBlueprint()
{
	if(mDefaultActor)
		ObjectAllocator::Destroy(mDefaultActor);
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
		GROOVY_LOG_WARN("%s Serialization skipped, actor class is NULL", GetAssetName().c_str());
		return;
	}

	ActorSerializer::SerializeActorPack(mActorPack, fileData);
}

void ActorBlueprint::Deserialize(BufferView fileData)
{
	if (!fileData.remaining())
	{
		GROOVY_LOG_WARN("%s file is empty, deserialization skipped", GetAssetName().c_str());
		return;
	}

	ActorSerializer::DeserializeActorPack(fileData, mActorPack);

	if (!mActorPack.actorClass)
	{
		GROOVY_LOG_WARN("%s actor class is NULL, skipping deserialization", GetAssetName().c_str());
		return;
	}

	mDefaultActor = ObjectAllocator::Instantiate<Actor>(mActorPack.actorClass);
	ActorSerializer::DeserializeActorPackData(mActorPack, mDefaultActor);
}

void ActorBlueprint::CopyProperties(Actor* actor)
{
	check(actor);
	
	if (mActorPack.actorClass)
	{
		ActorSerializer::DeserializeActorPackData(mActorPack, actor);
	}
	else
	{
		GROOVY_LOG_WARN("%s actor class is NULL, CopyProperties skipped", GetAssetName().c_str());
	}
}

void ActorBlueprint::SetupEmpty(GroovyClass* actorClass)
{
	mActorPack.actorClass = actorClass;

	mDefaultActor = ObjectAllocator::Instantiate<Actor>(actorClass);
}

void ActorBlueprint::RebuildPack(Actor* basedOn)
{
	checkslow(basedOn);
	checkslow(basedOn->GetClass() == mDefaultActor->GetClass());

	// cleanup
	mActorPack.actorProperties.desc.clear();
	mActorPack.actorProperties.data.free();
	mActorPack.actorComponents.clear();
	ObjectAllocator::Destroy(mDefaultActor);

	// rebuild pack
	ActorSerializer::CreateActorPack(basedOn, mActorPack);

	// create new default object and copy properties
	mDefaultActor = ObjectAllocator::Instantiate<Actor>(basedOn->GetClass());
	ActorSerializer::DeserializeActorPackData(mActorPack, mDefaultActor);
}

#if WITH_EDITOR

bool ActorBlueprint::Editor_FixDependencyDeletion(AssetHandle assetToBeDeleted)
{
	bool actorFixed = false;
	bool nativeComponentsFixed = false;
	bool editorComponentsFixed = false;

	actorFixed = DepencyDeletionFix(assetToBeDeleted, mActorPack.actorProperties);

	for (ComponentPack& pack : mActorPack.actorComponents)
	{
		if (DepencyDeletionFix(assetToBeDeleted, pack.componentProperties))
		{
			nativeComponentsFixed = true;
		}
	}

	return actorFixed || nativeComponentsFixed || editorComponentsFixed;
}

#endif