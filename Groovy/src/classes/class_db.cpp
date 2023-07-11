#include "class_db.h"
#include "runtime/object_allocator.h"

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

void ClassDB::BuildCDOs()
{
	for (auto [name, gClass] : mClassDB)
	{
		gClass->cdo = ObjectAllocator::Instantiate(gClass);
	}
}

void ClassDB::DestroyCDOs()
{
	for (auto [name, gClass] : mClassDB)
	{
		if(gClass)
			ObjectAllocator::Destroy(gClass->cdo);
	}
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

const GroovyProperty* ClassDB::FindProperty(GroovyClass* gClass, const std::string& propertyName)
{
	const std::vector<GroovyProperty>& props = (*this)[gClass];
	
	for (const GroovyProperty& prop : props)
		if (prop.name == propertyName)
			return &prop;

	return nullptr;
}
