#include "scene.h"
#include "actor_serializer.h"
#include "runtime/object_allocator.h"
#include "assets/asset_loader.h"
#include "assets/asset_serializer.h"
#include "assets/asset_manager.h"
#include "components/meshcomponent.h"

Scene::Scene()
	: mUUID(0), mLoaded(false), mCamera(nullptr)
{
}

Scene::~Scene()
{
	Clear();
}

void Scene::Load()
{
	AssetLoader::LoadGenericAsset(this);
	mLoaded = true;
}

void Scene::Unload()
{
	Clear();
	mLoaded = false;
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
			
			Actor* newActor = ConstructActor(pack.actorClass, bp);
			newActor->mTransform = transform;
			newActor->mName = name;
			ActorSerializer::DeserializeActorPackData(pack, newActor);
			newActor->InitializeComponents();
		}
	}
}

Actor* Scene::SpawnActor(GroovyClass* actorClass, ActorBlueprint* bp)
{
	Actor* newActor = ConstructActor(actorClass, bp);
	newActor->InitializeComponents();
	newActor->BeginPlay();

	if (newActor->mShouldTick)
		mActorTickQueue.push_back(newActor);
	
	return newActor;
}

#if WITH_EDITOR

#include "utils/reflection/reflection_utils.h"
bool Scene::Editor_FixDependencyDeletion(AssetHandle assetToBeDeleted)
{
	if (!mLoaded)
		return false;

	// remove blueprints instances if the asset type is ACTOR_BLUEPRINT
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
	
	// Replace assets's references with NULL
	uint32 refs = 0;
	AssetInstance* assetI = assetToBeDeleted.instance;
	AssetInstance* nullAsset = nullptr;
	for (Actor* actor : mActors)
	{
		reflectionUtils::ReplaceValueTypeProperty(actor, PROPERTY_TYPE_ASSET_REF, &assetI, &nullAsset);

		for (ActorComponent* comp : actor->mComponents)
		{
			reflectionUtils::ReplaceValueTypeProperty(comp, PROPERTY_TYPE_ASSET_REF, &assetI, &nullAsset);
		}
	}

	if (removeList.size() || refs)
	{
		Save();
		return true;
	}
	
	return false;
}

Actor* Scene::Editor_AddActor(GroovyClass* actorClass, ActorBlueprint* bp)
{
	Actor* newActor = ConstructActor(actorClass, bp);
	newActor->InitializeComponents();
	return newActor;
}

void Scene::Editor_DeleteActor(Actor* actor)
{
	check(actor);

	actor->UninitializeComponents();

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

	// push into kill queue
	mActorKillQueue.push_back(actor);

	// remove from actors
	auto it = std::find(mActors.begin(), mActors.end(), actor);
	mActors.erase(it);
}

void Scene::BeginPlay()
{
	for (Actor* actor : mActors)
	{
		if (actor->mShouldTick)
			mActorTickQueue.push_back(actor);

		actor->BeginPlay();
		actor->BeginPlayComponents();
	}
}

#define TICK_ACTORS_CREATED_DURING_TICK 0

void Scene::Tick(float deltaTime)
{
#if TICK_ACTORS_CREATED_DURING_TICK

	for (uint32 i = 0; i < mActorTickQueue.size(); i++)
	{
		mActorTickQueue[i]->Tick(deltaTime);
		mActorTickQueue[i]->TickComponents(deltaTime);
	}

#else

	uint32 tickQueueCount = mActorTickQueue.size();
	for (uint32 i = 0; i < tickQueueCount; i++)
	{
		mActorTickQueue[i]->Tick(deltaTime);
		mActorTickQueue[i]->TickComponents(deltaTime);
	}

#endif

	for (Actor* actor : mActorKillQueue)
	{
		// remove from tick list
		if (actor->mShouldTick)
		{
			auto it = std::find(mActorTickQueue.begin(), mActorTickQueue.end(), actor);
			mActorTickQueue.erase(it);
		}

		actor->UninitializeComponents();

		ObjectAllocator::Destroy(actor);
	}
}

void Scene::Clear()
{
	checkf(mActorKillQueue.size() == 0, "Can't call Scene::Clear during playtime");

	for (Actor* actor : mActors)
	{
		actor->UninitializeComponents();
		ObjectAllocator::Destroy(actor);
	}
	mActors.clear();
	mActorTickQueue.clear();

	checkf(mRenderQueue.size() == 0, "There's a bug, scene render queue not empty after clear");

	mCamera = nullptr;
}

void Scene::SubmitForRendering(MeshComponent* mesh)
{
	check(mesh);

	mRenderQueue.push_back(mesh);
}

void Scene::RemoveFromRenderQueue(MeshComponent* mesh)
{
	check(mesh);

	auto it = std::find(mRenderQueue.begin(), mRenderQueue.end(), mesh);
	mRenderQueue.erase(it);
}

void Scene::Copy(Scene* to)
{
	check(to);

	for (Actor* actor : mActors)
	{
		Actor* clone = to->ConstructActor(actor->GetClass(), actor->mTemplate);
		actor->Clone(clone);
		clone->InitializeComponents();
	}
}

Actor* Scene::ConstructActor(GroovyClass* actorClass, ActorBlueprint* bp)
{
	check(actorClass);
	check(GroovyClass_IsA(actorClass, Actor::StaticClass()));

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