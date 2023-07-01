#include "engine/application.h"
#include "platform/platform.h"
#include "project/project.h"

#include "assets/assets.h"
#include "asset_importer.h"

#include "renderer/api/renderer_api.h"
#include "renderer/api/framebuffer.h"
#include "renderer/api/texture.h"
#include "renderer/api/shader.h"
#include "renderer/renderer.h"
#include "renderer/material.h"

#include "math/matrix.h"

#include "vendor/imgui/imgui.h"
#include "vendor/imgui/backends/imgui_impl_win32.h"
#include "vendor/imgui/backends/imgui_impl_dx11.h"
#include "vendor/imgui/misc/cpp/imgui_stdlib.h"
#include "imgui_renderer/imgui_renderer.h"
#include "editor_window.h"

static ImGuiRenderer* sRenderer = nullptr;
extern ClearColor gScreenClearColor;
extern Window* gWindow;
extern bool gEngineShouldRun;
extern Project gProj;

static std::vector<AssetHandle> sPendingSaveAssets;
static bool sPendingSaveRegistry = false;

inline bool AssetsPendingSave() { return sPendingSaveAssets.size(); }

void FlagAssetPendingSave(const AssetHandle& handle)
{
	if (std::find_if(sPendingSaveAssets.begin(), sPendingSaveAssets.end(), [=](AssetHandle& h) { return h.uuid == handle.uuid; }) == sPendingSaveAssets.end())
		sPendingSaveAssets.push_back(handle);
}

void SaveAssets()
{
	for (AssetHandle& handle : sPendingSaveAssets)
		handle.instance->Save();
	sPendingSaveAssets.clear();
}

void SaveAssetRegistry()
{
	AssetManager::SaveRegistry();
	sPendingSaveRegistry = false;
}

void EditorInit();
void EditorUpdate(float deltaTime);
void EditorRender();
void EditorShutdown();

bool OnCloseRequested();
void OnFilesDropped(const std::vector<std::string>& files);

void Application::Init()
{
	gWindow->SubmitToWndCloseCallback(OnCloseRequested);
	gWindow->SubmitToWndFilesDropCallbacks(OnFilesDropped);

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
		std::string newFileName = std::filesystem::path(file).replace_extension(GROOVY_ASSET_EXT).filename().string();
		EAssetType assetType = AssetImporter::GetTypeFromFilename(file);

		switch (assetType)
		{
			case ASSET_TYPE_TEXTURE:	
				AssetImporter::ImportTexture(file, newFileName);
				sPendingSaveRegistry = true;
				break;

			case ASSET_TYPE_MESH:
				AssetImporter::ImportMesh(file, newFileName);
				sPendingSaveRegistry = true;
				break;
		}
	}
}

void SaveWork()
{
	if (AssetsPendingSave())
		SaveAssets();

	if (sPendingSaveRegistry)
		SaveAssetRegistry();
}

bool OnCloseRequested()
{
	if (sPendingSaveRegistry || AssetsPendingSave())
	{
		auto response = SysMessageBox::Show
		(
			"Exit & Save",
			"You are exiting without saving, do you want to save and exit?",
			MESSAGE_BOX_TYPE_WARNING,
			MESSAGE_BOX_OPTIONS_YESNOCANCEL
		);

		if (response == MESSAGE_BOX_RESPONSE_YES)
		{
			SaveWork();
			return true;
		}
		else if (response == MESSAGE_BOX_RESPONSE_NO)
		{
			return true;
		}
		return false;
	}
	return true;
}

static std::vector<EditorWindow*> sWindows;
static std::vector<EditorWindow*> sInsertQueue;
static std::vector<EditorWindow*> sRemoveQueue;

static Texture* sShaderAssetIcon;
static Texture* sMaterialAssetIcon;
static Texture* sMeshAssetIcon;
static Texture* sClassAssetIcon;

static FrameBuffer* sGameViewportFrameBuffer;
static ImVec2 sGameViewportSize;

static Mesh* testMesh;
static Vec3 camLoc = {0,1.0f,-3};
static Vec3 camRot = {0,0,0};
static float camFOV = 60;

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

extern std::vector<GroovyClass*> ENGINE_CLASSES;
extern std::vector<GroovyClass*> GAME_CLASSES;
static float sIconSize = 140.0f;
static const float ICON_SIZE_MAX = 256;
static const float ICON_SIZE_MIN = 100;

