#include "reflection.h"
#include "object.h"
#include "class_db.h"

struct SubmeshData
{
	uint32 vertexCount;
	uint32 indexCount;
};

size_t reflectionUtils::GetPropertySize(EPropertyType type)
{
	switch (type)
	{
		case PROPERTY_TYPE_INT32:		return sizeof(int32);
		case PROPERTY_TYPE_INT64:		return sizeof(int64);
		case PROPERTY_TYPE_UINT32:		return sizeof(uint32);
		case PROPERTY_TYPE_UINT64:		return sizeof(uint64);
		case PROPERTY_TYPE_BOOL:		return sizeof(bool);
		case PROPERTY_TYPE_FLOAT:		return sizeof(float);
		case PROPERTY_TYPE_VEC2:		return sizeof(Vec2);
		case PROPERTY_TYPE_VEC3:		return sizeof(Vec3);
		case PROPERTY_TYPE_VEC4:		return sizeof(Vec4);
		case PROPERTY_TYPE_TRANSFORM:	return sizeof(Transform);
		case PROPERTY_TYPE_STRING:		return sizeof(std::string);
		case PROPERTY_TYPE_BUFFER:		return sizeof(Buffer);
		case PROPERTY_TYPE_ASSET_REF:	return sizeof(AssetInstance*);
		case PROPERTY_TYPE_INTERNAL_SUBMESHDATA:	return sizeof(SubmeshData);
	}
	checkslowf(0, "Type not implemented");
	return 0;
}

// implementation for std::vector
#define GET_GDAPTR(Type)																					\
{																											\
	[](void* m) { return (void*)((std::vector<Type>*)m)->data(); },											\
	[](void* m) { return ((std::vector<Type>*)m)->size(); },												\
	[](void* m, size_t s) { ((std::vector<Type>*)m)->resize(s); },											\
	[](void* m) {((std::vector<Type>*)m)->clear(); },														\
	[](void* m, size_t i) { ((std::vector<Type>*)m)->erase(((std::vector<Type>*)m)->begin() + i); },		\
	[](void* m, size_t i) { ((std::vector<Type>*)m)->insert(((std::vector<Type>*)m)->begin() + i, 1, {}); },	\
	[](void* m) { ((std::vector<Type>*)m)->push_back({}); }													\
}

DynamicArrayPtr reflectionUtils::GetDynamicArrayPtr(EPropertyType type)
{
	switch (type)
	{
		case PROPERTY_TYPE_INT32:		return GET_GDAPTR(int32);
		case PROPERTY_TYPE_INT64:		return GET_GDAPTR(int64);
		case PROPERTY_TYPE_UINT32:		return GET_GDAPTR(uint32);
		case PROPERTY_TYPE_UINT64:		return GET_GDAPTR(uint64);
		// case PROPERTY_TYPE_BOOL:		return GET_GDAPTR(bool); std::vector<bool> has a strange specialization
		case PROPERTY_TYPE_FLOAT:		return GET_GDAPTR(float);
		case PROPERTY_TYPE_VEC2:		return GET_GDAPTR(Vec2);
		case PROPERTY_TYPE_VEC3:		return GET_GDAPTR(Vec3);
		case PROPERTY_TYPE_VEC4:		return GET_GDAPTR(Vec4);
		case PROPERTY_TYPE_TRANSFORM:	return GET_GDAPTR(Transform);
		case PROPERTY_TYPE_STRING:		return GET_GDAPTR(std::string);
		// case PROPERTY_TYPE_BUFFER:		return GET_GDAPTR(Buffer); not implemented
		case PROPERTY_TYPE_ASSET_REF:	return GET_GDAPTR(AssetInstance*);
		case PROPERTY_TYPE_INTERNAL_SUBMESHDATA:	return GET_GDAPTR(SubmeshData);
	}
	checkslowf(0, "Type not implemented");
	return {};
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
		DynamicArrayPtr dap = reflectionUtils::GetDynamicArrayPtr(prop->type);
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
		memcpy(dstProp, srcProp, reflectionUtils::GetPropertySize(prop->type) * propArrayCount);
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

extern ClassDB gClassDB;

void reflectionUtils::CopyProperties(GroovyObject* dst, const GroovyObject* src)
{
	check(dst && src);
	
	const std::vector<GroovyProperty>& props = gClassDB[src->GetClass()];
	
	for (const GroovyProperty& p : props)
		CopyProperty(dst, src, &p);
}
