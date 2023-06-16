#pragma once

#include "core/core.h"
#include "api/shader.h"
#include "api/texture.h"
#include "assets/asset.h"
#include "assets/asset_file.h"
#include "classes/object.h"

struct MaterialResource
{
	std::string name;
	Texture* res;
	uint32 slot;
};

GROOVY_CLASS_DECL(MaterialAssetFile)
class MaterialAssetFile : public AssetFile
{
	GROOVY_CLASS_BODY(MaterialAssetFile, AssetFile)
public:

	void SetShaderToSerialize(Shader* shader)
	{
		mShader = mShader;
	}

	void PushResourceToSerialize(const MaterialResource& res)
	{
		mShaderResNames.push_back(res.name);
		mShaderResources.push_back(res.res);
	}

	void SetConstBuffersDataToSerialize(const Buffer& data)
	{
		mConstBuffersData.resize(data.size());
		memcpy(mConstBuffersData.data(), data.data(), data.size());
	}

	virtual void DeserializeOntoMaterial(Material* mat);

private:
	Shader* mShader = nullptr;
	Buffer mConstBuffersData;
	std::vector<std::string> mShaderResNames;
	std::vector<Texture*> mShaderResources;
};

class Material : public AssetInstance
{
public:
	Material();

	void __internal_SetUUID(AssetUUID uuid) override { mUUID = uuid; }
	AssetUUID GetUUID() const override { return mUUID; }
	virtual bool IsLoaded() const override { return mLoaded; }
	virtual void Load();

	// shader descs and our data is same size ?
	bool Validate();
	
	const Shader* GetShader() const { return mShader; }
	const std::vector<MaterialResource>& GetResources() const { return mResources; }
	const Buffer& GetConstBuffersData() const { return mConstBuffersData; }

#if WITH_EDITOR

	Shader*& ShaderRef() { return mShader; }
	std::vector<MaterialResource>& ResourcesRef() { return mResources; }

#endif

private:
	Shader* mShader;
	std::vector<MaterialResource> mResources;
	Buffer mConstBuffersData;

	AssetUUID mUUID;
	bool mLoaded;

	friend class MaterialAssetFile;
};