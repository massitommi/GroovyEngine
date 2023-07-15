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

	inline const std::vector<GroovyClass*>& GetClasses() { return mClasses; }

	const GroovyProperty* FindProperty(GroovyClass* gClass, const std::string& propertyName);

private:
	std::vector<GroovyClass*> mClasses;
	std::map<std::string, GroovyClass*> mClassDB;
	std::map<GroovyClass*, std::vector<GroovyProperty>> mPropsDB;
};