void EditorInit()
{
	// assets used by the editor
	sShaderAssetIcon = LoadEditorIcon("res/shader_asset_icon.png");
	sMaterialAssetIcon = LoadEditorIcon("res/material_asset_icon.png");
	sMeshAssetIcon = LoadEditorIcon("res/mesh_asset_icon.png");
	sClassAssetIcon = LoadEditorIcon("res/class_asset_icon.png");
	
	// gameviewport framebuffer
	FrameBufferSpec gameViewportSpec;
	gameViewportSpec.colorAttachments = { COLOR_FORMAT_R8G8B8A8_UNORM };
	gameViewportSpec.hasDepthAttachment = true;
	gameViewportSpec.swapchainTarget = false;
	gameViewportSpec.width = sGameViewportSize.x = 100;
	gameViewportSpec.height = sGameViewportSize.y = 100;

	sGameViewportFrameBuffer = FrameBuffer::Create(gameViewportSpec);

	testMesh = (Mesh*)AssetManager::Editor_GetAssets(ASSET_TYPE_MESH)[0].instance;
	testMesh->FixForRendering();
}

namespace panels
{
	void Assets()
	{
		static const const char* TYPES_STR[] =
		{
			"TEXTURE",
			"SHADER",
			"MATERIAL",
			"MESH"
		};

		enum PanelAssetFlags
		{
			PANEL_ASSET_FLAG_IS_DEFAULT = BITFLAG(1)
		};

		struct PanelAsset
		{
			ImTextureID thumbnail;
			uint32 typeNameIndex;
			uint32 flags;
		};

		std::vector<AssetHandle> assets = AssetManager::Editor_GetAssets();
		std::vector<PanelAsset> panelAssets;
		panelAssets.resize(assets.size());
		for (uint32 i = 0; i < assets.size(); i++)
		{
			switch (assets[i].type)
			{
				case ASSET_TYPE_TEXTURE:
					panelAssets[i].thumbnail = ((Texture*)assets[i].instance)->GetRendererID();
					panelAssets[i].typeNameIndex = 0;
					break;
				case ASSET_TYPE_SHADER:
					panelAssets[i].thumbnail = sShaderAssetIcon->GetRendererID();
					panelAssets[i].typeNameIndex = 1;
					break;
				case ASSET_TYPE_MATERIAL:
					panelAssets[i].thumbnail = sMaterialAssetIcon->GetRendererID();
					panelAssets[i].typeNameIndex = 2;
					break;
				case ASSET_TYPE_MESH:
					panelAssets[i].thumbnail = sMeshAssetIcon->GetRendererID();
					panelAssets[i].typeNameIndex = 3;
					break;
			}

			if (assets[i].uuid <= 3)
				panelAssets[i].flags |= PANEL_ASSET_FLAG_IS_DEFAULT;
		}

		ImGui::Begin("Asset manager");

		// stolen ui code that works pretty well

		float contentWidth = ImGui::GetContentRegionAvail().x;
		float contentHeight = ImGui::GetContentRegionAvail().y;
		int columns = contentWidth / sIconSize;
		columns = columns < 1 ? 1 : columns;

		// end stolen ui code

		ImGui::Spacing();
		ImGui::SetNextItemWidth(200);
		ImGui::SliderFloat("Item size", &sIconSize, ICON_SIZE_MIN, ICON_SIZE_MAX);
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Columns(columns, nullptr, false);

		for (uint32 i = 0; i < assets.size(); ++i)
		{
			AssetHandle& asset = assets[i];
			PanelAsset panelAsset = panelAssets[i];

			ImGui::PushID(i);

			ImGui::BeginGroup();

			std::string fileNameNoExt = std::filesystem::path(asset.name).replace_extension().string();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {3,3});

			if (ImGui::ImageButton(fileNameNoExt.c_str(), panelAsset.thumbnail, { sIconSize - 10, sIconSize - 5 }, {0,0}, {1,1}, {1,1,1,1}))
			{
				switch (asset.type)
				{
					case ASSET_TYPE_TEXTURE:
						AddWindow<TexturePreviewWindow>((Texture*)asset.instance);
						break;

					case ASSET_TYPE_SHADER:
						break;

					case ASSET_TYPE_MATERIAL:
						AddWindow<EditMaterialWindow>((Material*)asset.instance);
						break;

					case ASSET_TYPE_MESH:
						AddWindow<MeshPreviewWindow>((Mesh*)asset.instance);
						break;
				}
			}

			ImGui::PopStyleVar();

			if (ImGui::BeginPopupContextItem(fileNameNoExt.c_str(), ImGuiPopupFlags_MouseButtonRight))
			{
				ImGui::SeparatorText("Actions");
				// delete asset
				if (!(panelAsset.flags & PANEL_ASSET_FLAG_IS_DEFAULT))
				{
					if (ImGui::Selectable("Delete"))
					{
						auto res = SysMessageBox::Show
						(
							"Asset deletion",
							"Deleting an asset is dangerous, it could break dependencies with other assets, do you want to continue?",
							MESSAGE_BOX_TYPE_WARNING, MESSAGE_BOX_OPTIONS_YESNOCANCEL
						);
						if (res == MESSAGE_BOX_RESPONSE_YES)
						{
							AssetManager::Editor_Delete(asset.uuid, sPendingSaveAssets);
							sPendingSaveRegistry = true;
						}
					}
				}

				switch (asset.type)
				{
					case ASSET_TYPE_SHADER:
					{
						if (ImGui::Selectable("Create material"))
							AddWindow<EditMaterialWindow>((Shader*)asset.instance);
					}
					break;
				}

				ImGui::EndPopup();
			}
			else if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("File: %s" "\n" "Type: %s", asset.name.c_str(), TYPES_STR[panelAsset.typeNameIndex]);
			}
			ImGui::Text(fileNameNoExt.c_str());
			

			
			ImGui::EndGroup();
			
