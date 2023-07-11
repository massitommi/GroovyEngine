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

extern GroovyProject gProj;

void EditorWindow::RenderWindow()
{
	ImGui::Begin(mTitle.c_str(), &mOpen, mFlags);
	RenderContent();
	ImGui::End();

	if (!mOpen)
	{
		bool shouldClose = OnClose();
		mOpen = !shouldClose;
	}
}

bool EditorWindow::OnClose()
{
	if (!mPendingSave)
		return true;
	EMessageBoxResponse res = SysMessageBox::Show("Unsaved work", "Unsaved work will be lost, continue?", MESSAGE_BOX_TYPE_WARNING, MESSAGE_BOX_OPTIONS_YESNOCANCEL);
	return res == MESSAGE_BOX_RESPONSE_YES;
}

void EditorWindow::SetPendingSave(bool pendingSave)
{
	if (pendingSave)
		mFlags |= ImGuiWindowFlags_UnsavedDocument;
	else
		mFlags &= ~ImGuiWindowFlags_UnsavedDocument;

	mPendingSave = pendingSave;
}

EditMaterialWindow::EditMaterialWindow(Material* mat)
	: EditorWindow("Material editor"), mMaterial(mat), mExistsOnDisk(true)
{
	mFileName = AssetManager::Get(mMaterial->GetUUID()).name;
	mMaterial->FixForRendering();
}

EditMaterialWindow::EditMaterialWindow(Shader* shader)
	: EditorWindow("Material editor"), mExistsOnDisk(false)
{
	checkslow(shader);
	mMaterial = new Material();
	mMaterial->SetShader(shader);
	mMaterial->FixForRendering();
	SetPendingSave(true);
	mFileName = "new_material" GROOVY_ASSET_EXT;
}

EditMaterialWindow::~EditMaterialWindow()
{
	if (!mExistsOnDisk)
	{
		delete mMaterial;
	}
}

void EditMaterialWindow::RenderContent()
{
	ImGui::Separator();

	std::vector<AssetHandle> textures = AssetManager::Editor_GetAssets(ASSET_TYPE_TEXTURE);

	for (MaterialResource& res : mMaterial->Editor_ResourcesRef())
	{
		ImGui::Text(res.name.c_str());
		ImGui::Spacing();

		std::string resName = AssetManager::Get(res.res->GetUUID()).name;

		if (ImGui::BeginCombo(res.name.c_str(), resName.c_str()))
		{
			for (const AssetHandle& tex : textures)
			{
				bool selected = res.res == tex.instance;
				if (ImGui::Selectable(tex.name.c_str(), &selected))
				{
					res.res = (Texture*)tex.instance;
				}
			}

			ImGui::EndCombo();
		}

		ImGui::Separator();
	}

	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Spacing();

	if (mExistsOnDisk)
		ImGui::BeginDisabled();

	ImGui::InputText("Filename", &mFileName);

	if (mExistsOnDisk)
		ImGui::EndDisabled();

	ImGui::Spacing();

	if (ImGui::Button(mExistsOnDisk ? "Save changes" : "Save"))
	{
		AssetSerializer::SerializeGenericAsset(mMaterial, (gProj.GetAssetsPath() / mFileName).string());
		if (!mExistsOnDisk)
		{
			AssetManager::Editor_OnImport(mFileName, ASSET_TYPE_MATERIAL);
			mExistsOnDisk = true;
			editor::FlagRegistryPendingSave();
		}
		SetPendingSave(false);
	}
}

