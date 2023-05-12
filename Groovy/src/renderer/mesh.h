#pragma once

#include "api/buffers.h"
#include "math/vector.h"
#include "assets/asset.h"

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

class Mesh : public AssetInstance
{
	friend class Renderer;

public:
	Mesh(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, const std::vector<SubmeshData>& submeshes);
	~Mesh();

	void __internal_SetUUID(AssetUUID uuid) override { mUUID = uuid; }
	AssetUUID GetUUID() const override { return mUUID; }

private:
	VertexBuffer* mVertexBuffer;
	IndexBuffer* mIndexBuffer;
	std::vector<SubmeshData> mSubmeshes;

	AssetUUID mUUID;
};