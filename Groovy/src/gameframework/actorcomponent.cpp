#include "actorcomponent.h"

GROOVY_CLASS_IMPL(ActorComponent)
	GROOVY_REFLECT_EX(mNative, PROPERTY_FLAG_NO_SERIALIZE | PROPERTY_FLAG_EDITOR_READONLY)
GROOVY_CLASS_END()

ActorComponent::ActorComponent()
	: mNative(true)
{
}

ActorComponent::~ActorComponent()
{

}
