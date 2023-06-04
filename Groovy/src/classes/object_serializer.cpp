#include "object_serializer.h"
#include "class_db.h"

static size_t GetPropertySize(EPropertyType type)
{
	switch (type)
	{
		case PROPERTY_TYPE_INT32:		return sizeof(int32);
		case PROPERTY_TYPE_INT64:		return sizeof(int64);
		case PROPERTY_TYPE_UINT32:		return sizeof(uint32);
		case PROPERTY_TYPE_UINT64:		return sizeof(uint64);
		case PROPERTY_TYPE_BOOL:		return sizeof(bool);
		case PROPERTY_TYPE_FLOAT:		return sizeof(float);
		case PROPERTY_TYPE_STRING:		return sizeof(std::string);
		case PROPERTY_TYPE_VEC3:		return sizeof(Vec3);
		case PROPERTY_TYPE_TRANSFORM:	return sizeof(Transform);
	}
	check(0);
	return 0;
}

struct DynamicArrayPtr
{
	void* (*data)(void*);
	size_t (*size)(void*);
	void (*resize)(void*, size_t);
};

#define GET_DVEC_PTR(Vec, Type)	{ \
[](void* m) { return (void*)((std::vector<Type>*)m)->data(); }, \
[](void* m) { return ((std::vector<Type>*)m)->size(); },		\
[](void* m, size_t s) { ((std::vector<Type>*)m)->resize(s); } }

DynamicArrayPtr GetDynamicArrayPtr(void* vecPtr, EPropertyType type)
{
	switch (type)
	{
		case PROPERTY_TYPE_INT32:		return GET_DVEC_PTR(vecPtr, int32);
		case PROPERTY_TYPE_INT64:		return GET_DVEC_PTR(vecPtr, int64);
		case PROPERTY_TYPE_UINT32:		return GET_DVEC_PTR(vecPtr, uint32);
		case PROPERTY_TYPE_UINT64:		return GET_DVEC_PTR(vecPtr, uint64);
		case PROPERTY_TYPE_BOOL:		return GET_DVEC_PTR(vecPtr, byte); // std::vector<bool> has a strange specialization
		case PROPERTY_TYPE_FLOAT:		return GET_DVEC_PTR(vecPtr, float);
		case PROPERTY_TYPE_STRING:		return GET_DVEC_PTR(vecPtr, std::string);
		case PROPERTY_TYPE_VEC3:		return GET_DVEC_PTR(vecPtr, Vec3);
		case PROPERTY_TYPE_TRANSFORM:	return GET_DVEC_PTR(vecPtr, Transform);
	}
	check(0);
	return {};
}

struct FileProperty
{
	std::string name;
	void* data;
	size_t arrayCount;
	EPropertyType type;
	uint64 flags;
};

static void SerializeProperty(const std::string& name, EPropertyType type, size_t arrayCount, uint64 flags, void* data, DynamicBuffer& outFileData)
{
	outFileData.push(name.size());
	outFileData.push(name.c_str(), name.size());
	outFileData.push(type);
	outFileData.push(arrayCount);
	outFileData.push(flags);

	if (type != PROPERTY_TYPE_STRING)
	{
		size_t dataWidth = GetPropertySize(type);
		outFileData.push_data(data, dataWidth * arrayCount);
	}
	else
	{
		std::string* strPtr = (std::string*)data;
		for (size_t i = 0; i < arrayCount; i++)
		{
			outFileData.push(strPtr->size());
			outFileData.push(strPtr->data());
			strPtr++;
		}
	}
}

extern ClassDB gClassDB;

void ObjectSerializer::SerializeObject(GroovyObject* obj, DynamicBuffer& outFileData)
{
	check(obj);
	GroovyClass* gClass = obj->GetClass();
	std::vector<const GroovyProperty*> propsToSerialize;
	
	// retrive props that need to be serialized
	{
		const std::vector<GroovyProperty>& props = gClassDB[gClass];
		propsToSerialize.reserve(props.size());
		for (const auto& prop : props)
			if (prop.editorFlags & PROPERTY_EDITOR_FLAG_NOSERIALIZE)
				propsToSerialize.push_back(&prop);
	}
	
	// file "header"
	outFileData.push(gClass->name.size());	// classname length
	outFileData.push(gClass->name.c_str(), gClass->name.size()); // classname
	outFileData.push(propsToSerialize.size()); // property count
	
	// serialize all the properties
	for (const GroovyProperty* prop : propsToSerialize)
	{
		// property "header"
		void* propDataPtr = (byte*)obj + prop->offset;
		size_t propArrayCount = prop->arrayCount;

		if (prop->flags & PROPERTY_FLAG_IS_DYNAMIC_ARRAY)
		{
			void* vecPtr = propDataPtr;
			DynamicArrayPtr vec = GetDynamicArrayPtr(vecPtr, prop->type);
			propArrayCount = vec.size(vecPtr);
			propDataPtr = vec.data(vecPtr);
		}

		SerializeProperty(prop->name, prop->type, propArrayCount, prop->flags, propDataPtr, outFileData);
	}
}

GroovyObject* ObjectSerializer::DeserializeObject(BufferView fileData)
{
	return nullptr;
}
