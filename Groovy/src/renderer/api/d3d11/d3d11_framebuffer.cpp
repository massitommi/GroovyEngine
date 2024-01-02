#if PLATFORM_WIN32

#include "d3d11_framebuffer.h"
#include "d3d11_utils.h"

static DXGI_FORMAT GetNativeFormat(EColorFormat format)
{
	switch (format)
	{
		case COLOR_FORMAT_R8G8B8A8_UNORM:	return DXGI_FORMAT_R8G8B8A8_UNORM;
		case COLOR_FORMAT_R32_UINT:			return DXGI_FORMAT_R32_UINT;
	}
	check(0);
	return DXGI_FORMAT_UNKNOWN;
}

D3D11FrameBuffer::D3D11FrameBuffer(const FrameBufferSpec& spec)
	: mSpec(spec)
{
	checkslow(spec.colorAttachments.size());
	
	mColorAttachments.resize(spec.colorAttachments.size());
	mColorViews.resize(spec.colorAttachments.size());
	mRenderTargets.resize(spec.colorAttachments.size());
	
	Create();
}

D3D11FrameBuffer::~D3D11FrameBuffer()
{
	Destroy();
}

void D3D11FrameBuffer::Bind()
{
	d3d11Utils::gContext->OMSetRenderTargets((UINT)mRenderTargets.size(), &mRenderTargets[0], mDepthBufferView);

	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (FLOAT)mSpec.width;
	viewport.Height = (FLOAT)mSpec.height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	d3d11Utils::gContext->RSSetViewports(1, &viewport);
}

void D3D11FrameBuffer::Resize(uint32 width, uint32 height)
{
	Destroy();
	
	if (mSpec.swapchainTarget)
	{
		checkslow(width && height);
		d3dcheckslow(d3d11Utils::gSwapChain->ResizeBuffers(1, width, height, DXGI_FORMAT_UNKNOWN, 0));
	}
	
	mSpec.width = width;
	mSpec.height = height;
	Create();
}

void D3D11FrameBuffer::ClearColorAttachment(uint32 colorIndex, ClearColor clearColor)
{
	check(colorIndex <= mRenderTargets.size());
	d3d11Utils::gContext->ClearRenderTargetView(mRenderTargets[colorIndex], (float*)&clearColor);
}

void D3D11FrameBuffer::ClearColorAttachments(ClearColor clearColor)
{
	for (auto renderTarget : mRenderTargets)
	{
		d3d11Utils::gContext->ClearRenderTargetView(renderTarget, (float*)&clearColor);
	}
}

void D3D11FrameBuffer::ClearDepthAttachment()
{
	check(mSpec.hasDepthAttachment);
	d3d11Utils::gContext->ClearDepthStencilView(mDepthBufferView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void* D3D11FrameBuffer::GetRendererID(uint32 colorIndex) const
{
	check(colorIndex <= mRenderTargets.size());
	return mColorViews[colorIndex];
}

void D3D11FrameBuffer::Create()
{
	if (mSpec.swapchainTarget)
	{
		d3dcheckslow(d3d11Utils::gSwapChain->GetBuffer(0 /*first buffer (back buffer)*/, __uuidof(ID3D11Texture2D), (void**)&mColorAttachments[0]));
		mColorViews[0] = nullptr;
		d3dcheckslow(d3d11Utils::gDevice->CreateRenderTargetView(mColorAttachments[0], nullptr, &mRenderTargets[0]));
	}

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	desc.Width = mSpec.width;
	desc.Height = mSpec.height;
	desc.MipLevels = desc.ArraySize = 1;
	desc.SampleDesc.Count = 1;

	for (uint32 i = mSpec.swapchainTarget ? 1 : 0; i < mColorAttachments.size(); i++)
	{
		desc.Format = GetNativeFormat(mSpec.colorAttachments[i]);

		d3dcheckslow(d3d11Utils::gDevice->CreateTexture2D(&desc, nullptr, &mColorAttachments[i]));
		d3dcheckslow(d3d11Utils::gDevice->CreateShaderResourceView(mColorAttachments[i], nullptr, &mColorViews[i]));
		d3dcheckslow(d3d11Utils::gDevice->CreateRenderTargetView(mColorAttachments[i], nullptr, &mRenderTargets[i]));
	}

	if (mSpec.hasDepthAttachment)
	{
		desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		d3dcheckslow(d3d11Utils::gDevice->CreateTexture2D(&desc, nullptr, &mDepthBuffer));
		d3dcheckslow(d3d11Utils::gDevice->CreateDepthStencilView(mDepthBuffer, nullptr, &mDepthBufferView));
	}
}

void D3D11FrameBuffer::Destroy()
{
	for (auto color : mColorAttachments)
	{
		color->Release();
	}

	for (auto view : mColorViews)
	{
		if(view)
			view->Release();
	}

	for (auto target : mRenderTargets)
	{
		target->Release();
	}

	if (mSpec.hasDepthAttachment)
	{
		mDepthBuffer->Release();
		mDepthBufferView->Release();
	}
}

#endif