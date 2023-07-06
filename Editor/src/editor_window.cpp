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
#include "classes/reflection.h"
#include "runtime/object_allocator.h"
#include "gameframework/actor.h"
#include "gameframework/actorcomponent.h"

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

extern void FlagRegistryPendingSave();

namespace GroovyGui
{
	bool AssetRef(const char* label, EAssetType type, void* data, GroovyClass* classFilter)
	{
		AssetInstance* assetPtr = *(AssetInstance**)data;
		std::vector<AssetHandle> assets;
		// filter stuff out
		if (classFilter)
		{
			for (const AssetHandle& handle : AssetManager::Editor_GetAssets())
			{
				if (handle.type == ASSET_TYPE_BLUEPRINT || handle.type == ASSET_TYPE_ACTOR_BLUEPRINT)
				{
					Blueprint* bp = (Blueprint*)handle.instance;
					if (classUtils::IsA(bp->GetClass(), classFilter))
					{
						assets.push_back(handle);
					}
				}
			}
		}
		else
		{
			assets = AssetManager::Editor_GetAssets(type);
		}

		// draw 

		std::string current = "NONE";
		if (assetPtr)
			current = AssetManager::Get(assetPtr->GetUUID()).name;

		if (ImGui::BeginCombo(label, current.c_str()))
		{
			if (ImGui::Selectable("NONE"))
			{
				assetPtr = nullptr;
				return true;
			}
			for (const AssetHandle& handle : assets)
			{
				if (ImGui::Selectable(handle.name.c_str()))
				{
					assetPtr = handle.instance;
					return true;
				}
			}
		}

		return false;
	}
	bool Transform(const char* label, void* data, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f)
	{
		struct Transform* t = (struct Transform*)data;
		bool click = false;
		click = click || ImGui::DragFloat3("##location", &t->location.x, v_speed, v_min, v_max);
		click = click || ImGui::DragFloat3("##rotation", &t->rotation.x, v_speed, v_min, v_max);
		click = click || ImGui::DragFloat3("##scale", &t->scale.x, v_speed, v_min, v_max);
		return click;
	}
}

