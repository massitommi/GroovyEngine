#pragma once

#include "core/core.h"
#include "math/vector.h"
#include "assets/asset.h"

enum EPropertyType : uint32
{
	PROPERTY_TYPE_INT32 = 0,
	PROPERTY_TYPE_INT64 = 1,
	PROPERTY_TYPE_UINT32 = 2,
	PROPERTY_TYPE_UINT64 = 3,
	PROPERTY_TYPE_BOOL = 4,
	PROPERTY_TYPE_FLOAT = 5,
	PROPERTY_TYPE_VEC2 = 6,
	PROPERTY_TYPE_VEC3 = 7,
	PROPERTY_TYPE_VEC4 = 8,
	PROPERTY_TYPE_TRANSFORM = 9,
	PROPERTY_TYPE_STRING = 10,
	PROPERTY_TYPE_BUFFER = 11,
	PROPERTY_TYPE_ASSET_REF = 12,
	PROPERTY_TYPE_INTERNAL_SUBMESHDATA = 13

	// add your custom stuff here
};

enum EPropertyFlags : uint32
{
	PROPERTY_FLAG_IS_ARRAY = BITFLAG(1),			// This property is an array
	PROPERTY_FLAG_IS_DYNAMIC_ARRAY = BITFLAG(2),	// This property is a dynamic array
	PROPERTY_FLAG_IS_COMPLEX = BITFLAG(3),			// This property is complex, and requires more than just a memcpy for serialization and deserialization (ex: Texture*)
	PROPERTY_FLAG_NO_SERIALIZE = BITFLAG(4),		// This property should not be serialized
	PROPERTY_FLAG_EDITOR_READONLY = BITFLAG(5),		// This property can't be edited inside the editor
	PROPERTY_FLAG_EDITOR_HIDDEN = BITFLAG(6),		// This property is hidden in the editor
	PROPERTY_FLAG_IS_NOT_VALUE_TYPE = BITFLAG(7),	// This property requires more than just a memcpy for copying (ex: String)
	PROPERTY_FLAG_EDITOR_NO_RESIZE = BITFLAG(8)		// This property is supposed to be a DynamicArray and can't be resized in the editor
};

struct GroovyProperty
{
	std::string name;
	EPropertyType type;
	uint32 flags;
	uint32 offset;
	uint32 arrayCount;
	// asset type for ASSET_REF properties
	uint64 param1;
	// subclass filter for BlueprintRef
	uint64 param2;
};

struct DynamicArrayPtr
{
	void* (*data)(void*);
	size_t(*size)(void*);
	void (*resize)(void*, size_t);
	void (*clear)(void*);
	void (*removeAt)(void*, size_t);
	void (*insertAt)(void*, size_t);
	void (*add)(void*);
};

template<typename T>
struct PropType
{
	enum : uint64
	{
		Type = 0,
		Flags = 0,
		ArrayCount = 0,
		Param1 = 0,
		Param2 = 0
	};

	static_assert(!(sizeof(T) > 0), "Not implemented");
};

#define IMPL_PROPERTY_TYPE_EX(CoreType, OutPropertyType, ExFlags, ExParam1, ExParam2)	\
template<>																				\
struct PropType<CoreType> {																\
	enum : uint64 {																		\
		Type = OutPropertyType,															\
		Flags = ExFlags,																\
		ArrayCount = 1,																	\
		Param1 = ExParam1,																\
		Param2 = ExParam2																\
	};																					\
};																						\
template<size_t InArrayCount>															\
struct PropType<CoreType[InArrayCount]> {												\
	enum : uint64 {																		\
		Type = OutPropertyType,															\
		Flags = ExFlags | PROPERTY_FLAG_IS_ARRAY,										\
		ArrayCount = InArrayCount,														\
		Param1 = ExParam1,																\
		Param2 = ExParam2																\
	};																					\
};																						\
template<>																				\
struct PropType<std::vector<CoreType>> {												\
	enum : uint64 {																		\
		Type = OutPropertyType,															\
		Flags = ExFlags | PROPERTY_FLAG_IS_ARRAY | PROPERTY_FLAG_IS_DYNAMIC_ARRAY,		\
		ArrayCount = 0,																	\
		Param1 = ExParam1,																\
		Param2 = ExParam2																\
	};																					\
};

