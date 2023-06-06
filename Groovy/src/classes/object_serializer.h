#pragma once

#include "object.h"

class ObjectSerializer
{
public:
	static void SerializeObject(GroovyObject* obj, GroovyObject* cdo, DynamicBuffer& outFileData);
	static GroovyObject* DeserializeObject(BufferView fileData);
};