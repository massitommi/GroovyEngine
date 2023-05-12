#include "material.h"

Material::Material()
	: mShader(nullptr), mUUID(0)
{
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

	if (mShader->GetPixelTexturesRes().size() != mTextures.size())
		return false;

	return true;
}