#define IMPL_PROPERTY_TYPE(CoreType, OutPropertyType) IMPL_PROPERTY_TYPE_EX(CoreType, OutPropertyType, 0, 0, 0)

IMPL_PROPERTY_TYPE(int32, PROPERTY_TYPE_INT32)
IMPL_PROPERTY_TYPE(int64, PROPERTY_TYPE_INT64)
IMPL_PROPERTY_TYPE(uint32, PROPERTY_TYPE_UINT32)
IMPL_PROPERTY_TYPE(uint64, PROPERTY_TYPE_UINT64)
IMPL_PROPERTY_TYPE(bool, PROPERTY_TYPE_BOOL)
IMPL_PROPERTY_TYPE(float, PROPERTY_TYPE_FLOAT)
IMPL_PROPERTY_TYPE(Vec2, PROPERTY_TYPE_VEC2)
IMPL_PROPERTY_TYPE(Vec3, PROPERTY_TYPE_VEC3)
IMPL_PROPERTY_TYPE(Vec4, PROPERTY_TYPE_VEC4)
IMPL_PROPERTY_TYPE(Transform, PROPERTY_TYPE_TRANSFORM)

IMPL_PROPERTY_TYPE_EX(std::string, PROPERTY_TYPE_STRING, PROPERTY_FLAG_IS_COMPLEX, 0, 0)
IMPL_PROPERTY_TYPE_EX(Buffer, PROPERTY_TYPE_BUFFER, PROPERTY_FLAG_IS_COMPLEX, 0, 0)

class Texture;
class Shader;
class Material;
class Mesh;
class Scene;

IMPL_PROPERTY_TYPE_EX(Texture*, PROPERTY_TYPE_ASSET_REF, PROPERTY_FLAG_IS_COMPLEX, ASSET_TYPE_TEXTURE, 0)
IMPL_PROPERTY_TYPE_EX(Shader*, PROPERTY_TYPE_ASSET_REF, PROPERTY_FLAG_IS_COMPLEX, ASSET_TYPE_SHADER, 0)
IMPL_PROPERTY_TYPE_EX(Material*, PROPERTY_TYPE_ASSET_REF, PROPERTY_FLAG_IS_COMPLEX, ASSET_TYPE_MATERIAL, 0)
IMPL_PROPERTY_TYPE_EX(Mesh*, PROPERTY_TYPE_ASSET_REF, PROPERTY_FLAG_IS_COMPLEX, ASSET_TYPE_MESH, 0)
IMPL_PROPERTY_TYPE_EX(Scene*, PROPERTY_TYPE_ASSET_REF, PROPERTY_FLAG_IS_COMPLEX, ASSET_TYPE_SCENE, 0)

template<typename TClass>
class BlueprintRef
{
public:
	using Class = TClass;

	AssetInstance* blueprint = nullptr;
};

static_assert(sizeof(BlueprintRef<int>) == sizeof(AssetInstance*));

template<typename TClass>
struct PropType<BlueprintRef<TClass>> {
	enum : uint64 {
		Type = PROPERTY_TYPE_ASSET_REF,
		Flags = PROPERTY_FLAG_IS_COMPLEX,
		ArrayCount = 1,
		Param1 = ASSET_TYPE_BLUEPRINT
	};

	static inline uint64 Param2 = (uint64)TClass::StaticClass();
};

template<typename TClass, size_t InArrayCount>									
struct PropType<BlueprintRef<TClass>[InArrayCount]> {
	enum : uint64 {
		Type = PROPERTY_TYPE_ASSET_REF,
		Flags = PROPERTY_FLAG_IS_COMPLEX | PROPERTY_FLAG_IS_ARRAY,
		ArrayCount = InArrayCount,
		Param1 = ASSET_TYPE_BLUEPRINT
	};

	static inline uint64 Param2 = (uint64)TClass::StaticClass();
};

template<typename TClass>
struct PropType<std::vector<BlueprintRef<TClass>>> {
	enum : uint64 {
		Type = PROPERTY_TYPE_ASSET_REF,
		Flags = PROPERTY_FLAG_IS_COMPLEX | PROPERTY_FLAG_IS_ARRAY | PROPERTY_FLAG_IS_DYNAMIC_ARRAY,
		ArrayCount = 0,
		Param1 = ASSET_TYPE_BLUEPRINT
	};

