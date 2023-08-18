#pragma once

#include "gameframework/actor.h"

GROOVY_CLASS_DECL(ExampleActor)
class ExampleActor : public Actor
{
	GROOVY_CLASS_BODY(ExampleActor, Actor)

public:
	ExampleActor();

protected:
	virtual void BeginPlay();
	virtual void Tick(float deltaTime);
	virtual void Destroy();

private:
	class ExampleComponent* mExampleComp;
	class MeshComponent* mMeshComp;
	std::vector<Material*> mMaterialsToUse;
	float mTimeToChangeMat;

	float mElapsedTimeSinceLastMatChange;
	int mCurrentMatIndex;
};