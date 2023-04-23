#include "engine/application.h"

#include "platform/platform.h"

#include "renderer/api/renderer_api.h"
#include "renderer/api/framebuffer.h"
#include "renderer/api/texture.h"
#include "renderer/api/shader.h"

#include "assets/assets.h"
#include "assets/asset_loader.h"
#include "asset_importer.h"

#include "vendor/imgui/imgui.h"
#include "vendor/imgui/backends/imgui_impl_win32.h"
#include "vendor/imgui/backends/imgui_impl_dx11.h"

#include "imgui_renderer/imgui_renderer.h"

static ImGuiRenderer* sRenderer = nullptr;
extern bool gEngineShouldRun;

void EditorInit();
void EditorRender();
void EditorShutdown();

void Application::Init()
{
	ImGui::CreateContext();
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	ImGui::StyleColorsDark();
	sRenderer = ImGuiRenderer::Create();
	sRenderer->Init();

	EditorInit();
}

void Application::Update(float deltaTime)
{

}

void Application::Render()
{
	sRenderer->NewFrame();
	ImGui::NewFrame();

	EditorRender();
}

void Application::Render2EditorOnly()
{
	ImGui::Render();
	sRenderer->RenderDrawData();
}

void Application::Shutdown()
{
	EditorShutdown();

	delete sRenderer;

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

static std::vector<std::pair<String, EAssetType>> sAssets;

void RefreshAssetList()
{
	sAssets = decltype(sAssets)();
	std::vector<String> assetFiles = FileSystem::GetFilesInDir("assets", { ".groovytexture", ".groovymodel3d" });
	for (const auto& file : assetFiles)
	{
		sAssets.push_back({ file, AssetImporter::GetTypeFromFilename(file) });
	}
}

void OnFilesDropped(const std::vector<String>& files)
{
	for (const String& file : files)
	{
		EAssetType assetType = AssetImporter::GetTypeFromFilename(file);

		switch (assetType)
		{
			case ASSET_TYPE_TEXTURE:	
			{
				auto res = SysMessageBox::Show_Info("Importer", "Click OK to import the texture");
				if (res == MESSAGE_BOX_RESPONSE_YES)
				{
					if (AssetImporter::ImportTexture(file, "assets"))
					{
						RefreshAssetList();
					}
					else
					{
						SysMessageBox::Show_Error("Import error", "Unable to import the asset");
					}
				}
			}
			break;

			default:
				SysMessageBox::Show_Error("Import error", "Not an importable asset : (");
				break;
		}
	}
}

static Texture* sAssetIcon;
static FrameBuffer* sGameViewportFrameBuffer;
static ImVec2 sGameViewportSize;

Texture* LoadEditorIcon(const String& path)
{
	Buffer data;
	TextureSpec spec;
	AssetImporter::GetRawTexture(path, data, spec);
	return Texture::Create(spec, data.data(), data.size());
}

void EditorInit()
{
	// assets used by the editor
	sAssetIcon = LoadEditorIcon("assets/asset_icon.png");
	
	// gameviewport framebuffer
	FrameBufferSpec gameViewportSpec;
	gameViewportSpec.colorAttachments = { COLOR_FORMAT_R8G8B8A8_UNORM };
	gameViewportSpec.hasDepthAttachment = true;
	gameViewportSpec.swapchainTarget = false;
	gameViewportSpec.width = sGameViewportSize.x = 100;
	gameViewportSpec.height = sGameViewportSize.y = 100;

	sGameViewportFrameBuffer = FrameBuffer::Create(gameViewportSpec);

	// trigger useful stuff
	RefreshAssetList();

	// test
	Buffer vertexSrc, pixelSrc;
	check(FileSystem::ReadFileBinary("assets/shaders/vertex.hlsl", vertexSrc) == FILE_OPEN_RESULT_OK);
	check(FileSystem::ReadFileBinary("assets/shaders/pixel.hlsl", pixelSrc) == FILE_OPEN_RESULT_OK);

	ShaderVariable att1;
	att1.name = "POSITION";
	att1.size = sizeof(float) * 4;
	att1.alignedOffset = 0;
	att1.type = SHADER_VARIABLE_TYPE_FLOAT4;

	ShaderVariable att2;
	att2.name = "COLOR";
	att2.size = sizeof(float) * 4;
	att2.alignedOffset = 4;
	att2.type = SHADER_VARIABLE_TYPE_FLOAT4;

	ShaderVariable att3;
	att3.name = "TEXTCOORDS";
	att3.size = sizeof(float) * 2;
	att3.alignedOffset = 8;
	att3.type = SHADER_VARIABLE_TYPE_FLOAT2;

	Shader* shader = Shader::Create( { *vertexSrc, vertexSrc.size() }, { *pixelSrc, pixelSrc.size() }, { att1, att2, att3 });
	int br = 1;
}

namespace panels
{
	void Assets()
	{
		static float zoom = 1.0f;

		float iconSize = 90 * zoom;
		uint32 numColumns = ImGui::GetContentRegionAvail().x / iconSize;
		if (!numColumns)
			numColumns = 1;

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 2,2 });
		ImGui::Begin("Asset manager");
		ImGui::Columns(numColumns, 0, false);

		for (const auto& asset : sAssets)
		{
			ImGui::ImageButton(asset.first.c_str(), sAssetIcon->GetRendererID(), { iconSize, iconSize }, { 0,0 }, { 1,1 }, { 1,1,1,1 });
			ImGui::TextWrapped(asset.first.c_str());
			ImGui::NextColumn();
		}

		ImGui::Columns(1);

		ImGui::Separator();
		ImGui::SetNextItemWidth(200.0f);
		ImGui::SliderFloat("Zoom level", &zoom, 0.5f, 2.0f);

		if (ImGui::Button("Refresh assets"))
		{
			RefreshAssetList();
		}

		ImGui::End();
		ImGui::PopStyleVar();
	}
	void EntityList()
	{
		ImGui::Begin("Enity list");
		ImGui::End();
	}
	void Properties()
	{
		ImGui::Begin("Properties");
		ImGui::End();
	}
	void GameViewport()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0,0 });
		ImGui::Begin("Game viewport");
		// resize framebuffer if needed
		ImVec2 wndSize = ImGui::GetContentRegionAvail();
		bool differentSize = wndSize.x != sGameViewportFrameBuffer->GetSpecs().width || wndSize.y != sGameViewportFrameBuffer->GetSpecs().height;
		bool okSize = wndSize.x > 0 && wndSize.y > 0;
		if (differentSize && okSize)
		{
			sGameViewportFrameBuffer->Resize(wndSize.x, wndSize.y);
		}
		// clear frame buffer
		sGameViewportFrameBuffer->ClearDepthAttachment();
		sGameViewportFrameBuffer->ClearColorAttachment(0, { 0.76f, 0.84f, 0.725f, 1.0f });

		// imgui stuff...
		ImGui::Image(sGameViewportFrameBuffer->GetRendererID(0), wndSize);
		ImGui::End();
		ImGui::PopStyleVar();
	}
}

void EditorRender()
{
    static bool p_open = true;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace", &p_open, window_flags);
    ImGui::PopStyleVar(3);

    // Submit the DockSpace
    ImGuiID dockspace_id = ImGui::GetID("Dockspaceeee");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
			if (ImGui::MenuItem("Exit"))
			{
				gEngineShouldRun = false;
			}

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    // Render windows here!
	
	panels::GameViewport();
	panels::EntityList();
	panels::Properties();
	panels::Assets();

	ImGui::End();
}

void EditorShutdown()
{
	delete sAssetIcon;
	delete sGameViewportFrameBuffer;
}

