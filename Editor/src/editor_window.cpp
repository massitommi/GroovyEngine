#include "editor_window.h"
#include "vendor/imgui/imgui.h"
#include "vendor/imgui/misc/cpp/imgui_stdlib.h"
#include "platform/messagebox.h"
#include "classes/object_serializer.h"
#include "assets/asset_serializer.h"
#include "assets/assets.h"
#include "project/project.h"
#include "classes/class.h"
#include "classes/class_db.h"
#include "renderer/mesh.h"

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
	if (!mat)
	{
		mMaterial = new Material();
		mMaterial->FixForRendering();
		check(mMaterial->Validate());
		SetPendingSave(true);
		mExistsOnDisk = false;
		mFileName = "new_material" GROOVY_ASSET_EXT;
	}
	else
	{
		mFileName = AssetManager::Get(mMaterial->GetUUID()).name;
		mMaterial->FixForRendering();
	}
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
		AssetSerializer::SerializeMesh(mMesh, (gProj.assets / mFileName).string());
	}
}

void PrintClasses(std::vector<GroovyClass*> classes)
{
	ImGui::Spacing();
	ImGui::Separator();
	for (GroovyClass* c : classes)
	{
		ImGui::Text("Name: %s", c->name.c_str());
		ImGui::Text("Size: %i bytes", c->size);
		ImGui::Text("Super class: %s", c->super ? c->super->name.c_str() : "NONE");
		ImGui::Text("CDO (Class Default Object): %p", c->cdo);

		ImGui::Separator();
	}
	ImGui::Spacing();
}

extern ClassDB gClassDB;
extern std::vector<GroovyClass*> ENGINE_CLASSES;
extern std::vector<GroovyClass*> GAME_CLASSES;

void ClassRegistryWindow::RenderContent()
{
	if (ImGui::CollapsingHeader("Engine classes", ImGuiTreeNodeFlags_DefaultOpen))
	{
		PrintClasses(ENGINE_CLASSES);
	}
	if (ImGui::CollapsingHeader("Game classes", ImGuiTreeNodeFlags_DefaultOpen))
	{
		//PrintClasses(GAME_CLASSES);
	}
}


GroovyClass* ClassInspectorWindow::sSelectedClass = nullptr;
bool ClassInspectorWindow::sShowInheritedProps = false;
std::vector<GroovyProperty> ClassInspectorWindow::sSelectedClassInheritedProps;
std::vector<GroovyProperty> ClassInspectorWindow::sSelectedClassProps;
std::vector<GroovyClass*> ClassInspectorWindow::sClasses;

ClassInspectorWindow::ClassInspectorWindow()
	: EditorWindow("Class Inspector")
{
	if (!sClasses.size())
	{
		sClasses = gClassDB.GetClasses();
		checkslow(sClasses.size());
		sSelectedClass = sClasses[0];
		UpdateData();
	}
}

void ClassInspectorWindow::UpdateData()
{
	sSelectedClassInheritedProps = gClassDB[sSelectedClass];
	sSelectedClassProps.clear();
	sSelectedClass->propertiesGetter(sSelectedClassProps);
}

const char* GetPropertyTypeStr(EPropertyType type)
{
	switch (type)
	{
		case PROPERTY_TYPE_INT32:		return "INT32";
		case PROPERTY_TYPE_INT64:		return "INT64";
		case PROPERTY_TYPE_UINT32:		return "UINT32";
		case PROPERTY_TYPE_UINT64:		return "UINT64";
		case PROPERTY_TYPE_BOOL:		return "BOOL";
		case PROPERTY_TYPE_FLOAT:		return "FLOAT";
		case PROPERTY_TYPE_STRING:		return "STRING";
		case PROPERTY_TYPE_VEC3:		return "VEC3";
		case PROPERTY_TYPE_TRANSFORM:	return "TRANSFORM";
		case PROPERTY_TYPE_BUFFER:		return "BUFFER";
		case PROPERTY_TYPE_ASSET_REF:	return "ASSET_REF";
		case PROPERTY_TYPE_INTERNAL_SUBMESHDATA:	return "INTERNAL_SUBMESHDATA";
	}
	check(0);
	return "Unknown";
}


void ClassInspectorWindow::RenderContent()
{
	if (ImGui::BeginCombo("Class", sSelectedClass->name.c_str()))
	{
		for (GroovyClass* c : sClasses)
		{
			bool currentlySelected = c == sSelectedClass;
			if (ImGui::Selectable(c->name.c_str(), &currentlySelected))
			{
				sSelectedClass = c;
				UpdateData();
			}
		}
		ImGui::EndCombo();
	}

	ImGui::Checkbox("Include inherited properties", &sShowInheritedProps);

	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::Text("Reflected properties:");
	
	ImGui::Spacing();
	ImGui::Spacing();

	std::vector<GroovyProperty>* props = sShowInheritedProps ? &sSelectedClassInheritedProps : &sSelectedClassProps;

	ImGui::Separator();
	ImGui::Spacing();
	for (const GroovyProperty& p : *props)
	{
		std::string varName = p.name;
		if (p.arrayCount > 1)
			varName += " [" + std::to_string(p.arrayCount) + "]";
		else if (p.arrayCount < 1)
			varName += " (dynamic array)";

		if (p.flags & PROPERTY_FLAG_NO_SERIALIZE)
			ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Text, 0xff00ffff);

		if (ImGui::BeginCombo(varName.c_str(), GetPropertyTypeStr(p.type)))
			ImGui::EndCombo();

		if (p.flags & PROPERTY_FLAG_NO_SERIALIZE)
			ImGui::PopStyleColor();

		ImGui::Spacing();
		ImGui::Separator();
	}
	
}