bool PropertyInput(const std::string& label, EPropertyType type, void* data, bool readonly, uint64 param1 = 0, uint64 param2 = 0)
{
	if (readonly)
		ImGui::BeginDisabled();

	bool click = false;

	std::string lblVal = "##" + label;

	ImGui::Text(label.c_str());
	ImGui::SameLine();

	switch (type)
	{
		case PROPERTY_TYPE_INT32:
			click = ImGui::InputScalar(lblVal.c_str(), ImGuiDataType_S32, data, 0, 0, 0);
			break;
		case PROPERTY_TYPE_INT64:
			click = ImGui::InputScalar(lblVal.c_str(), ImGuiDataType_S64, data, 0, 0, 0);
			break;
		case PROPERTY_TYPE_UINT32:
			click = ImGui::InputScalar(lblVal.c_str(), ImGuiDataType_U32, data, 0, 0, 0);
			break;
		case PROPERTY_TYPE_UINT64:
			click = ImGui::InputScalar(lblVal.c_str(), ImGuiDataType_U64, data, 0, 0, 0);
			break;
		case PROPERTY_TYPE_FLOAT:
			click = ImGui::InputScalar(lblVal.c_str(), ImGuiDataType_Float, data, 0, 0, 0);
			break;
		case PROPERTY_TYPE_BOOL:
			click = ImGui::Checkbox(lblVal.c_str(), (bool*)data);
			break;
		case PROPERTY_TYPE_VEC2:
			click = ImGui::DragFloat2(lblVal.c_str(), (float*)data, 1.0f, 0.0f, 0.0f);
			break;
		case PROPERTY_TYPE_VEC3:
			click = ImGui::DragFloat3(lblVal.c_str(), (float*)data, 1.0f, 0.0f, 0.0f);
			break;
		case PROPERTY_TYPE_VEC4:
			click = ImGui::DragFloat4(lblVal.c_str(), (float*)data, 1.0f, 0.0f, 0.0f);
			break;
		case PROPERTY_TYPE_TRANSFORM:
			click = GroovyGui::Transform(lblVal.c_str(), data, 1.0f, 0.0f, 0.0f);
			break;
		case PROPERTY_TYPE_STRING:
			click = ImGui::InputText(lblVal.c_str(), (std::string*)data, readonly ? ImGuiInputTextFlags_ReadOnly : 0);
			break;
		case PROPERTY_TYPE_ASSET_REF:
			click = GroovyGui::AssetRef(lblVal.c_str(), (EAssetType)param1, data, (GroovyClass*)param2);
			break;
	default:
		ImGui::Text("PROPERTY_TYPE_UI_NOT_IMPLEMENTED");
		click = false;
		break;
	}

	if (readonly)
		ImGui::EndDisabled();

	return click;
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
		AssetSerializer::SerializeGenericAsset(mMaterial, (gProj.assets / mFileName).string());
		if (!mExistsOnDisk)
		{
			AssetManager::Editor_OnImport(mFileName, ASSET_TYPE_MATERIAL);
			mExistsOnDisk = true;
			FlagRegistryPendingSave();
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

ObjectBlueprintEditorWindow::ObjectBlueprintEditorWindow(ObjectBlueprint* blueprint)
	: EditorWindow("Blueprint editor"), mBlueprint(blueprint), mExistsOnDisk(true)
{
	checkslow(blueprint);

	mFileName = AssetManager::Get(blueprint->GetUUID()).name;
	
	mObjInstance = ObjectAllocator::Instantiate(blueprint->GetObjectClass());

	mBlueprint->CopyProperties(mObjInstance);
}

ObjectBlueprintEditorWindow::ObjectBlueprintEditorWindow(GroovyClass* gClass)
	: EditorWindow("Blueprint editor"), mExistsOnDisk(false)
{
	checkslow(gClass);
	mBlueprint = new ObjectBlueprint();
	mBlueprint->Editor_ClassRef() = gClass;

	mFileName = "new_blueprint_" + gClass->name + GROOVY_ASSET_EXT;
	
	mObjInstance = ObjectAllocator::Instantiate(gClass);
}

ObjectBlueprintEditorWindow::~ObjectBlueprintEditorWindow()
{
	ObjectAllocator::Destroy(mObjInstance);

	if (!mExistsOnDisk)
		delete mBlueprint;
}

void Property(const GroovyProperty& prop, void* propData)
{
	ImGui::PushID(propData);
	bool readonly = prop.flags & PROPERTY_FLAG_EDITOR_READONLY;

	if (prop.flags & PROPERTY_FLAG_IS_ARRAY)
	{
		ImGui::PushStyleColor(ImGuiCol_Header, { 1.0f, 1.0f, 1.0f, 0.0f });
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, { 1.0f, 1.0f, 1.0f, 0.1f });
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, { 1.0f, 1.0f, 1.0f, 0.1f });
		if (ImGui::CollapsingHeader(prop.name.c_str()))
		{
			uint32 propSize = reflectionUtils::GetPropertySize(prop.type);
			if (prop.flags & PROPERTY_FLAG_IS_DYNAMIC_ARRAY)
			{
				DynamicArrayPtr arrayPtr = reflectionUtils::GetDynamicArrayPtr(prop.type);
				void* dataData = arrayPtr.data(propData);
				uint32 dataArrayCount = arrayPtr.size(propData);

				struct DynamicArrayPostRenderAction
				{
					uint32 action;
					uint32 param;
				};

				enum EPostRenderDynamicArrayAction
				{
					NONE,
					CLEAR,
					ADD,
					REMOVE_AT,
					INSERT_AT
				};

				DynamicArrayPostRenderAction postAction = { NONE , 0 };

				for (uint32 i = 0; i < dataArrayCount; i++)
				{
					std::string labelName = "[" + std::to_string(i) + "]";
					PropertyInput(labelName, prop.type, (byte*)dataData + (propSize * i), readonly, prop.param1, prop.param2);
					ImGui::SameLine();
					ImGui::PushID(i);
					if (ImGui::Button("X"))
					{
						postAction.action = REMOVE_AT;
						postAction.param = i;
					}
					ImGui::SameLine();
					if (ImGui::Button("+"))
					{
						postAction.action = INSERT_AT;
						postAction.param = i;
					}
					ImGui::PopID();
				}

				if (ImGui::Button("Clear"))
				{
					postAction.action = CLEAR;
				}
				ImGui::SameLine();
				if (ImGui::Button("Add"))
				{
					postAction.action = ADD;
				}

				// take action 
				switch (postAction.action)
				{
				case NONE:
					break;
				case CLEAR:
					arrayPtr.clear(propData);
					break;
				case REMOVE_AT:
					arrayPtr.removeAt(propData, postAction.param);
					break;
				case INSERT_AT:
					arrayPtr.insertAt(propData, postAction.param);
					break;
				case ADD:
					arrayPtr.add(propData);
					break;
				}
			}
			else
			{
				for (uint32 i = 0; i < prop.arrayCount; i++)
				{
					std::string labelName = "[" + std::to_string(i) + "]";
					PropertyInput(labelName, prop.type, (byte*)propData + (propSize * i), readonly, prop.param1, prop.param2);
				}
			}
		}
		ImGui::PopStyleColor(3);
	}
	else
	{
		PropertyInput(prop.name, prop.type, propData, readonly, prop.param1, prop.param2);
	}
	ImGui::PopID();
}

