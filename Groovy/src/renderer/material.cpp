#include "material.h"
#include "assets/asset_loader.h"
#include "assets/asset_manager.h"

void CreateBuffer(const std::vector<ConstBufferDesc>& bufferDesc, Buffer& outBuffer)
{
	uint32 size = 0;
	for (const auto& desc : bufferDesc)
		size += desc.size;
	if (!size) return;
	outBuffer.resize(size);
	memset(outBuffer.data(), 0, outBuffer.size());
}

Material::Material()
	: mUUID(AssetManager::GenUUID()), mShader(nullptr), mShaderID(0)
{
}

void Material::ConstructResources()
{
	check(mShader);
	CreateBuffer(mShader->GetVertexConstBuffersDesc(), mVertexConstBuffersData);
	CreateBuffer(mShader->GetPixelConstBuffersDesc(), mPixelConstBuffersData);
	mTexturesID.resize(mShader->GetPixelTexturesRes().size());
}

void Material::SubmitBuffersToShader()
{
	size_t vOffset = 0;
	for (uint32 i = 0; i < mShader->GetVertexConstBuffersDesc().size(); i++)
	{
		mShader->OverwriteVertexConstBuffer(i, mVertexConstBuffersData.data() + vOffset);
		vOffset = mShader->GetVertexConstBuffersDesc()[i].size;
	}

	size_t pOffset = 0;
	for (uint32 i = 0; i < mShader->GetPixelConstBuffersDesc().size(); i++)
	{
		mShader->OverwritePixelConstBuffer(i, mPixelConstBuffersData.data() + pOffset);
		pOffset = mShader->GetPixelConstBuffersDesc()[i].size;
	}
}

void Material::Serialize(Buffer& outBuffer)
{
	/*GroovyAssetHeader assetHeader;
	memcpy(assetHeader.magic, GROOVY_ASSET_MAGIC, GROOVY_ASSET_MAGIC_SIZE);
	assetHeader.type = ASSET_TYPE_MATERIAL;
	assetHeader.uuid = mUUID;

	GroovyAssetMaterialSubheader matHeader;
	matHeader.shaderID = mShaderID;
	matHeader.numTextures = mTexturesID.size();
	matHeader.vertexConstBufferSize = mVertexConstBuffersData.size();
	matHeader.pixelConstBufferSize = mPixelConstBuffersData.size();

	outBuffer.resize
	(
		sizeof(GroovyAssetHeader) +
		sizeof(GroovyAssetMaterialSubheader) +
		mVertexConstBuffersData.size() +
		mPixelConstBuffersData.size() +
		mTexturesID.size() * sizeof(AssetUUID)
	);

	byte* outBufferPtr = outBuffer.data();
	
	memcpy(outBufferPtr, &assetHeader, sizeof(GroovyAssetHeader));
	outBufferPtr += sizeof(GroovyAssetHeader);
	memcpy(outBufferPtr, &matHeader, sizeof(GroovyAssetMaterialSubheader));
	outBufferPtr += sizeof(GroovyAssetMaterialSubheader);
	memcpy(outBufferPtr, mVertexConstBuffersData.data(), mVertexConstBuffersData.size());
	outBufferPtr += mVertexConstBuffersData.size();
	memcpy(outBufferPtr, mPixelConstBuffersData.data(), mPixelConstBuffersData.size());
	outBufferPtr += mPixelConstBuffersData.size();
	for (uint32 i = 0; i < mTexturesID.size(); i++)
		((AssetUUID*)outBufferPtr)[i] = mTexturesID[i];*/
}

void Material::Deserialize(const Buffer& file)
{
	/*GroovyAssetHeader* assetHeader = (GroovyAssetHeader*)file.data();
	GroovyAssetMaterialSubheader* matHeader = (GroovyAssetMaterialSubheader*)((byte*)assetHeader + sizeof(GroovyAssetHeader));
	
	mUUID = assetHeader->uuid;
	mShaderID = matHeader->shaderID;

	const byte* matDataPtr = file.data() + sizeof(GroovyAssetMaterialSubheader) + sizeof(GroovyAssetHeader);

	mVertexConstBuffersData.resize(matHeader->vertexConstBufferSize);
	memcpy(mVertexConstBuffersData.data(), matDataPtr + 0, matHeader->vertexConstBufferSize);

	matDataPtr += matHeader->vertexConstBufferSize;

	mPixelConstBuffersData.resize(matHeader->pixelConstBufferSize);
	memcpy(mPixelConstBuffersData.data(), matDataPtr, matHeader->pixelConstBufferSize);

	matDataPtr += matHeader->pixelConstBufferSize;

	mTexturesID.resize(matHeader->numTextures);
	for (uint32 i = 0; i < matHeader->numTextures; i++)
	{
		mTexturesID[i] = *((AssetUUID*)matDataPtr);
		matDataPtr += sizeof(AssetUUID);
	}*/
}