void AssetRegistryWindow::RenderContent()
{
	ImGui::Spacing();
	ImGui::Separator();

	auto AssetTypeStr = [](EAssetType type)
	{
		return editor::utils::AssetTypeToStr(type);
	};

	for (const auto& [uuid, handle] : AssetManager::Editor_GetRegistry())
	{
		ImGui::InputText("Name", (std::string*)&(handle.name), ImGuiInputTextFlags_ReadOnly);
		ImGui::InputText("Type", &(std::string)AssetTypeStr(handle.type), ImGuiInputTextFlags_ReadOnly);
		ImGui::InputText("UUID", &std::to_string(handle.uuid), ImGuiInputTextFlags_ReadOnly);

		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
	}

	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Text("Debug add groovyasset");
	ImGui::InputText("Asset name (relative to assets folder)", &mDebugAssetName);
	if (ImGui::BeginCombo("Asset type", AssetTypeStr(mDebugAssetType)))
	{
		if (ImGui::Selectable(AssetTypeStr(ASSET_TYPE_TEXTURE), mDebugAssetType == ASSET_TYPE_TEXTURE))
			mDebugAssetType = ASSET_TYPE_TEXTURE;

		if (ImGui::Selectable(AssetTypeStr(ASSET_TYPE_SHADER), mDebugAssetType == ASSET_TYPE_SHADER))
			mDebugAssetType = ASSET_TYPE_SHADER;

		if (ImGui::Selectable(AssetTypeStr(ASSET_TYPE_MATERIAL), mDebugAssetType == ASSET_TYPE_MATERIAL))
			mDebugAssetType = ASSET_TYPE_MATERIAL;

		if (ImGui::Selectable(AssetTypeStr(ASSET_TYPE_MESH), mDebugAssetType == ASSET_TYPE_MESH))
			mDebugAssetType = ASSET_TYPE_MESH;

		if (ImGui::Selectable(AssetTypeStr(ASSET_TYPE_BLUEPRINT), mDebugAssetType == ASSET_TYPE_BLUEPRINT))
			mDebugAssetType = ASSET_TYPE_BLUEPRINT;

		if (ImGui::Selectable(AssetTypeStr(ASSET_TYPE_ACTOR_BLUEPRINT), mDebugAssetType == ASSET_TYPE_ACTOR_BLUEPRINT))
			mDebugAssetType = ASSET_TYPE_ACTOR_BLUEPRINT;

		if (ImGui::Selectable(AssetTypeStr(ASSET_TYPE_SCENE), mDebugAssetType == ASSET_TYPE_SCENE))
			mDebugAssetType = ASSET_TYPE_SCENE;

		ImGui::EndCombo();
	}

	bool canAdd = !mDebugAssetName.empty();

	if (!canAdd)
		ImGui::BeginDisabled();

	if (ImGui::Button("Add to registry"))
	{
		AssetManager::Editor_OnImport(mDebugAssetName, mDebugAssetType);
		AssetManager::SaveRegistry();
	}
	
	if (!canAdd)
		ImGui::EndDisabled();
}

void TexturePreviewWindow::RenderContent()
{
	ImGui::Image(mTexture->GetRendererID(), ImGui::GetContentRegionAvail());
}

MeshPreviewWindow::MeshPreviewWindow(Mesh* mesh)
	: EditorWindow("Mesh viewer"), mMesh(mesh)
{
	check(mesh);
	mesh->FixForRendering();
	mFileName = AssetManager::Get(mesh->GetUUID()).name;
}

void MeshPreviewWindow::RenderContent()
{
	ImGui::Separator();

	std::vector<AssetHandle> assets = AssetManager::Editor_GetAssets(ASSET_TYPE_MATERIAL);

	std::vector<Material*>& mats = mMesh->Editor_MaterialsRef();

	for (uint32 i = 0; i < mats.size(); i++)
	{
		Material*& mat = mats[i];

		AssetHandle currentMatHandle = AssetManager::Get(mat->GetUUID());
		std::string dropdropName = "[" + std::to_string(i) + "]";

		if (ImGui::BeginCombo(dropdropName.c_str(), currentMatHandle.name.c_str()))
		{
			for (const AssetHandle& assetMat : assets)
			{
				bool selected = mat == assetMat.instance;
				if (ImGui::Selectable(assetMat.name.c_str(), &selected))
				{
					mat = (Material*)assetMat.instance;
				}
			}

			ImGui::EndCombo();
		}
	}

	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::BeginDisabled();
	ImGui::InputText("Filename", &mFileName);
	ImGui::EndDisabled();

	if (ImGui::Button("Save changes"))
	{
		mMesh->Save();
	}
}

extern ClassDB gClassDB;
extern std::vector<GroovyClass*> ENGINE_CLASSES;
extern std::vector<GroovyClass*> GAME_CLASSES;

ObjectBlueprintEditorWindow::ObjectBlueprintEditorWindow(ObjectBlueprint* blueprint)
	: EditorWindow("Blueprint editor"), mBlueprint(blueprint), mExistsOnDisk(true)
{
	checkslow(blueprint);

	mFileName = AssetManager::Get(blueprint->GetUUID()).name;
}

