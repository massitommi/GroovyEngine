#include "actor.h"

GROOVY_CLASS_IMPL(Actor, GroovyObject)

GROOVY_CLASS_REFLECTION_BEGIN(Actor)
	GROOVY_REFLECT(mTransform)
GROOVY_CLASS_REFLECTION_END()

Actor::Actor()
	: mTransform{{ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }}
{
}

Actor::~Actor()
{
}