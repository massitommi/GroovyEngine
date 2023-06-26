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
	virtual void Load() override;
	virtual void Save() override;
#if WITH_EDITOR
	virtual bool Editor_FixDependencyDeletion(AssetHandle assetToBeDeleted) override;
#endif

	size_t GetVertexBufferSize() const { return mVertexBuffer->GetSize(); }
	size_t GetIndexBufferSize() const { return mIndexBuffer->GetSize(); }

	const std::vector<SubmeshData>& GetSubmeshes() const { return mSubmeshes; }
	const std::vector<Material*>& GetMaterials() const { return mMaterials; }

	void SetMaterial(Material* mat, uint32 index) { check(index < mMaterials.size()); mMaterials[index] = mat; }
	void FixForRendering();

	size_t GetAssetOffsetForSerialization() const { return sizeof(MeshAssetHeader) + mVertexBuffer->GetSize() + mIndexBuffer->GetSize(); }
	void Serialize(DynamicBuffer& fileData);
	void Deserialize(BufferView fileData);

#if WITH_EDITOR

	std::vector<Material*>& Editor_MaterialsRef() { return mMaterials; }

#endif

private:
	VertexBuffer* mVertexBuffer;
	IndexBuffer* mIndexBuffer;
	std::vector<SubmeshData> mSubmeshes;
	std::vector<Material*> mMaterials;

	AssetUUID mUUID;
	bool mLoaded;
};

GROOVY_CLASS_DECL(MeshAssetFile)
class MeshAssetFile : public GroovyObject
{
	GROOVY_CLASS_BODY(MeshAssetFile, GroovyObject)

public:
	std::vector<Material*> materials;
	std::vector<SubmeshData> submeshes;
};