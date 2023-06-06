#include "actorcomponent.h"

GROOVY_CLASS_IMPL(ActorComponent, GroovyObject)

GROOVY_CLASS_REFLECTION_BEGIN(ActorComponent)
	GROOVY_REFLECT(mNative)
GROOVY_CLASS_REFLECTION_END()

ActorComponent::ActorComponent()
	: mNative(true)
{
}

ActorComponent::~ActorComponent()
{

}