	static inline uint64 Param2 = (uint64)TClass::StaticClass();
};

typedef void(*GroovyConstructor)(void*);
typedef void(*GroovyDestructor)(void*);
typedef void(*GroovyPropertiesGetter)(std::vector<GroovyProperty>&);

class GroovyObject;

struct GroovyClass
{
	std::string name;
	uint32 size;
	GroovyConstructor constructor;
	GroovyDestructor destructor;
	GroovyClass* super;
	GroovyPropertiesGetter propertiesGetter;
	GroovyObject* cdo;
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
	static constexpr GroovyClass* StaticClass() { return &GROOVY_CLASS_NAME(Class); }				\
	inline GroovyObject* GetCDO() const { return GetClass()->cdo; }									\
	static GroovyObject* StaticCDO() { return GROOVY_CLASS_NAME(Class).cdo; }						\
private:

#define GROOVY_CLASS_IMPL(Class)	GroovyClass GROOVY_CLASS_NAME(Class) =						\
{																								\
	#Class,																						\
	sizeof(Class),																				\
	[](void* mem) { new(mem) Class(); },														\
	[](void* mem) { ((Class*)mem)->~Class(); },													\
	Class::Super::StaticClass(),																\
	&Class::GetClassProperties,																	\
	nullptr																						\
};																								\
void Class::GetClassPropertiesRecursive(std::vector<GroovyProperty>& outProps) const			\
{																								\
	Class::Super::GetClassProperties(outProps);													\
	Class::GetClassProperties(outProps);														\
}																								\
void Class::GetClassProperties(std::vector<GroovyProperty>& outProps) {						

#define GROOVY_PROPERTY(Class, Property, ExFlags)	{ #Property, (EPropertyType)PropType<decltype(Property)>::Type, PropType<decltype(Property)>::Flags | ExFlags, offsetof(Class, Property), PropType<decltype(Property)>::ArrayCount, PropType<decltype(Property)>::Param1, PropType<decltype(Property)>::Param2 }
#define GROOVY_REFLECT(Property)				outProps.push_back(GROOVY_PROPERTY(ThisClass, Property, 0));
#define GROOVY_REFLECT_EX(Property, ExFlags)	outProps.push_back(GROOVY_PROPERTY(ThisClass, Property, ExFlags));

#define GROOVY_CLASS_END() }

#define CLASS_LIST_BEGIN(ListName)				std::vector<GroovyClass*> ListName = 
#define CLASS_LIST_ADD(Class)					&GROOVY_CLASS_NAME(Class)
#define CLASS_LIST_END()						;

struct PropertyDesc
{
	const GroovyProperty* classProp;
	uint32 arrayCount;
	size_t sizeBytes;
};

struct PropertyPack
{
	PropertyPack() {}

	PropertyPack(const PropertyPack& copyPack)
		: desc(copyPack.desc), data(copyPack.data)
	{}

	PropertyPack(PropertyPack&& movePack)
		: desc(std::move(movePack.desc)), data(std::move(movePack.data))
	{}

	PropertyPack& operator=(const PropertyPack& copyPack)
	{
		desc = copyPack.desc;
		data = copyPack.data;
		return *this;
	}

	PropertyPack& operator=(PropertyPack&& movePack)
	{
		desc = std::move(movePack.desc);
		data = std::move(movePack.data);
		return *this;
	}

	std::vector<PropertyDesc> desc;
	DynamicBuffer data;
};

size_t GroovyProperty_GetSize(EPropertyType type);
DynamicArrayPtr GroovyProperty_GetDynamicArrayPtr(EPropertyType type);

GroovyObject* GroovyClass_DynamicCast(GroovyObject* obj, const GroovyClass* gClass);
bool GroovyClass_IsA(const GroovyClass* c1, const GroovyClass* c2);

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

	GROOVY_CLASS_IMPL(MyClass)
		GROOVY_REFLECT(mData1)
		GROOVY_REFLECT(mData2)
		GROOVY_REFLECT_EX(mDead, PROPERTY_FLAG_NOSERIALIZE)
	GROOVY_CLASS_END()

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