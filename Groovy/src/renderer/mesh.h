#pragma once

#include "api/buffers.h"
#include "math/vector.h"
#include "assets/asset.h"
#include "material.h"

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
	Mesh();
	~Mesh();

	void __internal_SetUUID(AssetUUID uuid) override { mUUID = uuid; }
	AssetUUID GetUUID() const override { return mUUID; }
	virtual bool IsLoaded() const override { return mLoaded; }
	virtual void Load() override {}

	size_t GetVertexBufferSize() const { return mVertexBuffer->GetSize(); }
	size_t GetIndexBufferSize() const { return mIndexBuffer->GetSize(); }

	const std::vector<SubmeshData>& GetSubmeshes() const { return mSubmeshes; }
	const std::vector<Material*>& GetMaterials() const { return mMaterials; }

	void SetMaterial(Material* mat, uint32 index) { check(index < mMaterials.size()); mMaterials[index] = mat; }

private:
	VertexBuffer* mVertexBuffer;
	IndexBuffer* mIndexBuffer;
	std::vector<SubmeshData> mSubmeshes;
	std::vector<Material*> mMaterials;

	AssetUUID mUUID;
	bool mLoaded;

};