ObjectBlueprintEditorWindow::ObjectBlueprintEditorWindow(GroovyClass* gClass)
	: EditorWindow("Blueprint editor"), mExistsOnDisk(false)
{
	checkslow(gClass);
	mBlueprint = new ObjectBlueprint();
	mBlueprint->SetupEmpty(gClass);

	mFileName = "new_blueprint_" + gClass->name + GROOVY_ASSET_EXT;
}

ObjectBlueprintEditorWindow::~ObjectBlueprintEditorWindow()
{
	if (!mExistsOnDisk)
		delete mBlueprint;
}

void ObjectBlueprintEditorWindow::RenderContent()
{
	if (mExistsOnDisk)
		ImGui::BeginDisabled();

	ImGui::InputText("Filename", &mFileName);

	if (mExistsOnDisk)
		ImGui::EndDisabled();

	ImGui::Spacing();

	if (ImGui::Button(mExistsOnDisk ? "Save changes" : "Save"))
	{
		mBlueprint->RebuildPack();
		AssetSerializer::SerializeGenericAsset(mBlueprint, (gProj.GetAssetsPath() / mFileName).string());
		if (!mExistsOnDisk)
		{
			AssetManager::Editor_OnImport(mFileName, ASSET_TYPE_BLUEPRINT);
			mExistsOnDisk = true;
			editor::FlagRegistryPendingSave();
		}
		SetPendingSave(false);
	}

	ImGui::Spacing();
	ImGui::Spacing();

	editorGui::PropertiesAllClasses(mBlueprint->GetDefaultObject());
}

ActorBlueprintEditorWindow::ActorBlueprintEditorWindow(ActorBlueprint* blueprint)
	: EditorWindow("Actor blueprint editor"), mBlueprint(blueprint), mExistsOnDisk(true)
{
	checkslow(blueprint);

	mFileName = AssetManager::Get(blueprint->GetUUID()).name;

	mSelected = blueprint->GetDefaultActor();
}

ActorBlueprintEditorWindow::ActorBlueprintEditorWindow(GroovyClass* inClass)
	: EditorWindow("Actor blueprint editor"), mBlueprint(nullptr), mExistsOnDisk(false)
{
	checkslow(inClass);
	checkslow(classUtils::IsA(inClass, Actor::StaticClass()));

	mFileName = "new_blueprint_" + inClass->name + GROOVY_ASSET_EXT;

	mBlueprint = new ActorBlueprint();
	mBlueprint->SetupEmpty(inClass);

	mSelected = mBlueprint->GetDefaultActor();
}

ActorBlueprintEditorWindow::~ActorBlueprintEditorWindow()
{
	if (!mExistsOnDisk)
		delete mBlueprint;
}

void ActorBlueprintEditorWindow::RenderContent()
{
	if (mExistsOnDisk)
		ImGui::BeginDisabled();

	ImGui::InputText("Filename", &mFileName);

	if (mExistsOnDisk)
		ImGui::EndDisabled();

	ImGui::Spacing();

	if (ImGui::Button(mExistsOnDisk ? "Save changes" : "Save"))
	{
		mBlueprint->RebuildPack();
		AssetSerializer::SerializeGenericAsset(mBlueprint, (gProj.GetAssetsPath() / mFileName).string());
		if (!mExistsOnDisk)
		{
			AssetManager::Editor_OnImport(mFileName, ASSET_TYPE_ACTOR_BLUEPRINT);
			mExistsOnDisk = true;
			editor::FlagRegistryPendingSave();
		}
		SetPendingSave(false);
	}

	ImGui::Spacing();
	ImGui::Spacing();

	// render groovy stuff

	ImGui::Columns(3);

	ImGui::BeginChild("Actor & Components");
	ImGui::Text("ACTOR AND COMPONENTS");
	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Spacing();

	std::string lbl = mBlueprint->GetActorClass()->name + " (self)";
	if (ImGui::Selectable(lbl.c_str(), mSelected == mBlueprint->GetDefaultActor()))
	{
		mSelected = mBlueprint->GetDefaultActor();
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	for (ActorComponent* comp : mBlueprint->GetDefaultActor()->GetComponents())
	{
		if (ImGui::Selectable(comp->GetName().c_str(), mSelected == comp))
		{
			mSelected = comp;
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

	if (editorGui::PropertiesAllClasses(mSelected))
	{
		SetPendingSave(true);
	}
	
	ImGui::EndChild();
	
	ImGui::Columns();
}
