#pragma once

#include "classes/class.h"

namespace reflectionUtils
{
	// Gets all the properties exposed by a groovy class, sorted means from base class to last derived class
	void GetClassPropertiesRecursiveSorted(GroovyClass* gClass, std::vector<GroovyProperty>& outProps);
	uint32 FindProperty(const std::vector<GroovyProperty>& props, const std::string& propName);

	void CopyProperty(GroovyObject* dst, const GroovyObject* src, const GroovyProperty* prop);
	void CopyProperties(GroovyObject* dst, const GroovyObject* src);

	void ReplaceValueTypeProperty(GroovyObject* obj, EPropertyType type, void* find, void* replaceWith);
}