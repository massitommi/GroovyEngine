#include "scene_renderer.h"
#include "renderer.h"
#include "math/math.h"
#include "gameframework/components/cameracomponent.h"
#include "gameframework/components/meshcomponent.h"
#include "gameframework/actor.h"
#include "renderer/api/framebuffer.h"

static FrameBuffer* sFrameBuffer;
static std::vector<MeshComponent*> sMeshes;

void SceneRenderer::SetFrameBuffer(FrameBuffer* frameBuffer)
{
	check(frameBuffer);
	sFrameBuffer = frameBuffer;
}

void SceneRenderer::ClearMeshQueue()
{
	sMeshes.clear();
}

void SceneRenderer::Submit(MeshComponent* mesh)
{
	sMeshes.push_back(mesh);
}

void SceneRenderer::Remove(MeshComponent* mesh)
{
	sMeshes.erase(std::find(sMeshes.begin(), sMeshes.end(), mesh));
}

void SceneRenderer::BeginScene(CameraComponent* camera)
{
	Vec3 camLocation = camera->mTransform.location;
	Vec3 camRotation = camera->mTransform.rotation;
	float fov = camera->mFOV;
	
	BeginScene(camLocation, camRotation, fov);
}

void SceneRenderer::BeginScene(Vec3 camLocation, Vec3 camRotation, float FOV)
{
	float aspectRatio = (float)sFrameBuffer->GetSpecs().width / (float)sFrameBuffer->GetSpecs().height;

	Mat4 vp =
		math::GetViewMatrix(camLocation, camRotation)
		*
		math::GetPerspectiveMatrix(aspectRatio, FOV, 0.01f, 1000.0f);
	vp = math::GetMatrixTransposed(vp);

	Renderer::SetCamera(vp);
}

void SceneRenderer::RenderScene()
{
	for (MeshComponent* meshComp : sMeshes)
	{
		if (!meshComp->mVisible)
			continue;

		Mesh* mesh = meshComp->mMesh;
		if (!mesh)
			continue;

		Transform componentTransform = meshComp->GetTransform();
		Transform transform = meshComp->GetOwner()->GetTransform();
		transform.location += componentTransform.location;
		transform.rotation += componentTransform.rotation;
		transform.scale *= componentTransform.scale;

		Mat4 model = math::GetModelMatrix(transform.location, transform.rotation, transform.scale);
		model = math::GetMatrixTransposed(model);

		Renderer::SetModel(model);
		Renderer::RenderMesh(mesh);
	}
}
