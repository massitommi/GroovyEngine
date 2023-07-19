#pragma once

#include "gameframework/actorcomponent.h"

GROOVY_CLASS_DECL(MeshComponent)
class MeshComponent : public SceneComponent
{
	GROOVY_CLASS_BODY(MeshComponent, SceneComponent)

public:
	MeshComponent();

	inline Mesh* GetMesh() const { return mMesh; }

	virtual void Initialize() override;
	virtual void Uninitialize() override;

public:
	bool mVisible;

private:
	Mesh* mMesh;


	friend class SceneRenderer;
};