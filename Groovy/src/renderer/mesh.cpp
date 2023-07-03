#include "mesh.h"
#include "classes/object_serializer.h"
#include "assets/assets.h"

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
	AssetLoader::LoadMesh(this);
	mLoaded = true;
}

void Mesh::Save()
{
	AssetSerializer::SerializeMesh(this);
}

#if WITH_EDITOR

bool Mesh::Editor_FixDependencyDeletion(AssetHandle assetToBeDeleted)
{
	extern Material* DEFAULT_MATERIAL;
	if (assetToBeDeleted.type == ASSET_TYPE_MATERIAL)
	{
		bool found = false;
		for (Material*& m : mMaterials)
		{
			if (m == assetToBeDeleted.instance)
			{
				m = DEFAULT_MATERIAL;
				found = true;
			}
		}
		return found;
	}
	return false;
}

#endif

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

	PropertyPack meshAssetPropPack;
	ObjectSerializer::CreatePropertyPack(&asset, MeshAssetFile::StaticClass()->cdo, meshAssetPropPack);
	ObjectSerializer::SerializePropertyPack(meshAssetPropPack, fileData);
}

void Mesh::Deserialize(BufferView fileData)
{
	MeshAssetHeader header = fileData.read<MeshAssetHeader>();
	// geometry data
	mVertexBuffer = VertexBuffer::Create(header.vertexBufferSize, fileData.read(header.vertexBufferSize), sizeof(MeshVertex));
	mIndexBuffer = IndexBuffer::Create(header.indexBufferSize, fileData.read(header.indexBufferSize));
	// submeshes and materials
	MeshAssetFile asset;
	PropertyPack meshAssetPropPack;
	ObjectSerializer::DeserializePropertyPack(MeshAssetFile::StaticClass(), fileData, meshAssetPropPack);
	ObjectSerializer::DeserializePropertyPackData(meshAssetPropPack, &asset);
	mSubmeshes = asset.submeshes;
	mMaterials = asset.materials;
}

IMPL_PROPERTY_TYPE(SubmeshData, PROPERTY_TYPE_INTERNAL_SUBMESHDATA)

GROOVY_CLASS_IMPL(MeshAssetFile)
	GROOVY_REFLECT(materials)
	GROOVY_REFLECT(submeshes)
GROOVY_CLASS_END()