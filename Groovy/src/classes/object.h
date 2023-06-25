#pragma once

#include "class.h"

GROOVY_CLASS_DECL(GroovyObject)
class GroovyObject
{
public:
	GroovyObject();
	virtual ~GroovyObject();

	virtual const char* GetClassName() const { return "GroovyObject"; }
	virtual GroovyClass* GetClass() const { return &GROOVY_CLASS_NAME(GroovyObject); }
	static void GetClassProperties(std::vector<GroovyProperty>& outProps);
	virtual void GetClassPropertiesRecursive(std::vector<GroovyProperty>& outProps) const;
	static GroovyClass* StaticClass() { return &GROOVY_CLASS_NAME(GroovyObject); }
	
	inline bool IsA(GroovyClass* gClass) { return classUtils::DynamicCast(this, gClass); }
	
	template<typename GroovyClassT>
	inline bool IsA() { return classUtils::DynamicCast(this, GroovyClassT::StaticClass()); }

	friend class ObjectSerializer;
};

template<typename GroovyClassT>
GroovyClassT* Cast(GroovyObject* obj)
{
	return (GroovyClassT*)classUtils::DynamicCast(obj, GroovyClassT::StaticClass());
}