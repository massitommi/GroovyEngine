#pragma once
#include "classes/object.h"

GROOVY_CLASS_DECL(Actor, GroovyObject)
class Actor : public GroovyObject
{
	GROOVY_CLASS_BODY(Actor, GroovyObject)
public:
	Actor();
	~Actor();

public:
	Transform mTransform;
	std::vector<std::string> mTest;
};