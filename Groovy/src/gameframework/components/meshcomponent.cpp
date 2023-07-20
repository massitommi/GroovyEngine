#include "meshcomponent.h"
#include "renderer/scene_renderer.h"
#include "renderer/mesh.h"

GROOVY_CLASS_IMPL(MeshComponent)
	GROOVY_REFLECT(mVisible)
	GROOVY_REFLECT(mMesh)
GROOVY_CLASS_END()

MeshComponent::MeshComponent()
	: mMesh(nullptr), mVisible(true)
{
}

void MeshComponent::Initialize()
{
	if (mMesh)
		mMesh->FixForRendering();

	SceneRenderer::Submit(this);
}

void MeshComponent::Uninitialize()
{
	SceneRenderer::Remove(this);
}