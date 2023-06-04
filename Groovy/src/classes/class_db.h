#pragma once

#include "class.h"
#include <map>

class ClassDB
{
public:
	ClassDB();

	void Register(GroovyClass* gClass);

	const std::vector<GroovyProperty>& operator[](GroovyClass* gClass);
	GroovyClass* operator[](const std::string& className);

	const GroovyProperty* GetProperty(GroovyClass* gClass, const std::string& name, EPropertyType type, uint64 flags);

private:
	std::map<std::string, GroovyClass*> mClassDB;
	std::map<GroovyClass*, std::vector<GroovyProperty>> mPropsDB;
};