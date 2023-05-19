#include "imgui_renderer.h"

#include "vendor/imgui/imgui.h"
#include "vendor/imgui/backends/imgui_impl_win32.h"
#include "vendor/imgui/backends/imgui_impl_dx11.h"

#include "platform/window.h"
#include "renderer/api/d3d11/d3d11_utils.h"

extern Window* gWindow;

void D3D11_ImGuiRenderer::Init()
{
	ImGui_ImplWin32_Init(gWindow->GetNativeHandle());
	ImGui_ImplDX11_Init(d3dUtils::gDevice, d3dUtils::gContext);
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