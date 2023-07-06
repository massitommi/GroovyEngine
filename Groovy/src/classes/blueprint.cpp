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
	AssetLoader::LoadGenericAsset(this);
	mLoaded = true;
}

void Blueprint::Save()
{
	AssetSerializer::SerializeGenericAsset(this);
}

void Blueprint::Serialize(DynamicBuffer& fileData) const
{
	if (!mGroovyClass)
	{
		// TODO: Warning: This blueprint cannot be serialized because the class is not set
		return;
	}

	fileData.push<std::string>(mGroovyClass->name);
	ObjectSerializer::SerializePropertyPack(mPropertyPack, fileData);
}

void Blueprint::Deserialize(BufferView fileData)
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

void Blueprint::Clear()
{
	mGroovyClass = nullptr;
	mPropertyPack.desc.clear();
	mPropertyPack.data.free();
}

void Blueprint::SetData(GroovyObject* obj)
{
	checkslow(obj);

	Clear();
	mGroovyClass = obj->GetClass();

	ObjectSerializer::CreatePropertyPack(obj, mGroovyClass->cdo, mPropertyPack);
}

void Blueprint::CopyProperties(GroovyObject* obj)
{
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

bool Blueprint::Editor_FixDependencyDeletion(AssetHandle assetToBeDeleted)
{
	return DepencyDeletionFix(assetToBeDeleted, mPropertyPack);
}

#include "gameframework/actor.h"
#include "gameframework/actorcomponent.h"

ActorBlueprint::ActorBlueprint()
	: mActorClass(nullptr), mUUID(0), mLoaded(false)
{
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

static void SerializeActorComponent(const ComponentPack& compPack, DynamicBuffer& outFileData)
{
	outFileData.push<std::string>(compPack.name);											// component name
	outFileData.push<std::string>(compPack.gClass->name);									// component class name
	outFileData.push<size_t>(compPack.pack.data.used());									// component serialized data size
	ObjectSerializer::SerializePropertyPack(compPack.pack, outFileData);					// actual serialized data
}

static void DeserializeActorComponent(BufferView& fileData, std::vector<ComponentPack>& outCompPack)
{
	extern ClassDB gClassDB;
	std::string compName = fileData.read<std::string>();									// component name
	std::string compClassName = fileData.read<std::string>();								// component class name
	size_t compDataSize = fileData.read<size_t>();											// component serialized data size
	GroovyClass* compClass = gClassDB[compClassName];

	if (compClass)
	{
		ComponentPack& compPack = outCompPack.emplace_back();
		compPack.name = compName;
		compPack.gClass = compClass;
		ObjectSerializer::DeserializePropertyPack(compClass, fileData, compPack.pack);		// actual serialized data
	}
	else
	{
		// TODO: Warning : This ActorComponent class does not exist anymore
		fileData.advance(compDataSize + sizeof(uint32) /* property count */);				// actual serialize data
	}
}

void ActorBlueprint::Serialize(DynamicBuffer& fileData) const
{
	if (!mActorClass)
	{
		// TODO: Warning: This blueprint cannot be serialized because the Actor class is not set
		return;
	}

	// serialize actor data
	fileData.push<std::string>(mActorClass->name);
	ObjectSerializer::SerializePropertyPack(mActorPropertyPack, fileData);
	// serialize native components
	{
		fileData.push<uint32>(mNativeComponents.size());
		for (const ComponentPack& comp : mNativeComponents)
			SerializeActorComponent(comp, fileData);
	}
	// serialize editor components
	{
		fileData.push<uint32>(mEditorComponents.size());
		for (const ComponentPack& comp : mEditorComponents)
			SerializeActorComponent(comp, fileData);
	}
}

void ActorBlueprint::Deserialize(BufferView fileData)
{
	if (!fileData.remaining())
	{
		// TODO: Warning: Empty blueprint file
		return;
	}

	extern ClassDB gClassDB;
	std::string actorClass = fileData.read<std::string>();
	mActorClass = gClassDB[actorClass];
	if (mActorClass)
	{
		// deserialize actor data
		ObjectSerializer::DeserializePropertyPack(mActorClass, fileData, mActorPropertyPack);
		// deserialize native components
		{
			uint32 compCount = fileData.read<uint32>();
			mNativeComponents.reserve(compCount);
			for(uint32 i = 0; i < compCount; i++)
				DeserializeActorComponent(fileData, mNativeComponents);
		}
		// deserialize editor components
		{
			uint32 compCount = fileData.read<uint32>();
			mEditorComponents.reserve(compCount);
			for (uint32 i = 0; i < compCount; i++)
				DeserializeActorComponent(fileData, mEditorComponents);
		}
	}
	else
	{
		// TODO: Warning : This Actor class does not exist anymore
	}
}

void ActorBlueprint::Clear()
{
	mActorClass = nullptr;
	mActorPropertyPack.desc.clear();
	mActorPropertyPack.data.free();

	mNativeComponents.clear();
	mEditorComponents.clear();
}

void ActorBlueprint::SetData(Actor* actor)
{
	checkslow(actor);

	Clear();
	mActorClass = actor->GetClass();
	// actor
	ObjectSerializer::CreatePropertyPack(actor, mActorClass->cdo, mActorPropertyPack);
	// native components
	{
		const std::vector<ActorComponent*>& comps = actor->GetNativeComponents();
		mNativeComponents.resize(comps.size());
		for (uint32 i = 0; i < comps.size(); i++)
		{
			mNativeComponents[i].name = comps[i]->GetName();
			mNativeComponents[i].gClass = comps[i]->GetClass();
			ObjectSerializer::CreatePropertyPack(comps[i], comps[i]->GetCDO(), mNativeComponents[i].pack);
		}
	}
	// editor components
	{
		const std::vector<ActorComponent*>& comps = actor->GetEditorComponents();
		mEditorComponents.resize(comps.size());
		for (uint32 i = 0; i < comps.size(); i++)
		{
			mEditorComponents[i].name = comps[i]->GetName();
			mEditorComponents[i].gClass = comps[i]->GetClass();
			ObjectSerializer::CreatePropertyPack(comps[i], comps[i]->GetCDO(), mEditorComponents[i].pack);
		}
	}
}

void ActorBlueprint::CopyProperties(Actor* actor)
{
	checkslow(actor);
	// copy actor data
	ObjectSerializer::DeserializePropertyPackData(mActorPropertyPack, actor);
	// copy native components data
	{
		for (const ComponentPack& pack : mNativeComponents)
		{
			ActorComponent* compInstance = actor->GetComponent(pack.name);

#if !BUILD_SHIPPING
			if (!compInstance)
			{
				// TODO: Warning: Component with this name was not found; Please sanitize this blueprint, in shipping this will cause a crash,
				// because for performance reasons we wont check for this kinda stuff
				continue;
			}

			if (compInstance->GetClass() != pack.gClass)
			{
				// TODO: Warning: Component class mismatch; Please sanitize this blueprint, in shipping this will cause a crash,
				// because for performance reasons we wont check for this kinda stuff
				continue;
			}
#endif
			ObjectSerializer::DeserializePropertyPackData(pack.pack, compInstance);
		}
	}
	// add editor components
	{
		for (const ComponentPack& pack : mEditorComponents)
		{
#if !BUILD_SHIPPING
			{
				ActorComponent* compInstance = actor->GetComponent(pack.name);
				if (compInstance)
				{
					// TODO: Warning: Component already exists, ignored; Please sanitize this blueprint, in shipping this will cause a crash,
				// because for performance reasons we wont check for this kinda stuff
					continue;
				}
			}
#endif
			ActorComponent* compInstance = actor->__internal_AddEditorComponent(pack.gClass, pack.name);
			ObjectSerializer::DeserializePropertyPackData(pack.pack, compInstance);
		}
	}
}

bool ActorBlueprint::Editor_FixDependencyDeletion(AssetHandle assetToBeDeleted)
{
	bool actorFixed = false;
	bool nativeComponentsFixed = false;
	bool editorComponentsFixed = false;

	actorFixed = DepencyDeletionFix(assetToBeDeleted, mActorPropertyPack);

	for (ComponentPack& pack : mNativeComponents)
	{
		if (DepencyDeletionFix(assetToBeDeleted, pack.pack))
		{
			nativeComponentsFixed = true;
		}
	}

	for (ComponentPack& pack : mEditorComponents)
	{
		if (DepencyDeletionFix(assetToBeDeleted, pack.pack))
		{
			editorComponentsFixed = true;
		}
	}

	return actorFixed || nativeComponentsFixed || editorComponentsFixed;
}
