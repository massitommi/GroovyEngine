#pragma once

#include "core/core.h"

enum ERendererAPI
{
	RENDERER_API_NONE,
	RENDERER_API_D3D11
};

class RendererAPI
{
public:
	virtual ~RendererAPI() = default;

	virtual void DrawIndexed(uint32 vertexOffset, uint32 indexOffset, uint32 indexCount) = 0;
	virtual void Present(uint32 syncInteval) = 0;

	inline static RendererAPI& Get() { return *sInstance; }
	inline static ERendererAPI GetAPI() { return sSelectedAPI; }
	
	static void Create(ERendererAPI api, class Window* wnd);
	static void Destroy();

private:
	static RendererAPI* sInstance;
	static ERendererAPI sSelectedAPI;
};