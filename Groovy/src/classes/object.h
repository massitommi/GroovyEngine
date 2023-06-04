#pragma once

#include "class.h"

GROOVY_CLASS_DECL(GroovyObject)
class GroovyObject
{
public:
	GroovyObject();
	virtual ~GroovyObject();

	virtual const char* GetClassName() const { return "GroovyObject"; }
	virtual GroovyClass* GetClass() const { return &GROOVY_CLASS_NAME(GroovyObject); }
	static void GetClassProperties(std::vector<GroovyProperty>& outProps);
	virtual void GetClassPropertiesRecursive(std::vector<GroovyProperty>& outProps) const;

	friend class ObjectSerializer;
};