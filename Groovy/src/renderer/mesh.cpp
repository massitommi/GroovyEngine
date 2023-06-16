#include "mesh.h"

Mesh::Mesh()
	: mVertexBuffer(nullptr), mIndexBuffer(nullptr), mUUID(0)
{
}

Mesh::~Mesh()
{
	delete mVertexBuffer;
	delete mIndexBuffer;
}
