#include "actor.h"

GROOVY_CLASS_IMPL(Actor, GroovyObject)

GROOVY_CLASS_REFLECTION_BEGIN(Actor)
	GROOVY_REFLECT(mTransform)
	GROOVY_REFLECT(mTest)
GROOVY_CLASS_REFLECTION_END()

Actor::Actor()
	: mTransform{{ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }}
{
	mTest.push_back("ciao");
	mTest.push_back("come");
	mTest.push_back("va");
}

Actor::~Actor()
{
}