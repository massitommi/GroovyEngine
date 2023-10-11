#include "class_db.h"
#include "runtime/object_allocator.h"
#include "utils/reflection_utils.h"

ClassDB::ClassDB()
{
}

void ClassDB::Register(GroovyClass* gClass)
{
	check(gClass);
	std::vector<GroovyProperty> props;
	reflectionUtils::GetClassPropertiesRecursiveSorted(gClass, props);
	
	mClasses.push_back(gClass);
	mClassDB[gClass->name] = gClass;
	mPropsDB[gClass] = props;
}

void ClassDB::BuildCDOs()
{
	for(GroovyClass* gClass : mClasses)
		gClass->cdo = ObjectAllocator::Instantiate(gClass);
}

void ClassDB::DestroyCDOs()
{
	for(GroovyClass* gClass : mClasses)
		if(gClass)
			ObjectAllocator::Destroy(gClass->cdo);
}

void ClassDB::BuildCDO(GroovyClass* gClass)
{
	gClass->cdo = ObjectAllocator::Instantiate(gClass);
}

const std::vector<GroovyProperty>& ClassDB::operator[](GroovyClass* gClass)
{
	return mPropsDB[gClass];
}

GroovyClass* ClassDB::operator[](const std::string& className)
{
	return mClassDB[className];
}

const GroovyProperty* ClassDB::FindProperty(GroovyClass* gClass, const std::string& propertyName)
{
	const std::vector<GroovyProperty>& props = (*this)[gClass];
	
	for (const GroovyProperty& prop : props)
		if (prop.name == propertyName)
			return &prop;

	return nullptr;
}
