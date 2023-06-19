#pragma once

#include "../texture.h"

class D3D11Texture : public Texture
{
public:
	D3D11Texture(TextureSpec specs, const void* data, size_t size);
	virtual ~D3D11Texture();

	virtual void __internal_SetUUID(AssetUUID uuid) override { mUUID = uuid; }
	virtual AssetUUID GetUUID() const override { return mUUID; }

	virtual void Bind(uint32 slot) override;
	virtual void* GetRendererID() const override { return mView; }
	virtual void SetData(void* data, size_t size) override;
	virtual TextureSpec GetSpecs() const override { return mSpecs; }

private:
	struct ID3D11Texture2D* mHandle;
	struct ID3D11ShaderResourceView* mView;
	TextureSpec mSpecs;

	AssetUUID mUUID;
};