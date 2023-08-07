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
		uint32 srcNum = dap.size(srcProp);
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
				propArrayCount = dap.size(propData);
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