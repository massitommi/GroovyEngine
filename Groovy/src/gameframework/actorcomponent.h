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
	// true if this component was created from code, false if it was created with the editor
	bool mNative;
};