			ImGui::PopID();
			ImGui::NextColumn();
		}

		ImGui::End();
	}

	void Classes()
	{
		ImGui::Begin("C++ classes");

		static bool showEngineClasses = true;
		static bool showGameClasses = true;
		
		std::vector<GroovyClass*> classes;
		
		if (showEngineClasses)
			for (GroovyClass* c : ENGINE_CLASSES)
				classes.push_back(c);

		/*if (showGameClasses)
			for (GroovyClass* c : GAME_CLASSES)
				classes.push_back(c);*/

		// stolen ui code that works pretty well

		float contentWidth = ImGui::GetContentRegionAvail().x;
		float contentHeight = ImGui::GetContentRegionAvail().y;
		int columns = contentWidth / sIconSize;
		columns = columns < 1 ? 1 : columns;

		// end stolen ui code

		ImGui::Spacing();
		ImGui::SetNextItemWidth(200);
		ImGui::SliderFloat("Item size", &sIconSize, ICON_SIZE_MIN, ICON_SIZE_MAX);
		ImGui::Checkbox("Show engine classes", &showEngineClasses);
		ImGui::SameLine();
		ImGui::Checkbox("Show game classes", &showGameClasses);
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Columns(columns, nullptr, false);

		for (size_t i = 0; i < classes.size(); ++i)
		{
			GroovyClass* gClass = classes[i];

			ImGui::PushID(i);

			ImGui::BeginGroup();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 3,3 });

			ImGui::ImageButton(gClass->name.c_str(), sClassAssetIcon->GetRendererID(), { sIconSize - 10, sIconSize - 5}, {0,0}, {1,1}, {1,1,1,1});

			ImGui::PopStyleVar();

			if (ImGui::BeginPopupContextItem(gClass->name.c_str(), ImGuiPopupFlags_MouseButtonRight))
			{
				// delete asset
				if (ImGui::Selectable("Create blueprint"))
				{
					AddWindow<BlueprintEditorWindow>(gClass);
				}

				ImGui::EndPopup();
			}
			else if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip
				(
					"Class: %s" "\n" "Size: %i bytes" "\n" "Super class: %s",
					gClass->name.c_str(), gClass->size, gClass->super ? gClass->super->name.c_str() : "NONE"
				);
			}
			ImGui::Text(gClass->name.c_str());

			ImGui::EndGroup();

			ImGui::PopID();
			ImGui::NextColumn();
		}

		ImGui::End();
	}

	void EntityList()
	{
		ImGui::Begin("Entity list");
		ImGui::End();
	}

	void Properties()
	{
		ImGui::Begin("Properties");
		ImGui::Text("Window size: %ix%i", gWindow->GetProps().width, gWindow->GetProps().height);
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
		sGameViewportFrameBuffer->ClearColorAttachment(0, gScreenClearColor);

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

	Renderer::BeginScene(mvp);
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
				SaveWork();

            ImGui::EndMenu();
        }

		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Asset registry"))
				AddWindow<AssetRegistryWindow>();

			ImGui::EndMenu();
		}

        ImGui::EndMenuBar();
    }

	panels::Assets();
	panels::Classes();
	panels::GameViewport();
	panels::EntityList();
	panels::Properties();
	UpdateWindows();

	ImGui::End();
}

void EditorShutdown()
{
	delete sClassAssetIcon;
	delete sShaderAssetIcon;
	delete sMaterialAssetIcon;
	delete sMeshAssetIcon;
	delete sGameViewportFrameBuffer;
}

