#pragma once

#include "core/core.h"
#include "renderer/color.h"
#include "assets/asset.h"

struct TextureSpec
{
	uint32 width, height;
	EColorFormat format;
};

class Texture : public AssetInstance
{
public:

	virtual bool LazyLoadAndUnload() const override { return false; }
	virtual bool IsLoaded() const override { return true; }
	virtual void Load() override {}
	virtual void Save() override {}
	virtual void Serialize(DynamicBuffer& fileData) const override {}
	virtual void Deserialize(BufferView fileData) override {}
#if WITH_EDITOR
	virtual bool Editor_FixDependencyDeletion(AssetHandle assetToBeDeleted) override { return false; }
#endif

	virtual void Bind(uint32 slot) = 0;
	virtual void* GetRendererID() const = 0;
	virtual void SetData(void* data, size_t size) = 0;
	virtual TextureSpec GetSpecs() const = 0;

	static Texture* Create(TextureSpec specs, const void* data, size_t size);
};

