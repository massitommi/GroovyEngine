#include "material.h"
#include "assets/asset_loader.h"
#include "classes/object_serializer.h"
#include "assets/asset_manager.h"
#include "project/project.h"
#include "platform/filesystem.h"

Material::Material()
	: mShader(nullptr), mUUID(0), mLoaded(false)
{
}

void Material::Load()
{
	if (mLoaded)
		return;

	extern Project gProj;
	AssetHandle myHandle = AssetManager::Get(mUUID);
	Buffer fileData;
	FileSystem::ReadFileBinary((gProj.assets / myHandle.name).string(), fileData);
	Deserialize(fileData);
	FixForRendering();

	mLoaded = true;
}

bool Material::Validate()
{
	if (!mShader)
		return false;

	size_t shaderBuffersSize = 0;
	for (const auto& bufferDesc : mShader->GetPixelConstBuffersDesc())
		shaderBuffersSize += bufferDesc.size;

	if (mConstBuffersData.size() != shaderBuffersSize)
		return false;

	if (mShader->GetPixelTexturesRes().size() != mResources.size())
		return false;

	return true;
}

void Material::SetShader(Shader* shader)
{
	if (shader == mShader)
		return;

	if (!shader)
	{
		mResources.clear();
		mConstBuffersData.resize(0);
		return;
	}

	// set shader
	mShader = shader;

	// set const buffers data
	size_t constBuffersSize = 0;
	for (const ConstBufferDesc& bufferDesc : shader->GetPixelConstBuffersDesc())
		constBuffersSize += bufferDesc.size;

	mConstBuffersData.resize(constBuffersSize);

	// set resources
	mResources.clear();
	for (const ShaderResTexture& shaderRes : shader->GetPixelTexturesRes())
	{
		MaterialResource& matRes = mResources.emplace_back();
		matRes.name = shaderRes.name;
		matRes.slot = shaderRes.bindSlot;
		matRes.res = nullptr;
	}
}

void Material::SetResource(Texture* texture, uint32 slot)
{
	if (slot < mResources.size())
		mResources[slot].res = texture;
}

void Material::SetResources(Texture* texture)
{
	for (MaterialResource& res : mResources)
		res.res = texture;
}

void Material::FixForRendering()
{
	extern Shader* DEFAULT_SHADER;
	extern Texture* DEFAULT_TEXTURE;

	// set shader to DEFAULT_SHADER if null
	if (!mShader)
		SetShader(DEFAULT_SHADER);
	
	// set null textures to DEFAULT_TEXTURE
	for (MaterialResource& res : mResources)
		if (!res.res)
			res.res = DEFAULT_TEXTURE;

	// clear const buffers data
	memset(mConstBuffersData.data(), 0, mConstBuffersData.size());
}

GROOVY_CLASS_IMPL(MaterialAssetFile, GroovyObject)

GROOVY_CLASS_REFLECTION_BEGIN(MaterialAssetFile)
	GROOVY_REFLECT(shader)
	GROOVY_REFLECT(constBuffersData)
	GROOVY_REFLECT(shaderResNames)
	GROOVY_REFLECT(shaderRes)
GROOVY_CLASS_REFLECTION_END()

void Material::Serialize(DynamicBuffer& fileData)
{
	checkslowf(Validate(), "Trying to serialize a material that is not ready for rendering");
	
	MaterialAssetFile asset;

	// shader
	asset.shader = mShader;

	// resources
	for (const MaterialResource& res : mResources)
	{
		asset.shaderResNames.push_back(res.name);
		asset.shaderRes.push_back(res.res);
	}

	// const buffers
	asset.constBuffersData.resize(mConstBuffersData.size());
	memcpy(asset.constBuffersData.data(), mConstBuffersData.data(), mConstBuffersData.size());

	ObjectSerializer::SerializeSimpleObject(&asset, MaterialAssetFile::StaticClass()->cdo, fileData);
}

void Material::Deserialize(BufferView fileData)
{
	MaterialAssetFile asset;
	ObjectSerializer::DeserializeSimpleObject(&asset, fileData);

	// shader
	SetShader(asset.shader);

	// resources
	for (const MaterialResource& res : mResources)
	{
		auto it = std::find(asset.shaderResNames.begin(), asset.shaderResNames.end(), res.name);
		uint32 resIndex = it - asset.shaderResNames.begin();

		if (resIndex < asset.shaderResNames.size())
		{
			mResources[resIndex].res = asset.shaderRes[resIndex];
		}
	}

	// const buffers data
	if (mConstBuffersData.size() == mConstBuffersData.size())
	{
		memcpy(mConstBuffersData.data(), asset.constBuffersData.data(), asset.constBuffersData.size());
	}
}