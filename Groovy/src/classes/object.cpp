#include "object.h"

GroovyClass GROOVY_CLASS_NAME(GroovyObject) =
{
	GVY_STRINGIFY_MACRO(GroovyObject),							// classname
	sizeof(GroovyObject),										// size in bytes
	[](void* mem) { new(mem) GroovyObject(); },					// constructor
	[](void* mem) { ((GroovyObject*)mem)->~GroovyObject(); },	// destructor
	nullptr,													// super class
	&GroovyObject::GetClassProperties,							// props getter
	new GroovyObject()											// cdo
};

void GroovyObject::GetClassProperties(std::vector<GroovyProperty>& outProps)
{
}

void GroovyObject::GetClassPropertiesRecursive(std::vector<GroovyProperty>& outProps) const
{
	GetClassProperties(outProps);
}

GroovyObject::GroovyObject()
{
}

GroovyObject::~GroovyObject()
{
}
