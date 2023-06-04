#include "class_db.h"

ClassDB::ClassDB()
{
}

void ClassDB::Register(GroovyClass* gClass)
{
	check(gClass);
	std::vector<GroovyProperty> props;
	groovyclassUtils::GetClassPropertiesRecursiveSorted(gClass, props);
	
	mClassDB[gClass->name] = gClass;
	mPropsDB[gClass] = props;
}

const std::vector<GroovyProperty>& ClassDB::operator[](GroovyClass* gClass)
{
	return mPropsDB[gClass];
}

GroovyClass* ClassDB::operator[](const std::string& className)
{
	return mClassDB[className];
}

const GroovyProperty* ClassDB::GetProperty(GroovyClass* gClass, const std::string& name, EPropertyType type, uint64 flags)
{
	std::vector<GroovyProperty>& props = mPropsDB[gClass];

	for (GroovyProperty& p : props)
	{
		bool sameName = p.name == name;
		bool sameType = p.type == type;
		bool sameFlags = p.flags == flags;
		if (sameName && sameType && sameFlags)
		{
			return &p;
		}
#if BUILD_DEBUG
		if (sameName)
		{
			// console log
		}
#endif
	}

	return nullptr;
}
