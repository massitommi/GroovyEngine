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

	// slow
	std::vector<GroovyClass*> GetClasses();

	const GroovyProperty* FindProperty(GroovyClass* gClass, const std::string& propertyName);

	// useful for blueprint deserialization in the editor, what if someone changes the type of a var but the var has the same name? kaboom
	const GroovyProperty* FindProperty(GroovyClass* gClass, const std::string& propertyName, EPropertyType typeFilter);

private:
	std::map<std::string, GroovyClass*> mClassDB;
	std::map<GroovyClass*, std::vector<GroovyProperty>> mPropsDB;
};