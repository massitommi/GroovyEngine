#pragma once

#include "class.h"

struct DynamicArrayPtr
{
	void* (*data)(void*);
	size_t(*size)(void*);
	void (*resize)(void*, size_t);
	void (*clear)(void*);
	void (*removeAt)(void*, size_t);
	void (*insertAt)(void*, size_t);
	void (*add)(void*);
};

namespace reflectionUtils
{
	size_t GetPropertySize(EPropertyType type);
	DynamicArrayPtr GetDynamicArrayPtr(EPropertyType type);
	void CopyProperty(GroovyObject* dst, const GroovyObject* src, const GroovyProperty* prop);
	void CopyProperties(GroovyObject* dst, const GroovyObject* src);
}