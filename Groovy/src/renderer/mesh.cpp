#include "mesh.h"

Mesh::Mesh(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, const std::vector<SubmeshData>& submeshes)
	: mVertexBuffer(vertexBuffer), mIndexBuffer(indexBuffer), mSubmeshes(submeshes)
{

}

Mesh::~Mesh()
{

}
