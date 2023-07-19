#include "meshcomponent.h"
#include "renderer/scene_renderer.h"

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
	SceneRenderer::Submit(this);
}

void MeshComponent::Uninitialize()
{
	SceneRenderer::Remove(this);
}