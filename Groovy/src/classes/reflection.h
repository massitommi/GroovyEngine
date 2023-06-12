#pragma once

#include "class.h"

struct DynamicArrayPtr
{
	void* (*data)(void*);
	size_t(*size)(void*);
	void (*resize)(void*, size_t);
};

namespace reflectionUtils
{
	size_t GetPropertySize(EPropertyType type);
	DynamicArrayPtr GetDynamicArrayPtr(EPropertyType type);
}