#include "renderer.h"
#include "api/renderer_api.h"

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
		mat->mShader->Bind();
		for (MaterialResource& res : mat->mResources)
			res.res->Bind(res.slot);

		RendererAPI::Get().DrawIndexed(vertexOffset, indexOffset, mesh->mSubmeshes[i].indexCount);
		vertexOffset += mesh->mSubmeshes[i].vertexCount;
		indexOffset += mesh->mSubmeshes[i].indexCount;
	}
}