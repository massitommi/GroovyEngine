#include "scene_renderer.h"
#include "renderer.h"
#include "math/math.h"
#include "gameframework/scene.h"
#include "gameframework/components/camera_component.h"
#include "gameframework/components/mesh_component.h"

void SceneRenderer::BeginScene(CameraComponent* camera, float aspectRatio)
{	
	BeginScene(camera->GetAbsoluteLocation(), camera->GetAbsoluteRotation(), camera->mFOV, aspectRatio);
}

void SceneRenderer::BeginScene(Vec3 camLocation, Vec3 camRotation, float FOV, float aspectRatio)
{
	Mat4 vp =
		math::GetViewMatrix(camLocation, camRotation)
		*
		math::GetPerspectiveMatrix(aspectRatio, FOV, 0.01f, 1000.0f);
	vp = math::GetMatrixTransposed(vp);

	Renderer::SetCamera(vp);
}

void SceneRenderer::RenderScene(Scene* scene)
{
	for (MeshComponent* meshComp : scene->GetRenderQueue())
	{
		if (!meshComp->mVisible)
			continue;

		Mesh* mesh = meshComp->mMesh;
		if (!mesh)
			continue;

		Mat4 model = math::GetModelMatrix(meshComp->GetAbsoluteLocation(), meshComp->GetAbsoluteRotation(), meshComp->GetAbsoluteScale());
		model = math::GetMatrixTransposed(model);

		Renderer::SetModel(model);

		std::vector<Material*> materials = meshComp->mMesh->GetMaterials();
		for (uint32 i = 0; i < materials.size(); i++)
		{
			Material* matOverride = meshComp->mMaterialOverrides[i];
			if (matOverride)
				materials[i] = matOverride;
		}

		Renderer::RenderMesh(mesh, materials);
	}
}
