#pragma once
#include "classes/object.h"

class Actor;

GROOVY_CLASS_DECL(ActorComponent)
class ActorComponent : public GroovyObject
{
	GROOVY_CLASS_BODY(ActorComponent, GroovyObject)

public:
	ActorComponent();
	~ActorComponent();

	inline const std::string& GetName() const { return mName; }

private:
	std::string mName;
};