void PropertiesSingleClass(GroovyObject* obj, GroovyClass* gClass, const std::vector<GroovyProperty>& props)
{
	ImGui::Text(gClass->name.c_str());
	ImGui::Spacing();

	for (const GroovyProperty& prop : props)
	{
		ImGui::Spacing();
		ImGui::Spacing();
		Property(prop, (byte*)obj + prop.offset);
		ImGui::Spacing();
		ImGui::Spacing();
	}
}

void PropertiesAllClasses(GroovyObject* obj)
{
	GroovyClass* superClass = obj->GetClass();

	while (superClass)
	{
		std::vector<GroovyProperty> props;
		superClass->propertiesGetter(props);

		if (props.size())
		{
			ImGui::Spacing();
			ImGui::Separator();

			PropertiesSingleClass(obj, superClass, props);
		}

		superClass = superClass->super;
	} 
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
		mBlueprint->SetData(mObjInstance);
		AssetSerializer::SerializeGenericAsset(mBlueprint, (gProj.assets / mFileName).string());
		if (!mExistsOnDisk)
		{
			AssetManager::Editor_OnImport(mFileName, ASSET_TYPE_BLUEPRINT);
			mExistsOnDisk = true;
			FlagRegistryPendingSave();
		}
		SetPendingSave(false);
	}

	ImGui::Spacing();
	ImGui::Spacing();

	PropertiesAllClasses(mObjInstance);
}

ActorBlueprintEditorWindow::ActorBlueprintEditorWindow(ActorBlueprint* blueprint)
	: EditorWindow("Actor blueprint editor"), mBlueprint(blueprint), mActorInstance(nullptr), mExistsOnDisk(true)
{
	checkslow(blueprint);

	mFileName = AssetManager::Get(blueprint->GetUUID()).name;

	mActorInstance = ObjectAllocator::Instantiate<Actor>(blueprint->GetActorClass());
	mBlueprint->CopyProperties(mActorInstance);

	mSelected = mActorInstance;
}

ActorBlueprintEditorWindow::ActorBlueprintEditorWindow(GroovyClass* inClass)
	: EditorWindow("Actor blueprint editor"), mBlueprint(nullptr), mActorInstance(nullptr), mExistsOnDisk(false)
{
	checkslow(inClass);
	checkslow(classUtils::IsA(inClass, Actor::StaticClass()));

	mFileName = "new_blueprint_" + inClass->name + GROOVY_ASSET_EXT;

	mBlueprint = new ActorBlueprint();
	mBlueprint->Editor_ActorClassRef() = inClass;

	mActorInstance = ObjectAllocator::Instantiate<Actor>(inClass);

	mSelected = mActorInstance;
}

ActorBlueprintEditorWindow::~ActorBlueprintEditorWindow()
{
	ObjectAllocator::Destroy(mActorInstance);

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
		mBlueprint->SetData(mActorInstance);
		AssetSerializer::SerializeGenericAsset(mBlueprint, (gProj.assets / mFileName).string());
		if (!mExistsOnDisk)
		{
			AssetManager::Editor_OnImport(mFileName, ASSET_TYPE_ACTOR_BLUEPRINT);
			mExistsOnDisk = true;
			FlagRegistryPendingSave();
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
	if (ImGui::Selectable(lbl.c_str(), mSelected == mActorInstance))
	{
		mSelected = mActorInstance;
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	for (ActorComponent* comp : mActorInstance->GetComponents())
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

	PropertiesAllClasses(mSelected);
	
	ImGui::EndChild();
	
	ImGui::Columns();
}
