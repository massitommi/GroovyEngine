#include "example_component.h"
#include "gameframework/actor.h"
#include "platform/input.h"

// Class implementation
GROOVY_CLASS_IMPL(ExampleComponent)
	// Reflect variables
	GROOVY_REFLECT(mRotationSpeed)
GROOVY_CLASS_END()

// Initialize variables...
ExampleComponent::ExampleComponent()
	: mRotationSpeed(5.0f)
{
}

void ExampleComponent::Tick(float deltaTime)
{
	// Call base class implementation
	Super::Tick(deltaTime);

	// Our implementation

	if (Input::IsKeyPressed(EKeyCode::Space))
		mRotationSpeed *= -1.0f;

	Actor* owner = GetOwner();
	Vec3 actorRotation = owner->GetRotation();
	actorRotation.y += mRotationSpeed * deltaTime;
	owner->SetRotation(actorRotation);
}
