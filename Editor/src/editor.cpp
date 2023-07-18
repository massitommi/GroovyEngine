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
#include "gameframework/blueprint.h"

#include "engine/world.h"

#include "gameframework/actor.h"
#include "gameframework/actorcomponent.h"
#include "gameframework/scene.h"
#include "editor.h"
#include "classes/reflection.h"
#include "classes/class_db.h"

#include "utils/string/string_utils.h"

extern ClearColor gScreenClearColor;
extern Window* gWindow;
extern GroovyProject gProj;
extern std::vector<GroovyClass*> ENGINE_CLASSES;
extern std::vector<GroovyClass*> GAME_CLASSES;
extern ClassDB gClassDB;

static FrameBuffer* sGameViewportFrameBuffer = nullptr;

struct EditorScene
{
	Scene* scene;
	std::string name;
	bool pendingSave;
};

static EditorScene sEditorScene = { nullptr, "Unsaved_Scene", false };

namespace res
{
	static Texture* sShaderAssetIcon = nullptr;
	static Texture* sMaterialAssetIcon = nullptr;
	static Texture* sMeshAssetIcon = nullptr;
	static Texture* sClassAssetIcon = nullptr;
	static Texture* sBlueprintAssetIcon = nullptr;
	static Texture* sSceneAssetIcon = nullptr;

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
	if (sEditorScene.scene)
	{
		sEditorScene.scene->Unload();
	}

	sEditorScene.scene = scene;

	if (scene)
	{
		sEditorScene.name = AssetManager::Get(scene->GetUUID()).name;
		sEditorScene.scene->Load();
	}

	sEditorScene.pendingSave = false;
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
	return true;
}

namespace newAsset
{
	static EAssetType sNewAssetType = ASSET_TYPE_NONE;
	static std::string sNewAssetName = "New_Asset";

	bool IsValidAssetName()
	{
		return
			!sNewAssetName.empty() &&													// path is not empty
			sNewAssetName.find(' ') == std::string::npos &&								// path does not contain whitespaces
			!AssetManager::FindByPath(sNewAssetName + GROOVY_ASSET_EXT).instance &&		// there is no asset with the same path
			!stringUtils::EqualsCaseInsensitive(sNewAssetName, "DEFAULT_SHADER") &&		// blacklisted filename
			!stringUtils::EqualsCaseInsensitive(sNewAssetName, "DEFAULT_TEXTURE") &&	// blacklisted filename
			!stringUtils::EqualsCaseInsensitive(sNewAssetName, "DEFAULT_MATERIAL") &&	// blacklisted filename
			!stringUtils::EqualsCaseInsensitive(sNewAssetName, "Empty_Scene");			// blacklisted filename
	}

	void CreateNewAsset()
	{
		auto FilePathInput = [&]()
		{
			ImGui::Text("Filename");

			bool validPath = IsValidAssetName();

			if (!validPath)
				ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 0.7f, 0.0f, 1.0f });

