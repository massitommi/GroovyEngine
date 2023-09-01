#include "editor.h"

#include "engine/application.h"

#include "vendor/imgui/imgui.h"
#include "vendor/imgui/backends/imgui_impl_win32.h"
#include "vendor/imgui/backends/imgui_impl_dx11.h"
#include "vendor/imgui/misc/cpp/imgui_stdlib.h"

#include "editor_window.h"

#include "imgui_renderer/imgui_renderer.h"

#include "platform/platform.h"
#include "project/project.h"
#include "assets/assets.h"
#include "asset_importer.h"

#include "gameframework/actor.h"
#include "gameframework/actors/meshactor.h"
#include "gameframework/actors/spectator.h"
#include "gameframework/actorcomponent.h"
#include "gameframework/blueprint.h"
#include "gameframework/scene.h"
#include "gameframework/components/cameracomponent.h"
#include "gameframework/components/meshcomponent.h"
#include "gameframework/blueprint.h"

#include "classes/class_db.h"

#include "renderer/api/renderer_api.h"
#include "renderer/api/framebuffer.h"
#include "renderer/api/texture.h"
#include "renderer/api/shader.h"
#include "renderer/renderer.h"
#include "renderer/material.h"
#include "renderer/scene_renderer.h"

#include "math/math.h"

#include "utils/string/string_utils.h"

void EditorSettings::Load()
{
	Buffer settingsFile;
	FileSystem::ReadFileBinary(EDITOR_SETTINGS_FILE, settingsFile);
	if (settingsFile.size())
	{
		PropertyPack pack;
		BufferView settingsFileView(settingsFile);
		ObjectSerializer::DeserializePropertyPack(EditorSettings::StaticClass(), settingsFileView, pack);
		ObjectSerializer::DeserializePropertyPackData(pack, this);
	}
}

void EditorSettings::Save()
{
	PropertyPack pack;
	ObjectSerializer::CreatePropertyPack(this, EditorSettings::StaticCDO(), pack);
	DynamicBuffer settingsFile;
	ObjectSerializer::SerializePropertyPack(pack, settingsFile);
	FileSystem::WriteFileBinary(EDITOR_SETTINGS_FILE, settingsFile);
}

GROOVY_CLASS_IMPL(EditorSettings)
	GROOVY_REFLECT(mCameraFOV)
	GROOVY_REFLECT(mCameraMoveSpeed)
	GROOVY_REFLECT(mCameraRotationSpeed)
	GROOVY_REFLECT(mContentBrowserIconSize)
	GROOVY_REFLECT(mClassBrowserShowEngineClasses)
	GROOVY_REFLECT(mClassBrowserShowGameClasses)
GROOVY_CLASS_END()

struct EditorScene
{
	Scene* scene = nullptr;
	Actor* selectedActor = nullptr;
	GroovyObject* selectedSubobject = nullptr;
};

enum EEditorSceneState
{
	EDITOR_SCENE_STATE_EDIT,
	EDITOR_SCENE_STATE_PLAY
};

struct EditorCamera
{
	Vec3 location = { 0.0f, 0.0f, -3.0f };
	Vec3 rotation = { 0.0f, 0.0f, 0.0f };
};

static FrameBuffer* sGameViewportFrameBuffer = nullptr;
static bool sGameViewportFocused = false;
static bool sGameVieportHovered = false;
static std::string sEditSceneName;
static bool sEditScenePendingSave = false;
static EditorCamera sEditorCamera;
static EditorScene sEditScene;
static EditorScene sPlayScene;
static EEditorSceneState sEditorSceneState = EDITOR_SCENE_STATE_EDIT;
static EditorScene* sCurrentScene = nullptr;

extern ImGuiRenderer* gGroovyGuiRenderer;

EditorSettings gEditorSettings;

namespace res
{
	static Texture* sShaderAssetIcon = nullptr;
	static Texture* sMaterialAssetIcon = nullptr;
	static Texture* sMeshAssetIcon = nullptr;
	static Texture* sClassAssetIcon = nullptr;
	static Texture* sBlueprintAssetIcon = nullptr;
	static Texture* sSceneAssetIcon = nullptr;
	static Texture* sPlayBtnIcon = nullptr;
	static Texture* sPauseBtnIcon = nullptr;
	static Texture* sStopBtnIcon = nullptr;

	Texture* LoadEditorIcon(const std::string& path)
	{
		Buffer data;
		TextureSpec spec;
		AssetImporter::GetRawTexture(path, data, spec);
		return Texture::Create(spec, data.data(), data.size());
	}

	void Load()
	{
		sShaderAssetIcon = LoadEditorIcon("res/shader_asset_icon.png");
		sMaterialAssetIcon = LoadEditorIcon("res/material_asset_icon.png");
		sMeshAssetIcon = LoadEditorIcon("res/mesh_asset_icon.png");
		sClassAssetIcon = LoadEditorIcon("res/class_asset_icon.png");
		sBlueprintAssetIcon = LoadEditorIcon("res/blueprint_asset_icon.png");
		sSceneAssetIcon = LoadEditorIcon("res/scene_asset_icon.png");
		sPlayBtnIcon = LoadEditorIcon("res/play_btn_icon.png");
		sPauseBtnIcon = LoadEditorIcon("res/pause_btn_icon.png");
		sStopBtnIcon = LoadEditorIcon("res/stop_btn_icon.png");
	}

	void Unload()
	{
		delete sShaderAssetIcon;
		delete sMaterialAssetIcon;
		delete sMeshAssetIcon;
		delete sClassAssetIcon;
		delete sBlueprintAssetIcon;
		delete sSceneAssetIcon;
		delete sPlayBtnIcon;
		delete sPauseBtnIcon;
		delete sStopBtnIcon;
	}
}

namespace windows
{
	static std::vector<EditorWindow*> sWindows;
	static std::vector<EditorWindow*> sRemoveQueue;

	static std::map<std::string, EditorWindow*> sWndMap;

	template<typename WndType, typename ...Args>
	void AddWindow(const std::string& title, Args... args)
	{
		// check if window already exists
		EditorWindow*& wnd = sWndMap[title];
		
		if (!wnd)
		{
			wnd = new WndType(args...);
			wnd->SetTitle(title);
			sWindows.push_back(wnd);
		}
		else
		{
			ImGui::SetWindowFocus(title.c_str());
		}
	}

	void RemoveWindow(EditorWindow* wnd)
	{
		check(wnd);
		sRemoveQueue.push_back(wnd);
	}

	void UpdateWindows()
	{
		// remove pending destoy windows
		for (EditorWindow* wnd : sRemoveQueue)
		{
			sWndMap.erase(wnd->GetTitle());
			sWindows.erase(std::find(sWindows.begin(), sWindows.end(), wnd));
			delete wnd;
		}
		sRemoveQueue.clear();

		// update the windows
		for (EditorWindow* wnd : sWindows)
		{
			wnd->RenderWindow();
			if (wnd->ShouldClose())
			{
				sRemoveQueue.push_back(wnd);
			}
		}
	}

	void Shutdown()
	{
		for (EditorWindow* wnd : sWindows)
			delete wnd;
	}
}

void TravelToScene(Scene* scene)
{
	if (sEditScene.scene)
	{
		sEditScene.scene->Unload();
	}

	sEditScene.scene = scene;

	if (scene)
	{
		sEditSceneName = AssetManager::Get(scene->GetUUID()).name;
		sEditScene.scene->Load();
	}

	sEditScenePendingSave = false;
	sEditScene.selectedActor = nullptr;
	sEditScene.selectedSubobject = nullptr;
	sCurrentScene = &sEditScene;
}

