#include "engine/application.h"

#include "platform/platform.h"

#include "renderer/api/renderer_api.h"
#include "renderer/api/framebuffer.h"
#include "renderer/api/texture.h"
#include "renderer/api/shader.h"

#include "assets/asset.h"
#include "assets/asset_manager.h"
#include "assets/asset_loader.h"
#include "asset_importer.h"

#include "vendor/imgui/imgui.h"
#include "vendor/imgui/backends/imgui_impl_win32.h"
#include "vendor/imgui/backends/imgui_impl_dx11.h"
#include "vendor/imgui/misc/cpp/imgui_stdlib.h"

#include "imgui_renderer/imgui_renderer.h"

#include "renderer/material.h"
#include "project/Project.h"

#include "editor_window.h"

#include "renderer/renderer.h"

#include "math/matrix.h"

#include "assets/asset_serializer.h"

static ImGuiRenderer* sRenderer = nullptr;
extern bool gEngineShouldRun;
extern Project gProj;

static bool gPendingSave = false;

void EditorInit();
void EditorUpdate(float deltaTime);
void EditorRender();
void EditorShutdown();

bool OnCloseRequested();
void OnFilesDropped(const std::vector<std::string>& files);

void Application::Init()
{
	Window::Get()->SetTitle(gProj.name);

	Window::Get()->SubmitToWndCloseCallback(OnCloseRequested);
	Window::Get()->SubmitToWndFilesDropCallbacks(OnFilesDropped);

	ImGui::CreateContext();
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	ImGui::StyleColorsDark();
	sRenderer = ImGuiRenderer::Create();
	sRenderer->Init();

	EditorInit();
}

