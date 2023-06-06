#include "class.h"

void groovyclassUtils::GetClassPropertiesRecursiveSorted(GroovyClass* gClass, std::vector<GroovyProperty>& outProps)
{
	GroovyClass* c = gClass;
	if (c->super)
	{
		GetClassPropertiesRecursiveSorted(c->super, outProps);
	}
	check(gClass->propertiesGetter);
	gClass->propertiesGetter(outProps);
}

size_t groovyclassUtils::FindProperty(const std::vector<GroovyProperty>& props, const std::string& propName)
{
	for (size_t i = 0; i < props.size(); i++)
		if (props[i].name == propName)
			return i;
	return ~((size_t)0);
}
