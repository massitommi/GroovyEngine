#pragma once

#include "core/core.h"
#include "api/shader.h"
#include "api/texture.h"
#include "assets/asset.h"

class Material
{
public:
	Material();

	const Shader* GetShader() const { return mShader; }

	const Buffer& GetVertexConstBuffersData() const { return mVertexConstBuffersData; }
	const Buffer& GetPixelConstBuffersData() const { return mVertexConstBuffersData; }

	void SetShader(Shader* shader) { mShader = shader; }

	void ConstructResources();

	AssetUUID GetUUID() const { return mUUID; }

#if WITH_EDITOR
	AssetUUID& GetShaderID() { return mShaderID; }
	std::vector<AssetUUID>& GetTexturesID() { return mTexturesID; }
#endif

	void SubmitBuffersToShader();

	void Serialize(Buffer& outBuffer);
	void Deserialize(const Buffer& file);

private:
	AssetUUID mUUID;
	AssetUUID mShaderID;
	std::vector<AssetUUID> mTexturesID;

	Buffer mVertexConstBuffersData;
	Buffer mPixelConstBuffersData;
	Shader* mShader;
	std::vector<Texture*> mTextures;
};