void TryTravelToScene(Scene* scene)
{
	if (sEditorSceneState == EDITOR_SCENE_STATE_EDIT && sEditScene.scene != scene)
	{
		if (sEditScenePendingSave)
		{
			auto res = SysMessageBox::Show
			(
				"Changing scene without saving", "Do you want to save and travel to the new scene?",
				MESSAGE_BOX_TYPE_WARNING, MESSAGE_BOX_OPTIONS_YESNOCANCEL
			);

			if (res == MESSAGE_BOX_RESPONSE_YES)
			{
				sEditScene.scene->Save();
				TravelToScene(scene);
			}
			else if (res == MESSAGE_BOX_RESPONSE_NO)
			{
				TravelToScene(scene);
			}
		}
		else
		{
			TravelToScene(scene);
		}
	}
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
			break;

		case ASSET_TYPE_MESH:
			AssetImporter::ImportMesh(file, newFileName);
			break;
		}
	}
}

bool OnCloseRequested()
{
	if (!sEditScenePendingSave)
		return true;

	auto res = SysMessageBox::Show
	(
		"Quitting without saving", "Do you want to save and quit?",
		MESSAGE_BOX_TYPE_WARNING, MESSAGE_BOX_OPTIONS_YESNOCANCEL
	);

	if (res == MESSAGE_BOX_RESPONSE_YES)
	{
		sEditScene.scene->Save();
		return true;
	}
	else if (res == MESSAGE_BOX_RESPONSE_NO)
	{
		return true;
	}
	return false;
}

void Play()
{
	sEditScene.scene->Copy(sPlayScene.scene);
	sPlayScene.scene->BeginPlay();
	sCurrentScene = &sPlayScene;
}

void Pause()
{
}

void Stop()
{
	sPlayScene.selectedActor = nullptr;
	sPlayScene.selectedSubobject = nullptr;
	sPlayScene.scene->Clear();

	sCurrentScene = &sEditScene;
}

namespace assetPopups
{
	bool IsValidAssetName(const std::string& name)
	{
		return
			!name.empty() &&													// path is not empty
			name.find(' ') == std::string::npos &&								// path does not contain whitespaces
			!AssetManager::FindByPath(name + GROOVY_ASSET_EXT).instance &&		// there is no asset with the same path
			!AssetManager::FindByPath(name).instance;							// there is no default asset with the same path
	}

