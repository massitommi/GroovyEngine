#include "engine/application.h"
#include "editor.h"
#include <Windows.h>
#include "vendor/imgui/imgui.h"
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

void Application::PreInit()
{
	SetGroovyLogger(editor::ConsoleLog);
}

void Application::Init()
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

void Application::Update(float deltaTime)
{
	editor::Update(deltaTime);
}

void Application::Render()
{
	gGroovyGuiRenderer->NewFrame();
	
	ImGui::NewFrame();

	editor::Render();

	ImGui::Render();

	gScreenFrameBuffer->Bind();
	
	gGroovyGuiRenderer->RenderDrawData();
}

void Application::Shutdown()
{
	editor::Shutdown();

	gGroovyGuiRenderer->Shutdown();
	delete gGroovyGuiRenderer;

	ImGui::DestroyContext();
}
