#pragma once

#include "api/buffers.h"
#include "math/vector.h"

struct MeshVertex
{
	Vec4 position;
	Vec4 color;
	Vec2 textCoords;
};

typedef uint32 MeshIndex;

struct SubmeshData
{
	uint32 vertexCount;
	uint32 indexCount;
};

class Mesh
{
	friend class Renderer;

public:
	Mesh(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, const std::vector<SubmeshData>& submeshes);
	~Mesh();

private:
	VertexBuffer* mVertexBuffer;
	IndexBuffer* mIndexBuffer;
	std::vector<SubmeshData> mSubmeshes;
};