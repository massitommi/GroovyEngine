#include "object_allocator.h"
#include "classes/object.h"

static std::vector<GroovyObject*> sObjects;

GroovyObject* ObjectAllocator::Instantiate(GroovyClass* gClass)
{
	check(gClass);

	GroovyObject* obj = (GroovyObject*)malloc(gClass->size);
	gClass->constructor(obj);

	sObjects.push_back(obj);

	return obj;
}

void ObjectAllocator::Destroy(GroovyObject* instance)
{
	check(instance);

	instance->GetClass()->destructor(instance);
	free(instance);

	sObjects.erase(std::find(sObjects.begin(), sObjects.end(), instance));
}

uint32 ObjectAllocator::Debug_GetLiveObjectsCount()
{
	return sObjects.size();
}
