#pragma once

#include "object.h"
#include "reflection.h"

struct PropertyDesc
{
	const GroovyProperty* classProp;
	uint32 arrayCount;
	size_t sizeBytes;
};

struct PropertyPack
{
	std::vector<PropertyDesc> desc;
	DynamicBuffer data;
};

class ObjectSerializer
{
public:
	static void DeserializeOntoObject(const std::vector<PropertyDesc>& desc, byte* data, GroovyObject* obj);
	static void CreatePropertyPack(PropertyPack& pack, GroovyObject* obj, GroovyObject* cdo);

	static void SerializeSimpleObject(GroovyObject* obj, GroovyObject* cdo, DynamicBuffer& fileData);
	static void DeserializeSimpleObject(GroovyObject* obj, BufferView& fileData);
};