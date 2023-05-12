#if PLATFORM_WIN32

#include "d3d11_texture.h"
#include "d3d11_utils.h"

static DXGI_FORMAT GetNativeFormat(EColorFormat format)
{
	switch (format)
	{
		case COLOR_FORMAT_R8G8B8A8_UNORM:	return DXGI_FORMAT_R8G8B8A8_UNORM;
	}
	check(0);
	return DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
}

D3D11Texture::D3D11Texture(TextureSpec specs, const void* data, size_t size)
	: mSpecs(specs), mUUID(0)
{
	mHandle = d3dUtils::CreateTexture(specs.width, specs.height, D3D11_BIND_SHADER_RESOURCE, GetNativeFormat(specs.format), data, size);
	mView = d3dUtils::CreateShaderResourceView(mHandle);
}

D3D11Texture::~D3D11Texture()
{
	mHandle->Release();
	mView->Release();
}

void D3D11Texture::Bind(uint32 slot)
{
	check(slot <= 32);
	d3dUtils::gContext->PSSetShaderResources(slot, 1, &mView);
}

void D3D11Texture::SetData(void* data, size_t size)
{
}

#endif