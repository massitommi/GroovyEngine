#include "editor_window.h"
#include "vendor/imgui/imgui.h"
#include "vendor/imgui/misc/cpp/imgui_stdlib.h"
#include "platform/messagebox.h"
#include "classes/object_serializer.h"

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

#include "assets/assets.h"
#include "project/project.h"
#include "classes/class.h"
#include "classes/class_db.h"
#include "renderer/mesh.h"

extern Project gProj;

EditMaterialWindow::EditMaterialWindow(Material* mat)
	: EditorWindow("Material editor"), mMaterial(mat)
{
	if (!mat)
	{
		mMaterial = new Material();
		mMaterial->FixForRendering();
		check(mMaterial->Validate());
		SetPendingSave(true);
	}
}

EditMaterialWindow::~EditMaterialWindow()
{
	if (PendingSave())
	{
		delete mMaterial;
	}
}

void EditMaterialWindow::RenderContent()
{
	extern Texture* DEFAULT_TEXTURE;

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
}

//void EditMaterialWindow::ShowVar(const ShaderVariable& var, byte* bufferPtr)
//{
//	byte* varPtr = bufferPtr + var.alignedOffset;
//	static bool vec3AsColor = true;
//	static bool vec4AsColor = true;
//
//	switch (var.type)
//	{
//		case SHADER_VARIABLE_TYPE_FLOAT1:
//			ImGui::DragFloat(var.name.c_str(), (float*)varPtr);
//			break;
//
//		case SHADER_VARIABLE_TYPE_FLOAT2:
//			ImGui::DragFloat2(var.name.c_str(), (float*)varPtr);
//			break;
//
//		case SHADER_VARIABLE_TYPE_FLOAT3:
//			ImGui::Checkbox("Use Vec3 as color", &vec3AsColor);
//			if (vec3AsColor)
//			{
//				ImGui::ColorEdit3(var.name.c_str(), (float*)varPtr);
//			}
//			else
//			{
//				ImGui::DragFloat3(var.name.c_str(), (float*)varPtr);
//			}
//			break;
//
//		case SHADER_VARIABLE_TYPE_FLOAT4:
//			ImGui::Checkbox("Use Vec4 as color", &vec4AsColor);
//			if (vec4AsColor)
//			{
//				ImGui::ColorEdit4(var.name.c_str(), (float*)varPtr);
//			}
//			else
//			{
//				ImGui::DragFloat4(var.name.c_str(), (float*)varPtr);
//			}
//			break;
//
//		default:
//			ImGui::Text("Variable format not implemented!");
//			break;
//	}
//}

const char* AssetRegistryWindow::AssetTypeStr(EAssetType type)
{
	switch (type)
	{
		case ASSET_TYPE_TEXTURE:	return "TEXTURE";
		case ASSET_TYPE_SHADER:		return "SHADER";
		case ASSET_TYPE_MATERIAL:	return "MATERIAL";
		case ASSET_TYPE_MESH:		return "MESH";
	}
	return "UNKNOWN";
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

	/*const auto& reg = AssetManager::GetRegistry();
	ImGui::Separator();
	for (const auto& handle : reg)
	{
		ImGui::Text("Name: %s", handle.name.c_str());
		ImGui::Spacing();
		if (ImGui::BeginCombo("Type", AssetTypeStr(handle.type)))
		{
			ImGui::EndCombo();
		}
		ImGui::Spacing();
		ImGui::Text("Path: %s", handle.path.c_str());
		ImGui::Spacing();
		ImGui::Text("UUID: %s", std::to_string(handle.uuid).c_str());
		ImGui::Spacing();
		ImGui::Text("Instance: %p", handle.instance);

		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Separator();
	}
	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Text("Debug");

	static EAssetType type = ASSET_TYPE_NONE;
	static std::string filePath;

	ImGui::InputText("File path", &filePath);
	if (ImGui::BeginCombo("Asset type", AssetTypeStr(type)))
	{
		ImGui::Selectable(AssetTypeStr(ASSET_TYPE_NONE));
		
		if (ImGui::Selectable(AssetTypeStr(ASSET_TYPE_TEXTURE)))
			type = ASSET_TYPE_TEXTURE;
		if (ImGui::Selectable(AssetTypeStr(ASSET_TYPE_SHADER)))
			type = ASSET_TYPE_SHADER;
		if (ImGui::Selectable(AssetTypeStr(ASSET_TYPE_MESH)))
			type = ASSET_TYPE_MESH;

		ImGui::EndCombo();
	}
	if (type == ASSET_TYPE_NONE || filePath.empty())
		ImGui::BeginDisabled();
	if (ImGui::Button("Add to registry"))
	{
		AssetManager::AddEditorNew(filePath, type);
	}
	if (type == ASSET_TYPE_NONE || filePath.empty())
		ImGui::EndDisabled();*/
}

void TexturePreviewWindow::RenderContent()
{
	ImGui::Image(mTexture->GetRendererID(), ImGui::GetContentRegionAvail());
}

void MeshPreviewWindow::RenderContent()
{
	ImGui::Text("Submeshes");
	ImGui::Separator();

	/*const std::vector<AssetHandle>& matHandles = AssetManager::GetRegistryFiltered(ASSET_TYPE_MATERIAL);
	PropFieldArray propFieldArray;

	for (uint32 i = 0; i < mMesh->GetSubmeshes().size(); i++)
		propFieldArray.fields.push_back({ std::to_string(i), mMesh->GetMaterials()[i] });
	propFieldArray.options.push_back({ "DEFAULT_MATERIAL", DEFAULT_MATERIAL });
	for (const AssetHandle& handle : matHandles)
		propFieldArray.options.push_back({ handle.name, handle.instance });

	RenderAssetSelectionMenu(propFieldArray, [&](uint32 i, uint32 j)
	{
		mMesh->SetMaterial((Material*)propFieldArray.options[j].data, i);
	});

	if (ImGui::Button("Save"))
	{
		const AssetHandle& handle = AssetManager::Get(mMesh->GetUUID());
		AssetSerializer::SerializeMesh(mMesh, handle.path);
	}*/
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
