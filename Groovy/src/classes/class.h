#pragma once

#include "core/core.h"
#include "math/vector.h"

enum EPropertyType : uint32
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

enum EPropertyFlags : uint64
{
	PROPERTY_FLAG_IS_ARRAY = BITFLAG(1),
	PROPERTY_FLAG_IS_DYNAMIC_ARRAY = BITFLAG(2)
};

enum EPropertyEditorFlags : uint64
{
	PROPERTY_EDITOR_FLAG_NOSERIALIZE = BITFLAG(1),
	PROPERTY_EDITOR_FLAG_READONLY = BITFLAG(2)
};

struct GroovyProperty
{
	std::string name;
	EPropertyType type;
	size_t offset;
	size_t arrayCount;
	uint64 flags;
	uint64 editorFlags;
};

template<typename T>
struct PropType
{
	enum : uint32
	{
		Type = 0,
		ArrayCount = 0,
		Flags = 0
	};

	static_assert(!(sizeof(T) > 0), "Not implemented");
};

#define IMPL_PROPERTY_TYPE(CoreType, OutPropertyType)	\
template<>												\
struct PropType<CoreType> {								\
	enum : uint32 {										\
		Type = OutPropertyType,							\
		ArrayCount = 1,									\
		Flags = 0										\
	};													\
};														\
template<size_t InArrayCount>							\
struct PropType<CoreType[InArrayCount]> {				\
	enum : uint32 {										\
		Type = OutPropertyType,							\
		ArrayCount = InArrayCount,						\
		Flags = PROPERTY_FLAG_IS_ARRAY					\
	};													\
};														\
template<>												\
struct PropType<std::vector<CoreType>> {				\
	enum : uint32 {										\
		Type = OutPropertyType,							\
		ArrayCount = 0,									\
		Flags = PROPERTY_FLAG_IS_DYNAMIC_ARRAY			\
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
	void* cdo;
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
	new Class()																					\
};

#define GROOVY_PROPERTY(Class, Property, EditorFlags)	{ #Property, (EPropertyType)PropType<decltype(Property)>::Type, offsetof(Class, Property), PropType<decltype(Property)>::ArrayCount, PropType<decltype(Property)>::Flags, EditorFlags }


#define GROOVY_CLASS_REFLECTION_BEGIN(Class)			void Class::GetClassPropertiesRecursive(std::vector<GroovyProperty>& outProps) const { Super::GetClassProperties(outProps); ThisClass::GetClassProperties(outProps); } void Class::GetClassProperties(std::vector<GroovyProperty>& outProps) {
#define GROOVY_REFLECT(Property)						outProps.push_back(GROOVY_PROPERTY(ThisClass, Property, 0));
#define GROOVY_REFLECT_EX(Property, EditorFlags)		outProps.push_back(GROOVY_PROPERTY(ThisClass, Property, EditorFlags));
#define GROOVY_CLASS_REFLECTION_END()					}

#define CLASS_LIST_BEGIN(ListName)				std::vector<GroovyClass*> ListName = 
#define CLASS_LIST_ADD(Class)					&GROOVY_CLASS_NAME(Class)
#define CLASS_LIST_END()						;

namespace classUtils
{
	// Gets all the properties exposed by a groovy class, sorted means that the first are the base class ones, and the last are the gClass ones
	void GetClassPropertiesRecursiveSorted(GroovyClass* gClass, std::vector<GroovyProperty>& outProps);
	size_t FindProperty(const std::vector<GroovyProperty>& props, const std::string& propName);
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