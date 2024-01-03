#include "scene.h"
#include "actor_serializer.h"
#include "runtime/object_allocator.h"
#include "assets/asset_loader.h"
#include "assets/asset_serializer.h"
#include "assets/asset_manager.h"
#include "components/mesh_component.h"
#include "utils/reflection_utils.h"
#include "classes/class_db.h"

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
	fileData.push<uint32>((uint32)mActors.size());
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
	newActor->BeginPlayComponents();

	if (newActor->mShouldTick)
		mActorTickQueue.push_back(newActor);
	
	return newActor;
}

#if WITH_EDITOR

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

	return removeList.size() || refs;
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

uint32 Scene::Editor_OnBlueprintUpdated(ActorBlueprint* bp, Actor* oldTemplate, Actor* newTemplate)
{
	extern ClassDB gClassDB;

	// find blueprint instances
	std::vector<Actor*> bpInstances;

	for (Actor* actor : mActors)
		if (actor->GetTemplate() == bp)
			bpInstances.push_back(actor);

	if (bpInstances.size())
	{
		// figure out what components have been added and removed

		std::vector<ActorComponent*> bpComponentsToAdd;
		std::vector<ActorComponent*> bpComponentsToRemove;

		for (ActorComponent* comp : newTemplate->GetComponents())
		{
			if (comp->GetType() != ACTOR_COMPONENT_TYPE_EDITOR_BP)
				continue;

			ActorComponent* oldComp = oldTemplate->GetComponent(comp->GetName());

			if (!oldComp)
			{
				bpComponentsToAdd.push_back(comp);
			}
			else
			{
				if (comp->GetClass() != oldComp->GetClass())
				{
					bpComponentsToRemove.push_back(comp);
					bpComponentsToAdd.push_back(comp);
				}
			}
		}

		for (ActorComponent* comp : oldTemplate->GetComponents())
		{
			if (comp->GetType() != ACTOR_COMPONENT_TYPE_EDITOR_BP)
				continue;

			if (!newTemplate->HasComponent(comp->GetName()))
			{
				bpComponentsToRemove.push_back(comp);
			}
		}

		// fix the scene

		for (Actor* actor : bpInstances)
		{
			// remove deleted components
			for (ActorComponent* comp : bpComponentsToRemove)
				actor->__internal_Editor_RemoveEditorComponent(actor->GetComponent(comp->GetName()));

			// add added components
			for (ActorComponent* comp : bpComponentsToAdd)
			{
				ActorComponent* newComp = actor->__internal_Editor_AddEditorcomponent_BP(comp->GetClass(), comp->GetName());
				comp->CopyProperties(newComp);
			}

			// overwrite actor properties but only if they were equal to the old template

			const std::vector<GroovyProperty>& actorProps = gClassDB[actor->GetClass()];

			for (const GroovyProperty& prop : actorProps)
			{
				if (reflectionUtils::PropertyIsEqual(actor, oldTemplate, &prop))
					reflectionUtils::CopyProperty(actor, newTemplate, &prop);
			}

			// overwrite component properties but only if they were equal to the old template
			for (uint32 i = 0; i < actor->GetComponents().size() - bpComponentsToAdd.size(); i++)
			{
				ActorComponent* actorComp = actor->GetComponents()[i];
				ActorComponent* oldTemplateComp = oldTemplate->GetComponent(actorComp->GetName());
				ActorComponent* newTemplateComp = newTemplate->GetComponent(actorComp->GetName());

				const std::vector<GroovyProperty>& compProps = gClassDB[actorComp->GetClass()];

				for (const GroovyProperty& prop : compProps)
				{
					if (reflectionUtils::PropertyIsEqual(actorComp, oldTemplateComp, &prop))
						reflectionUtils::CopyProperty(actorComp, newTemplateComp, &prop);
				}
			}
		}
	}

	return (uint32)bpInstances.size();
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

	uint32 tickQueueCount = (uint32)mActorTickQueue.size();
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

bool Scene::ReferencesBlueprint(ActorBlueprint* bp)
{
	for (Actor* actor : mActors)
		if (actor->mTemplate == bp)
			return true;
	return false;
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