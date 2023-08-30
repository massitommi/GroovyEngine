#pragma once

#include "core/core.h"

enum ERendererAPI
{
	RENDERER_API_NONE,
	RENDERER_API_D3D11
};

enum ERasterizerFillMode
{
	RASTERIZER_FILL_MODE_SOLID,
	RASTERIZER_FILL_MODE_WIREFRAME
};

enum ERasterizerCullMode
{
	RASTERIZER_CULL_MODE_BACK,
	RASTERIZER_CULL_MODE_FRONT,
	RASTERIZER_CULL_MODE_NONE
};

struct RendererAPISpec
{
	uint32 refreshrate;
	uint32 vsync;
};

struct RasterizerState
{
	ERasterizerFillMode fillMode;
	ERasterizerCullMode cullMode;
};

class CORE_API RendererAPI
{
public:
	virtual ~RendererAPI() = default;

	virtual void DrawIndexed(uint32 vertexOffset, uint32 indexOffset, uint32 indexCount) = 0;
	virtual void Present() = 0;
	virtual void SetFullscreen(bool fullscreen) = 0;
	virtual void SetVSync(uint32 syncInterval) = 0;
	virtual RendererAPISpec GetSpec() const = 0;
	virtual RasterizerState GetRasterizerState() const = 0;
	virtual void SetRasterizerState(RasterizerState newState) = 0;

	inline static RendererAPI& Get() { return *sInstance; }
	inline static ERendererAPI GetAPI() { return sSelectedAPI; }
	
	static void Create(ERendererAPI api, RendererAPISpec spec, class Window* wnd);
	static void Destroy();

private:
	static RendererAPI* sInstance;
	static ERendererAPI sSelectedAPI;
};