	bool AssetNameInput(std::string& filename)
	{
		bool validPath = IsValidAssetName(filename);

		if (!validPath)
			ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 0.7f, 0.0f, 1.0f });

		ImGui::InputText("##new_asset_name", &filename);

		if (!validPath)
			ImGui::PopStyleColor();

		ImGui::SameLine();
		ImGui::Text(GROOVY_ASSET_EXT);

		if (!validPath)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, { 0.8f, 0.0f, 0.0f, 1.0f });
			ImGui::Text("Filenames can't contain whitespaces, filenames are case-insensitive");
			ImGui::PopStyleColor();
		}
		else
		{
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetTextLineHeight());
			ImGui::Spacing();
		}

		return validPath;
	}

	enum EAssetAction
	{
		ASSET_ACTION_NONE,
		ASSET_ACTION_DELETE,
		ASSET_ACTION_RENAME
	};

	static EAssetAction sAssetAction = ASSET_ACTION_NONE;
	static AssetHandle sAssetActionHandle = {};
	static EAssetType sNewAssetType = ASSET_TYPE_NONE;
	static std::string sAssetRename;

	void RenderPopups()
	{
		// delete // rename
		if (sAssetAction != ASSET_ACTION_NONE)
		{
			switch (sAssetAction)
			{
				case ASSET_ACTION_DELETE:
					ImGui::OpenPopup("Delete asset");
					break;

				case ASSET_ACTION_RENAME:
					ImGui::OpenPopup("Rename asset");
					break;
			}

			if (ImGui::BeginPopupModal("Delete asset", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text("Are you sure you want to permanently delete \"%s\" ?", sAssetRename.c_str());
				bool popupShouldClose = false;

				ImGui::Spacing();
				ImGui::Spacing();

				if (ImGui::Button("Delete"))
				{
					switch (sAssetActionHandle.type)
					{
						case ASSET_TYPE_SCENE:
						{
							if (sAssetActionHandle.instance == sEditScene.scene)
								TravelToScene(nullptr);

							if (sAssetActionHandle.instance == gProj.GetStartupScene())
								gProj.SetStartupScene(nullptr);
						}
						break;

						case ASSET_TYPE_ACTOR_BLUEPRINT:
						{
							if (sEditScene.scene && sEditScene.scene->ReferencesBlueprint((ActorBlueprint*)sAssetActionHandle.instance))
								sEditScenePendingSave = true;
						}
						break;
					}

					AssetManager::Editor_Remove(sAssetActionHandle);

					FileSystem::DeleteFile((gProj.GetAssetsPath() / sAssetActionHandle.name).string());

					popupShouldClose = true;
				}

				if (ImGui::IsKeyPressed(ImGuiKey_Escape))
					popupShouldClose = true;

				if (popupShouldClose)
				{
					ImGui::CloseCurrentPopup();
					sAssetAction = ASSET_ACTION_NONE;
					sAssetActionHandle = {};
					sAssetRename = "";
				}

				ImGui::EndPopup();
			}

			if (ImGui::BeginPopupModal("Rename asset", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			{
				bool validName = AssetNameInput(sAssetRename);
				bool popupShouldClose = false;

				ImGui::Spacing();
				ImGui::Spacing();

				if (!validName)
					ImGui::BeginDisabled();

				if (ImGui::Button("Rename"))
				{
					AssetManager::Editor_Rename(sAssetActionHandle, sAssetRename + GROOVY_ASSET_EXT);
					FileSystem::Rename((gProj.GetAssetsPath() / sAssetActionHandle.name).string(), (gProj.GetAssetsPath() / (sAssetRename + GROOVY_ASSET_EXT)).string());

					popupShouldClose = true;
				}

				if (ImGui::IsKeyPressed(ImGuiKey_Escape))
					popupShouldClose = true;

				if (popupShouldClose)
				{
					ImGui::CloseCurrentPopup();
					sAssetAction = ASSET_ACTION_NONE;
					sAssetActionHandle = {};
					sAssetRename = "";
				}

				if (!validName)
					ImGui::EndDisabled();

				ImGui::EndPopup();
			}
		}
		// create
		else if (sNewAssetType != ASSET_TYPE_NONE)
		{
			switch (sNewAssetType)
			{
				case ASSET_TYPE_MATERIAL:
					ImGui::OpenPopup("New material");
					break;

				case ASSET_TYPE_BLUEPRINT:
					ImGui::OpenPopup("New blueprint");
					break;

				case ASSET_TYPE_ACTOR_BLUEPRINT:
					ImGui::OpenPopup("New actor blueprint");
					break;

				case ASSET_TYPE_SCENE:
					ImGui::OpenPopup("New scene");
					break;
			}

			if (ImGui::BeginPopupModal("New material"))
			{
				bool validName = AssetNameInput(sAssetRename);
				bool popupShouldClose = false;

				if (!validName)
					ImGui::BeginDisabled();

				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Text("Select a shader program:");
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();

				for (AssetHandle& asset : AssetManager::GetAssets(ASSET_TYPE_SHADER))
				{
					if (ImGui::Selectable(asset.name.c_str()))
					{
						// create asset
						Material* newMat = new Material();
						newMat->SetShader((Shader*)asset.instance);

						// notify asset manager and save
						AssetManager::Editor_Add(sAssetRename + GROOVY_ASSET_EXT, ASSET_TYPE_MATERIAL, newMat);
						newMat->Save();

						// reset popup
						popupShouldClose = true;
					}
					ImGui::Spacing();
				}

				if (ImGui::IsKeyPressed(ImGuiKey_Escape))
					popupShouldClose = true;

				if (popupShouldClose)
				{
					ImGui::CloseCurrentPopup();
					sNewAssetType = ASSET_TYPE_NONE;
					sAssetRename = "";
				}

				if (!validName)
					ImGui::EndDisabled();

				ImGui::EndPopup();
			}

			if (ImGui::BeginPopupModal("New blueprint"))
			{
				bool validName = AssetNameInput(sAssetRename);
				bool popupShouldClose = false;

				if (!validName)
					ImGui::BeginDisabled();

				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Text("Select a groovy class:");
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();

				for (const GroovyClass* gClass : gClassDB.GetClasses())
				{
					if (!GroovyClass_IsA(gClass, Actor::StaticClass()) && ImGui::Selectable(gClass->name.c_str()))
					{
						// create asset
						ObjectBlueprint* newBP = new ObjectBlueprint();
						newBP->SetupEmpty((GroovyClass*)gClass);

						// notify asset manager and save
						AssetManager::Editor_Add(sAssetRename + GROOVY_ASSET_EXT, ASSET_TYPE_BLUEPRINT, newBP);
						newBP->Save();

						// reset popup
						popupShouldClose = true;

						ImGui::Spacing();
					}
				}

				if (ImGui::IsKeyPressed(ImGuiKey_Escape))
					popupShouldClose = true;

				if (popupShouldClose)
				{
					ImGui::CloseCurrentPopup();
					sNewAssetType = ASSET_TYPE_NONE;
					sAssetRename = "";
				}

				if (!validName)
					ImGui::EndDisabled();

				ImGui::EndPopup();
			}

			if (ImGui::BeginPopupModal("New actor blueprint"))
			{
				bool validName = AssetNameInput(sAssetRename);
				bool popupShouldClose = false;

				if (!validName)
					ImGui::BeginDisabled();

				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Text("Select an actor class:");
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();


				for (const GroovyClass* gClass : gClassDB.GetClasses())
				{
					if (GroovyClass_IsA(gClass, Actor::StaticClass()) && ImGui::Selectable(gClass->name.c_str()))
					{
						// create asset
						ActorBlueprint* newBP = new ActorBlueprint();
						newBP->SetupEmpty((GroovyClass*)gClass);

						// notify asset manager and save
						AssetManager::Editor_Add(sAssetRename + GROOVY_ASSET_EXT, ASSET_TYPE_ACTOR_BLUEPRINT, newBP);
						newBP->Save();

						// reset popup
						popupShouldClose = true;

						ImGui::Spacing();
					}
				}

				if (ImGui::IsKeyPressed(ImGuiKey_Escape))
					popupShouldClose = true;

				if (popupShouldClose)
				{
					ImGui::CloseCurrentPopup();
					sNewAssetType = ASSET_TYPE_NONE;
					sAssetRename = "";
				}

				if (!validName)
					ImGui::EndDisabled();

				ImGui::EndPopup();
			}

			if (ImGui::BeginPopupModal("New scene"))
			{
				bool validName = AssetNameInput(sAssetRename);
				bool popupShouldClose = false;

				if (!validName)
					ImGui::BeginDisabled();

				ImGui::Spacing();
				ImGui::Spacing();

				if (ImGui::Button("Create"))
				{
					// create asset
					Scene* newScene = new Scene();

					// notify asset manager and save
					AssetManager::Editor_Add(sAssetRename + GROOVY_ASSET_EXT, ASSET_TYPE_SCENE, newScene);
					newScene->Save();

					// reset popup
					popupShouldClose = true;
				}

				if (ImGui::IsKeyPressed(ImGuiKey_Escape))
					popupShouldClose = true;

				if (popupShouldClose)
				{
					ImGui::CloseCurrentPopup();
					sNewAssetType = ASSET_TYPE_NONE;
					sAssetRename = "";
				}

				if (!validName)
					ImGui::EndDisabled();

				ImGui::EndPopup();
			}
		}
	}
}

namespace panels
{
	static Actor* sActorToRename = nullptr;
	static bool sShowActorRename = false;
	static std::string sActorRename;

	static const float ICON_SIZE_MAX = 256.0f;
	static const float ICON_SIZE_MIN = 100.0f;

	static const const char* TYPES_STR[] =
	{
		"TEXTURE",
		"SHADER",
		"MATERIAL",
		"MESH",
		"BLUEPRINT",
		"ACTOR BLUEPRINT",
		"SCENE"
	};

	void Assets()
	{
		struct PanelAsset
		{
			ImTextureID thumbnail;
			uint32 typeNameIndex;
		};

		const std::vector<AssetHandle>& assets = AssetManager::GetAssets();
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
				panelAssets[i].thumbnail = res::sShaderAssetIcon->GetRendererID();
				panelAssets[i].typeNameIndex = 1;
				break;
			case ASSET_TYPE_MATERIAL:
				panelAssets[i].thumbnail = res::sMaterialAssetIcon->GetRendererID();
				panelAssets[i].typeNameIndex = 2;
				break;
			case ASSET_TYPE_MESH:
				panelAssets[i].thumbnail = res::sMeshAssetIcon->GetRendererID();
				panelAssets[i].typeNameIndex = 3;
				break;
			case ASSET_TYPE_BLUEPRINT:
				panelAssets[i].thumbnail = res::sBlueprintAssetIcon->GetRendererID();
				panelAssets[i].typeNameIndex = 4;
				break;
			case ASSET_TYPE_ACTOR_BLUEPRINT:
				panelAssets[i].thumbnail = res::sBlueprintAssetIcon->GetRendererID();
				panelAssets[i].typeNameIndex = 5;
				break;

			case ASSET_TYPE_SCENE:
				panelAssets[i].thumbnail = res::sSceneAssetIcon->GetRendererID();
				panelAssets[i].typeNameIndex = 6;
				break;
			}
		}

		ImGui::Begin("Content browser");

		// stolen ui code that works pretty well

		float contentWidth = ImGui::GetContentRegionAvail().x;
		float contentHeight = ImGui::GetContentRegionAvail().y;
		int columns = contentWidth / gEditorSettings.mContentBrowserIconSize;
		columns = columns < 1 ? 1 : columns;

		// end stolen ui code

		ImGui::Spacing();
		ImGui::SetNextItemWidth(200);
		ImGui::SliderFloat("Item size", &gEditorSettings.mContentBrowserIconSize, ICON_SIZE_MIN, ICON_SIZE_MAX);
		ImGui::Spacing();
		ImGui::Spacing();

		if (ImGui::BeginPopupContextWindow("Asset manager", ImGuiPopupFlags_MouseButtonRight))
		{
			if (ImGui::BeginMenu("Create"))
			{
				if (ImGui::MenuItem("Material"))
				{
					assetPopups::sNewAssetType = ASSET_TYPE_MATERIAL;
					assetPopups::sAssetRename = "My_New_Material";
				}
				if (ImGui::MenuItem("Blueprint"))
				{
					assetPopups::sNewAssetType = ASSET_TYPE_BLUEPRINT;
					assetPopups::sAssetRename = "My_New_Blueprint";
				}
				if (ImGui::MenuItem("Actor blueprint"))
				{
					assetPopups::sNewAssetType = ASSET_TYPE_ACTOR_BLUEPRINT;
					assetPopups::sAssetRename = "My_New_Actor_Blueprint";
				}
				if (ImGui::MenuItem("Scene"))
				{
					assetPopups::sNewAssetType = ASSET_TYPE_SCENE;
					assetPopups::sAssetRename = "My_New_Scene";
				}

				ImGui::EndMenu();
			}

			ImGui::EndPopup();
		}

		ImGui::Columns(columns, nullptr, false);

		for (uint32 i = 0; i < assets.size(); i++)
		{
			const AssetHandle& asset = assets[i];
			PanelAsset panelAsset = panelAssets[i];

			ImGui::BeginGroup();

			std::string fileNameNoExt = std::filesystem::path(asset.name).replace_extension().string();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 3,3 });

			ImGui::ImageButton(fileNameNoExt.c_str(), panelAsset.thumbnail, { gEditorSettings.mContentBrowserIconSize - 10, gEditorSettings.mContentBrowserIconSize - 5 }, { 0,0 }, { 1,1 }, { 1,1,1,1 });
			if (ImGui::BeginDragDropSource())
			{
				ImGui::SetDragDropPayload("drag_and_drop_asset", &asset.uuid, sizeof(AssetUUID));
				ImGui::EndDragDropSource();
			}
			else if (ImGui::BeginPopupContextItem(fileNameNoExt.c_str(), ImGuiPopupFlags_MouseButtonRight))
			{
				// rename
				{
					bool canRename = asset.uuid > 4;

					if (!canRename)
						ImGui::BeginDisabled();

					if (ImGui::Selectable("Rename"))
					{
						assetPopups::sAssetAction = assetPopups::ASSET_ACTION_RENAME;
						assetPopups::sAssetActionHandle = asset;
						assetPopups::sAssetRename = fileNameNoExt;
					}

					if (!canRename)
						ImGui::EndDisabled();
				}
				// delete
				{
					bool canDelete = sEditorSceneState == EDITOR_SCENE_STATE_EDIT && asset.uuid > 4;

					if (!canDelete)
						ImGui::BeginDisabled();

					if (ImGui::Selectable("Delete"))
					{
						assetPopups::sAssetAction = assetPopups::ASSET_ACTION_DELETE;
						assetPopups::sAssetActionHandle = asset;
						assetPopups::sAssetRename = fileNameNoExt;
					}

					if (!canDelete)
						ImGui::EndDisabled();
				}

				ImGui::EndPopup();
			}
			else if (ImGui::IsItemActive() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (asset.uuid > 4)
				{
					switch (asset.type)
					{
						case ASSET_TYPE_TEXTURE:
							windows::AddWindow<TexturePreviewWindow>(asset.name, asset);
							break;

						case ASSET_TYPE_SHADER:
							break;

						case ASSET_TYPE_MATERIAL:
							windows::AddWindow<EditMaterialWindow>(asset.name, asset);
							break;

						case ASSET_TYPE_MESH:
							windows::AddWindow<MeshPreviewWindow>(asset.name, asset);
							break;

						case ASSET_TYPE_BLUEPRINT:
							windows::AddWindow<ObjectBlueprintEditorWindow>(asset.name, asset);
							break;

						case ASSET_TYPE_ACTOR_BLUEPRINT:
							windows::AddWindow<ActorBlueprintEditorWindow>(asset.name, asset);
							break;

						case ASSET_TYPE_SCENE:
							TryTravelToScene((Scene*)asset.instance);
							break;
					}
				}
			}
			else if (ImGui::IsItemHovered())
			{
				switch (asset.type)
				{
					case ASSET_TYPE_BLUEPRINT:
					case ASSET_TYPE_ACTOR_BLUEPRINT:
					{
						std::string bpClass = ((Blueprint*)asset.instance)->GetClass() ? ((Blueprint*)asset.instance)->GetClass()->name : "NULL";
						ImGui::SetTooltip
						(
							"File: %s" "\n" "Type: %s" "\n" "Class: %s",
							asset.name.c_str(),
							TYPES_STR[panelAsset.typeNameIndex],
							bpClass.c_str()
						);
					}
					break;

					default:
					{
						ImGui::SetTooltip
						(
							"File: %s" "\n" "Type: %s",
							asset.name.c_str(),
							TYPES_STR[panelAsset.typeNameIndex]
						);
					}
					break;
				}
			}

			ImGui::PopStyleVar();

			ImGui::Text(fileNameNoExt.c_str());

			ImGui::EndGroup();

			ImGui::NextColumn();
		}

		ImGui::End();
	}

	void Classes()
	{
		ImGui::Begin("C++ classes");

		std::vector<GroovyClass*> classes;

		if (gEditorSettings.mClassBrowserShowEngineClasses)
			for (GroovyClass* c : ENGINE_CLASSES)
				classes.push_back(c);

		if (gEditorSettings.mClassBrowserShowGameClasses)
			for (GroovyClass* c : GAME_CLASSES)
				classes.push_back(c);

		// stolen ui code that works pretty well

		float contentWidth = ImGui::GetContentRegionAvail().x;
		float contentHeight = ImGui::GetContentRegionAvail().y;
		int columns = contentWidth / gEditorSettings.mContentBrowserIconSize;
		columns = columns < 1 ? 1 : columns;

		// end stolen ui code

		ImGui::Spacing();
		ImGui::SetNextItemWidth(200);
		ImGui::SliderFloat("Item size", &gEditorSettings.mContentBrowserIconSize, ICON_SIZE_MIN, ICON_SIZE_MAX);
		ImGui::Checkbox("Show engine classes", &gEditorSettings.mClassBrowserShowEngineClasses);
		ImGui::SameLine();
		ImGui::Checkbox("Show game classes", &gEditorSettings.mClassBrowserShowGameClasses);
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Columns(columns, nullptr, false);

		for (size_t i = 0; i < classes.size(); ++i)
		{
			GroovyClass* gClass = classes[i];

			ImGui::PushID(i);

			ImGui::BeginGroup();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 3,3 });

			ImGui::ImageButton(gClass->name.c_str(), res::sClassAssetIcon->GetRendererID(), { gEditorSettings.mContentBrowserIconSize - 10, gEditorSettings.mContentBrowserIconSize - 5 }, { 0,0 }, { 1,1 }, { 1,1,1,1 });

			ImGui::PopStyleVar();

			if (ImGui::IsItemHovered())
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

		if (!sCurrentScene->scene)
		{
			ImGui::Text("No scene loaded");
			ImGui::End();
			return;
		}

		if (sEditorSceneState != EDITOR_SCENE_STATE_EDIT)
			ImGui::BeginDisabled();

		if (ImGui::Button("Add actor"))
		{
			ImGui::OpenPopup("Select actor template / class to add");
		}

		if (sEditorSceneState != EDITOR_SCENE_STATE_EDIT)
			ImGui::EndDisabled();

		if (ImGui::BeginPopupModal("Select actor template / class to add"))
		{
			if (ImGui::BeginTabBar("add_actor_popup"))
			{
				if (ImGui::BeginTabItem("Generic"))
				{
					ImGui::Spacing();
					ImGui::Spacing();

					if (ImGui::Selectable("Empty actor"))
					{
						sCurrentScene->scene->Editor_AddActor(Actor::StaticClass(), nullptr);
						sEditScenePendingSave = true;
						ImGui::CloseCurrentPopup();
					}
					if (ImGui::Selectable("Mesh actor"))
					{
						sCurrentScene->scene->Editor_AddActor(MeshActor::StaticClass(), nullptr);
						sEditScenePendingSave = true;
						ImGui::CloseCurrentPopup();
					}
					if (ImGui::Selectable("Spectator"))
					{
						sCurrentScene->scene->Editor_AddActor(Spectator::StaticClass(), nullptr);
						sEditScenePendingSave = true;
						ImGui::CloseCurrentPopup();
					}

					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Blueprints"))
				{
					ImGui::Spacing();
					ImGui::Spacing();

					for (AssetHandle& bpHandle : AssetManager::GetAssets(ASSET_TYPE_ACTOR_BLUEPRINT))
					{
						ActorBlueprint* bp = (ActorBlueprint*)bpHandle.instance;
						if (ImGui::Selectable(bpHandle.name.c_str()))
						{
							if (bp->GetActorClass())
							{
								sCurrentScene->scene->Editor_AddActor(bp->GetActorClass(), bp);
								sEditScenePendingSave = true;
							}
							else
							{
								SysMessageBox::Show_Warning("Corrupted blueprint", "This blueprint is corrupted, please delete it");
							}
							ImGui::CloseCurrentPopup();
							break;
						}
					}

					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("All classes"))
				{
					ImGui::Spacing();
					ImGui::Spacing();

					for (const GroovyClass* groovyClass : gClassDB.GetClasses())
					{
						if (GroovyClass_IsA(groovyClass, Actor::StaticClass()))
						{
							if (ImGui::Selectable(groovyClass->name.c_str()))
							{
								sCurrentScene->scene->Editor_AddActor((GroovyClass*)groovyClass, nullptr);
								sEditScenePendingSave = true;
								ImGui::CloseCurrentPopup();
								break;
							}
						}
					}

					ImGui::EndTabItem();
				}

				ImGui::EndTabBar();
			}

			if (ImGui::IsKeyPressed(ImGuiKey_Escape))
				ImGui::CloseCurrentPopup();

			ImGui::EndPopup();
		}

		Actor* actorToDelete = nullptr;
		for (Actor* actor : sCurrentScene->scene->GetActors())
		{
			std::string actorName = actor->GetName() + "##" + std::to_string((uint64)actor);
			ActorBlueprint* bp = actor->GetTemplate();
			ImGui::Columns(2);
			if (ImGui::Selectable(actorName.c_str(), sCurrentScene->selectedActor == actor))
			{
				sCurrentScene->selectedActor = actor;
				sCurrentScene->selectedSubobject = actor;
			}
			if (ImGui::BeginPopupContextItem(actorName.c_str(), ImGuiPopupFlags_MouseButtonRight))
			{
				if (sEditorSceneState != EDITOR_SCENE_STATE_EDIT)
					ImGui::BeginDisabled();

				if (ImGui::Selectable("Rename"))
				{
					sActorToRename = actor;
					sShowActorRename = true;
				}
				if (ImGui::Selectable("Remove"))
				{
					actorToDelete = actor;
				}

				if (sEditorSceneState != EDITOR_SCENE_STATE_EDIT)
					ImGui::EndDisabled();

				if (bp && ImGui::Selectable("Open blueprint"))
				{
					AssetHandle bpAsset = AssetManager::Get(bp->GetUUID());
					windows::AddWindow<ActorBlueprintEditorWindow>(bpAsset.name, bpAsset);
				}
				ImGui::EndPopup();
			}
			ImGui::NextColumn();
			if (bp)
			{
				std::filesystem::path bpFilename(AssetManager::Get(bp->GetUUID()).name);
				ImGui::Text(bpFilename.replace_extension().string().c_str());
			}
			else
			{
				ImGui::Text(actor->GetClass()->name.c_str());
			}
			ImGui::Columns();
		}
		if (actorToDelete)
		{
			if (sCurrentScene->selectedActor == actorToDelete)
				sCurrentScene->selectedActor = nullptr;

			if (sCurrentScene->selectedSubobject)
			{
				if (sCurrentScene->selectedSubobject == actorToDelete)
					sCurrentScene->selectedSubobject = nullptr;

				else if (ActorComponent* comp = Cast<ActorComponent>(sCurrentScene->selectedSubobject))
					if (comp->GetOwner() == actorToDelete)
						sCurrentScene->selectedSubobject = nullptr;
			}

			sCurrentScene->scene->Editor_DeleteActor(actorToDelete);
			sEditScenePendingSave = true;
			
			actorToDelete = nullptr;
		}
		if (sShowActorRename)
		{
			ImGui::OpenPopup("Rename actor");
			sActorRename = sActorToRename->GetName();
			sShowActorRename = false;
		}
		if (ImGui::BeginPopupModal("Rename actor"))
		{
			bool invalidName = sActorRename.empty() || std::count(sActorRename.begin(), sActorRename.end(), ' ') == sActorRename.length();

			if (invalidName)
				ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 0.7f, 0.0f, 1.0f });

			ImGui::InputText("##actor_rename", &sActorRename);

			if (invalidName)
				ImGui::PopStyleColor();

			if (invalidName)
				ImGui::BeginDisabled();

			if (ImGui::Button("Rename"))
			{
				sActorToRename->Editor_NameRef() = sActorRename;
				sEditScenePendingSave = true;
				ImGui::CloseCurrentPopup();
			}

			if (invalidName)
				ImGui::EndDisabled();

			if (ImGui::IsKeyPressed(ImGuiKey_Escape))
			{
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		ImGui::End();
	}

	void Properties()
	{
		ImGui::Begin("Properties");

		if (sCurrentScene->selectedActor)
		{
			// add component
			{
				static std::string sNewCompName = "";
				static bool sCanAddNewComp = false;

				if (sEditorSceneState != EDITOR_SCENE_STATE_EDIT)
					ImGui::BeginDisabled();
				
				if (ImGui::Button("Add component"))
				{
					sNewCompName = "My new component";
					sCanAddNewComp = !sNewCompName.empty() && !sCurrentScene->selectedActor->GetComponent(sNewCompName);
					ImGui::OpenPopup("Add component");
				}

				if (sEditorSceneState != EDITOR_SCENE_STATE_EDIT)
					ImGui::EndDisabled();

				if (ImGui::BeginPopupModal("Add component"))
				{
					if (!sCanAddNewComp)
						ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 0.7f, 0.0f, 1.0f });

					bool needCheck = ImGui::InputText("##add_comp_name", &sNewCompName);

					if (!sCanAddNewComp)
						ImGui::PopStyleColor();

					ImGui::Spacing();
					ImGui::Spacing();
					ImGui::Spacing();

					if (!sCanAddNewComp)
						ImGui::BeginDisabled();

					for (const GroovyClass* gClass : gClassDB.GetClasses())
					{
						if (GroovyClass_IsA(gClass, ActorComponent::StaticClass()))
						{
							if (ImGui::Selectable(gClass->name.c_str()))
							{
								sCurrentScene->selectedSubobject = sCurrentScene->selectedActor->__internal_Editor_AddEditorcomponent_Scene((GroovyClass*)gClass, sNewCompName);
								sEditScenePendingSave = true;
								break;
							}
						}
					}

					if (!sCanAddNewComp)
						ImGui::EndDisabled();

					if (needCheck)
						sCanAddNewComp = !sNewCompName.empty() && !sCurrentScene->selectedActor->GetComponent(sNewCompName);

					if (ImGui::IsKeyPressed(ImGuiKey_Escape))
						ImGui::CloseCurrentPopup();

					ImGui::EndPopup();
				}
			}

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();

			// actor components
			{
				if (ImGui::Selectable(sCurrentScene->selectedActor->GetClass()->name.c_str(), sCurrentScene->selectedSubobject == sCurrentScene->selectedActor))
					sCurrentScene->selectedSubobject = sCurrentScene->selectedActor;

				ImGui::Indent(20.0f);

				static ActorComponent* sPendingRename = nullptr;
				static ActorComponent* sPendingRemove = nullptr;
				static bool sOpenRenamePopup = false;

				for (ActorComponent* component : sCurrentScene->selectedActor->GetComponents())
				{
					std::string name = component->GetName();
					bool isInherited = component->GetType() != ACTOR_COMPONENT_TYPE_EDITOR_SCENE;

					if (isInherited)
						name += " (inherited)";

					if (ImGui::Selectable(name.c_str(), sCurrentScene->selectedSubobject == component))
						sCurrentScene->selectedSubobject = component;
					
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip(component->GetClass()->name.c_str());

					if (!isInherited && ImGui::BeginPopupContextItem(name.c_str(), ImGuiPopupFlags_MouseButtonRight))
					{
						if (sEditorSceneState != EDITOR_SCENE_STATE_EDIT)
							ImGui::BeginDisabled();

						if (ImGui::Selectable("Rename"))
						{
							sOpenRenamePopup = true;
							sPendingRename = component;
						}
						else if (ImGui::Selectable("Remove"))
						{
							sPendingRemove = component;
						}

						if (sEditorSceneState != EDITOR_SCENE_STATE_EDIT)
							ImGui::EndDisabled();

						ImGui::EndPopup();
					}
				}

				ImGui::Unindent(20.0f);

				// rename / remove
				{
					static std::string sCompRename = "";
					static bool sCanRename = false;

					if (sOpenRenamePopup)
					{
						ImGui::OpenPopup("Rename component");
						sCompRename = sPendingRename->GetName();
						sCanRename = !sCompRename.empty() && !sCurrentScene->selectedActor->GetComponent(sCompRename);
						sOpenRenamePopup = false;
					}

					if (ImGui::BeginPopupModal("Rename component", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
					{
						if (!sCanRename)
							ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 0.7f, 0.0f, 1.0f });

						bool needCheck = ImGui::InputText("##rename_comp_name", &sCompRename);

						if (!sCanRename)
							ImGui::PopStyleColor();

						if (!sCanRename)
							ImGui::BeginDisabled();

						if (ImGui::Button("Rename##rename_comp_btn"))
						{
							sCurrentScene->selectedActor->__internal_Editor_RenameEditorComponent(sPendingRename, sCompRename);
							sPendingRename = nullptr;
							sEditScenePendingSave = true;
							ImGui::CloseCurrentPopup();
						}

						if (!sCanRename)
							ImGui::EndDisabled();

						if (needCheck)
							sCanRename = !sCompRename.empty() && !sCurrentScene->selectedActor->GetComponent(sCompRename);

						if (ImGui::IsKeyPressed(ImGuiKey_Escape))
						{
							sPendingRename = nullptr;
							ImGui::CloseCurrentPopup();
						}

						ImGui::EndPopup();
					}
					else if (sPendingRemove)
					{
						sCurrentScene->selectedActor->__internal_Editor_RemoveEditorComponent(sPendingRemove);
						sEditScenePendingSave = true;
						if (sCurrentScene->selectedSubobject == sPendingRemove)
							sCurrentScene->selectedSubobject = nullptr;
						sPendingRemove = nullptr;
					}
				}
			}
		}

		ImGui::End();
	}
	
	void Subproperties()
	{
		ImGui::Begin("Subproperties");

		if (sCurrentScene->selectedSubobject)
		{
			bool transformChanged = false;
			bool propsChanged = false;
			if (sCurrentScene->selectedSubobject == sCurrentScene->selectedActor)
			{
				ImGui::Text("Transform");
				transformChanged = editorGui::Transform("##actor_transform", &sCurrentScene->selectedActor->Editor_TransformRef());
				ImGui::Spacing();
				ImGui::Spacing();
			}
			else if (SceneComponent* sceneComp = Cast<SceneComponent>(sCurrentScene->selectedSubobject))
			{
				ImGui::Text("Transform (relative)");
				transformChanged = editorGui::Transform("##scene_comp_transform", &sceneComp->Editor_TransformRef());
				ImGui::Spacing();
				ImGui::Spacing();
			}
			propsChanged = editorGui::PropertiesAllClasses(sCurrentScene->selectedSubobject);

			if(sEditorSceneState == EDITOR_SCENE_STATE_EDIT)
				if (transformChanged || propsChanged)
					sEditScenePendingSave = true;
		}

		ImGui::End();
	}

	void GameViewport()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 2, 2 });
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);

		ImVec4 wndBorderCol = { 0.0f, 0.0f, 0.0f, 1.0f };
		ImVec2 btnsSize = { 38.0f, 38.0f };
		float btnsSpacing = 10.0f;

		if (sEditorSceneState == EDITOR_SCENE_STATE_PLAY)
			wndBorderCol = { 1.0f, 1.0f, 0.0f, 1.0f };

		ImGui::PushStyleColor(ImGuiCol_WindowBg, wndBorderCol);

		ImGui::Begin("Game viewport", nullptr, sEditScenePendingSave ? ImGuiWindowFlags_UnsavedDocument : 0);

		sGameViewportFocused = ImGui::IsWindowFocused();
		sGameVieportHovered = ImGui::IsWindowHovered();

		ImVec2 startCursorPos = ImGui::GetCursorPos();

		ImVec2 wndSize = ImGui::GetContentRegionAvail();

		if (sCurrentScene->scene)
		{
			// resize framebuffer if needed
			bool differentSize = wndSize.x != sGameViewportFrameBuffer->GetSpecs().width || wndSize.y != sGameViewportFrameBuffer->GetSpecs().height;
			bool okSize = wndSize.x > 0 && wndSize.y > 0;
			if (differentSize && okSize)
			{
				sGameViewportFrameBuffer->Resize(wndSize.x, wndSize.y);
			}
			// clear frame buffer
			sGameViewportFrameBuffer->ClearDepthAttachment();
			sGameViewportFrameBuffer->ClearColorAttachment(0, gScreenClearColor);
			// bind framebuffer
			sGameViewportFrameBuffer->Bind();

			float aspectRatio = (float)wndSize.x / (float)wndSize.y;

			if (sEditorSceneState == EDITOR_SCENE_STATE_PLAY && sCurrentScene->scene->mCamera)
			{
				CameraComponent* cam = sCurrentScene->scene->mCamera;
				SceneRenderer::BeginScene(cam, aspectRatio);
			}
			else
			{
				SceneRenderer::BeginScene(sEditorCamera.location, sEditorCamera.rotation, gEditorSettings.mCameraFOV, aspectRatio);
			}

			SceneRenderer::RenderScene(sCurrentScene->scene);

			// draw framebuffer
			gGroovyGuiRenderer->SetGroovyRenderState();

			ImGui::Image(sGameViewportFrameBuffer->GetRendererID(0), wndSize);

			if (sEditorSceneState == EDITOR_SCENE_STATE_EDIT)
				if (ImGui::BeginDragDropTarget())
				{
					const ImGuiPayload* payload = ImGui::GetDragDropPayload();
					AssetHandle asset = AssetManager::Get(*(AssetUUID*)payload->Data);

					bool canAccept =
						asset.type == ASSET_TYPE_SCENE ||
						asset.type == ASSET_TYPE_MESH ||
						asset.type == ASSET_TYPE_ACTOR_BLUEPRINT;

					if (canAccept && ImGui::AcceptDragDropPayload("drag_and_drop_asset"))
					{
						switch (asset.type)
						{
							case ASSET_TYPE_SCENE:
							{
								TryTravelToScene((Scene*)asset.instance);
							}
							break;

							case ASSET_TYPE_MESH:
							{
								MeshActor* meshActor = (MeshActor*)sEditScene.scene->Editor_AddActor(MeshActor::StaticClass(), nullptr);
								meshActor->GetMesh()->SetMesh((Mesh*)asset.instance);
								sEditScenePendingSave = true;
							}
							break;

							case ASSET_TYPE_ACTOR_BLUEPRINT:
							{
								ActorBlueprint* bp = (ActorBlueprint*)asset.instance;
								sEditScene.scene->Editor_AddActor(bp->GetActorClass(), bp);
								sEditScenePendingSave = true;
							}
							break;
						}

						ImGui::SetWindowFocus();
					}
					

					ImGui::EndDragDropTarget();
				}

			gGroovyGuiRenderer->SetImguiRenderState();

			// on screen text
			ImGui::SetCursorPosY(ImGui::GetWindowSize().y - wndSize.y + 8);
			ImGui::SetCursorPosX(8);
			ImGui::Text("Viewport %ix%i", sGameViewportFrameBuffer->GetSpecs().width, sGameViewportFrameBuffer->GetSpecs().height);

			//// play / pause / stop 

			ImVec2 btnsCursorPos = startCursorPos;
			btnsCursorPos.y += 8;

			if (sEditorSceneState != EDITOR_SCENE_STATE_PLAY)
			{
				// 1 button
				btnsCursorPos.x += (wndSize.x / 2.0f) - (btnsSize.x / 2.0f + ImGui::GetStyle().FramePadding.x);
			}
			else
			{
				// 2 buttons
				btnsCursorPos.x += (wndSize.x / 2.0f) - (btnsSize.x + ImGui::GetStyle().FramePadding.x * 2 + btnsSpacing / 2);
			}

			ImGui::SetCursorPos(btnsCursorPos);

			ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 1,1,1,0.9f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 1,1,1,0.6f });
			ImGui::PushStyleColor(ImGuiCol_Button, { 1,1,1,0 });

			EEditorSceneState newSceneState = sEditorSceneState;

			if (sEditorSceneState == EDITOR_SCENE_STATE_EDIT)
			{
				if (ImGui::ImageButton("play_btn", res::sPlayBtnIcon->GetRendererID(), btnsSize))
				{
					newSceneState = EDITOR_SCENE_STATE_PLAY;
				}
			}
			else
			{
				if (ImGui::ImageButton("pause_btn", res::sPauseBtnIcon->GetRendererID(), btnsSize))
				{
					// sEditorSceneState = EDITOR_SCENE_STATE_EDIT;
				}
				if(ImGui::IsItemHovered())
					ImGui::SetTooltip("Not supported right now");

				ImGui::SameLine(0.0f, btnsSpacing);
				if (ImGui::ImageButton("stop_btn", res::sStopBtnIcon->GetRendererID(), btnsSize))
				{
					newSceneState = EDITOR_SCENE_STATE_EDIT;
				}

				if (ImGui::IsKeyDown(ImGuiKey_Escape))
				{
					newSceneState = EDITOR_SCENE_STATE_EDIT;
				}
			}
			
			ImGui::PopStyleColor(3);

			if (newSceneState != sEditorSceneState)
			{
				if (newSceneState == EDITOR_SCENE_STATE_PLAY)
				{
					Play();
					gWindow->SetCursorPos(wndSize.x / 2, wndSize.y / 2);
					gWindow->EnableCursor(false);
				}
				else if (newSceneState == EDITOR_SCENE_STATE_EDIT)
				{
					Stop();
					gWindow->EnableCursor(true);
					Input::Editor_BlockInput(false);
				}

				// play , pause etc..
				sEditorSceneState = newSceneState;
			}

			if (sEditorSceneState == EDITOR_SCENE_STATE_PLAY)
			{
				if (!gWindow->IsCursorEnabled())
				{
					if (ImGui::IsKeyPressed(ImGuiKey_F1) && ImGui::IsKeyDown(ImGuiKey_LeftShift))
					{
						gWindow->EnableCursor(true);
						Input::Editor_BlockInput(true);
					}
				}
				else
				{
					if (ImGui::IsKeyPressed(ImGuiKey_MouseLeft) && ImGui::IsWindowHovered())
					{
						gWindow->SetCursorPos(wndSize.x / 2, wndSize.y / 2);
						gWindow->EnableCursor(false);
						Input::Editor_BlockInput(false);
					}
				}
			}

		}
		else
		{
			ImGui::NewLine();
			ImGui::Text("No scene loaded");
			ImGui::Image(0, ImGui::GetContentRegionAvail());
			if (ImGui::BeginDragDropTarget())
			{
				const ImGuiPayload* payload = ImGui::GetDragDropPayload();
				AssetHandle asset = AssetManager::Get(*(AssetUUID*)payload->Data);

				if (asset.type == ASSET_TYPE_SCENE && ImGui::AcceptDragDropPayload("drag_and_drop_asset"))
				{
					TryTravelToScene((Scene*)asset.instance);
					ImGui::SetWindowFocus();
				}

				ImGui::EndDragDropTarget();
			}
			
		}

		ImGui::End();

		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor();
	}
}

