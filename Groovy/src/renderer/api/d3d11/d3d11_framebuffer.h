#pragma once

#include "../framebuffer.h"

class D3D11FrameBuffer : public FrameBuffer
{
public:
	D3D11FrameBuffer(const FrameBufferSpec& spec);
	virtual ~D3D11FrameBuffer();

	virtual const FrameBufferSpec& GetSpecs() const override { return mSpec; }
	virtual void Bind() override;
	virtual void Resize(uint32 width, uint32 height) override;
	virtual void ClearColorAttachment(uint32 colorIndex, ClearColor clearColor) override;
	virtual void ClearColorAttachments(ClearColor clearColor) override;
	virtual void ClearDepthAttachment() override;
	virtual void* GetRendererID(uint32 colorIndex) const override;

private:
	std::vector<struct ID3D11Texture2D*> mColorAttachments;
	std::vector<struct ID3D11ShaderResourceView*> mColorViews;
	std::vector<struct ID3D11RenderTargetView*> mRenderTargets;
	struct ID3D11Texture2D* mDepthBuffer = nullptr;
	struct ID3D11DepthStencilView* mDepthBufferView = nullptr;
	FrameBufferSpec mSpec;

	void Create();
	void Destroy();
};
