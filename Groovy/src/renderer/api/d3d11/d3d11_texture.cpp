#if PLATFORM_WIN32

#include "d3d11_texture.h"
#include "d3d11_utils.h"

static DXGI_FORMAT GetNativeFormat(EColorFormat format)
{

	switch (format)
	{
		case COLOR_FORMAT_R8G8B8A8_UNORM:	return DXGI_FORMAT_R8G8B8A8_UNORM;
	}
	checkf(0, "Unknown EColorFormat value: %u", (int)format);
	return DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
}

D3D11Texture::D3D11Texture(TextureSpec specs, const void* data, size_t size)
	: mSpecs(specs), mUUID(0)
{
	DXGI_FORMAT format = GetNativeFormat(specs.format);

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.Format = format;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.Width = specs.width;
	desc.Height = specs.height;
	desc.MipLevels = desc.ArraySize = 1;
	desc.SampleDesc.Count = 1;

	if (data)
	{
		D3D11_SUBRESOURCE_DATA initialData = {};
		initialData.pSysMem = data;

		switch (format)
		{
			case DXGI_FORMAT_R8G8B8A8_UNORM:
				initialData.SysMemPitch = specs.width * 4;
				break;

			default:
				checkf(0, "D3D11Texture unknown format for pitch calculation");
		}

		d3dcheckslow(d3d11Utils::gDevice->CreateTexture2D(&desc, &initialData, &mHandle));
	}
	else
	{
		d3dcheckslow(d3d11Utils::gDevice->CreateTexture2D(&desc, nullptr, &mHandle));
	}


	d3dcheckslow(d3d11Utils::gDevice->CreateShaderResourceView(mHandle, nullptr, &mView));
}

D3D11Texture::~D3D11Texture()
{
	mHandle->Release();
	mView->Release();
}

void D3D11Texture::Bind(uint32 slot)
{
	check(slot <= 32);
	d3d11Utils::gContext->PSSetShaderResources(slot, 1, &mView);
}

void D3D11Texture::SetData(void* data, size_t size)
{
}

#endif