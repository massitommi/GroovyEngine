#include "scene.h"
#include "actor_serializer.h"
#include "runtime/object_allocator.h"
#include "assets/asset_loader.h"
#include "assets/asset_serializer.h"
#include "assets/asset_manager.h"

Scene::Scene()
	: mUUID(0), mLoaded(false)
{
}

void Scene::Load()
{
	AssetLoader::LoadGenericAsset(this);
	mLoaded = true;
}

void Scene::Save()
{
	AssetSerializer::SerializeGenericAsset(this);
}

void Scene::Serialize(DynamicBuffer& fileData) const
{
	fileData.push<uint32>(mGenericActors.size());
	for (Actor* actor : mGenericActors)
	{
		ActorPack pack;
		ActorSerializer::CreateActorPack(actor, (Actor*)actor->GetCDO(), pack);
		ActorSerializer::SerializeActorPack(pack, fileData);
	}

	fileData.push<uint32>(mBPActors.size());
	for (auto [actor, bp] : mBPActors)
	{
		fileData.push<AssetUUID>(bp->GetUUID());
		ActorPack pack;
		ActorSerializer::CreateActorPack(actor, bp->GetDefaultActor(), pack);
		ActorSerializer::SerializeActorPack(pack, fileData);
	}
}

void Scene::Deserialize(BufferView fileData)
{
	uint32 genericActorsCount = fileData.read<uint32>();
	for (uint32 i = 0; i < genericActorsCount; i++)
	{
		ActorPack pack;
		ActorSerializer::DeserializeActorPack(fileData, pack);

		if (pack.actorClass)
		{
			Actor* newActor = SpawnActor<Actor>(pack.actorClass);
			ActorSerializer::DeserializeActorPackData(pack, newActor);
			mGenericActors.push_back(newActor);
		}
		else
		{
			// Warning: this file is not up to date, this actor class does not exist anymore
		}
	}

	uint32 bpActors = fileData.read<uint32>();
	for (uint32 i = 0; i < bpActors; i++)
	{
		AssetUUID bpUUID = fileData.read<AssetUUID>();
		ActorBlueprint* bpInstance = AssetManager::Get<ActorBlueprint>(bpUUID);

		ActorPack pack;
		ActorSerializer::DeserializeActorPack(fileData, pack);

		if (pack.actorClass && pack.actorClass == bpInstance->GetActorClass())
		{
			ActorBP newActorBP;
			newActorBP.instance = SpawnActor<Actor>(pack.actorClass);
			newActorBP.bp = bpInstance;
			ActorSerializer::DeserializeActorPackData(pack, newActorBP.instance);
			mBPActors.push_back(newActorBP);
		}
		else
		{
			// Warning: this file is not up to date, this actor class does not exist anymore or does not match the blueprint class
		}
	}
}

bool Scene::Editor_FixDependencyDeletion(AssetHandle assetToBeDeleted)
{
	return false;
}

Actor* Scene::SpawnActor(GroovyClass* actorClass, ActorBlueprint* bp)
{
	check(actorClass);
	check(classUtils::IsA(actorClass, Actor::StaticClass()));

	if (bp)
	{
		check(bp->GetActorClass() == actorClass);
	}

	Actor* newActor = ObjectAllocator::Instantiate<Actor>(actorClass);

	if (bp)
	{
		bp->CopyProperties(newActor);
	}

	mActors.push_back(newActor);

	return newActor;
}


void Scene::DestroyActor(Actor* actor)
{
	check(actor);

	auto it = std::find(mActors.begin(), mActors.end(), actor);

	if (it != mActors.end())
	{
		ObjectAllocator::Destroy(actor);
		mActors.erase(it);
	}
}