			ImGui::InputText("##new_asset_name", &sNewAssetName);
			ImGui::SameLine();
			ImGui::Text(GROOVY_ASSET_EXT);
			ImGui::SameLine();
			ImGui::TextDisabled("(?)");
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort) && ImGui::BeginTooltip())
			{
				ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
				ImGui::TextUnformatted("Filenames can't contain whitespaces, filenames are case-insensitive");
				ImGui::PopTextWrapPos();
				ImGui::EndTooltip();
			}

			if (!validPath)
				ImGui::PopStyleColor();

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();

			return validPath;
		};

		if (ImGui::BeginPopupModal("New material"))
		{
			bool validPath = FilePathInput();

			ImGui::Text("Select shader:");
			ImGui::Spacing();

			if (!validPath)
				ImGui::BeginDisabled();

			for (AssetHandle& asset : AssetManager::GetAssets(ASSET_TYPE_SHADER))
			{
				if (ImGui::Selectable(asset.name.c_str()))
				{
					// create asset
					Material* newMat = new Material();
					newMat->SetShader((Shader*)asset.instance);
					// pending save stuff
					AssetManager::Editor_OnAdd(sNewAssetName + GROOVY_ASSET_EXT, ASSET_TYPE_MATERIAL, newMat);
					newMat->Save();
					// close popup
					ImGui::CloseCurrentPopup();
				}
			}

			if (!validPath)
				ImGui::EndDisabled();

			if (ImGui::IsKeyPressed(ImGuiKey_Escape))
				ImGui::CloseCurrentPopup();

			ImGui::EndPopup();
		}

		if (ImGui::BeginPopupModal("New blueprint"))
		{
			bool validPath = FilePathInput();

			ImGui::Text("Select class:");
			ImGui::Spacing();

			if (!validPath)
				ImGui::BeginDisabled();

			for (const GroovyClass* gClass : gClassDB.GetClasses())
			{
				if (ImGui::Selectable(gClass->name.c_str()))
				{
					// create asset
					ObjectBlueprint* bp = new ObjectBlueprint();
					bp->SetupEmpty((GroovyClass*)gClass);
					// pending save stuff
					AssetManager::Editor_OnAdd(sNewAssetName + GROOVY_ASSET_EXT, ASSET_TYPE_BLUEPRINT, bp);
					bp->Save();
					// close popup
					ImGui::CloseCurrentPopup();
				}
			}

			if (!validPath)
				ImGui::EndDisabled();

			if (ImGui::IsKeyPressed(ImGuiKey_Escape))
				ImGui::CloseCurrentPopup();

			ImGui::EndPopup();
		}

		if (ImGui::BeginPopupModal("New actor blueprint"))
		{
			bool validPath = FilePathInput();

			ImGui::Text("Select class:");
			ImGui::Spacing();

			if (!validPath)
				ImGui::BeginDisabled();

			for (const GroovyClass* gClass : gClassDB.GetClasses())
			{
				if (classUtils::IsA(gClass, Actor::StaticClass()) && ImGui::Selectable(gClass->name.c_str()))
				{
					// create asset
					ActorBlueprint* bp = new ActorBlueprint();
					bp->SetupEmpty((GroovyClass*)gClass);
					// pending save stuff
					AssetManager::Editor_OnAdd(sNewAssetName + GROOVY_ASSET_EXT, ASSET_TYPE_ACTOR_BLUEPRINT, bp);
					bp->Save();
					// close popup
					ImGui::CloseCurrentPopup();
				}
			}

			if (!validPath)
				ImGui::EndDisabled();

			if (ImGui::IsKeyPressed(ImGuiKey_Escape))
				ImGui::CloseCurrentPopup();

			ImGui::EndPopup();
		}

		if (ImGui::BeginPopupModal("New scene"))
		{
			bool validPath = FilePathInput();
			
			ImGui::Spacing();

			if (!validPath)
				ImGui::BeginDisabled();

			if (ImGui::Button("Save"))
			{
				Scene* newScene = new Scene();
				AssetManager::Editor_OnAdd(sNewAssetName + GROOVY_ASSET_EXT, ASSET_TYPE_SCENE, newScene);
				newScene->Save();
				ImGui::CloseCurrentPopup();
			}

			if (!validPath)
				ImGui::EndDisabled();

			if (ImGui::IsKeyPressed(ImGuiKey_Escape))
				ImGui::CloseCurrentPopup();

			ImGui::EndPopup();
		}
	}

	void NewAssetPopup()
	{
		if (sNewAssetType != ASSET_TYPE_NONE)
		{
			switch (sNewAssetType)
			{
				case ASSET_TYPE_MATERIAL:
					ImGui::OpenPopup("New material");
					sNewAssetName = "My_New_Material";
					break;
				case ASSET_TYPE_BLUEPRINT:
					ImGui::OpenPopup("New blueprint");
					sNewAssetName = "My_New_Blueprint";
					break;
				case ASSET_TYPE_ACTOR_BLUEPRINT:
					ImGui::OpenPopup("New actor blueprint");
					sNewAssetName = "My_New_Actor_Blueprint";
					break;
				case ASSET_TYPE_SCENE:
					ImGui::OpenPopup("New scene");
					sNewAssetName = "My_New_Scene";
					break;
			}

			sNewAssetType = ASSET_TYPE_NONE;
		}
	}
}

