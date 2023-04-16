#if PLATFORM_WIN32

#include "d3d11_framebuffer.h"
#include "d3d11_utils.h"

static DXGI_FORMAT GetNativeFormat(EFrameBufferTextureFormat format)
{
	switch (format)
	{
		case FRAME_BUFFER_TEXTURE_FORMAT_RGBA:		return DXGI_FORMAT_R8G8B8A8_UNORM;
		case FRAME_BUFFER_TEXTURE_FORMAT_REDINT:	return DXGI_FORMAT_R32_UINT;
	}
	check(0);
	return DXGI_FORMAT_UNKNOWN;
}

D3D11FrameBuffer::D3D11FrameBuffer(const FrameBufferSpec& specs)
	: mSpecs(specs)
{
	checkslow(specs.colorAttachments.size());
	
	mColorAttachments.resize(specs.colorAttachments.size());
	mColorViews.resize(specs.colorAttachments.size());
	mRenderTargets.resize(specs.colorAttachments.size());
	
	Create();
}

D3D11FrameBuffer::~D3D11FrameBuffer()
{
	Destroy();
}

void D3D11FrameBuffer::Bind()
{
	d3dUtils::gContext->OMSetRenderTargets(mRenderTargets.size(), &mRenderTargets[0], mDepthBufferView);

	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = mSpecs.width;
	viewport.Height = mSpecs.height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	d3dUtils::gContext->RSSetViewports(1, &viewport);
}

void D3D11FrameBuffer::Resize(uint32 width, uint32 height)
{
	Destroy();
	d3dUtils::ResizeBackBuffer(width, height);
	mSpecs.width = width;
	mSpecs.height = height;
	Create();
}

void D3D11FrameBuffer::ClearColorAttachment(uint32 colorIndex, ClearColor clearColor)
{
	check(colorIndex <= mRenderTargets.size());
	d3dUtils::gContext->ClearRenderTargetView(mRenderTargets[colorIndex], &clearColor.r);
}

void D3D11FrameBuffer::ClearDepthAttachment()
{
	check(mSpecs.hasDepthAttachment);
	d3dUtils::gContext->ClearDepthStencilView(mDepthBufferView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

uint64 D3D11FrameBuffer::GetRendererID(uint32 colorIndex) const
{
	check(colorIndex <= mRenderTargets.size());
	return (uint64)mColorViews[colorIndex];
}

void D3D11FrameBuffer::Create()
{
	if (mSpecs.swapchainTarget)
	{
		mColorAttachments[0] = d3dUtils::GetBackBuffer();
		mColorViews[0] = nullptr;
		mRenderTargets[0] = d3dUtils::CreateRenderTargetView(mColorAttachments[0]);
	}

	for (uint32 i = mSpecs.swapchainTarget ? 1 : 0; i < mColorAttachments.size(); i++)
	{
		mColorAttachments[i] = d3dUtils::CreateTexture
		(
			mSpecs.width, mSpecs.height, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET,
			GetNativeFormat(mSpecs.colorAttachments[i]), nullptr, 0
		);
		mColorViews[i] = d3dUtils::CreateShaderResourceView(mColorAttachments[i]);
		mRenderTargets[i] = d3dUtils::CreateRenderTargetView(mColorAttachments[i]);
	}

	if (mSpecs.hasDepthAttachment)
	{
		mDepthBuffer = d3dUtils::CreateTexture(mSpecs.width, mSpecs.height, D3D11_BIND_DEPTH_STENCIL, DXGI_FORMAT_D24_UNORM_S8_UINT, nullptr, 0);
		mDepthBufferView = d3dUtils::CreateDepthStencilView(mDepthBuffer);
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

	if (mSpecs.hasDepthAttachment)
	{
		mDepthBuffer->Release();
		mDepthBufferView->Release();
	}
}

#endif