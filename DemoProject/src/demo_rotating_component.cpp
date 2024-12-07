#include "demo_rotating_component.h"
#include "gameframework/actor.h"
#include "utils/string_utils.h"

GROOVY_CLASS_IMPL(DemoRotatingComponent)
	GROOVY_REFLECT(mRotation)
GROOVY_CLASS_END()

DemoRotatingComponent::DemoRotatingComponent()
{
	mRotation = { 0.0f, 2.5f, 0.0f };
}

void DemoRotatingComponent::BeginPlay()
{
	GROOVY_LOG_INFO("Rotating at %s", stringUtils::ToString(mRotation).c_str());
}

void DemoRotatingComponent::Tick(float deltaTime)
{
	Super::Tick(deltaTime);

	Vec3 rot = GetOwner()->GetRotation();
	rot += mRotation * deltaTime;
	GetOwner()->SetRotation(rot);
}