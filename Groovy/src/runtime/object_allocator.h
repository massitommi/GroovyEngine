#pragma once

#include "classes/class.h"

class CORE_API ObjectAllocator
{
public:

	static GroovyObject* Instantiate(GroovyClass* gClass);
	static void Destroy(GroovyObject* instance);

	template<typename TCastClass>
	inline static TCastClass* Instantiate(GroovyClass* gClass)
	{
		return (TCastClass*)Instantiate(gClass);
	}

	template<typename TClass>
	inline static TClass* Instantiate()
	{
		return (TClass*)Instantiate(TClass::StaticClass());
	}

	static uint32 Debug_GetLiveObjectsCount();
};