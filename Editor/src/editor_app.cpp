#include "engine/application.h"
#include <Windows.h>
#include "vendor/imgui/imgui.h"
#include "vendor/imgui/backends/imgui_impl_win32.h"
#include "vendor/imgui/backends/imgui_impl_dx11.h"
#include "imgui_renderer/imgui_renderer.h"
#include "editor.h"
#include "renderer/api/framebuffer.h"
#include "platform/messagebox.h"

IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT Win32_EditorWndProcCallback(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
}

static ImGuiRenderer* sRenderer = nullptr;

extern FrameBuffer* gScreenFrameBuffer;

void Application::Init()
{
	ImGui::CreateContext();
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	ImGui::StyleColorsDark();

	sRenderer = ImGuiRenderer::Create();
	sRenderer->Init();

	editor::Init();
}

void Application::Update(float deltaTime)
{
	editor::Update(deltaTime);
}

void Application::Render()
{
	sRenderer->NewFrame();
	ImGui::NewFrame();

	editor::Render();

	gScreenFrameBuffer->Bind();
	sRenderer->RenderDrawData();
}

void Application::Shutdown()
{
	editor::Shutdown();

	delete sRenderer;

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void Application::Travel(Scene* scene)
{
	SysMessageBox::Show_Warning("Can't travel in the editor", "Traveling in the editor isn't currently supported");
}
