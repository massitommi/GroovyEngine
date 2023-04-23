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
	d3dUtils::gContext->OMSetRenderTargets(mRenderTargets.size(), &mRenderTargets[0], mDepthBufferView);

	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = mSpec.width;
	viewport.Height = mSpec.height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	d3dUtils::gContext->RSSetViewports(1, &viewport);
}

void D3D11FrameBuffer::Resize(uint32 width, uint32 height)
{
	Destroy();
	if(mSpec.swapchainTarget)
		d3dUtils::ResizeBackBuffer(width, height);
	mSpec.width = width;
	mSpec.height = height;
	Create();
}

void D3D11FrameBuffer::ClearColorAttachment(uint32 colorIndex, ClearColor clearColor)
{
	check(colorIndex <= mRenderTargets.size());
	d3dUtils::gContext->ClearRenderTargetView(mRenderTargets[colorIndex], &clearColor.r);
}

void D3D11FrameBuffer::ClearColorAttachments(ClearColor clearColor)
{
	for (auto renderTarget : mRenderTargets)
	{
		d3dUtils::gContext->ClearRenderTargetView(renderTarget, &clearColor.r);
	}
}

void D3D11FrameBuffer::ClearDepthAttachment()
{
	check(mSpec.hasDepthAttachment);
	d3dUtils::gContext->ClearDepthStencilView(mDepthBufferView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
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
		mColorAttachments[0] = d3dUtils::GetBackBuffer();
		mColorViews[0] = nullptr;
		mRenderTargets[0] = d3dUtils::CreateRenderTargetView(mColorAttachments[0]);
	}

	for (uint32 i = mSpec.swapchainTarget ? 1 : 0; i < mColorAttachments.size(); i++)
	{
		mColorAttachments[i] = d3dUtils::CreateTexture
		(
			mSpec.width, mSpec.height, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET,
			GetNativeFormat(mSpec.colorAttachments[i]), nullptr, 0
		);
		mColorViews[i] = d3dUtils::CreateShaderResourceView(mColorAttachments[i]);
		mRenderTargets[i] = d3dUtils::CreateRenderTargetView(mColorAttachments[i]);
	}

	if (mSpec.hasDepthAttachment)
	{
		mDepthBuffer = d3dUtils::CreateTexture(mSpec.width, mSpec.height, D3D11_BIND_DEPTH_STENCIL, DXGI_FORMAT_D24_UNORM_S8_UINT, nullptr, 0);
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

	if (mSpec.hasDepthAttachment)
	{
		mDepthBuffer->Release();
		mDepthBufferView->Release();
	}
}

#endif