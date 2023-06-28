#include "actor.h"

GROOVY_CLASS_IMPL(Actor)
	GROOVY_REFLECT(mTransform)
GROOVY_CLASS_END()

Actor::Actor()
	: mTransform{{ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }}
{
}

Actor::~Actor()
{
}