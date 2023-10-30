#include "editor.h"

#include "imgui_renderer.h"

#include "vendor/imgui/imgui.h"
#include "vendor/imgui/backends/imgui_impl_win32.h"
#include "vendor/imgui/backends/imgui_impl_dx11.h"

#include "platform/window.h"
#include "renderer/api/d3d11/d3d11_utils.h"

static ID3D11BlendState* imguiBlendState;
static float imguiBlendFactor[4];
static uint32 imguiBlendMask;

void D3D11_ImGuiRenderer::Init()
{
	ImGui_ImplWin32_Init(gWindow->GetNativeHandle());
	ImGui_ImplDX11_Init(d3d11Utils::gDevice, d3d11Utils::gContext);
}

void D3D11_ImGuiRenderer::Shutdown()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
}

void D3D11_ImGuiRenderer::NewFrame()
{
	ImGui_ImplWin32_NewFrame();
	ImGui_ImplDX11_NewFrame();
}

void D3D11_ImGuiRenderer::RenderDrawData()
{
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void D3D11_ImGuiRenderer::SetGroovyRenderState()
{
	ImGui::GetWindowDrawList()->AddCallback(
		[](const ImDrawList* imdrawlist, const ImDrawCmd* cmd)
		{
			d3d11Utils::gContext->OMGetBlendState(&imguiBlendState, imguiBlendFactor, &imguiBlendMask);
			imguiBlendState->Release();
			d3d11Utils::gContext->OMSetBlendState(0, 0, 0xffffffff);

		}, nullptr);
}

void D3D11_ImGuiRenderer::SetImguiRenderState()
{
	ImGui::GetWindowDrawList()->AddCallback(
		[](const ImDrawList* imdrawlist, const ImDrawCmd* cmd)
		{
			d3d11Utils::gContext->OMSetBlendState(imguiBlendState, imguiBlendFactor, imguiBlendMask);

		}, nullptr);
}
