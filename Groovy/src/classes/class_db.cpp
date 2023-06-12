#include "class_db.h"

ClassDB::ClassDB()
{
}

void ClassDB::Register(GroovyClass* gClass)
{
	check(gClass);
	std::vector<GroovyProperty> props;
	classUtils::GetClassPropertiesRecursiveSorted(gClass, props);
	
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

std::vector<GroovyClass*> ClassDB::GetClasses()
{
	std::vector<GroovyClass*> res;
	for (const auto& [n, c] : mClassDB)
	{
		res.push_back(c);
	}
	return res;
}
