#pragma once

#include "renderer/texture.h"

class D3D11Texture : public Texture
{
public:
	D3D11Texture(TextureSpec specs, void* data, size_t size);
	virtual ~D3D11Texture();

	virtual void Bind(uint32 slot) override;
	virtual uint64 GetRendererID() const override { return (uint64)mView; }
	virtual void SetData(void* data, size_t size) override;
	virtual TextureSpec GetSpecs() const override { return mSpecs; }

private:
	struct ID3D11Texture2D* mHandle;
	struct ID3D11ShaderResourceView* mView;
	TextureSpec mSpecs;
};