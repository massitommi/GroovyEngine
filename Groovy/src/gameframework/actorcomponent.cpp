#include "actorcomponent.h"
#include "math/math.h"
#include "actor.h"

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

Transform SceneComponent::GetAbsoluteTransform() const
{
	Transform absoluteTransform;
	absoluteTransform.location = GetOwner()->GetLocation() + mTransform.location;
	absoluteTransform.rotation += GetOwner()->GetRotation() + mTransform.rotation;
	absoluteTransform.scale = GetOwner()->GetScale() * mTransform.scale;
	return absoluteTransform;
}

Vec3 SceneComponent::GetAbsoluteLocation() const
{
	return GetOwner()->GetLocation() + mTransform.location;
}

Vec3 SceneComponent::GetAbsoluteRotation() const
{
	return GetOwner()->GetRotation() + mTransform.rotation;
}

Vec3 SceneComponent::GetAbsoluteScale() const
{
	return GetOwner()->GetScale() * mTransform.scale;
}
