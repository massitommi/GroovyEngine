#include "editor_window.h"
#include "vendor/imgui/imgui.h"
#include "vendor/imgui/misc/cpp/imgui_stdlib.h"
#include "platform/messagebox.h"
#include "classes/object_serializer.h"
#include "assets/assets.h"
#include "project/project.h"
#include "classes/class.h"
#include "classes/class_db.h"
#include "renderer/mesh.h"
#include "gameframework/blueprint.h"
#include "classes/reflection.h"
#include "runtime/object_allocator.h"
#include "gameframework/actor.h"
#include "gameframework/actorcomponent.h"
#include "editor.h"

void EditorWindow::RenderWindow()
{
	ImGui::Begin(mTitle.c_str(), &mOpen, mFlags | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);
	RenderContent();
	ImGui::End();

	if (!mOpen)
		OnCloseRequested();
}

AssetEditorWindow::AssetEditorWindow(const AssetHandle& asset)
	: EditorWindow(), mAsset(asset), mPendingSave(false)
{
	checkslowf(asset.instance, "NULL asset?!?!");

	SetTitle(asset.name);
}

void AssetEditorWindow::OnCloseRequested()
{
	if (!mPendingSave)
	{
		Close();
		return;
	}

	ImGui::OpenPopup("Save content");

	if (ImGui::BeginPopupModal("Save content", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("You are exiting without saving");
		
		if (ImGui::Button("Save"))
		{
			Save();
			Close();
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Discard"))
		{
			Close();
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape))
		{
			mOpen = true;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void AssetEditorWindow::Save()
{
	mPendingSave = false;
	mFlags &= ~ImGuiWindowFlags_UnsavedDocument;
}

inline void AssetEditorWindow::FlagPendingSave()
{
	mPendingSave = true;
	mFlags |= ImGuiWindowFlags_UnsavedDocument;
}

EditMaterialWindow::EditMaterialWindow(const AssetHandle& asset)
	: AssetEditorWindow(asset), mMaterial((Material*)asset.instance)
{
	checkslowf(asset.type == ASSET_TYPE_MATERIAL, "Invalid asset type");

	mMatResources = mMaterial->GetResources();
}

void EditMaterialWindow::RenderContent()
{
	if (!IsPendingSave())
		ImGui::BeginDisabled();

	bool click = ImGui::Button("Save");

	if (!IsPendingSave())
		ImGui::EndDisabled();

	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Spacing();

	for (MaterialResource& res : mMatResources)
	{
		std::string resLbl = "##" + res.name;
		ImGui::Text(res.name.c_str());
		ImGui::SameLine();
		if (editorGui::AssetRef(resLbl.c_str(), ASSET_TYPE_TEXTURE, &res.res))
			FlagPendingSave();
	}

	if (click)
	{
		Save();
	}
}

void EditMaterialWindow::Save()
{
	mMaterial->Editor_ResourcesRef() = mMatResources;
	mMaterial->Save();
	AssetEditorWindow::Save();
}

TexturePreviewWindow::TexturePreviewWindow(const AssetHandle& asset)
	: EditorWindow(), mTexture((Texture*)asset.instance)
{
}

void TexturePreviewWindow::RenderContent()
{
	ImGui::Image(mTexture->GetRendererID(), ImGui::GetContentRegionAvail());
}

MeshPreviewWindow::MeshPreviewWindow(const AssetHandle& asset)
	: AssetEditorWindow(asset), mMesh((Mesh*)asset.instance)
{
	checkslowf(asset.type == ASSET_TYPE_MESH, "Invalid asset type");

	mMeshMats = mMesh->GetMaterials();
}

void MeshPreviewWindow::RenderContent()
{
	if (!IsPendingSave())
		ImGui::BeginDisabled();

	bool click = ImGui::Button("Save");

	if (!IsPendingSave())
		ImGui::EndDisabled();

	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Spacing();

	for (uint32 i = 0; i < mMeshMats.size(); i++)
	{
		std::string lbl = "##_mesh_res_" + std::to_string(i);
		ImGui::Text("[%i]", i);
		ImGui::SameLine();
		if (editorGui::AssetRef(lbl.c_str(), ASSET_TYPE_MATERIAL, &mMeshMats[i]))
			FlagPendingSave();
	}

	if (click)
	{
		Save();
	}
}

void MeshPreviewWindow::Save()
{
	mMesh->Editor_MaterialsRef() = mMeshMats;
	mMesh->Save();
	AssetEditorWindow::Save();
}

ObjectBlueprintEditorWindow::ObjectBlueprintEditorWindow(const AssetHandle& asset)
	: AssetEditorWindow(asset), mBlueprint((ObjectBlueprint*)asset.instance)
{
	checkslow(asset.type == ASSET_TYPE_BLUEPRINT);

	mLiveObject = ObjectAllocator::Instantiate(mBlueprint->GetClass());
	mBlueprint->CopyProperties(mLiveObject);
}

ObjectBlueprintEditorWindow::~ObjectBlueprintEditorWindow()
{
	ObjectAllocator::Destroy(mLiveObject);
}

void ObjectBlueprintEditorWindow::RenderContent()
{
	if (!IsPendingSave())
		ImGui::BeginDisabled();

	bool click = ImGui::Button("Save");

	if (!IsPendingSave())
		ImGui::EndDisabled();

	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Spacing();

	if (editorGui::PropertiesAllClasses(mLiveObject))
		FlagPendingSave();

	if (click)
	{
		Save();
	}
}

void ObjectBlueprintEditorWindow::Save()
{
	mBlueprint->RebuildPack(mLiveObject);
	mBlueprint->Save();
	AssetEditorWindow::Save();
}

ActorBlueprintEditorWindow::ActorBlueprintEditorWindow(const AssetHandle& asset)
	: AssetEditorWindow(asset), mBlueprint((ActorBlueprint*)asset.instance)
{
	checkslow(asset.type == ASSET_TYPE_ACTOR_BLUEPRINT);

	mLiveActor = ObjectAllocator::Instantiate<Actor>(mBlueprint->GetClass());
	mBlueprint->CopyProperties(mLiveActor);

	mTmpCompName = "";
	mCanRenameOrAddComp = false;

	mPendingRemove = nullptr;
	mPendingRename = nullptr;
	mShowRenamePopup = false;
	
	mSelected = mLiveActor;
}

ActorBlueprintEditorWindow::~ActorBlueprintEditorWindow()
{
	ObjectAllocator::Destroy(mLiveActor);
}

void ActorBlueprintEditorWindow::RenderContent()
{
	if (!IsPendingSave())
		ImGui::BeginDisabled();

	bool click = ImGui::Button("Save");

	if (!IsPendingSave())
		ImGui::EndDisabled();

	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Spacing();

	
	// groovy stuff
	{
		ImGui::Columns(3);
		ImGui::BeginChild("Actor & Components");
		ImGui::Text("ACTOR AND COMPONENTS");
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();

		// add component
		{
			if (ImGui::Button("Add component"))
			{
				mTmpCompName = "My new component";
				mCanRenameOrAddComp = 
					!mTmpCompName.empty() &&
					std::count(mTmpCompName.begin(), mTmpCompName.end(), ' ') < mTmpCompName.length() &&
					!mLiveActor->GetComponent(mTmpCompName);
				ImGui::OpenPopup("Add component");
			}

			if (ImGui::BeginPopupModal("Add component"))
			{
				if (!mCanRenameOrAddComp)
					ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 0.7f, 0.0f, 1.0f });

				bool needCheck = ImGui::InputText("##add_comp_name", &mTmpCompName);

				if (!mCanRenameOrAddComp)
					ImGui::PopStyleColor();

				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();

				if (!mCanRenameOrAddComp)
					ImGui::BeginDisabled();

				extern ClassDB gClassDB;
				for (const GroovyClass* gClass : gClassDB.GetClasses())
				{
					if (classUtils::IsA(gClass, ActorComponent::StaticClass()))
					{
						if (ImGui::Selectable(gClass->name.c_str()))
						{
							mLiveActor->__internal_Editor_AddEditorcomponent_BP((GroovyClass*)gClass, mTmpCompName);
							FlagPendingSave();
							break;
						}
					}
				}

				if (!mCanRenameOrAddComp)
					ImGui::EndDisabled();

				if (needCheck)
				{
					mCanRenameOrAddComp =
						!mTmpCompName.empty() &&
						std::count(mTmpCompName.begin(), mTmpCompName.end(), ' ') < mTmpCompName.length() &&
						!mLiveActor->GetComponent(mTmpCompName);
				}

				if (ImGui::IsKeyPressed(ImGuiKey_Escape))
					ImGui::CloseCurrentPopup();

				ImGui::EndPopup();
			}
		}

		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();

		// components
		{
			if (ImGui::Selectable(mLiveActor->GetClass()->name.c_str(), mSelected == mLiveActor))
				mSelected = mLiveActor;

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			for (ActorComponent* component : mLiveActor->GetComponents())
			{
				std::string name = component->GetName();
				bool isInherited = component->GetType() == ACTOR_COMPONENT_TYPE_NATIVE;

				if (isInherited)
					name += " (inherited)";

				if (ImGui::Selectable(name.c_str(), mSelected == component))
					mSelected = component;

				if (!isInherited && ImGui::BeginPopupContextItem(name.c_str(), ImGuiPopupFlags_MouseButtonRight))
				{
					if (ImGui::Selectable("Rename"))
					{
						mShowRenamePopup = true;
						mPendingRename = component;
					}
					else if (ImGui::Selectable("Remove"))
					{
						mPendingRemove = component;
					}

					ImGui::EndPopup();
				}
				else if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip(component->GetClass()->name.c_str());
				}
			}

			// rename / remove
			{

				if (mShowRenamePopup)
				{
					ImGui::OpenPopup("Rename component");
					mTmpCompName = mPendingRename->GetName();
					mCanRenameOrAddComp =
						!mTmpCompName.empty() &&
						std::count(mTmpCompName.begin(), mTmpCompName.end(), ' ') < mTmpCompName.length() &&
						!mLiveActor->GetComponent(mTmpCompName);
					mShowRenamePopup = false;
				}

				if (ImGui::BeginPopupModal("Rename component", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
				{
					if (!mCanRenameOrAddComp)
						ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 0.7f, 0.0f, 1.0f });

					bool needCheck = ImGui::InputText("##rename_comp_name", &mTmpCompName);

					if (!mCanRenameOrAddComp)
						ImGui::PopStyleColor();

					if (!mCanRenameOrAddComp)
						ImGui::BeginDisabled();

					if (ImGui::Button("Rename##rename_comp_btn"))
					{
						mLiveActor->__internal_Editor_RenameEditorComponent(mPendingRename, mTmpCompName);
						mPendingRename = nullptr;
						FlagPendingSave();
						ImGui::CloseCurrentPopup();
					}

					if (!mCanRenameOrAddComp)
						ImGui::EndDisabled();

					if (needCheck)
					{
						mCanRenameOrAddComp =
							!mTmpCompName.empty() &&
							std::count(mTmpCompName.begin(), mTmpCompName.end(), ' ') < mTmpCompName.length() &&
							!mLiveActor->GetComponent(mTmpCompName);
					}

					if (ImGui::IsKeyPressed(ImGuiKey_Escape))
					{
						mPendingRename = nullptr;
						ImGui::CloseCurrentPopup();
					}

					ImGui::EndPopup();
				}
				else if (mPendingRemove)
				{
					mLiveActor->__internal_Editor_RemoveEditorComponent(mPendingRemove);
					if (mSelected == mPendingRemove)
						mSelected = mLiveActor;
					mPendingRemove = nullptr;
					FlagPendingSave();
				}
			}
		}

		ImGui::EndChild();
		ImGui::NextColumn();
		ImGui::BeginChild("Viewport");
		ImGui::Text("VIEWPORT");
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();


		ImGui::EndChild();
		ImGui::NextColumn();
		ImGui::BeginChild("Properties");
		ImGui::Text("PROPERTIES");
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();

		bool transformChanged = false;
		if (SceneComponent* sceneComp = Cast<SceneComponent>(mSelected))
		{
			ImGui::Text("Transform (relative)");
			transformChanged = editorGui::Transform("##scene_comp_transform", &sceneComp->Editor_TransformRef());
			ImGui::Spacing();
			ImGui::Spacing();
		}
		bool propsChanged = editorGui::PropertiesAllClasses(mSelected);
		if (transformChanged || propsChanged)
		{
			FlagPendingSave();
		}

		ImGui::EndChild();
		ImGui::Columns();
	}

	if (click)
	{
		Save();
	}
}

void ActorBlueprintEditorWindow::Save()
{
	mBlueprint->RebuildPack(mLiveActor);
	mBlueprint->Save();
	AssetEditorWindow::Save();
}

extern GroovyProject gProj;

ProjectSettingsWindow::ProjectSettingsWindow()
	: EditorWindow()
{
	mProjName = gProj.GetName();
	mStartupScene = gProj.GetStartupScene();
}

void ProjectSettingsWindow::RenderContent()
{
	float colWidth = ImGui::GetContentRegionAvail().x / 100 * 30;
	editorGui::PropertyInput("Project name", PROPERTY_TYPE_STRING, &mProjName, false, colWidth);
	editorGui::PropertyInput("Startup scene", PROPERTY_TYPE_ASSET_REF, &mStartupScene, false, colWidth, ASSET_TYPE_SCENE);

	bool invalidName = mProjName.empty() || std::count(mProjName.begin(), mProjName.end(), ' ') == mProjName.length();

	if (invalidName)
		ImGui::BeginDisabled();

	ImGui::Spacing();
	ImGui::Spacing();
	if (ImGui::Button("Save settings"))
	{
		gProj.__internal_Editor_Rename(mProjName);
		gProj.SetStartupScene(mStartupScene);
	}

	if (invalidName)
		ImGui::EndDisabled();
}

void EditorSettingsWindow::RenderContent()
{
	extern EditorSettings gEditorSettings;

	float colWidth = ImGui::GetContentRegionAvail().x / 100 * 30;
	ImGui::Columns(2);

	ImGui::Text("Editor camera FOV");
	
	ImGui::NextColumn();
	
	ImGui::DragFloat("##editor_camera_fov", &gEditorSettings.mEditorCameraFOV, 0.1f, 0.1f, 360.f);
	ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort) && ImGui::BeginTooltip())
	{
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted("Field of view of the editor camera, expressed in degrees");
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}

	ImGui::NextColumn();

	ImGui::Text("Editor camera move speed");

	ImGui::NextColumn();
	
	ImGui::DragFloat("##editor_camera_move_speed", &gEditorSettings.mEditorCameraMoveSpeed, 0.1f, 0.001f, 1000.0f);
	ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort) && ImGui::BeginTooltip())
	{
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted("Move speed of the editor camera");
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}

	ImGui::NextColumn();

	ImGui::Text("Editor camera rotation speed");
	
	ImGui::NextColumn();

	ImGui::DragFloat("##editor_camera_rot_speed", &gEditorSettings.mEditorCameraRotationSpeed, 0.1f, 0.001f, 1000.0f);
	ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort) && ImGui::BeginTooltip())
	{
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted("Rotation speed of the editor camera");
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}

	ImGui::Columns();

	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Spacing();

	if (ImGui::Button("Reset to defaults"))
	{
		EditorSettings* editorSettingsCDO = (EditorSettings*)EditorSettings::StaticCDO();
		gEditorSettings.mEditorCameraFOV = editorSettingsCDO->mEditorCameraFOV;
		gEditorSettings.mEditorCameraMoveSpeed = editorSettingsCDO->mEditorCameraMoveSpeed;
		gEditorSettings.mEditorCameraRotationSpeed = editorSettingsCDO->mEditorCameraRotationSpeed;
	}
}
