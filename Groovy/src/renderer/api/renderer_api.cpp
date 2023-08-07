#include "renderer_api.h"
#include "platform/window.h"

#include "d3d11/d3d11_renderer_api.h"

RendererAPI* RendererAPI::sInstance = nullptr;
ERendererAPI RendererAPI::sSelectedAPI = RENDERER_API_NONE;

void RendererAPI::Create(ERendererAPI api, RendererAPISpec spec, Window* wnd)
{
	switch (api)
	{
#if PLATFORM_WIN32
	case RENDERER_API_D3D11:
	{
		sInstance = new D3D11RendererAPI(spec, wnd);
		sSelectedAPI = RENDERER_API_D3D11;
		return;
	}
#endif
	}
	checkslowf(0, "No supported renderer API selected!");
}

void RendererAPI::Destroy()
{
	delete sInstance;
}
