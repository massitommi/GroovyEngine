#pragma once

#include "../renderer_api.h"

class D3D11RendererAPI : public RendererAPI
{
public:
	D3D11RendererAPI(class Window* wnd);
	virtual ~D3D11RendererAPI();

	virtual void DrawIndexed(uint32 vertexOffset, uint32 indexOffset, uint32 indexCount) override;
	virtual void Present(uint32 syncInteval) override;
};