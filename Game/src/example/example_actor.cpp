#include "example_actor.h"
#include "example_component.h"
#include "gameframework/components/meshcomponent.h"

// Class implementation
GROOVY_CLASS_IMPL(ExampleActor)
	// Reflect variables
	GROOVY_REFLECT(mMaterialsToUse)
	GROOVY_REFLECT(mTimeToChangeMat)
	GROOVY_REFLECT_EX(mElapsedTimeSinceLastMatChange, PROPERTY_FLAG_EDITOR_READONLY | PROPERTY_FLAG_NO_SERIALIZE)
	GROOVY_REFLECT_EX(mCurrentMatIndex, PROPERTY_FLAG_EDITOR_READONLY | PROPERTY_FLAG_NO_SERIALIZE)
GROOVY_CLASS_END()

// Initalize variables, add components
ExampleActor::ExampleActor()
{
	mExampleComp = AddComponent<ExampleComponent>("MyExampleComponent");
	mMeshComp = AddComponent<MeshComponent>("MyMeshComp");

	mTimeToChangeMat = 3.0f;
	mElapsedTimeSinceLastMatChange = 0.0f;
	mCurrentMatIndex = -1;
}

void ExampleActor::BeginPlay()
{
	// Call base class implementation
	Super::BeginPlay();
}

void ExampleActor::Tick(float deltaTime)
{
	// Call base class implementation
	Super::Tick(deltaTime);

	mElapsedTimeSinceLastMatChange += deltaTime;

	if (mElapsedTimeSinceLastMatChange >= mTimeToChangeMat)
	{
		mCurrentMatIndex++;

		if (mCurrentMatIndex >= mMaterialsToUse.size())
			mCurrentMatIndex = 0;

		if (mMaterialsToUse.size())
			mMeshComp->SetMaterialOverride(0, mMaterialsToUse[mCurrentMatIndex]);

		// reset timer
		mElapsedTimeSinceLastMatChange = 0.0f;
	}
}

void ExampleActor::Destroy()
{
	// Call base class implementation
	Super::Destroy();
}
