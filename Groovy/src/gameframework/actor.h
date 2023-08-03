#pragma once
#include "classes/object.h"
#include <map>

class ActorComponent;
class ActorBlueprint;
class Scene;

GROOVY_CLASS_DECL(Actor)
class Actor : public GroovyObject
{
	GROOVY_CLASS_BODY(Actor, GroovyObject)
public:
	Actor();
	~Actor();

	ActorComponent* GetComponent(const std::string& name) const;
	ActorComponent* GetComponent(GroovyClass* componentClass) const;
	ActorComponent* GetComponentExact(GroovyClass* componentClass) const;

	uint32 GetComponents(GroovyClass* componentClass, std::vector<ActorComponent*>& outComponents) const;
	uint32 GetComponentsExact(GroovyClass* componentClass, std::vector<ActorComponent*>& outComponents) const;

	template<typename TComponent>
	TComponent* GetComponent() const
	{
		return (TComponent*)GetComponent(TComponent::StaticClass());
	}

	template<typename TComponent>
	TComponent* GetComponentExact() const
	{
		return (TComponent*)GetComponentExact(TComponent::StaticClass());
	}

	template<typename TComponent>
	uint32 GetComponents(std::vector<ActorComponent*>& outComponents) const
	{
		return GetComponents(TComponent::StaticClass(), outComponents);
	}

	template<typename TComponent>
	uint32 GetComponentsExact(std::vector<ActorComponent*>& outComponents) const
	{
		return GetComponentsExact(TComponent::StaticClass(), outComponents);
	}

protected:
	template<typename TComponent>
	TComponent* AddComponent(const std::string& name)
	{
		return (TComponent*)AddComponent(TComponent::StaticClass(), name);
	}

	void InitializeComponents();
	void UninitializeComponents();
	
	virtual void BeginPlay();
	virtual void Tick(float deltaTime);
	virtual void Destroy();

public:
	inline const std::vector<ActorComponent*>& GetComponents() const { return mComponents; }

	inline const Transform& GetTransform() const { return mTransform; }
	inline Vec3 GetLocation() const { return mTransform.location; }
	inline Vec3 GetRotation() const { return mTransform.rotation; }
	inline Vec3 GetScale() const { return mTransform.scale; }

	inline const std::string& GetName() const { return mName; }
	inline ActorBlueprint* GetTemplate() const { return mTemplate; }
	inline Scene* GetScene() const { return mScene; }

protected:
	ActorComponent* AddComponent(GroovyClass* componentClass, const std::string& name);

public:

#if WITH_EDITOR

	ActorComponent* __internal_Editor_AddEditorcomponent_BP(GroovyClass* componentClass, const std::string& name);
	ActorComponent* __internal_Editor_AddEditorcomponent_Scene(GroovyClass* componentClass, const std::string& name);
	void __internal_Editor_RemoveEditorComponent(ActorComponent* component);
	void __internal_Editor_RenameEditorComponent(ActorComponent* component, const std::string& newName);

	Transform& Editor_TransformRef() { return mTransform; }
	std::string& Editor_NameRef() { return mName; }

#endif

private:
	Transform mTransform;
	std::string mName;
	bool mShouldTick;
	Scene* mScene;

	ActorBlueprint* mTemplate;
	
	std::vector<ActorComponent*> mComponents;
	std::map<std::string, ActorComponent*> mComponentsDB;

	friend class ActorSerializer;
	friend class Scene;
};