void editor::Init()
{
	gClassDB.Register(EditorSettings::StaticClass());
	gClassDB.BuildCDO(EditorSettings::StaticClass());

	gWindow->SubmitToWndCloseCallback(OnCloseRequested);
	gWindow->SubmitToWndFilesDropCallbacks(OnFilesDropped);

	FrameBufferSpec gameViewportSpec;
	gameViewportSpec.colorAttachments = { COLOR_FORMAT_R8G8B8A8_UNORM };
	gameViewportSpec.hasDepthAttachment = true;
	gameViewportSpec.swapchainTarget = false;
	gameViewportSpec.width = 100;
	gameViewportSpec.height = 100;

	sGameViewportFrameBuffer = FrameBuffer::Create(gameViewportSpec);

	gEditorSettings.Load();

	res::Load();

	sEditScene.scene = nullptr;
	sEditScene.selectedActor = nullptr;
	sEditScene.selectedSubobject = nullptr;

	sPlayScene.scene = new Scene();
	sPlayScene.selectedActor = nullptr;
	sPlayScene.selectedSubobject = nullptr;

	sCurrentScene = &sEditScene;

	if (gProj.GetStartupScene())
	{
		TravelToScene(gProj.GetStartupScene());
	}
	else
	{
		std::vector<AssetHandle> scenes = AssetManager::GetAssets(ASSET_TYPE_SCENE);
		if (scenes.size())
		{
			TravelToScene((Scene*)scenes[0].instance);
		}
		else
		{
			Scene* newEmptyScene = new Scene();
			AssetManager::Editor_Add("Empty_Scene" GROOVY_ASSET_EXT, ASSET_TYPE_SCENE, newEmptyScene);
			newEmptyScene->Save();
			TravelToScene(newEmptyScene);
		}
	}
}

