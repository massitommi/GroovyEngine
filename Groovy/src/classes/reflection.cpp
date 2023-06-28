#include "reflection.h"

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
		case PROPERTY_TYPE_STRING:		return sizeof(std::string);
		case PROPERTY_TYPE_VEC3:		return sizeof(Vec3);
		case PROPERTY_TYPE_TRANSFORM:	return sizeof(Transform);
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
		//	case PROPERTY_TYPE_BOOL:		return GET_GDAPTR(bool); // std::vector<bool> has a strange specialization
		case PROPERTY_TYPE_FLOAT:		return GET_GDAPTR(float);
		case PROPERTY_TYPE_STRING:		return GET_GDAPTR(std::string);
		case PROPERTY_TYPE_VEC3:		return GET_GDAPTR(Vec3);
		case PROPERTY_TYPE_TRANSFORM:	return GET_GDAPTR(Transform);
		case PROPERTY_TYPE_ASSET_REF:	return GET_GDAPTR(AssetInstance*);
		case PROPERTY_TYPE_INTERNAL_SUBMESHDATA:	return GET_GDAPTR(SubmeshData);
	}
	checkslowf(0, "Type not implemented");
	return {};
}
