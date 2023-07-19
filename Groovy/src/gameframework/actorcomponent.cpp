#include "actorcomponent.h"

GROOVY_CLASS_IMPL(ActorComponent)
GROOVY_CLASS_END()

ActorComponent::ActorComponent()
	: mType(ACTOR_COMPONENT_TYPE_NATIVE), mOwner(nullptr)
{
}

GROOVY_CLASS_IMPL(SceneComponent)
	GROOVY_REFLECT_EX(mTransform, PROPERTY_FLAG_EDITOR_HIDDEN) // for UI purposes
GROOVY_CLASS_END()

SceneComponent::SceneComponent()
	: mTransform{ { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f } }
{
}