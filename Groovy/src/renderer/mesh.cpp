#include "mesh.h"

Mesh::Mesh(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, const std::vector<SubmeshData>& submeshes, const std::vector<Material*> materials)
	: mVertexBuffer(vertexBuffer), mIndexBuffer(indexBuffer), mSubmeshes(submeshes), mMaterials(materials), mUUID(0)
{
}

Mesh::~Mesh()
{
	delete mVertexBuffer;
	delete mIndexBuffer;
}
