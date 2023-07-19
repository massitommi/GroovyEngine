#pragma once

#include "actor.h"

GROOVY_CLASS_DECL(MeshActor)
class MeshActor : public Actor
{
	GROOVY_CLASS_BODY(MeshActor, Actor)
public:
	MeshActor();

	inline class MeshComponent* GetMesh() const { return mMesh; }

private:
	class MeshComponent* mMesh;
};