void editor::Update(float deltaTime)
{
	if (!sEditScene.scene)
		return;

	// tick scene
	if (sEditorSceneState == EDITOR_SCENE_STATE_PLAY)
	{
		sPlayScene.scene->Tick(deltaTime);
	}

	// save
	if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_S))
	{
		if (sEditScene.scene)
			sEditScene.scene->Save();

		sEditScenePendingSave = false;
		AssetManager::SaveRegistry();
	}

	// toggle between wireframe and solid fill mode
	if (ImGui::IsKeyPressed(ImGuiKey_F3))
	{
		RasterizerState rasterState = RendererAPI::Get().GetRasterizerState();

		if (rasterState.fillMode == RASTERIZER_FILL_MODE_SOLID)
		{
			rasterState.fillMode = RASTERIZER_FILL_MODE_WIREFRAME;
			rasterState.cullMode = RASTERIZER_CULL_MODE_NONE;
		}
		else
		{
			rasterState.fillMode = RASTERIZER_FILL_MODE_SOLID;
			rasterState.cullMode = RASTERIZER_CULL_MODE_BACK;
		}

		RendererAPI::Get().SetRasterizerState(rasterState);
	}

	// update editor camera
	if (sEditorSceneState == EDITOR_SCENE_STATE_EDIT && sGameViewportFocused && sGameVieportHovered)
	{
		if (ImGui::IsKeyPressed(ImGuiKey_MouseRight))
		{
			gWindow->EnableCursor(false);
		}
		if (ImGui::IsKeyReleased(ImGuiKey_MouseRight))
		{
			gWindow->EnableCursor(true);
		}

		// update camera rotation
		{
			if (ImGui::IsKeyDown(ImGuiKey_MouseRight))
			{
				// camera rotation

				MouseDelta mouseDelta = Input::GetMouseDelta();

				sEditorCamera.rotation.y += mouseDelta.x * gEditorSettings.mCameraRotationSpeed;
				sEditorCamera.rotation.x += mouseDelta.y * gEditorSettings.mCameraRotationSpeed;

				// camera location

				Vec3 inputDirection = { 0.0f, 0.0f, 0.0f };

				if (ImGui::IsKeyDown(ImGuiKey_W))
					inputDirection.z += 1.0f;
				if (ImGui::IsKeyDown(ImGuiKey_S))
					inputDirection.z += -1.0f;
				if (ImGui::IsKeyDown(ImGuiKey_D))
					inputDirection.x += 1.0f;
				if (ImGui::IsKeyDown(ImGuiKey_A))
					inputDirection.x += -1.0f;
				if (ImGui::IsKeyDown(ImGuiKey_Q))
					inputDirection.y += 1.0f;
				if (ImGui::IsKeyDown(ImGuiKey_E))
					inputDirection.y += -1.0f;

				if (inputDirection.x || inputDirection.y || inputDirection.z)
				{
					Vec3 movement =
						math::GetForwardVector(sEditorCamera.rotation) * inputDirection.z
						+
						math::GetRightVector(sEditorCamera.rotation) * inputDirection.x
						+
						math::GetUpVector(sEditorCamera.rotation) * inputDirection.y;

					movement = math::Normalize(movement);
					movement *= gEditorSettings.mCameraMoveSpeed * deltaTime;

					sEditorCamera.location += movement;
				}
			}
		}
	}
}

