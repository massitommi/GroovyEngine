#include "actor.h"
#include "actorcomponent.h"
#include "runtime/object_allocator.h"

GROOVY_CLASS_IMPL(Actor)
GROOVY_CLASS_END()

Actor::Actor()
	: mTransform{{ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }}
{
}

Actor::~Actor()
{
	for (ActorComponent* component : mComponents)
	{
		ObjectAllocator::Destroy(component);
	}
}

ActorComponent* Actor::GetComponent(const std::string& name) const
{
	return mComponentsDB.find(name)->second;
}

ActorComponent* Actor::GetComponent(GroovyClass* componentClass) const
{
	for (ActorComponent* comp : mComponents)
		if (comp->IsA(componentClass))
			return comp;
	return nullptr;
}

ActorComponent* Actor::GetComponentExact(GroovyClass* componentClass) const
{
	for (ActorComponent* comp : mComponents)
		if (comp->GetClass() == componentClass)
			return comp;
	return nullptr;
}

uint32 Actor::GetComponents(GroovyClass* componentClass, std::vector<ActorComponent*>& outComponents) const
{
	uint32 found = 0;
	
	for (ActorComponent* comp : mComponents)
		if (comp->IsA(componentClass))
		{
			outComponents.push_back(comp);
			found++;
		}

	return found;
}

uint32 Actor::GetComponentsExact(GroovyClass* componentClass, std::vector<ActorComponent*>& outComponents) const
{
	uint32 found = 0;

	for (ActorComponent* comp : mComponents)
		if (comp->GetClass() == componentClass)
		{
			outComponents.push_back(comp);
			found++;
		}

	return found;
}

ActorComponent* Actor::__internal_AddComponent(GroovyClass* componentClass, const std::string& name)
{
	check(componentClass);
	checkslow(classUtils::IsA(componentClass, ActorComponent::StaticClass()));
	check(!name.empty());

	ActorComponent*& dbRecord = mComponentsDB[name];
	checkf(dbRecord == nullptr, "Component with name %s already exists!", name.c_str());

	ActorComponent* newComponent = ObjectAllocator::Instantiate<ActorComponent>(componentClass);

	dbRecord = newComponent;
	mComponents.push_back(newComponent);

	newComponent->mName = name;

	return newComponent;
}

ActorComponent* Actor::AddComponent(GroovyClass* componentClass, const std::string& name)
{
	ActorComponent* newComponent = __internal_AddComponent(componentClass, name);
	mNativeComponents.push_back(newComponent);
	return newComponent;
}

ActorComponent* Actor::__internal_AddEditorComponent(GroovyClass* componentClass, const std::string& name)
{
	ActorComponent* newComponent = __internal_AddComponent(componentClass, name);
	mEditorComponents.push_back(newComponent);
	return newComponent;
}

#if WITH_EDITOR

void Actor::__internal_Editor_RemoveEditorComponent(ActorComponent* component)
{
	checkslow(component);
	
	auto it = std::find(mComponents.begin(), mComponents.end(), component);

	checkslowf(it != mComponents.end(), "Editor bug, trying to remove a component that doesn't belong to this actor");

	mComponents.erase(it);
	mComponentsDB.erase(component->GetName());

	ObjectAllocator::Destroy(component);
}

#endif