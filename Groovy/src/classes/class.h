#pragma once

#include "core/core.h"
#include "math/vector.h"

enum EPropertyType
{
	PROPERTY_TYPE_INT32,
	PROPERTY_TYPE_INT64,
	PROPERTY_TYPE_UINT32,
	PROPERTY_TYPE_UINT64,
	PROPERTY_TYPE_BOOL,
	PROPERTY_TYPE_FLOAT,
	PROPERTY_TYPE_STRING,
	PROPERTY_TYPE_VEC3,
	PROPERTY_TYPE_TRANSFORM
};

enum EPropertyFlags
{
	PROPERTY_FLAG_NOSERIALIZE = BITFLAG(1),
	PROPERTY_FLAG_READONLY = BITFLAG(2)
};

struct GroovyProperty
{
	std::string name;
	size_t arrayCount;
	size_t classOffset;
	EPropertyType type;
	uint32 flags;
};

template<typename T>
struct PropType
{
	enum : size_t
	{
		Type = 0,
		ArrayCount = 0
	};

	static_assert(!(sizeof(T) > 0), "Not implemented");
};

#define IMPL_PROPERTY_TYPE(CoreType, OutPropertyType)	\
template<>												\
struct PropType<CoreType> {								\
	enum : size_t {										\
		Type = OutPropertyType,							\
		ArrayCount = 1,									\
	};													\
};														\
template<size_t InArrayCount>							\
struct PropType<CoreType[InArrayCount]> {				\
	enum : size_t {										\
		Type = OutPropertyType,							\
		ArrayCount = InArrayCount,						\
	};													\
};

IMPL_PROPERTY_TYPE(int32, PROPERTY_TYPE_INT32)
IMPL_PROPERTY_TYPE(int64, PROPERTY_TYPE_INT64)
IMPL_PROPERTY_TYPE(uint32, PROPERTY_TYPE_UINT32)
IMPL_PROPERTY_TYPE(uint64, PROPERTY_TYPE_UINT64)
IMPL_PROPERTY_TYPE(bool, PROPERTY_TYPE_BOOL)
IMPL_PROPERTY_TYPE(float, PROPERTY_TYPE_FLOAT)
IMPL_PROPERTY_TYPE(std::string, PROPERTY_TYPE_STRING)
IMPL_PROPERTY_TYPE(Vec3, PROPERTY_TYPE_VEC3)
IMPL_PROPERTY_TYPE(Transform, PROPERTY_TYPE_TRANSFORM)

// default property types:

typedef void(*GroovyConstructor)(void*);
typedef void(*GroovyDestructor)(void*);
typedef void(*GroovyPropertiesGetter)(std::vector<GroovyProperty>&);

struct GroovyClass
{
	std::string name;
	size_t size;
	GroovyConstructor constructor;
	GroovyDestructor destructor;
	GroovyClass* super;
	GroovyPropertiesGetter propertiesGetter;
};

#define GROOVY_CLASS_NAME(Class)				__internal_groovyclass_##Class

#define GROOVY_CLASS_DECL(Class)				extern GroovyClass GROOVY_CLASS_NAME(Class);

#define GROOVY_CLASS_BODY(Class, SuperClass)														\
public:																								\
	typedef SuperClass Super;																		\
	typedef Class ThisClass;																		\
	virtual const char* GetClassName() const override { return #Class; }							\
	virtual GroovyClass* GetClass() const override { return &GROOVY_CLASS_NAME(Class); }			\
	static void GetClassProperties(std::vector<GroovyProperty>& outProps);							\
	virtual void GetClassPropertiesRecursive(std::vector<GroovyProperty>& outProps) const override;	\
private:

#define GROOVY_CLASS_IMPL(Class, SuperClass)	GroovyClass GROOVY_CLASS_NAME(Class) =			\
{																								\
	#Class,																						\
	sizeof(Class),																				\
	[](void* mem) { new(mem) Class(); },														\
	[](void* mem) { ((Class*)mem)->~Class(); },													\
	&GROOVY_CLASS_NAME(SuperClass),																\
	&Class::GetClassProperties,																	\
};

#define GROOVY_PROPERTY(Class, Property, Flags)	{ #Property, PropType<decltype(Property)>::ArrayCount, offsetof(Class, Property), (EPropertyType)PropType<decltype(Property)>::Type, Flags }

#define GROOVY_CLASS_REFLECTION_BEGIN(Class)	void Class::GetClassPropertiesRecursive(std::vector<GroovyProperty>& outProps) const { Super::GetClassProperties(outProps); ThisClass::GetClassProperties(outProps); } void Class::GetClassProperties(std::vector<GroovyProperty>& outProps) {
#define GROOVY_REFLECT(Property)				outProps.push_back(GROOVY_PROPERTY(ThisClass, Property, 0));
#define GROOVY_REFLECT_EX(Property, Flags)		outProps.push_back(GROOVY_PROPERTY(ThisClass, Property, Flags));
#define GROOVY_CLASS_REFLECTION_END()			}

namespace groovyclassUtils
{
	// Gets all the properties exposed by a groovy class, sorted means that the first are the base class ones, and the last are the gClass ones
	void GetClassPropertiesRecursiveSorted(GroovyClass* gClass, std::vector<GroovyProperty>& outProps);
}

/*
	How to create a groovy class:

	myclass.h

	GROOVY_CLASS_DECL(MyClass)
	class MyClass : public GroovyObject
	{
		GROOVY_CLASS_BODY(MyClass, GroovyObject)

	public:
		MyClass();
		~MyClass();

		void MyMethod();

	private:
		int32 mData1;
		int32 mData2[5];
		bool mDead;
	};


	myclass.cpp

	GROOVY_CLASS_IMPL(MyClass, GroovyObject)

	GROOVY_CLASS_BEGIN_REFLECTION(MyClass)
		GROOVY_REFLECT(mData1)
		GROOVY_REFLECT(mData2)
		GROOVY_REFLECT_EX(mDead, PROPERTY_FLAG_NOSERIALIZE)
	GROOVY_CLASS_END_REFLECTION()

	MyClass::MyClass()
		: mData1(0), mData2{1,2,3,4,5}
	{}

	MyClass::~MyClass()
	{}

	void MyClass::MyMethod()
	{
		mData1 = 1;
	}
*/