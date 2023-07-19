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

Scene::~Scene()
{
	for (Actor* actor : mActors)
		mActorKillQueue.push_back(actor);
	FinishDestroyingActors();
}

void Scene::Load()
{
	AssetLoader::LoadGenericAsset(this);
	mLoaded = true;
}

void Scene::Unload()
{
}

void Scene::Save()
{
	AssetSerializer::SerializeGenericAsset(this);
}

void Scene::Serialize(DynamicBuffer& fileData) const
{
	fileData.push<uint32>(mActors.size());
	for (Actor* actor : mActors)
	{
		fileData.push<std::string>(actor->mName);
		fileData.push<AssetUUID>(actor->mTemplate ? actor->mTemplate->GetUUID() : 0);
		fileData.push<Transform>(actor->GetTransform());

		ActorPack pack;
		ActorSerializer::CreateActorPack(actor, pack);
		ActorSerializer::SerializeActorPack(pack, fileData);
	}
}

void Scene::Deserialize(BufferView fileData)
{
	uint32 actorsCount = fileData.read<uint32>();
	for (uint32 i = 0; i < actorsCount; i++)
	{
		std::string name = fileData.read<std::string>();
		AssetUUID bpUUID = fileData.read<AssetUUID>();
		Transform transform = fileData.read<Transform>();

		ActorPack pack;
		ActorSerializer::DeserializeActorPack(fileData, pack);

		if (pack.actorClass)
		{
			ActorBlueprint* bp = nullptr;
			if (bpUUID)
			{
				bp = AssetManager::Get<ActorBlueprint>(bpUUID);
				if (!bp)
					continue;
			}
			
			Actor* newActor = SpawnActor(pack.actorClass, bp);
			newActor->mTransform = transform;
			newActor->mName = name;
			ActorSerializer::DeserializeActorPackData(pack, newActor);
		}
	}
}

void Scene::Initialize()
{
	for (Actor* actor : mActors)
		actor->InitializeComponents();
}

void Scene::Uninitialize()
{
	for (Actor* actor : mActors)
		actor->UninitializeComponents();
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
	newActor->mScene = this;

	if (bp)
	{
		bp->CopyProperties(newActor);
		newActor->mTemplate = bp;
	}

	mActors.push_back(newActor);

	return newActor;
}

#if WITH_EDITOR

bool Scene::Editor_FixDependencyDeletion(AssetHandle assetToBeDeleted)
{
	return false;
	bool wasLoaded = mLoaded;

	if (!wasLoaded)
		Load();

	std::vector<Actor*> removeList;

	if (assetToBeDeleted.type == ASSET_TYPE_ACTOR_BLUEPRINT)
	{
		for (Actor* actor : mActors)
		{
			if (actor->mTemplate == assetToBeDeleted.instance)
			{
				removeList.push_back(actor);
			}
		}

		for (Actor* actor : removeList)
		{
			Editor_DeleteActor(actor);
		}

	}

	if (!wasLoaded)
	{
		Save();
		// unload
		for (Actor* actor : mActors)
			ObjectAllocator::Destroy(actor);
		mActors.clear();
	}

	return removeList.size();
}

void Scene::Editor_SpawnActor(GroovyClass* actorClass, ActorBlueprint* bp)
{
	Actor* newActor = SpawnActor(actorClass, bp);
	newActor->InitializeComponents();
}

void Scene::Editor_DeleteActor(Actor* actor)
{
	// free memory
	ObjectAllocator::Destroy(actor);

	// remove from actors list
	auto it = std::find(mActors.begin(), mActors.end(), actor);
	mActors.erase(it);
}

#endif

void Scene::DestroyActor(Actor* actor)
{
	check(actor);

	// notify the actor
	actor->Destroy();

	// remove from alive actors
	auto it = std::find(mActorsPlaying.begin(), mActorsPlaying.end(), actor);
	mActorsPlaying.erase(it);

	// push into kill queue
	mActorKillQueue.push_back(actor);
}

void Scene::Tick(float deltaTime)
{
	for (Actor* actor : mActors)
	{
		actor->Tick(deltaTime);
	}
	
	if (mActorKillQueue.size())
		FinishDestroyingActors();
}

void Scene::FinishDestroyingActors()
{
	for (Actor* actor : mActorKillQueue)
	{
		// free memory
		ObjectAllocator::Destroy(actor);

		// remove from actors list
		auto it = std::find(mActors.begin(), mActors.end(), actor);
		mActors.erase(it);
	}

	mActorKillQueue.clear();
}