#pragma once
#include "classes/object.h"

GROOVY_CLASS_DECL(ActorComponent)
class ActorComponent : public GroovyObject
{
	GROOVY_CLASS_BODY(ActorComponent, GroovyObject)

public:
	ActorComponent();
	~ActorComponent();

private:
};