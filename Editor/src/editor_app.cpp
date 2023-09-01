#include "editor.h"
#include "editor_app.h"
#include <Windows.h>
#include "vendor/imgui/imgui.h"
#include "vendor/imgui/backends/imgui_impl_win32.h"
#include "vendor/imgui/backends/imgui_impl_dx11.h"
#include "imgui_renderer/imgui_renderer.h"
#include "editor.h"
#include "renderer/api/framebuffer.h"
#include "platform/messagebox.h"
#include "renderer/api/shader.h"
#include "platform/window.h"

IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

extern CORE_API LRESULT(*EditorWndProcCallback)(HWND, UINT, WPARAM, LPARAM);

ImGuiRenderer* gGroovyGuiRenderer = nullptr;

extern CORE_API Shader* DEFAULT_SHADER;

Application* GetApplication() { return new EditorApplication(); }

void EditorApplication::Init()
{
	EditorWndProcCallback = ImGui_ImplWin32_WndProcHandler;

	gWindow->SetTitle("Groovy Editor - " + gProj.GetProjectName());

	ImGui::CreateContext();
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	ImGui::StyleColorsDark();

	gGroovyGuiRenderer = ImGuiRenderer::Create();
	gGroovyGuiRenderer->Init();

	editor::Init();
}

void EditorApplication::Update(float deltaTime)
{
	editor::Update(deltaTime);
}

void EditorApplication::Render()
{
	gGroovyGuiRenderer->NewFrame();
	
	ImGui::NewFrame();

	editor::Render();

	ImGui::Render();

	gScreenFrameBuffer->Bind();
	
	gGroovyGuiRenderer->RenderDrawData();
}

void EditorApplication::Shutdown()
{
	editor::Shutdown();

	delete gGroovyGuiRenderer;

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void EditorApplication::Travel(Scene* scene)
{
	SysMessageBox::Show_Warning("Can't travel in the editor", "Traveling in the editor isn't currently supported");
}