void editor::Render()
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
				if (sEditScene.scene)
					sEditScene.scene->Save();

				sEditScenePendingSave = false;
				AssetManager::SaveRegistry();
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Settings"))
		{
			if(ImGui::MenuItem("Project settings"))
				windows::AddWindow<ProjectSettingsWindow>("Project settings");
			if (ImGui::MenuItem("Editor settings"))
				windows::AddWindow<EditorSettingsWindow>("Editor settings");
			
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Help"))
		{
			windows::AddWindow<HelpWindow>("Help window");
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Create"))
		{
			if (ImGui::MenuItem("Material"))
			{
				assetPopups::sNewAssetType = ASSET_TYPE_MATERIAL;
				assetPopups::sAssetRename = "My_New_Material";
			}
			if (ImGui::MenuItem("Blueprint"))
			{
				assetPopups::sNewAssetType = ASSET_TYPE_BLUEPRINT;
				assetPopups::sAssetRename = "My_New_Blueprint";
			}
			if (ImGui::MenuItem("Actor blueprint"))
			{
				assetPopups::sNewAssetType = ASSET_TYPE_ACTOR_BLUEPRINT;
				assetPopups::sAssetRename = "My_New_Actor_Blueprint";
			}
			if (ImGui::MenuItem("Scene"))
			{
				assetPopups::sNewAssetType = ASSET_TYPE_SCENE;
				assetPopups::sAssetRename = "My_New_Scene";
			}

			ImGui::EndMenu();
		}

		ImGui::SameLine(ImGui::GetWindowWidth() - 650);
		ImGui::Text("Frametime: %.4fms FPS: %.3f", (float)gDeltaTime * 1000.0f, 1.0f / (float)gDeltaTime);

		ImGui::EndMenuBar();
	}

	panels::Classes();
	panels::Assets();
	panels::GameViewport();
	panels::EntityList();
	panels::Properties();
	panels::Subproperties();

	assetPopups::RenderPopups();
	
	windows::UpdateWindows();

	ImGui::End();
}

