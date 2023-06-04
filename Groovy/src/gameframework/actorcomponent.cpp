#include "actorcomponent.h"

GROOVY_CLASS_IMPL(ActorComponent, GroovyObject)

GROOVY_CLASS_REFLECTION_BEGIN(ActorComponent)
	GROOVY_REFLECT_EX(mNative, PROPERTY_EDITOR_FLAG_NOSERIALIZE)
GROOVY_CLASS_REFLECTION_END()

ActorComponent::ActorComponent()
{
}

ActorComponent::~ActorComponent()
{

}
