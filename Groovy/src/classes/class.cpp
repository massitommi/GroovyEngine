#include "class.h"

void classUtils::GetClassPropertiesRecursiveSorted(GroovyClass* gClass, std::vector<GroovyProperty>& outProps)
{
	GroovyClass* c = gClass;
	if (c->super)
	{
		GetClassPropertiesRecursiveSorted(c->super, outProps);
	}
	check(gClass->propertiesGetter);
	gClass->propertiesGetter(outProps);
}

uint32 classUtils::FindProperty(const std::vector<GroovyProperty>& props, const std::string& propName)
{
	for (uint32 i = 0; i < props.size(); i++)
		if (props[i].name == propName)
			return i;
	return ~((uint32)0);
}