void editor::Shutdown()
{
	delete sPlayScene.scene;

	gEditorSettings.Save();
	windows::Shutdown();
	res::Unload();
	delete sGameViewportFrameBuffer;
}

Scene* editor::GetEditScene()
{
	return sEditScene.scene;
}

void editor::FlagEditScenePendingSave()
{
	sEditScenePendingSave = true;
}

void editor::OnBPUpdated(ActorBlueprint* bp)
{
	if (sEditScene.selectedActor && sEditScene.selectedActor->GetTemplate() == bp)
		sEditScene.selectedSubobject = nullptr;
}

const char* editor::AssetTypeToStr(EAssetType type)
{
	switch (type)
	{
		case ASSET_TYPE_NONE:
			return "NONE";
		case ASSET_TYPE_TEXTURE:
			return "TEXTURE";
		case ASSET_TYPE_SHADER:
			return "SHADER";
		case ASSET_TYPE_MATERIAL:
			return "MATERIAL";
		case ASSET_TYPE_MESH:
			return "MESH";
		case ASSET_TYPE_BLUEPRINT:
			return "BLUEPRINT";
		case ASSET_TYPE_ACTOR_BLUEPRINT:
			return "ACTOR BLUEPRINT";
		case ASSET_TYPE_SCENE:
			return "SCENE";
	}
	return "UNKNOWN ASSET TYPE ?!?";
}