namespace panels
{
	static ImVec2 sGameViewportSize = { 100, 100 };

	static Actor* sCurrentlySelectedActor = nullptr;
	static GroovyObject* sCurrentlySelectedObject = nullptr;
	static Actor* sActorToRename = nullptr;
	static bool sShowActorRename = false;
	static std::string sActorRename;

	static float sIconSize = 140.0f;
	static const float ICON_SIZE_MAX = 256.0f;
	static const float ICON_SIZE_MIN = 100.0f;

	static bool sShowEngineClasses = true;
	static bool sShowGameClasses = true;

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
		enum EPanelAssetFlags
		{
			PANEL_ASSET_FLAG_IS_DEFAULT = BITFLAG(1)
		};

		struct PanelAsset
		{
			ImTextureID thumbnail;
			uint32 typeNameIndex;
			uint32 flags;
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

		if (ImGui::BeginPopupContextWindow("Asset manager", ImGuiPopupFlags_MouseButtonRight))
		{
			if (ImGui::BeginMenu("Create"))
			{
				if (ImGui::MenuItem("Material"))
					newAsset::sNewAssetType = ASSET_TYPE_MATERIAL;
				if (ImGui::MenuItem("Blueprint"))
					newAsset::sNewAssetType = ASSET_TYPE_BLUEPRINT;
				if (ImGui::MenuItem("Actor blueprint"))
					newAsset::sNewAssetType = ASSET_TYPE_ACTOR_BLUEPRINT;
				if (ImGui::MenuItem("Scene"))
					newAsset::sNewAssetType = ASSET_TYPE_SCENE;

				ImGui::EndMenu();
			}

			ImGui::EndPopup();
		}

		ImGui::Columns(columns, nullptr, false);

		for (uint32 i = 0; i < assets.size(); ++i)
		{
			const AssetHandle& asset = assets[i];
			PanelAsset panelAsset = panelAssets[i];

			ImGui::PushID(i);

			ImGui::BeginGroup();

			std::string fileNameNoExt = std::filesystem::path(asset.name).replace_extension().string();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 3,3 });

