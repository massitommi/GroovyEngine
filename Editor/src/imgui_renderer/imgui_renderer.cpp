#include "imgui_renderer.h"
#include "renderer/api/renderer_api.h"

ImGuiRenderer* ImGuiRenderer::Create()
{
	switch (RendererAPI::GetAPI())
	{
		case RENDERER_API_D3D11: return new D3D11_ImGuiRenderer();
	}
	checkslowf(0, "No rendering api for editor WTF?");
	return nullptr;
}