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
	static constexpr GroovyClass* StaticClass() { return &GROOVY_CLASS_NAME(GroovyObject); }
	virtual GroovyObject* GetCDO() const { return GROOVY_CLASS_NAME(GroovyObject).cdo; }
	
	inline bool IsA(GroovyClass* gClass) const { return GroovyClass_DynamicCast((GroovyObject*)this, gClass); }
	
	template<typename GroovyClassT>
	inline bool IsA() const { return GroovyClass_DynamicCast((GroovyObject*)this, GroovyClassT::StaticClass()); }

	void CopyProperties(GroovyObject* to);

#if WITH_EDITOR

	virtual void Editor_OnPropertyChanged(const GroovyProperty* prop) {}

#endif

	friend class ObjectSerializer;
};

template<typename GroovyClassT>
inline GroovyClassT* Cast(GroovyObject* obj)
{
	return (GroovyClassT*)GroovyClass_DynamicCast(obj, GroovyClassT::StaticClass());
}

template<typename GroovyClassT>
inline const GroovyClassT* Cast(const GroovyObject* obj)
{
	return (const GroovyClassT*)GroovyClass_DynamicCast((GroovyObject*)obj, GroovyClassT::StaticClass());
}