			if (ImGui::ImageButton(fileNameNoExt.c_str(), panelAsset.thumbnail, { sIconSize - 10, sIconSize - 5 }, { 0,0 }, { 1,1 }, { 1,1,1,1 }))
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
					if (sEditorScene.scene != asset.instance)
					{
						TravelToScene((Scene*)asset.instance);
					}
					break;
				}
			}

			ImGui::PopStyleVar();

			if (ImGui::BeginPopupContextItem(fileNameNoExt.c_str(), ImGuiPopupFlags_MouseButtonRight))
			{
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
							FileSystem::DeleteFile((gProj.GetAssetsPath() / asset.name).string());

							if (asset.instance == sEditorScene.scene)
								TravelToScene(nullptr);

							sCurrentlySelectedActor = nullptr;
							sCurrentlySelectedObject = nullptr;

							AssetManager::Editor_Delete(asset.uuid);
						}
					}
				}

				ImGui::EndPopup();
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

		std::vector<GroovyClass*> classes;

		if (sShowEngineClasses)
			for (GroovyClass* c : ENGINE_CLASSES)
				classes.push_back(c);

		if (sShowGameClasses)
			for (GroovyClass* c : GAME_CLASSES)
				classes.push_back(c);

		// stolen ui code that works pretty well

		float contentWidth = ImGui::GetContentRegionAvail().x;
		float contentHeight = ImGui::GetContentRegionAvail().y;
		int columns = contentWidth / sIconSize;
		columns = columns < 1 ? 1 : columns;

		// end stolen ui code

		ImGui::Spacing();
		ImGui::SetNextItemWidth(200);
		ImGui::SliderFloat("Item size", &sIconSize, ICON_SIZE_MIN, ICON_SIZE_MAX);
		ImGui::Checkbox("Show engine classes", &sShowEngineClasses);
		ImGui::SameLine();
		ImGui::Checkbox("Show game classes", &sShowGameClasses);
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Columns(columns, nullptr, false);

		for (size_t i = 0; i < classes.size(); ++i)
		{
			GroovyClass* gClass = classes[i];

			ImGui::PushID(i);

			ImGui::BeginGroup();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 3,3 });

			ImGui::ImageButton(gClass->name.c_str(), res::sClassAssetIcon->GetRendererID(), { sIconSize - 10, sIconSize - 5 }, { 0,0 }, { 1,1 }, { 1,1,1,1 });

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

		if (!sEditorScene.scene)
		{
			ImGui::Text("No scene loaded");
			ImGui::End();
			return;
		}

		if (ImGui::Button("Add actor"))
		{
			ImGui::OpenPopup("Select actor template / class to add");
		}

		if (ImGui::BeginPopupModal("Select actor template / class to add"))
		{
			ImGui::Text("Templates:");
			ImGui::Spacing();
			for (AssetHandle bpHandle : AssetManager::GetAssets(ASSET_TYPE_ACTOR_BLUEPRINT))
			{
				ActorBlueprint* bp = (ActorBlueprint*)bpHandle.instance;
				if (bp->GetActorClass())
				{
					if (ImGui::Selectable(bpHandle.name.c_str()))
					{
						sEditorScene.scene->SpawnActor(bp->GetActorClass(), bp);
						sEditorScene.pendingSave = true;
						ImGui::CloseCurrentPopup();
						break;
					}
				}
			}
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Text("Classes:");
			ImGui::Spacing();
			for (const GroovyClass* groovyClass : gClassDB.GetClasses())
			{
				if (classUtils::IsA(groovyClass, Actor::StaticClass()))
				{
					if (ImGui::Selectable(groovyClass->name.c_str()))
					{
						sEditorScene.scene->SpawnActor((GroovyClass*)groovyClass);
						sEditorScene.pendingSave = true;
						ImGui::CloseCurrentPopup();
						break;
					}
				}
			}

			if (ImGui::IsKeyPressed(ImGuiKey_Escape))
				ImGui::CloseCurrentPopup();

			ImGui::EndPopup();
		}

		Actor* actorToDelete = nullptr;
		for (Actor* actor : sEditorScene.scene->GetActors())
		{
			std::string actorName = actor->GetName() + "##" + std::to_string((uint64)actor);
			ActorBlueprint* bp = actor->GetTemplate();
			ImGui::Columns(2);
			if (ImGui::Selectable(actorName.c_str(), sCurrentlySelectedActor == actor))
			{
				sCurrentlySelectedActor = actor;
				sCurrentlySelectedObject = actor;
			}
			if (ImGui::BeginPopupContextItem(actorName.c_str(), ImGuiPopupFlags_MouseButtonRight))
			{
				if (ImGui::Selectable("Rename"))
				{
					sActorToRename = actor;
					sShowActorRename = true;
				}
				else if (ImGui::Selectable("Remove"))
				{
					actorToDelete = actor;
				}
				else if (bp && ImGui::Selectable("Open blueprint"))
				{
					AssetHandle bpAsset = AssetManager::Get(bp->GetUUID());
					windows::AddWindow<ActorBlueprintEditorWindow>(bpAsset.name, bpAsset);
				}
				ImGui::EndPopup();
			}
			ImGui::NextColumn();
			if (bp)
			{
				ImGui::Text(AssetManager::Get(bp->GetUUID()).name.c_str());
			}
			else
			{
				ImGui::Text(actor->GetClass()->name.c_str());
			}
			ImGui::Columns();
		}
		if (actorToDelete)
		{
			if (sCurrentlySelectedActor == actorToDelete)
				sCurrentlySelectedActor = nullptr;

			if (sCurrentlySelectedObject)
			{
				if (sCurrentlySelectedObject == actorToDelete)
					sCurrentlySelectedObject = nullptr;

				else if (ActorComponent* comp = Cast<ActorComponent>(sCurrentlySelectedObject))
					if (comp->GetOwner() == actorToDelete)
						sCurrentlySelectedObject = nullptr;
			}

			sEditorScene.scene->Editor_DeleteActor(actorToDelete);
			sEditorScene.pendingSave = true;
			
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

		if (sCurrentlySelectedActor)
		{
			// add component
			{
				static std::string sNewCompName = "";
				static bool sCanAddNewComp = false;

				if (ImGui::Button("Add component"))
				{
					sNewCompName = "My new component";
					sCanAddNewComp = !sNewCompName.empty() && !sCurrentlySelectedActor->GetComponent(sNewCompName);
					ImGui::OpenPopup("Add component");
				}

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
						if (classUtils::IsA(gClass, ActorComponent::StaticClass()))
						{
							if (ImGui::Selectable(gClass->name.c_str()))
							{
								sCurrentlySelectedActor->__internal_Editor_AddEditorcomponent_Scene((GroovyClass*)gClass, sNewCompName);
								sEditorScene.pendingSave = true;
								break;
							}
						}
					}

					if (!sCanAddNewComp)
						ImGui::EndDisabled();

					if (needCheck)
						sCanAddNewComp = !sNewCompName.empty() && !sCurrentlySelectedActor->GetComponent(sNewCompName);

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
				if (ImGui::Selectable(sCurrentlySelectedActor->GetClass()->name.c_str(), sCurrentlySelectedObject == sCurrentlySelectedActor))
					sCurrentlySelectedObject = sCurrentlySelectedActor;

				ImGui::Indent(20.0f);

				static ActorComponent* sPendingRename = nullptr;
				static ActorComponent* sPendingRemove = nullptr;
				static bool sOpenRenamePopup = false;

				for (ActorComponent* component : sCurrentlySelectedActor->GetComponents())
				{
					std::string name = component->GetName();
					bool isInherited = component->GetType() != ACTOR_COMPONENT_TYPE_EDITOR_SCENE;

					if (isInherited)
						name += " (inherited)";

					if (ImGui::Selectable(name.c_str(), sCurrentlySelectedObject == component))
						sCurrentlySelectedObject = component;

					if (!isInherited && ImGui::BeginPopupContextItem(name.c_str(), ImGuiPopupFlags_MouseButtonRight))
					{
						if (ImGui::Selectable("Rename"))
						{
							sOpenRenamePopup = true;
							sPendingRename = component;
						}
						else if (ImGui::Selectable("Remove"))
						{
							sPendingRemove = component;
						}

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
						sCanRename = !sCompRename.empty() && !sCurrentlySelectedActor->GetComponent(sCompRename);
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
							sCurrentlySelectedActor->__internal_Editor_RenameEditorComponent(sPendingRename, sCompRename);
							sPendingRename = nullptr;
							sEditorScene.pendingSave = true;
							ImGui::CloseCurrentPopup();
						}

						if (!sCanRename)
							ImGui::EndDisabled();

						if (needCheck)
							sCanRename = !sCompRename.empty() && !sCurrentlySelectedActor->GetComponent(sCompRename);

						if (ImGui::IsKeyPressed(ImGuiKey_Escape))
						{
							sPendingRename = nullptr;
							ImGui::CloseCurrentPopup();
						}

						ImGui::EndPopup();
					}
					else if (sPendingRemove)
					{
						sCurrentlySelectedActor->__internal_Editor_RemoveEditorComponent(sPendingRemove);
						sEditorScene.pendingSave = true;
						if (sCurrentlySelectedObject == sPendingRemove)
							sCurrentlySelectedObject = sCurrentlySelectedActor;
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

		if (sCurrentlySelectedObject)
		{
			bool transformChanged = false;
			bool propsChanged = false;
			if (sCurrentlySelectedObject == sCurrentlySelectedActor)
			{
				ImGui::Text("Transform");
				transformChanged = editorGui::Transform("##actor_transform", &sCurrentlySelectedActor->Editor_TransformRef());
				ImGui::Spacing();
				ImGui::Spacing();
			}
			else if (SceneComponent* sceneComp = Cast<SceneComponent>(sCurrentlySelectedObject))
			{
				ImGui::Text("Transform (relative)");
				transformChanged = editorGui::Transform("##scene_comp_transform", &sceneComp->Editor_TransformRef());
				ImGui::Spacing();
				ImGui::Spacing();
			}
			propsChanged = editorGui::PropertiesAllClasses(sCurrentlySelectedObject);

			if (transformChanged || propsChanged)
				sEditorScene.pendingSave = true;
		}

		ImGui::End();
	}

	void GameViewport()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0,0 });
		ImGui::Begin("Game viewport", nullptr, sEditorScene.pendingSave ? ImGuiWindowFlags_UnsavedDocument : 0);

		if (sEditorScene.scene)
		{
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

			// draw framebuffer
			ImGui::Image(sGameViewportFrameBuffer->GetRendererID(0), wndSize);

			// on screen text
			ImGui::SetCursorPosY(ImGui::GetWindowSize().y - wndSize.y + 8);
			ImGui::SetCursorPosX(8);
			ImGui::Text("Framebuffer %ix%i", sGameViewportFrameBuffer->GetSpecs().width, sGameViewportFrameBuffer->GetSpecs().height);
		}
		else
		{
			ImGui::NewLine();
			ImGui::Text("No scene loaded");
		}

		ImGui::End();
		ImGui::PopStyleVar();
	}
}

