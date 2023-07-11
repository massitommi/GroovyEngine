#pragma once

#include "class.h"
#include <map>

class ClassDB
{
public:
	ClassDB();

	void Register(GroovyClass* gClass);

	// called at engine startup
	void BuildCDOs();
	// called at engine shutdown
	void DestroyCDOs();

	const std::vector<GroovyProperty>& operator[](GroovyClass* gClass);
	GroovyClass* operator[](const std::string& className);

	// slow
	std::vector<GroovyClass*> GetClasses();

	const GroovyProperty* FindProperty(GroovyClass* gClass, const std::string& propertyName);

private:
	std::map<std::string, GroovyClass*> mClassDB;
	std::map<GroovyClass*, std::vector<GroovyProperty>> mPropsDB;
};