#include "renderer.h"
#include "api/renderer_api.h"

// register 0 = view projection
static ConstBuffer* sCameraVPBuffer;
// register 1 = model
static ConstBuffer* sModelBuffer;

static Shader* sCurrentlyBoundShader;

void Renderer::Init()
{
	sCameraVPBuffer = ConstBuffer::Create(sizeof(Mat4), nullptr);
	sCameraVPBuffer->BindForVertexShader(VIEW_PROJECTION_BUFFER_INDEX);
	sModelBuffer = ConstBuffer::Create(sizeof(Mat4), nullptr);
	sModelBuffer->BindForVertexShader(MODEL_BUFFER_INDEX);
	sCurrentlyBoundShader = nullptr;
}

void Renderer::Shutdown()
{
	delete sCameraVPBuffer;
	delete sModelBuffer;
}

void Renderer::SetCamera(Mat4& vpMatrix)
{
	sCameraVPBuffer->Overwrite(&vpMatrix, sizeof(Mat4));
}

void Renderer::SetModel(Mat4& modelMatrix)
{
	sModelBuffer->Overwrite(&modelMatrix, sizeof(Mat4));
}

void Renderer::RenderMesh(Mesh* mesh)
{
	check(mesh);

	mesh->mVertexBuffer->Bind();
	mesh->mIndexBuffer->Bind();

	uint32 indexOffset = 0;
	uint32 vertexOffset = 0;
	for (uint32 i = 0; i < mesh->mSubmeshes.size(); i++)
	{
		Material* mat = mesh->GetMaterials()[i];
		
		if (mat->mShader != sCurrentlyBoundShader)
		{
			mat->mShader->Bind();
			sCurrentlyBoundShader = mat->mShader;
		}

		for (MaterialResource& res : mat->mResources)
			res.res->Bind(res.slot);

		RendererAPI::Get().DrawIndexed(vertexOffset, indexOffset, mesh->mSubmeshes[i].indexCount);
		
		vertexOffset += mesh->mSubmeshes[i].vertexCount;
		indexOffset += mesh->mSubmeshes[i].indexCount;
	}
}