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

struct PropertyData
{
	void* data;
	size_t arrayCount;
};

struct PropertyToSerialize
{
	std::string name;
	PropertyData data;
	uint64 flags;
	EPropertyType type;
	size_t totalSize;
};

PropertyData GetPropertyData(const GroovyProperty& prop, void* obj)
{
	void* propInObj = (byte*)obj + prop.offset;

	if (prop.flags & PROPERTY_FLAG_IS_DYNAMIC_ARRAY)
	{
		DynamicArrayPtr dap = GetDynamicArrayPtr(propInObj, prop.type);
		return { dap.data(propInObj), dap.size(propInObj) };
	}
	else
	{
		return { propInObj, prop.arrayCount };
	}
}

static bool PropertyNeedsSerialization(PropertyData propData1, PropertyData propData2, EPropertyType type)
{
	if (propData1.arrayCount != propData2.arrayCount)
		return true;

	if (type != PROPERTY_TYPE_STRING)
	{
		return memcmp(propData1.data, propData2.data, GetPropertySize(type) * propData1.arrayCount) != 0;
	}
	else
	{
		std::string* strPtr1 = (std::string*)propData1.data;
		std::string* strPtr2 = (std::string*)propData2.data;
		for (size_t i = 0; i < propData1.arrayCount; i++)
		{
			if (*strPtr1 != *strPtr2)
			{
				return true;
			}

			strPtr1++;
			strPtr2++;
		}

		return false;
	}
}

static void SerializeString(const std::string& str, DynamicBuffer& outFileData)
{
	outFileData.push(str.data(), str.length());
	outFileData.push<char>(0); // null termination character
}

static std::string DeserializeString(BufferView& fileData)
{
	std::string str((char*)fileData.seek());
	fileData.advance(str.length() + 1 /* null termination character */);
	return str;
}

static size_t SerializePropertyData(const PropertyToSerialize& prop, DynamicBuffer& outFileData)
{
	if (prop.type != PROPERTY_TYPE_STRING)
	{
		size_t dataWidth = GetPropertySize(prop.type) * prop.data.arrayCount;
		outFileData.push_data(prop.data.data, dataWidth);
		return dataWidth;
	}
	else
	{
		size_t serializedBytes = 0;
		std::string* strPtr = (std::string*)prop.data.data;
		for (size_t i = 0; i < prop.data.arrayCount; i++)
		{
			SerializeString(*strPtr, outFileData);
			serializedBytes += strPtr->size() + 1 /* null ternmination character */;
			strPtr++;
		}
		return serializedBytes;
	}
}

void SerializeProperties(const std::vector<PropertyToSerialize>& props, DynamicBuffer& outFileData)
{
	outFileData.push(props.size());
	for (const auto& prop : props)
	{
		SerializeString(prop.name, outFileData);
		outFileData.push(prop.data.arrayCount);
		size_t* bytesUsedFile = outFileData.push<size_t>(4);
		size_t bytesUsed = SerializePropertyData(prop, outFileData);
		*(size_t*)(outFileData.current() - bytesUsed - sizeof(size_t)) = bytesUsed;
	}
}

extern ClassDB gClassDB;

void ObjectSerializer::SerializeObject(GroovyObject* obj, GroovyObject* cdo, DynamicBuffer& outFileData)
{
	checkslow(obj && cdo);
	checkslowf(obj->GetClass() == cdo->GetClass(), "Trying to serialize object with a CDO of a different class");
	
	GroovyClass* gClass = obj->GetClass();
	std::vector<PropertyToSerialize> propsToSerialize;
	
	// retrive props that need to be serialized
	{
		const std::vector<GroovyProperty>& props = gClassDB[gClass];
		for (const auto& prop : props)
		{	
			if (prop.editorFlags & PROPERTY_EDITOR_FLAG_NOSERIALIZE)
				continue;	// flagged not to be serialized

			PropertyData objData = GetPropertyData(prop, obj);
			PropertyData cdoData = GetPropertyData(prop, cdo);

			if (!PropertyNeedsSerialization(objData, cdoData, prop.type))
				continue; // property is same as cdo's, no need to serialize

			PropertyToSerialize& pts = propsToSerialize.emplace_back();
			pts.name = prop.name;
			pts.data = objData;
			pts.flags = prop.flags;
			pts.type = prop.type;
		}
	}
	
	// serialize classname
	SerializeString(gClass->name, outFileData);
	
	// serialize all the properties
	SerializeProperties(propsToSerialize, outFileData);
}

struct PropertyToDeserialize
{
	const GroovyProperty* prop;
	PropertyData data;
};

static void DeserializePropertyData(PropertyToDeserialize prop, void* outData)
{
	if (prop.prop->type != PROPERTY_TYPE_STRING)
	{
		size_t dataWidth = GetPropertySize(prop.prop->type);
		memcpy(outData, prop.data.data, dataWidth * prop.data.arrayCount);
	}
	else
	{
		byte* propPtr = (byte*)prop.data.data;
		std::string* strPtr = (std::string*)outData;
		for (size_t i = 0; i < prop.data.arrayCount; i++)
		{
			*strPtr = std::string((char*)propPtr);
			propPtr += strPtr->length() + 1 /* null termination character */;
			strPtr++;
		}
	}
}

static void DeserializeProperty(PropertyToDeserialize prop, GroovyObject* obj)
{
	void* objPropData = (byte*)obj + prop.prop->offset;

	if (prop.prop->flags & PROPERTY_FLAG_IS_DYNAMIC_ARRAY)
	{
		DynamicArrayPtr dap = GetDynamicArrayPtr(objPropData, prop.prop->type);
		dap.resize(objPropData, prop.data.arrayCount);
		DeserializePropertyData(prop, dap.data(objPropData));
	}
	else
	{
		DeserializePropertyData(prop, objPropData);
	}
}

GroovyObject* ObjectSerializer::DeserializeObject(BufferView fileData)
{
	std::string className = DeserializeString(fileData);

	GroovyClass* gClass = gClassDB[className];
	checkslowf(gClass, "Unable to deserialize class %s, not found in classDB", className.c_str());

	GroovyObject* newObj = (GroovyObject*)malloc(gClass->size);
	gClass->constructor(newObj);

	const std::vector<GroovyProperty>& props = gClassDB[gClass];

	size_t serializedPropsCount = fileData.read<size_t>();

	for (size_t i = 0; i < serializedPropsCount; i++)
	{
		std::string propName = DeserializeString(fileData);
		size_t propArrayCount = fileData.read<size_t>();
		size_t propBytesUsed = fileData.read<size_t>();
		
		size_t propIndex = groovyclassUtils::FindProperty(props, propName);

		if (propIndex < props.size()) // property found!
		{
			PropertyToDeserialize ptd;
			ptd.prop = &props[propIndex];
			ptd.data.data = fileData.seek();
			ptd.data.arrayCount = propArrayCount;
			DeserializeProperty(ptd, newObj);
		}

		fileData.advance(propBytesUsed);
	}

	return newObj;
}
