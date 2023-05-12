#pragma once

#include "core/core.h"
#include "api/shader.h"
#include "api/texture.h"
#include "assets/asset.h"

template<typename T>
class AssetSerializer;

class Material : public AssetInstance
{
public:
	Material();

	void __internal_SetUUID(AssetUUID uuid) override { mUUID = uuid; }

	AssetUUID GetUUID() const override { return mUUID; }
	const Shader* GetShader() const { return mShader; }
	const Buffer& GetConstBuffersData() const { return mConstBuffersData; }

	// shader descs and our data is same size ?
	bool Validate();

private:
	Shader* mShader;
	std::vector<Texture*> mTextures;
	Buffer mConstBuffersData;

	AssetUUID mUUID;

public:
	friend class AssetSerializer<Material>;
};