void editor::internal::Init()
{
	gWindow->SubmitToWndCloseCallback(OnCloseRequested);
	gWindow->SubmitToWndFilesDropCallbacks(OnFilesDropped);

	FrameBufferSpec gameViewportSpec;
	gameViewportSpec.colorAttachments = { COLOR_FORMAT_R8G8B8A8_UNORM };
	gameViewportSpec.hasDepthAttachment = true;
	gameViewportSpec.swapchainTarget = false;
	gameViewportSpec.width = panels::sGameViewportSize.x = 100;
	gameViewportSpec.height = panels::sGameViewportSize.y = 100;

	sGameViewportFrameBuffer = FrameBuffer::Create(gameViewportSpec);

	res::Load();

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
			AssetManager::Editor_OnAdd("Empty_Scene" GROOVY_ASSET_EXT, ASSET_TYPE_SCENE, newEmptyScene);
			newEmptyScene->Save();
			TravelToScene(newEmptyScene);
		}
	}
}

void editor::internal::Update(float deltaTime)
{

}

void editor::internal::Render()
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
				if (sEditorScene.scene)
				{
					sEditorScene.scene->Save();
					sEditorScene.pendingSave = false;
				}
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Settings"))
		{
			if(ImGui::MenuItem("Project settings"))
				windows::AddWindow<ProjectSettingsWindow>("Project settings");
			
			ImGui::EndMenu();
		}


		if (ImGui::BeginMenu("Create"))
		{
			if (ImGui::MenuItem("Material"))
				newAsset::sNewAssetType = ASSET_TYPE_MATERIAL;
			if (ImGui::MenuItem("Blueprint"))
				newAsset::sNewAssetType = ASSET_TYPE_BLUEPRINT;
			if (ImGui::MenuItem("Actor blueprint"))
				newAsset::sNewAssetType = ASSET_TYPE_ACTOR_BLUEPRINT;
			if (ImGui::MenuItem("Scene"))
				newAsset::sNewAssetType = ASSET_TYPE_SCENE;

			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}

	panels::Classes();
	panels::Assets();
	panels::GameViewport();
	panels::EntityList();
	panels::Properties();
	panels::Subproperties();

	newAsset::NewAssetPopup();
	newAsset::CreateNewAsset();
	
	windows::UpdateWindows();

	ImGui::End();

	ImGui::Render();
}

void editor::internal::Shutdown()
{
	windows::Shutdown();
	delete res::sBlueprintAssetIcon;
	delete res::sClassAssetIcon;
	delete res::sShaderAssetIcon;
	delete res::sMaterialAssetIcon;
	delete res::sMeshAssetIcon;
	delete sGameViewportFrameBuffer;
}

const char* editor::utils::AssetTypeToStr(EAssetType type)
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