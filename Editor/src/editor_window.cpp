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
#include "classes/blueprint.h"

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

extern Project gProj;

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
		AssetSerializer::SerializeMaterial(mMaterial, (gProj.assets / mFileName).string());
		if (!mExistsOnDisk)
		{
			AssetManager::Editor_OnImport(mFileName, ASSET_TYPE_MATERIAL);
			mExistsOnDisk = true;
		}
		SetPendingSave(false);
	}
}

const char* AssetRegistryWindow::AssetTypeStr(EAssetType type)
{
	switch (type)
	{
		case ASSET_TYPE_TEXTURE:	return "TEXTURE";
		case ASSET_TYPE_SHADER:		return "SHADER";
		case ASSET_TYPE_MATERIAL:	return "MATERIAL";
		case ASSET_TYPE_MESH:		return "MESH";
	}
	return "NONE";
}

void AssetRegistryWindow::RenderContent()
{
	ImGui::Spacing();
	ImGui::Separator();

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

BlueprintEditorWindow::BlueprintEditorWindow(Blueprint* blueprint)
	: EditorWindow("Blueprint editor"), mBlueprint(blueprint), mExistsOnDisk(true)
{
	checkslow(blueprint);
	mClass = blueprint->GetClass();
	
	mObjInstance = (GroovyObject*)malloc(mClass->size);
	mClass->constructor(mObjInstance);
}

BlueprintEditorWindow::BlueprintEditorWindow(GroovyClass* gClass)
	: EditorWindow("Blueprint editor"), mExistsOnDisk(false)
{
	checkslow(gClass);
	mClass = gClass;
	mBlueprint = new Blueprint(gClass);
	
	mObjInstance = (GroovyObject*)malloc(mClass->size);
	mClass->constructor(mObjInstance);
}

BlueprintEditorWindow::~BlueprintEditorWindow()
{
	mClass->destructor(mObjInstance);
	free(mObjInstance);

	if (!mExistsOnDisk)
		delete mBlueprint;
}
