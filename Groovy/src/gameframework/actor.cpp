#include "actor.h"
#include "actorcomponent.h"
#include "blueprint.h"
#include "runtime/object_allocator.h"

GROOVY_CLASS_IMPL(Actor)
	GROOVY_REFLECT(mShouldTick)
GROOVY_CLASS_END()

Actor::Actor()
	: mTransform{{ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }},
	mName("Actor"), mShouldTick(true), mScene(nullptr), mTemplate(nullptr)
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
	auto it = mComponentsDB.find(name);
	if (it != mComponentsDB.end())
		return it->second;
	return nullptr;
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

void Actor::InitializeComponents()
{
	for (ActorComponent* comp : mComponents)
		comp->Initialize();
}

void Actor::UninitializeComponents()
{
	for (ActorComponent* comp : mComponents)
		comp->Uninitialize();
}

void Actor::BeginPlay()
{
}

void Actor::Tick(float deltaTime)
{
}

void Actor::Destroy()
{
}

void Actor::BeginPlayComponents()
{
	for (ActorComponent* comp : mComponents)
		comp->BeginPlay();
}

void Actor::TickComponents(float deltaTime)
{
	for (ActorComponent* comp : mComponents)
		comp->Tick(deltaTime);
}

void Actor::SetLocation(Vec3 location)
{
	mTransform.location = location;
}

void Actor::SetRotation(Vec3 rotation)
{
	mTransform.rotation = rotation;
}

void Actor::SetScale(Vec3 scale)
{
	mTransform.scale = scale;
}

void Actor::Clone(Actor* to)
{
	CopyProperties(to);
	to->mTransform = mTransform;
	to->mName = mName;

	for (ActorComponent* comp : mComponents)
	{
		ActorComponent* compClone = nullptr;

		if (comp->mType != ACTOR_COMPONENT_TYPE_EDITOR_SCENE)
		{
			compClone = to->GetComponent(comp->GetName());
		}
		else
		{
			compClone = to->AddComponent(comp->GetClass(), comp->mName);
			compClone->mType = ACTOR_COMPONENT_TYPE_EDITOR_SCENE;
		}

		check(compClone);
		check(compClone->GetClass() == comp->GetClass());

		comp->CopyProperties(compClone);
	}
}

ActorComponent* Actor::AddComponent(GroovyClass* componentClass, const std::string& name)
{
	checkf(componentClass, "Component class is NULL");
	checkf(GroovyClass_IsA(componentClass, ActorComponent::StaticClass()), "Component class is not an ActorComponent");
	checkf(name.length() && std::count(name.begin(), name.end(), ' ') < name.length(), "Invalid component name");

	ActorComponent*& dbRecord = mComponentsDB[name];
	checkf(dbRecord == nullptr, "Component with name %s already exists!", name.c_str());

	ActorComponent* newComponent = ObjectAllocator::Instantiate<ActorComponent>(componentClass);

	newComponent->mName = name;
	newComponent->mOwner = this;

	dbRecord = newComponent;
	mComponents.push_back(newComponent);

	return newComponent;
}

#if WITH_EDITOR

ActorComponent* Actor::__internal_Editor_AddEditorcomponent_BP(GroovyClass* componentClass, const std::string& name)
{
	ActorComponent* newComponent = AddComponent(componentClass, name);
	newComponent->mType = ACTOR_COMPONENT_TYPE_EDITOR_BP;
	if (mScene)
	{
		newComponent->Initialize();
	}
	return newComponent;
}

ActorComponent* Actor::__internal_Editor_AddEditorcomponent_Scene(GroovyClass* componentClass, const std::string& name)
{
	ActorComponent* newComponent = AddComponent(componentClass, name);
	newComponent->mType = ACTOR_COMPONENT_TYPE_EDITOR_SCENE;
	if (mScene)
	{
		newComponent->Initialize();
	}
	return newComponent;
}

void Actor::__internal_Editor_RemoveEditorComponent(ActorComponent* component)
{
	check(component);
	check(component->mType != ACTOR_COMPONENT_TYPE_NATIVE);
	
	auto it = std::find(mComponents.begin(), mComponents.end(), component);
	checkf(it != mComponents.end(), "Editor bug, trying to remove a component that doesn't belong to this actor");

	component->Uninitialize();

	mComponents.erase(it);
	mComponentsDB.erase(component->GetName());

	ObjectAllocator::Destroy(component);
}

void Actor::__internal_Editor_RenameEditorComponent(ActorComponent* component, const std::string& newName)
{
	check(component);
	check(component->mType != ACTOR_COMPONENT_TYPE_NATIVE);

	auto it = std::find(mComponents.begin(), mComponents.end(), component);

	checkf(it != mComponents.end(), "Editor bug, trying to remove a component that doesn't belong to this actor");

	checkf(!newName.empty() && GetComponent(newName) == nullptr, "Editor bug, invalid name");

	// update map
	mComponentsDB.erase(component->mName);
	mComponentsDB[newName] = component;

	// update component's name
	(*it._Ptr)->mName = newName;
}

#endif