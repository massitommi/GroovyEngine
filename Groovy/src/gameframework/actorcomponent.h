#pragma once
#include "classes/object.h"

enum EActorComponentType : byte
{
	// Component added via C++
	ACTOR_COMPONENT_TYPE_NATIVE,
	// Component added in the blueprint editor
	ACTOR_COMPONENT_TYPE_EDITOR_BP,
	// Component added in the scene
	ACTOR_COMPONENT_TYPE_EDITOR_SCENE
};

class Actor;
class Scene;

GROOVY_CLASS_DECL(ActorComponent)
class ActorComponent : public GroovyObject
{
	GROOVY_CLASS_BODY(ActorComponent, GroovyObject)

public:
	ActorComponent();

	inline const std::string& GetName() const { return mName; }
	inline EActorComponentType GetType() const { return mType; }
	inline Actor* GetOwner() const { return mOwner; }

protected:
	virtual void Initialize() {}
	virtual void Uninitialize() {}

private:
	std::string mName;
	EActorComponentType mType;
	Actor* mOwner;

	friend class Actor;
	friend class ActorSerializer;
	friend class Scene;
};

GROOVY_CLASS_DECL(SceneComponent)
class SceneComponent : public ActorComponent
{
	GROOVY_CLASS_BODY(SceneComponent, ActorComponent)

public:
	SceneComponent();

#if WITH_EDITOR
	Transform& Editor_TransformRef() { return mTransform; }
#endif

	inline const Transform& GetTransform() const { return mTransform; }
	inline Vec3 GetLocation() const { return mTransform.location; }
	inline Vec3 GetRotation() const { return mTransform.rotation; }
	inline Vec3 GetScale() const { return mTransform.scale; }

	Transform GetAbsoluteTransform() const;
	Vec3 GetAbsoluteLocation() const;
	Vec3 GetAbsoluteRotation() const;
	Vec3 GetAbsoluteScale() const;

private:
	// Relative to parent component / actor
	Transform mTransform;

	friend class Actor;
	friend class ActorSerializer;
	friend class Scene;
	friend class SceneRenderer;
};