#include "mesh.h"
#include "classes/object_serializer.h"
#include "assets/asset_manager.h"
#include "project/project.h"
#include "platform/filesystem.h"

Mesh::Mesh()
	: mVertexBuffer(nullptr), mIndexBuffer(nullptr), mUUID(0), mLoaded(false)
{
}

Mesh::~Mesh()
{
	delete mVertexBuffer;
	delete mIndexBuffer;
}

void Mesh::Load()
{
	if (mLoaded)
		return;

	extern Project gProj;
	AssetHandle myHandle = AssetManager::Get(mUUID);
	Buffer fileData;
	FileSystem::ReadFileBinary((gProj.assets / myHandle.name).string(), fileData);
	Deserialize(fileData);

	mLoaded = true;
}

void Mesh::FixForRendering()
{
	extern Material* DEFAULT_MATERIAL;
	for (Material*& mat : mMaterials)
	{
		if (!mat)
			mat = DEFAULT_MATERIAL;
		mat->FixForRendering();
	}
}

void Mesh::Serialize(DynamicBuffer& fileData)
{
	MeshAssetFile asset;
	asset.materials = mMaterials;
	asset.submeshes = mSubmeshes;

	ObjectSerializer::SerializeSimpleObject(&asset, (GroovyObject*)MeshAssetFile::StaticClass()->cdo, fileData);
}

void Mesh::Deserialize(BufferView fileData)
{
	MeshAssetHeader header = fileData.read<MeshAssetHeader>();
	// geometry data
	mVertexBuffer = VertexBuffer::Create(header.vertexBufferSize, fileData.read(header.vertexBufferSize), sizeof(MeshVertex));
	mIndexBuffer = IndexBuffer::Create(header.indexBufferSize, fileData.read(header.indexBufferSize));
	// submeshes and materials
	MeshAssetFile asset;
	ObjectSerializer::DeserializeSimpleObject(&asset, fileData);
	mSubmeshes = asset.submeshes;
	mMaterials = asset.materials;
}

IMPL_PROPERTY_TYPE(SubmeshData, PROPERTY_TYPE_INTERNAL_SUBMESHDATA)

GROOVY_CLASS_IMPL(MeshAssetFile, GroovyObject)

GROOVY_CLASS_REFLECTION_BEGIN(MeshAssetFile)
	GROOVY_REFLECT(materials)
	GROOVY_REFLECT(submeshes)
GROOVY_CLASS_REFLECTION_END()