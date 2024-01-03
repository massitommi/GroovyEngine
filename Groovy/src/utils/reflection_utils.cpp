#include "reflection_utils.h"
#include "classes/class_db.h"
#include "classes/object.h"

extern ClassDB gClassDB;

void reflectionUtils::GetClassPropertiesRecursiveSorted(GroovyClass* gClass, std::vector<GroovyProperty>& outProps)
{
	GroovyClass* c = gClass;
	if (c->super)
	{
		GetClassPropertiesRecursiveSorted(c->super, outProps);
	}
	check(gClass->propertiesGetter);
	gClass->propertiesGetter(outProps);
}

uint32 reflectionUtils::FindProperty(const std::vector<GroovyProperty>& props, const std::string& propName)
{
	for (uint32 i = 0; i < props.size(); i++)
		if (props[i].name == propName)
			return i;
	return ~((uint32)0);
}

void reflectionUtils::CopyProperty(GroovyObject* dst, const GroovyObject* src, const GroovyProperty* prop)
{
	check(dst && src && prop);
	check(dst->GetClass() == src->GetClass());

	void* dstProp = (byte*)dst + prop->offset;
	void* srcProp = (byte*)src + prop->offset;
	uint32 propArrayCount = prop->arrayCount;

	if (prop->flags & PROPERTY_FLAG_IS_DYNAMIC_ARRAY)
	{
		DynamicArrayPtr dap = GroovyProperty_GetDynamicArrayPtr(prop->type);
		// resize
		uint32 srcNum = (uint32)dap.size(srcProp);
		propArrayCount = srcNum;
		dap.resize(dstProp, srcNum);
		// update ptrs
		dstProp = dap.data(dstProp);
		srcProp = dap.data(srcProp);
	}

	if (!(prop->flags & PROPERTY_FLAG_IS_NOT_VALUE_TYPE))
	{
		memcpy(dstProp, srcProp, GroovyProperty_GetSize(prop->type) * propArrayCount);
	}
	else
	{
		switch (prop->type)
		{
			case PROPERTY_TYPE_STRING:
			{
				for (uint32 i = 0; i < propArrayCount; i++)
				{
					std::string& dstStr = ((std::string*)dstProp)[i];
					std::string& srcStr = ((std::string*)srcProp)[i];

					dstStr = srcStr;
				}
			}
			break;

			default:
			{
				checkslowf(0, "Property type not implemented");
			}
		}
	}
}

void reflectionUtils::CopyProperties(GroovyObject* dst, const GroovyObject* src)
{
	check(dst && src);

	const std::vector<GroovyProperty>& props = gClassDB[src->GetClass()];

	for (const GroovyProperty& p : props)
		CopyProperty(dst, src, &p);
}

bool reflectionUtils::PropertyIsEqual(GroovyObject* obj1, GroovyObject* obj2, const GroovyProperty* prop)
{
	void* objProp1 = (byte*)obj1 + prop->offset;
	void* objProp2 = (byte*)obj2 + prop->offset;
	uint32 objProp1ArrayCount = prop->arrayCount;
	uint32 objProp2ArrayCount = prop->arrayCount;

	if (prop->flags & PROPERTY_FLAG_IS_DYNAMIC_ARRAY)
	{
		DynamicArrayPtr dap = GroovyProperty_GetDynamicArrayPtr(prop->type);
		objProp1ArrayCount = (uint32)dap.size(objProp1);
		objProp2ArrayCount = (uint32)dap.size(objProp2);
		objProp1 = dap.data(objProp1);
		objProp2 = dap.data(objProp2);

		if (objProp1ArrayCount != objProp2ArrayCount)
			return false;
	}

	if (!(prop->flags & PROPERTY_FLAG_IS_NOT_VALUE_TYPE))
	{
		return memcmp(objProp1, objProp2, GroovyProperty_GetSize(prop->type) * objProp1ArrayCount) == 0;
	}
	else
	{
		switch (prop->type)
		{
			case PROPERTY_TYPE_STRING:
			{
				std::string* strPtr1 = (std::string*)objProp1;
				std::string* strPtr2 = (std::string*)objProp2;
				for (uint32 i = 0; i < objProp1ArrayCount; i++)
				{
					if (strPtr1[i] != strPtr2[i])
						return false;
				}
				return true;
			}
			break;

			case PROPERTY_TYPE_BUFFER:
			{
				Buffer* bufferPtr1 = (Buffer*)objProp1;
				Buffer* bufferPtr2 = (Buffer*)objProp2;
				for (uint32 i = 0; i < objProp1ArrayCount; i++)
				{
					if (bufferPtr1[i].size() != bufferPtr2[i].size())
						return false;
					if (memcmp(bufferPtr1[i].data(), bufferPtr2[i].data(), bufferPtr1[i].size()) != 0)
						return false;
				}
				return true;
			}
			break;

			default:
				checkslowf(0, "Property comparison for this type not implemented");
				return false;
		}
	}
}

void reflectionUtils::ReplaceValueTypeProperty(GroovyObject* obj, EPropertyType type, void* find, void* replaceWith)
{
	check(obj);
	check(find && replaceWith);

	const std::vector<GroovyProperty>& props = gClassDB[obj->GetClass()];

	for (const GroovyProperty& prop : props)
	{
		if (prop.type == type)
		{
			byte* propData = (byte*)obj + prop.offset;
			uint32 propArrayCount = prop.arrayCount;
			size_t propSize = GroovyProperty_GetSize(type);

			if (prop.flags & PROPERTY_FLAG_IS_DYNAMIC_ARRAY)
			{
				DynamicArrayPtr dap = GroovyProperty_GetDynamicArrayPtr(type);
				propArrayCount = (uint32)dap.size(propData);
				propData = (byte*)dap.data(propData);
			}

			for (uint32 i = 0; i < propArrayCount; i++)
			{
				if (memcmp(propData, find, propSize) == 0)
				{
					memcpy(propData, replaceWith, propSize);
				}

				propData += propSize;
			}
		}
	}
}