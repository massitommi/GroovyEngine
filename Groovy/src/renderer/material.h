#pragma once

#include "core/core.h"
#include "api/shader.h"
#include "api/texture.h"
#include "assets/asset.h"
#include "classes/object.h"

struct MaterialResource
{
	std::string name;
	Texture* res;
	uint32 slot;
};

class Material : public AssetInstance
{
	friend class Renderer;

public:
	Material();

	void __internal_SetUUID(AssetUUID uuid) override { mUUID = uuid; }
	AssetUUID GetUUID() const override { return mUUID; }
	virtual bool IsLoaded() const override { return mLoaded; }
	virtual void Load() override;
	virtual void Save() override;
#if WITH_EDITOR
	virtual bool Editor_FixDependencyDeletion(AssetHandle assetToBeDeleted) override;
#endif

	// shader descs and our data is same size ?
	bool Validate() const;

	void SetShader(Shader* shader);
	void SetResource(Texture* texture, uint32 slot);
	void SetResources(Texture* texture);

	void FixForRendering();

	const Shader* GetShader() const { return mShader; }
	const std::vector<MaterialResource>& GetResources() const { return mResources; }
	const Buffer& GetConstBuffersData() const { return mConstBuffersData; }

	virtual void Serialize(DynamicBuffer& fileData) const override;
	virtual void Deserialize(BufferView fileData) override;

#if WITH_EDITOR

	std::vector<MaterialResource>& Editor_ResourcesRef() { return mResources; }

#endif

private:
	Shader* mShader;
	std::vector<MaterialResource> mResources;
	Buffer mConstBuffersData;

	AssetUUID mUUID;
	bool mLoaded;
};

GROOVY_CLASS_DECL(MaterialAssetFile)
class MaterialAssetFile : public GroovyObject
{
	GROOVY_CLASS_BODY(MaterialAssetFile, GroovyObject)

public:
	Shader* shader = nullptr;
	Buffer constBuffersData;
	std::vector<std::string> shaderResNames;
	std::vector<Texture*> shaderRes;
};