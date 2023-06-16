#include "material.h"
#include "assets/asset_loader.h"

extern Texture* DEFAULT_TEXTURE;

GROOVY_CLASS_IMPL(MaterialAssetFile, AssetFile)

GROOVY_CLASS_REFLECTION_BEGIN(MaterialAssetFile)
	GROOVY_REFLECT(mShader)
	GROOVY_REFLECT(mConstBuffersData)
	GROOVY_REFLECT(mShaderResNames)
	GROOVY_REFLECT(mShaderResources)
GROOVY_CLASS_REFLECTION_END()

void MaterialAssetFile::DeserializeOntoMaterial(Material* mat)
{
	check(mat);
	checkf(mShader, "Corrupted material file, missing shader");
	checkf(mShaderResNames.size() == mShaderResources.size(), "Corrupted material file, resource bindings count mismatch");

	// set shader
	mat->mShader = mShader;
	// set const buffers data
	mat->mConstBuffersData.resize(mConstBuffersData.size());
	memcpy(mat->mConstBuffersData.data(), mConstBuffersData.data(), mConstBuffersData.size());
	// set resources (textures)
	mat->mResources.clear();
	for (const auto& shaderRes : mShader->GetPixelTexturesRes())
	{
		MaterialResource& matRes = mat->mResources.emplace_back();
		matRes.name = shaderRes.name;
		matRes.slot = shaderRes.bindSlot;
		matRes.res = nullptr;

		auto it = std::find(mShaderResNames.begin(), mShaderResNames.end(), shaderRes.name);
		uint32 resIndex = it - mShaderResNames.begin();
		if (resIndex < mShaderResNames.size())
			matRes.res = mShaderResources[resIndex];
		
		if (!matRes.res)
			matRes.res = DEFAULT_TEXTURE;
	}
}

Material::Material()
	: mShader(nullptr), mUUID(0), mLoaded(false)
{
}

void Material::Load()
{
	if (mLoaded)
		return;

	AssetLoader::LoadMaterial(this);

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