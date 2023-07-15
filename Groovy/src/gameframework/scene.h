#pragma once

#include "actor.h"
#include "actorcomponent.h"
#include "blueprint.h"

class Scene : public AssetInstance
{
public:
	Scene();
	~Scene();

	virtual void __internal_SetUUID(AssetUUID uuid) override { mUUID = uuid; }

	virtual AssetUUID GetUUID() const override { return mUUID; }

	virtual bool IsLoaded() const override { return mLoaded; }
	virtual void Load() override;
	virtual void Save() override;

	void Unload();

	virtual void Serialize(DynamicBuffer& fileData) const override;
	virtual void Deserialize(BufferView fileData) override;

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

	// uses after play
	void DestroyActor(Actor* actor);

	inline const std::vector<Actor*>& GetActors() const { return mActors; }

	void Tick(float deltaTime);

#if WITH_EDITOR

	// returns true if assetToBeDeleted was referenced
	virtual bool Editor_FixDependencyDeletion(AssetHandle assetToBeDeleted) override;

	virtual void Editor_SpawnActor(GroovyClass* actorClass, ActorBlueprint* bp);
	virtual void Editor_DeleteActor(Actor* actor);

#endif

private:
	// used after play
	void FinishDestroyingActors();

private:
	std::vector<Actor*> mActors;

	// empty before play
	std::vector<Actor*> mActorsPlaying;
	// empty before play
	std::vector<Actor*> mActorKillQueue;

	AssetUUID mUUID;
	bool mLoaded;
};