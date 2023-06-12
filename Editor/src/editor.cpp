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
#include "project/project.h"

#include "editor_window.h"

#include "renderer/renderer.h"

#include "math/matrix.h"

#include "assets/asset_serializer.h"

static ImGuiRenderer* sRenderer = nullptr;
extern ClearColor gScreenClearColor;
extern Window* gWindow;
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
	gWindow->SetTitle(gProj.name);

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
		std::string newFileName = gProj.assetsPath + FileSystem::GetFilenameNoExt(file) + GROOVY_ASSET_EXT;
		EAssetType assetType = AssetImporter::GetTypeFromFilename(file);

		switch (assetType)
		{
			case ASSET_TYPE_TEXTURE:	
				AssetImporter::ImportTexture(file, newFileName);
				AssetManager::SaveRegistry();
				break;

			case ASSET_TYPE_MESH:
				AssetImporter::ImportMesh(file, newFileName);
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

#include "classes/object_serializer.h"
#include "classes/class_db.h"
extern ClassDB gClassDB;

GROOVY_CLASS_DECL(TestClassBase)
class TestClassBase : public GroovyObject
{
	GROOVY_CLASS_BODY(TestClassBase, GroovyObject)

public:
	std::vector<std::string> strs = { "arinza" ,"arunza", "stappi", "sdunza" };
	uint64 ids2[3] = { 3, 2, 1 };
	Vec3 pos = { 0.5f, 0.6f, 0.7f };
};

GROOVY_CLASS_IMPL(TestClassBase, GroovyObject)
GROOVY_CLASS_REFLECTION_BEGIN(TestClassBase)
	GROOVY_REFLECT(strs)
	GROOVY_REFLECT(pos)
	GROOVY_REFLECT(ids2)
GROOVY_CLASS_REFLECTION_END()

GROOVY_CLASS_DECL(TestClass)
class TestClass : public TestClassBase
{
	GROOVY_CLASS_BODY(TestClass, TestClassBase)

public:
	int32 intVar = 5;
	std::string strVar = "mhanz";
	std::vector<uint64> ids = { 1, 2, 3 };
	std::string strs2[4] = { "sdunza", "stappi", "arunza", "arinza" };
};

GROOVY_CLASS_IMPL(TestClass, TestClassBase)
GROOVY_CLASS_REFLECTION_BEGIN(TestClass)
	GROOVY_REFLECT_EX(intVar, PROPERTY_EDITOR_FLAG_NOSERIALIZE)
	GROOVY_REFLECT(ids)
	GROOVY_REFLECT(strVar)
	GROOVY_REFLECT(strs2)
GROOVY_CLASS_REFLECTION_END()

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

	AssetHandle modelHandle;

	for (const auto& asset : reg)
		if (asset.type == ASSET_TYPE_MESH)
			modelHandle = asset;

	testMesh = (Mesh*)modelHandle.instance;
	testShader = (Shader*)testMesh->GetMaterials()[0]->GetShader();

	gClassDB.Register(&__internal_groovyclass_TestClass);

	TestClass test;
	test.intVar = -1;
	test.ids = { 4,5,6,7,8,9,10 };
	test.strVar = "strVar";
	test.strs = { "gg", "GG" };
	test.strs2[0] = "ss1";
	test.strs2[1] = "ss2";
	test.strs2[2] = "ss3";
	test.strs2[3] = "ss4";
	test.pos = { 1,2,4 };
	test.ids2[0] = 98;
	test.ids2[1] = 99;
	test.ids2[2] = 100;
	test.pos = { 2,3,4 };

	PropertyPack propertyPack = {};

	ObjectSerializer::CreatePropertyPack(propertyPack, &test, (GroovyObject*)test.GetClass()->cdo);

	TestClass tCopy;

	ObjectSerializer::DeserializeOntoObject(propertyPack.desc, propertyPack.data.data(), &tCopy);

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
		for (const auto& asset : AssetManager::GetRegistry())
		{
			std::string fileName = FileSystem::GetFilenameNoExt(asset.name);
			if (ImGui::ImageButton(asset.name.c_str(), sAssetIcon->GetRendererID(), { iconSize, iconSize }, { 0,0 }, { 1,1 }, { 1,1,1,1 }))
			{
				switch (asset.type)
				{
					case ASSET_TYPE_TEXTURE:
						AddWindow<TexturePreviewWindow>((Texture*)asset.instance);
						break;

					case ASSET_TYPE_MATERIAL:
						AddWindow<EditMaterialWindow>((Material*)asset.instance);
						break;

					case ASSET_TYPE_MESH:
						AddWindow<MeshPreviewWindow>((Mesh*)asset.instance);
						break;
				}
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
				SaveWork();

            ImGui::EndMenu();
        }

		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Asset registry"))
				AddWindow<AssetRegistryWindow>();

			if (ImGui::MenuItem("Class registry"))
				AddWindow<ClassRegistryWindow>();

			if (ImGui::MenuItem("Class inspector"))
				AddWindow<ClassInspectorWindow>();

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("New asset"))
		{
			if (ImGui::MenuItem("New material"))
				AddWindow<EditMaterialWindow>(nullptr);

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

