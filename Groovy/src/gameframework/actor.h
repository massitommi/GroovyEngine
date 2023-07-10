#pragma once
#include "classes/object.h"
#include <map>

class ActorComponent;

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

public:
	inline const std::vector<ActorComponent*>& GetComponents() const { return mComponents; }
	inline const std::vector<ActorComponent*>& GetNativeComponents() const { return mNativeComponents; }
	inline const std::vector<ActorComponent*>& GetEditorComponents() const { return mEditorComponents; }

	inline const Transform& GetTransform() const { return mTransform; }
	inline Vec3 GetLocation() const { return mTransform.location; }
	inline Vec3 GetRotation() const { return mTransform.rotation; }
	inline Vec3 GetScale() const { return mTransform.scale; }

	inline const std::string& GetName() const { return mName; }

	// internal stuff
private:
	ActorComponent* __internal_AddComponent(GroovyClass* componentClass, const std::string& name);

protected:
	ActorComponent* AddComponent(GroovyClass* componentClass, const std::string& name);

public:
	ActorComponent* __internal_AddEditorComponent(GroovyClass* componentClass, const std::string& name);

#if WITH_EDITOR
	void __internal_Editor_RemoveEditorComponent(ActorComponent* component);
#endif

private:
	Transform mTransform;
	std::string mName;
	
	std::vector<ActorComponent*> mComponents;
	std::map<std::string, ActorComponent*> mComponentsDB;

	std::vector<ActorComponent*> mNativeComponents;
	std::vector<ActorComponent*> mEditorComponents;

	friend class ActorSerializer;
	friend class Scene;
};

// used for serializing components
struct ComponentPack
{
	std::string componentName;
	GroovyClass* componentClass;
	PropertyPack componentProperties;
};

// used for serializing actors
struct ActorPack
{
	GroovyClass* actorClass;
	PropertyPack actorProperties;
	std::vector<ComponentPack> nativeComponents;
	std::vector<ComponentPack> editorComponents;
};