#include "meshcomponent.h"
#include "renderer/scene_renderer.h"
#include "renderer/mesh.h"
#include "gameframework/actor.h"
#include "gameframework/scene.h"

GROOVY_CLASS_IMPL(MeshComponent)
	GROOVY_REFLECT(mVisible)
	GROOVY_REFLECT(mMesh)
	GROOVY_REFLECT_EX(mMaterialOverrides, PROPERTY_FLAG_EDITOR_NO_RESIZE)
GROOVY_CLASS_END()

MeshComponent::MeshComponent()
	: mMesh(nullptr), mVisible(true)
{
}

void MeshComponent::Initialize()
{
	GetOwner()->GetScene()->SubmitForRendering(this);
}

void MeshComponent::Uninitialize()
{
	GetOwner()->GetScene()->RemoveFromRenderQueue(this);
}

void MeshComponent::SetMaterialOverride(uint32 index, Material* mat)
{
	check(index < mMaterialOverrides.size());

	mMaterialOverrides[index] = mat;
}

void MeshComponent::SetMesh(Mesh* mesh)
{
	if (mesh != mMesh)
	{
		mMaterialOverrides.clear();
		if (mesh)
			mMaterialOverrides.resize(mesh->GetMaterials().size(), nullptr);
	}

	mMesh = mesh;
}

#if WITH_EDITOR

void MeshComponent::Editor_OnPropertyChanged(const GroovyProperty* prop)
{
	if (prop->name == "mMesh")
	{
		mMaterialOverrides.clear();
		if (mMesh)
			mMaterialOverrides.resize(mMesh->GetMaterials().size(), nullptr);
	}
}

#endif