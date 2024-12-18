#include "material.h"
#include "assets/asset_loader.h"
#include "classes/object_serializer.h"
#include "assets/asset_manager.h"
#include "assets/asset_serializer.h"

extern Shader* DEFAULT_SHADER;
extern Texture* DEFAULT_TEXTURE;

Material::Material()
	: mShader(nullptr), mUUID(0), mLoaded(false)
{
}

void Material::Load()
{
	AssetLoader::LoadGenericAsset(this);
	mLoaded = true;
}

void Material::Save()
{
	AssetSerializer::SerializeGenericAsset(this);
}

#if WITH_EDITOR

bool Material::Editor_FixDependencyDeletion(AssetHandle assetToBeDeleted)
{
	extern Shader* DEFAULT_SHADER;
	extern Texture* DEFAULT_TEXTURE;

	if (assetToBeDeleted.type == ASSET_TYPE_SHADER && mShader == assetToBeDeleted.instance)
	{
		SetShader(DEFAULT_SHADER);
		SetResources(DEFAULT_TEXTURE);
		return true;
	}

	else if (assetToBeDeleted.type == ASSET_TYPE_TEXTURE)
	{
		bool found = false;
		for (MaterialResource& res : mResources)
		{
			if (res.res == assetToBeDeleted.instance)
			{
				res.res = DEFAULT_TEXTURE;
				found = true;
			}
		}
		return found;
	}

	return false;
}

#endif

bool Material::Validate() const
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

GROOVY_CLASS_IMPL(MaterialAssetFile)
	GROOVY_REFLECT(shader)
	GROOVY_REFLECT(constBuffersData)
	GROOVY_REFLECT(shaderResNames)
	GROOVY_REFLECT(shaderRes)
GROOVY_CLASS_END()

void Material::Serialize(DynamicBuffer& fileData) const
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

	// mat asset file
	PropertyPack matAssetPropPack;
	ObjectSerializer::CreatePropertyPack(&asset, MaterialAssetFile::StaticCDO(), matAssetPropPack);
	ObjectSerializer::SerializePropertyPack(matAssetPropPack, fileData);
}

void Material::Deserialize(BufferView fileData)
{
	MaterialAssetFile asset;
	PropertyPack matAssetPropPack;
	ObjectSerializer::DeserializePropertyPack(MaterialAssetFile::StaticClass(), fileData, matAssetPropPack);
	ObjectSerializer::DeserializePropertyPackData(matAssetPropPack, &asset);

	// shader
	Shader* shaderToSet = asset.shader;
	if (!shaderToSet)
		shaderToSet = DEFAULT_SHADER;
	SetShader(shaderToSet);

	// resources
	for (MaterialResource& res : mResources)
	{
		auto it = std::find(asset.shaderResNames.begin(), asset.shaderResNames.end(), res.name);
		uint32 resIndex = (uint32)(it - asset.shaderResNames.begin());

		if (resIndex < asset.shaderResNames.size())
		{
			mResources[resIndex].res = asset.shaderRes[resIndex];
		}
	}

	for (MaterialResource& res : mResources)
		if (!res.res)
			res.res = DEFAULT_TEXTURE;

	// const buffers data
	if (mConstBuffersData.size() == mConstBuffersData.size())
	{
		memcpy(mConstBuffersData.data(), asset.constBuffersData.data(), asset.constBuffersData.size());
	}
}