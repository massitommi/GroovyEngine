#include "material.h"

Material::Material(Shader* shader)
	: mShader(nullptr), mUUID(0)
{
	SetShader(shader);
}

void Material::SetShader(Shader* shader)
{
	mShader = shader;

	// resize constbuffers
	size_t shaderBuffersSize = 0;
	for (const auto& bufferDesc : shader->GetPixelConstBuffersDesc())
		shaderBuffersSize += bufferDesc.size;
	mConstBuffersData.resize(shaderBuffersSize);
	// resize texture slots
	mTextures.resize(shader->GetPixelTexturesRes().size(), nullptr);
}

void Material::SetTexture(Texture* texture, uint32 slot)
{
	check(slot < mTextures.size());

	mTextures[slot] = texture;
}

void Material::SetTextures(Texture* texture)
{
	for (Texture*& tex : mTextures)
		tex = texture;
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