void Application::Update(float deltaTime)
{
	EditorUpdate(deltaTime);
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

void OnFilesDropped(const std::vector<std::string>& files)
{
	for (const std::string& file : files)
	{
		std::string newFileName = gProj.assetsPath + FileSystem::GetFilenameNoExt(file) + GROOVY_ASSET_EXT;
		EAssetType assetType = AssetImporter::GetTypeFromFilename(file);

		switch (assetType)
		{
			case ASSET_TYPE_TEXTURE:	
				AssetImporter::ImportTexture(file, newFileName);
				AssetManager::SaveRegistry();
				break;

			case ASSET_TYPE_MESH:
				AssetImporter::ImportModel3D(file, newFileName);
				AssetManager::SaveRegistry();
				break;
		}
	}
}

bool OnCloseRequested()
{
	if (gPendingSave)
	{
		auto response = SysMessageBox::Show
		(
			"Unsaved work",
			"You are leaving without saving, if you continue you will loose all the latest changes, are you sure?",
			MESSAGE_BOX_TYPE_WARNING,
			MESSAGE_BOX_OPTIONS_YESNOCANCEL
		);

		if (response != MESSAGE_BOX_RESPONSE_YES)
		{
			return false;
		}
	}
	return true;
}

static std::vector<EditorWindow*> sWindows;
static std::vector<EditorWindow*> sInsertQueue;
static std::vector<EditorWindow*> sRemoveQueue;

static Texture* sAssetIcon;
static FrameBuffer* sGameViewportFrameBuffer;
static ImVec2 sGameViewportSize;

static Shader* testShader;
static Mesh* testMesh;
static Texture* testTexture;
static Vec3 camLoc = {0,1.0f,-3};
static Vec3 camRot = {0,0,0};
static float camFOV = 60;

void SaveWork()
{
	AssetManager::SaveRegistry();
	gPendingSave = false;
}

Texture* LoadEditorIcon(const std::string& path)
{
	Buffer data;
	TextureSpec spec;
	AssetImporter::GetRawTexture(path, data, spec);
	return Texture::Create(spec, data.data(), data.size());
}

template<typename WndType, typename ...Args>
void AddWindow(Args... args)
{
	WndType* wnd = new WndType(args...);
;	sInsertQueue.push_back(wnd);
}

void RemoveWindow(EditorWindow* wnd)
{
	check(wnd);
	sRemoveQueue.push_back(wnd);
}

void UpdateWindows()
{
	// update the windows
	for (EditorWindow* wnd : sWindows)
	{
		wnd->RenderWindow();
		if (wnd->ShouldClose())
		{
			sRemoveQueue.push_back(wnd);
		}
	}
	
	// delete windows that should be deleted
	for (EditorWindow* wnd : sRemoveQueue)
	{
		delete wnd;
		sWindows.erase(std::find(sWindows.begin(), sWindows.end(), wnd));
	}
	sRemoveQueue.clear();

	// add the windows that should be added
	for (EditorWindow* wnd : sInsertQueue)
	{
		sWindows.push_back(wnd);
	}
	sInsertQueue.clear();
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

	const auto& reg = AssetManager::GetRegistry();

	AssetHandle modelHandle, shaderHandle, textureHandle;

	for (const auto& asset : reg)
		if (asset.type == ASSET_TYPE_MESH)
			modelHandle = asset;
		else if (asset.type == ASSET_TYPE_SHADER)
			shaderHandle = asset;
		else if (asset.type == ASSET_TYPE_TEXTURE)
			textureHandle = asset;

	testShader = (Shader*)shaderHandle.instance;
	testShader->Bind();

	testMesh = (Mesh*)modelHandle.instance;

	testTexture = (Texture*)textureHandle.instance;
	testTexture->Bind(0);

	Material* m;
	AssetSerializer<Material>::Serialize(m, Buffer());
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
		for (const auto& asset : AssetManager::GetRegistry())
		{
			std::string fileName = FileSystem::GetFilenameNoExt(asset.name);
			if (ImGui::ImageButton(asset.name.c_str(), sAssetIcon->GetRendererID(), { iconSize, iconSize }, { 0,0 }, { 1,1 }, { 1,1,1,1 }))
			{
				/*switch (asset.type)
				{
					case ASSET_TYPE_MATERIAL:
						AddWindow<EditMaterialWindow>("Edit material: " + fileName, asset);
						break;
				}*/
			}
			if (ImGui::BeginPopupContextItem(std::to_string(asset.uuid).c_str(), ImGuiPopupFlags_MouseButtonRight))
			{
				ImGui::SameLine();
				ImGui::SeparatorText("Actions");

				if (ImGui::Selectable("Delete"))
				{
					auto res = SysMessageBox::Show
					(
						"Asset deletion warning", 
						"Deleting an asset that is references by other asset could result in a crash, do it at your own risk muhahah",
						MESSAGE_BOX_TYPE_WARNING,
						MESSAGE_BOX_OPTIONS_YESNOCANCEL
					);
					if (res == MESSAGE_BOX_RESPONSE_YES)
					{
						AssetManager::EditorDelete(asset);
						AssetManager::SaveRegistry();
					}
				}

				/*if (asset.type == ASSET_TYPE_SHADER)
				{
					if (ImGui::Selectable("New material"))
					{
						AddWindow<EditMaterialWindow>("New material", asset);
					}
				}*/
				ImGui::EndPopup();
			}

			switch (asset.type)
			{
				case ASSET_TYPE_TEXTURE:	fileName += " (TEXTURE)"; break;
				case ASSET_TYPE_SHADER:		fileName += " (SHADER)"; break;
				case ASSET_TYPE_MATERIAL:	fileName += " (MATERIAL)"; break;
				case ASSET_TYPE_MESH:		fileName += " (MESH)"; break;
				default:					fileName += " (UNKNOWN?!?)"; break;
			}
			ImGui::TextWrapped(fileName.c_str());
			ImGui::NextColumn();
		}

		ImGui::Columns(1);

		ImGui::Separator();
		ImGui::SetNextItemWidth(200.0f);
		ImGui::SliderFloat("Zoom level", &zoom, 0.5f, 2.0f);

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
		ImGui::Text("Window size: %ix%i", Window::Get()->GetProps().width, Window::Get()->GetProps().height);
		ImGui::DragFloat3("cam loc", &camLoc.x, 0.05f);
		ImGui::DragFloat3("cam rot", &camRot.x, 0.05f);
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

		sGameViewportFrameBuffer->Bind();
		Renderer::RenderMesh(testMesh);

		// draw framebuffer
		ImGui::Image(sGameViewportFrameBuffer->GetRendererID(0), wndSize);
		
		// on screen text
		ImGui::SetCursorPosY( ImGui::GetWindowSize().y - wndSize.y + 8);
		ImGui::SetCursorPosX(8);
		ImGui::Text("Framebuffer %ix%i", sGameViewportFrameBuffer->GetSpecs().width, sGameViewportFrameBuffer->GetSpecs().height);
		
		ImGui::End();
		ImGui::PopStyleVar();
	}
}

void EditorUpdate(float deltaTime)
{
	Mat4 mvp;

	const auto& specs = sGameViewportFrameBuffer->GetSpecs();
	float aspectRatio = (float)specs.width / specs.height;

	mvp = math::GetModelMatrix({ 0,0,0 }, { 0,180,0 }, {1,1,1})
		*
		math::GetViewMatrix(camLoc, camRot)
		*
		math::GetPerspectiveMatrix(aspectRatio, camFOV, 0.1f, 1000.0f);

	mvp = math::GetMatrixTransposed(mvp);

	testShader->OverwriteVertexConstBuffer(0, &mvp);
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
			if (ImGui::MenuItem("Save"))
			{
				SaveWork();
			}

            ImGui::EndMenu();
        }

		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Asset registry"))
			{
				AddWindow<AssetRegistryWindow>("Asset Registry");
			}

			ImGui::EndMenu();
		}

        ImGui::EndMenuBar();
    }

	panels::Assets();
	panels::GameViewport();
	panels::EntityList();
	panels::Properties();
	UpdateWindows();

	ImGui::End();
}

void EditorShutdown()
{
	delete sAssetIcon;
	delete sGameViewportFrameBuffer;
}

