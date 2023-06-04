#pragma once

#include "core/core.h"
#include "api/shader.h"
#include "api/texture.h"
#include "assets/asset.h"
#include "classes/object.h"

class Material : public AssetInstance
{
public:
	Material(Shader* shader);

	void __internal_SetUUID(AssetUUID uuid) override { mUUID = uuid; }

	AssetUUID GetUUID() const override { return mUUID; }
	const Shader* GetShader() const { return mShader; }
	const Buffer& GetConstBuffersData() const { return mConstBuffersData; }

	const std::vector<Texture*> GetTextures() const { return mTextures; }
	void SetShader(Shader* shader);
	void SetTexture(Texture* texture, uint32 slot);
	void SetTextures(Texture* texture);

	// shader descs and our data is same size ?
	bool Validate();

private:
	Shader* mShader;
	std::vector<Texture*> mTextures;
	Buffer mConstBuffersData;

	AssetUUID mUUID;

public:
	friend class AssetLoader;
	friend class AssetSerializer;
};