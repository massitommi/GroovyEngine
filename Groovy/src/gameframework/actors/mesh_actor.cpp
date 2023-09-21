#include "mesh_actor.h"
#include "gameframework/components/mesh_component.h"

GROOVY_CLASS_IMPL(MeshActor)
GROOVY_CLASS_END()

MeshActor::MeshActor()
{
	mMesh = AddComponent<MeshComponent>("MeshComponent");
}
