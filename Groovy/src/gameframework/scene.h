#pragma once

#include "actor.h"
#include "actorcomponent.h"
#include "blueprint.h"

class Scene : public AssetInstance
{
public:
	Scene();

	virtual void __internal_SetUUID(AssetUUID uuid) override { mUUID = uuid; }

	virtual AssetUUID GetUUID() const override { return mUUID; }

	virtual bool LazyLoadAndUnload() const override { return true; }
	virtual bool IsLoaded() const override { return mLoaded; }
	virtual void Load() override;
	virtual void Save() override;

	virtual void Serialize(DynamicBuffer& fileData) const override;
	virtual void Deserialize(BufferView fileData) override;

#if WITH_EDITOR

	// returns true if assetToBeDeleted was referenced
	virtual bool Editor_FixDependencyDeletion(AssetHandle assetToBeDeleted) override;

#endif


	Actor* SpawnActor(GroovyClass* actorClass, ActorBlueprint* bp = nullptr);

	template<typename TActor>
	TActor* SpawnActor(GroovyClass* actorClass, ActorBlueprint* bp = nullptr)
	{
		return (TActor*)SpawnActor(actorClass, bp);
	}

	template<typename TActor>
	TActor* SpawnActor(ActorBlueprint* bp = nullptr)
	{
		return (TActor*)SpawnActor(TActor::StaticClass(), bp);
	}

	void DestroyActor(Actor* actor);

private:
	std::vector<Actor*> mActors;

	struct ActorBP
	{
		Actor* instance;
		ActorBlueprint* bp;
	};

	std::vector<ActorBP> mBPActors;
	std::vector<Actor*> mGenericActors;

	AssetUUID mUUID;
	bool mLoaded;
};