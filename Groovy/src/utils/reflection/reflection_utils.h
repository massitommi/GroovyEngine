#pragma once

#include "classes/class.h"

namespace reflectionUtils
{
	// Gets all the properties exposed by a groovy class, sorted means from base class to last derived class
	CORE_API void GetClassPropertiesRecursiveSorted(GroovyClass* gClass, std::vector<GroovyProperty>& outProps);
	CORE_API uint32 FindProperty(const std::vector<GroovyProperty>& props, const std::string& propName);

	CORE_API void CopyProperty(GroovyObject* dst, const GroovyObject* src, const GroovyProperty* prop);
	CORE_API void CopyProperties(GroovyObject* dst, const GroovyObject* src);

	CORE_API bool PropertyIsEqual(GroovyObject* obj1, GroovyObject* obj2, const GroovyProperty* prop);

	CORE_API void ReplaceValueTypeProperty(GroovyObject* obj, EPropertyType type, void* find, void* replaceWith);
}