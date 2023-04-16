#include "editor.h"

#include "engine/engine.h"

#include "vendor/imgui/imgui.h"
#include "vendor/imgui/backends/imgui_impl_win32.h"
#include "vendor/imgui/backends/imgui_impl_dx11.h"

#include "imgui_renderer/imgui_renderer.h"

#include "renderer/renderer_api.h"
#include "renderer/framebuffer.h"
#include "renderer/texture.h"

#include "platform/platform.h"

#include "assets/assets.h"
#include "assets/asset_loader.h"

static ImGuiRenderer* sRenderer = nullptr;
static FrameBuffer* gameFrameBuffer = nullptr;
static Texture* testTexture = nullptr;

void OnFilesDropped(const std::vector<String>& files)
{
	for (const String& file : files)
	{
		// import if groovyfile
	}
}

void Engine::Init()
{
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	sRenderer = ImGuiRenderer::Create();
	sRenderer->Init();

	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	const WindowProps& wndProps = Window::Get()->GetProps();
	FrameBufferSpec gameVieportSpec;
	gameVieportSpec.width = 400;
	gameVieportSpec.height = 400;
	gameVieportSpec.hasDepthAttachment = true;
	gameVieportSpec.swapchainTarget = false;
	gameVieportSpec.colorAttachments = { FRAME_BUFFER_TEXTURE_FORMAT_RGBA };
	gameFrameBuffer = FrameBuffer::Create(gameVieportSpec);

	SUBSCRIBE_TO_EVENT(DropFilesEvent, OnFilesDropped);
	
	testTexture = AssetLoader::Load<Texture>("assets/ue4.gvytexture");
}

void Engine::Update(float deltaTime)
{

}

void Engine::Render()
{
	gameFrameBuffer->ClearColorAttachment(0, {});
	gameFrameBuffer->ClearDepthAttachment();

	sRenderer->NewFrame();
	ImGui::NewFrame();
	
	// BEGIN rendering and offscreen framebuffers rendering

	gameFrameBuffer->Bind();
	
	ImGui::Begin("Test");
	ImGui::Image((void*)testTexture->GetRendererID(), { 200.0f, 200.0f });
	ImGui::End();
	

	ImGui::Begin("ciao");
	ImGui::Text("ooo");
	ImGui::End();

	// END
}

void Engine::Render2()
{
	ImGui::Render();
	sRenderer->RenderDrawData();
}

void Engine::Shutdown()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

