#include "meshactor.h"
#include "components/meshcomponent.h"

GROOVY_CLASS_IMPL(MeshActor)
GROOVY_CLASS_END()

MeshActor::MeshActor()
{
	mMesh = AddComponent<MeshComponent>("MeshComponent");
}
