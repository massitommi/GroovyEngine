#pragma once

#include "../renderer_api.h"

class D3D11RendererAPI : public RendererAPI
{
public:
	D3D11RendererAPI(RendererAPISpec spec, class Window* wnd);
	virtual ~D3D11RendererAPI();

	virtual void DrawIndexed(uint32 vertexOffset, uint32 indexOffset, uint32 indexCount) override;
	virtual void Present() override;
	virtual void SetFullscreen(bool fullscreen) override;
	virtual void SetVSync(uint32 syncInterval) override;
	virtual RendererAPISpec GetSpec() const override { return mSpec; }
	virtual RasterizerState GetRasterizerState() const override { return mRasterizerState; }
	virtual void SetRasterizerState(RasterizerState newState) override;

private:
	RendererAPISpec mSpec;
	RasterizerState mRasterizerState;
};