#include "object_serializer.h"
#include "class_db.h"

static void DeserializePropertyData(const PropertyDesc& desc, byte* data, GroovyObject* obj)
{
	void* objProp = (byte*)obj + desc.classProp->offset;

	if (desc.classProp->flags & PROPERTY_FLAG_IS_DYNAMIC_ARRAY)
	{
		DynamicArrayPtr dap = reflectionUtils::GetDynamicArrayPtr(desc.classProp->type);
		dap.resize(objProp, desc.arrayCount);
		objProp = dap.data(objProp);
	}

	if (desc.classProp->type != PROPERTY_TYPE_STRING)
	{
		memcpy(objProp, data, desc.sizeBytes);
	}
	else
	{
		std::string* strPtr = (std::string*)objProp;
		for (size_t i = 0; i < desc.arrayCount; i++)
		{
			*strPtr = (char*)data;
			data += strPtr->length() + 1;
			strPtr++;
		}
	}
}

void ObjectSerializer::DeserializeOntoObject(const std::vector<PropertyDesc>& desc, byte* data, GroovyObject* obj)
{
	for (const PropertyDesc& d : desc)
	{
		DeserializePropertyData(d, data, obj);
		data += d.sizeBytes;
	}
}

static bool PropertyIsEqual(const GroovyProperty& prop, GroovyObject* obj1, GroovyObject* obj2)
{
	void* objProp1 = (byte*)obj1 + prop.offset;
	void* objProp2 = (byte*)obj2 + prop.offset;
	size_t objProp1ArrayCount = prop.arrayCount;
	size_t objProp2ArrayCount = prop.arrayCount;

	if (prop.flags & PROPERTY_FLAG_IS_DYNAMIC_ARRAY)
	{
		DynamicArrayPtr dap = reflectionUtils::GetDynamicArrayPtr(prop.type);
		objProp1ArrayCount = dap.size(objProp1);
		objProp2ArrayCount = dap.size(objProp2);
		objProp1 = dap.data(objProp1);
		objProp2 = dap.data(objProp2);
	}

	if (objProp1ArrayCount != objProp2ArrayCount)
		return false;

	if (prop.type != PROPERTY_TYPE_STRING)
	{
		return memcmp(objProp1, objProp2, reflectionUtils::GetPropertySize(prop.type) * objProp1ArrayCount) == 0;
	}
	else
	{
		std::string* strPtr1 = (std::string*)objProp1;
		std::string* strPtr2 = (std::string*)objProp2;
		for (size_t i = 0; i < objProp1ArrayCount; i++)
		{
			if (*strPtr1 != *strPtr2)
				return false;
			strPtr1++;
			strPtr2++;
		}
		return true;
	}
}

static void SerializePropertyData(PropertyPack& pack, const GroovyProperty& prop, GroovyObject* obj)
{
	void* objProp = (byte*)obj + prop.offset;
	size_t objPropArrayCount = prop.arrayCount;

	if (prop.flags & PROPERTY_FLAG_IS_DYNAMIC_ARRAY)
	{
		DynamicArrayPtr dap = reflectionUtils::GetDynamicArrayPtr(prop.type);
		objPropArrayCount = dap.size(objProp);
		objProp = dap.data(objProp);
	}

	PropertyDesc& desc = pack.desc.emplace_back();
	desc.arrayCount = objPropArrayCount;
	desc.classProp = &prop;

	if (prop.type != PROPERTY_TYPE_STRING)
	{
		size_t dataWidth = reflectionUtils::GetPropertySize(prop.type) * objPropArrayCount;
		pack.data.push_data(objProp, dataWidth);
		desc.sizeBytes = dataWidth;
	}
	else
	{
		std::string* strPtr = (std::string*)objProp;
		for (size_t i = 0; i < objPropArrayCount; i++)
		{
			pack.data.push_data(strPtr->data(), strPtr->length());
			pack.data.push<char>(0);
			desc.sizeBytes += strPtr->length() + 1;
			strPtr++;
		}
	}
}

void ObjectSerializer::CreatePropertyPack(PropertyPack& pack, GroovyObject* obj, GroovyObject* cdo)
{
	extern ClassDB gClassDB;

	check(obj && cdo);
	check(obj->GetClass() == cdo->GetClass());

	const std::vector<GroovyProperty>& props = gClassDB[obj->GetClass()];

	for (const GroovyProperty& p : props)
	{
		if (p.editorFlags & PROPERTY_EDITOR_FLAG_NOSERIALIZE)
			continue;

		if (PropertyIsEqual(p, obj, cdo))
			continue;

		SerializePropertyData(pack, p, obj);
	}
}
