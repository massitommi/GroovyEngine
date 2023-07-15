#include "actorcomponent.h"

GROOVY_CLASS_IMPL(ActorComponent)
GROOVY_CLASS_END()

ActorComponent::ActorComponent()
	: mType(ACTOR_COMPONENT_TYPE_NATIVE), mOwner(nullptr)
{
}

ActorComponent::~ActorComponent()
{
}

GROOVY_CLASS_IMPL(SceneComponent)
GROOVY_CLASS_END()