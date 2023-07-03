#pragma once

#include "object.h"

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
	static void CreatePropertyPack(GroovyObject* obj, GroovyObject* cdo, PropertyPack& outPack);

	static void SerializePropertyPack(const PropertyPack& pack, DynamicBuffer& fileData);
	static void DeserializePropertyPack(GroovyClass* gClass, BufferView& fileData, PropertyPack& outPack);

	static void DeserializePropertyPackData(const PropertyPack& pack, GroovyObject* obj);
};