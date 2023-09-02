#include "editor.h"
#include "vendor/imgui/imgui.h"
#include "vendor/imgui/misc/cpp/imgui_stdlib.h"
#include "assets/asset_manager.h"
#include "classes/class.h"
#include "gameframework/blueprint.h"
#include "gameframework/actor.h"
#include "gameframework/actorcomponent.h"

bool editorGui::AssetRef(const char* label, EAssetType type, void* data, bool allowNull, GroovyClass* classFilter)
{
	AssetInstance** assetPtr = (AssetInstance**)data;
	std::vector<AssetHandle> assets;
	// filter stuff out

	bool isBlueprint = type == ASSET_TYPE_BLUEPRINT || type == ASSET_TYPE_ACTOR_BLUEPRINT;

	if (isBlueprint && classFilter)
	{
		for (const AssetHandle& handle : AssetManager::GetAssets())
		{
			if (handle.type == ASSET_TYPE_BLUEPRINT || handle.type == ASSET_TYPE_ACTOR_BLUEPRINT)
			{
				Blueprint* bp = (Blueprint*)handle.instance;
				if (GroovyClass_IsA(bp->GetClass(), classFilter))
				{
					assets.push_back(handle);
				}
			}
		}
	}
	else
	{
		assets = AssetManager::GetAssets(type);
	}

	// draw 

	std::string current = "NONE";
	if (*assetPtr)
	{
		current = AssetManager::Get((*assetPtr)->GetUUID()).name;
	}

	bool validAsset = true;

	if (*assetPtr == nullptr)
		validAsset = allowNull;
	else if (*assetPtr && isBlueprint && classFilter)
		if (!GroovyClass_IsA(((Blueprint*)(*assetPtr))->GetClass(), classFilter))
			validAsset = false;

	if (!validAsset)
		ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 0.0f, 0.0f, 1.0f });


	bool edited = false;

	if (ImGui::BeginCombo(label, current.c_str()))
	{
		if (!validAsset)
		{
			ImGui::PopStyleColor();
			validAsset = true; // just for imgui
		}

		if (!allowNull)
			ImGui::BeginDisabled();

		if (ImGui::Selectable("NONE", *assetPtr == nullptr))
		{
			if (*assetPtr != nullptr)
			{
				*assetPtr = nullptr;
				edited = true;
			}
		}

		if (!allowNull)
			ImGui::EndDisabled();

		for (const AssetHandle& handle : assets)
		{
			if (ImGui::Selectable(handle.name.c_str(), *assetPtr == handle.instance))
			{
				if (*assetPtr != handle.instance)
				{
					*assetPtr = handle.instance;
					edited = true;
				}
			}
		}
		ImGui::EndCombo();
	}


	if (ImGui::IsItemHovered())
		if (isBlueprint)
			ImGui::SetTooltip("%s / %s", editor::AssetTypeToStr(ASSET_TYPE_BLUEPRINT), editor::AssetTypeToStr(ASSET_TYPE_ACTOR_BLUEPRINT));
		else
			ImGui::SetTooltip("%s", editor::AssetTypeToStr(type));

	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload = ImGui::GetDragDropPayload();
		AssetHandle handle = AssetManager::Get(*(AssetUUID*)payload->Data);

		bool canAccept = handle.type == type;

		if (isBlueprint)
		{
			canAccept = handle.type == ASSET_TYPE_BLUEPRINT || handle.type == ASSET_TYPE_ACTOR_BLUEPRINT;

			Blueprint* bp = (Blueprint*)handle.instance;

			if (classFilter)
			{
				if (!GroovyClass_IsA(bp->GetClass(), classFilter))
					canAccept = false;
			}
		}

		if (canAccept && ImGui::AcceptDragDropPayload("drag_and_drop_asset"))
		{
			*assetPtr = handle.instance;
			edited = true;
		}
		ImGui::EndDragDropTarget();
	}

	if (!validAsset)
		ImGui::PopStyleColor();

	return edited;
}

namespace ImGui
{
	void PushMultiItemsWidths(int components, float w_full);
}

static bool Vec3Control(const char* label, void* data)
{
	// stolen ui code for Vector3 control, thanks Cherno

	ImGui::PushID(data);

	ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Separator, { 0,0,0,0 });

	ImFont* boldFont = ImGui::GetIO().Fonts[0].Fonts[1];

	ImGui::Columns(2);
	ImGui::SetColumnWidth(0, 100.0f);
	ImGui::Text(label);
	ImGui::NextColumn();

	ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

	float lineHeight = ImGui::GetFont()->FontSize + ImGui::GetStyle().FramePadding.y * 2.0f;
	ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };
	bool changed = false;

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
	ImGui::PushFont(boldFont);
	if (ImGui::Button("X", buttonSize))
	{

	}
	ImGui::PopFont();
	ImGui::PopStyleColor(3);

	ImGui::SameLine();

	if (ImGui::DragFloat("##X", &((Vec3*)data)->x, 0.01f, 0.0f, 0.0f))
		changed = true;

	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
	ImGui::PushFont(boldFont);
	if (ImGui::Button("Y", buttonSize))
	{

	}
	ImGui::PopFont();
	ImGui::PopStyleColor(3);

	ImGui::SameLine();

	if (ImGui::DragFloat("##Y", &((Vec3*)data)->y, 0.01f, 0.0f, 0.0f))
		changed = true;

	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
	ImGui::PushFont(boldFont);
	if (ImGui::Button("Z", buttonSize))
	{

	}
	ImGui::PopFont();
	ImGui::PopStyleColor(3);

	ImGui::SameLine();

	if (ImGui::DragFloat("##Z", &((Vec3*)data)->z, 0.01f, 0.0f, 0.0f))
		changed = true;

	ImGui::PopItemWidth();

	ImGui::PopStyleVar();

	ImGui::Columns(1);

	ImGui::PopStyleColor();
	
	ImGui::PopID();

	return changed;
}

bool editorGui::Transform(const char* label, void* data)
{
	struct Transform* t = (struct Transform*)data;
	ImGui::NewLine();
	bool loc = Vec3Control("Location", &t->location);
	bool rot = Vec3Control("Rotation", &t->rotation);
	bool scale = Vec3Control("Scale", &t->scale);
	return loc || rot || scale;
}

bool editorGui::PropertyInput(const std::string& label, EPropertyType type, void* data, bool readonly, float lblColWidth, uint64 param1, uint64 param2)
{
	if (readonly)
		ImGui::BeginDisabled();

	bool click = false;

	std::string lblVal = "##" + label;

	ImGui::Columns(2, "propName_propInput", false);

	ImGui::SetColumnWidth(0, lblColWidth);

	ImGui::Text(label.c_str());
	ImGui::SameLine();

	ImGui::NextColumn();

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
			click = ImGui::DragFloat(lblVal.c_str(), (float*)data, 0.01f, 0.0f, 0.0f);
			break;
		case PROPERTY_TYPE_BOOL:
			click = ImGui::Checkbox(lblVal.c_str(), (bool*)data);
			break;
		case PROPERTY_TYPE_VEC2:
			click = ImGui::DragFloat2(lblVal.c_str(), (float*)data, 0.01f, 0.0f, 0.0f);
			break;
		case PROPERTY_TYPE_VEC3:
			click = ImGui::DragFloat3(lblVal.c_str(), (float*)data, 0.01f, 0.0f, 0.0f);
			break;
		case PROPERTY_TYPE_VEC4:
			click = ImGui::DragFloat4(lblVal.c_str(), (float*)data, 0.01f, 0.0f, 0.0f);
			break;
		case PROPERTY_TYPE_TRANSFORM:
			click = Transform(lblVal.c_str(), data);
			break;
		case PROPERTY_TYPE_STRING:
			click = ImGui::InputText(lblVal.c_str(), (std::string*)data);
			break;
		case PROPERTY_TYPE_ASSET_REF:
			click = AssetRef(lblVal.c_str(), (EAssetType)param1, data, true, (GroovyClass*)param2);
			break;
		default:
			ImGui::Text("PROPERTY_TYPE_UI_NOT_IMPLEMENTED");
			click = false;
			break;
	}

	ImGui::Columns();


	if (readonly)
		ImGui::EndDisabled();

	return click;
}

bool editorGui::Property(const GroovyProperty& prop, void* propData)
{
	ImGui::PushID(propData);

	float lblColWidth = ImGui::GetContentRegionAvail().x / 100 * 22;

	bool readonly = prop.flags & PROPERTY_FLAG_EDITOR_READONLY;

	bool changed = false;

	if (prop.flags & PROPERTY_FLAG_IS_ARRAY)
	{
		ImGui::PushStyleColor(ImGuiCol_Header, { 1.0f, 1.0f, 1.0f, 0.0f });
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, { 1.0f, 1.0f, 1.0f, 0.1f });
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, { 1.0f, 1.0f, 1.0f, 0.1f });
		if (ImGui::CollapsingHeader(prop.name.c_str()))
		{
			uint32 propSize = GroovyProperty_GetSize(prop.type);
			if (prop.flags & PROPERTY_FLAG_IS_DYNAMIC_ARRAY)
			{
				bool cantResize = prop.flags & PROPERTY_FLAG_EDITOR_NO_RESIZE;

				DynamicArrayPtr arrayPtr = GroovyProperty_GetDynamicArrayPtr(prop.type);
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
					if (PropertyInput(labelName, prop.type, (byte*)dataData + (propSize * i), readonly, lblColWidth, prop.param1, prop.param2))
						changed = true;

					ImGui::SameLine();
					ImGui::PushID(i);
					
					if (readonly || cantResize)
						ImGui::BeginDisabled();

					if (ImGui::Button("X"))
					{
						postAction.action = REMOVE_AT;
						postAction.param = i;
						changed = true;
					}
					ImGui::SameLine();
					if (ImGui::Button("+"))
					{
						postAction.action = INSERT_AT;
						postAction.param = i;
						changed = true;
					}

					if (readonly || cantResize)
						ImGui::EndDisabled();

					ImGui::PopID();
				}

				if (readonly || cantResize)
					ImGui::BeginDisabled();

				if (ImGui::Button("Clear"))
				{
					postAction.action = CLEAR;
					changed = true;
				}
				ImGui::SameLine();
				if (ImGui::Button("Add"))
				{
					postAction.action = ADD;
					changed = true;
				}

				if (readonly || cantResize)
					ImGui::EndDisabled();

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
					if (PropertyInput(labelName, prop.type, (byte*)propData + (propSize * i), readonly, lblColWidth, prop.param1, prop.param2))
						changed = true;
				}
			}
		}
		ImGui::PopStyleColor(3);
	}
	else
	{
		changed = PropertyInput(prop.name, prop.type, propData, readonly, lblColWidth, prop.param1, prop.param2);
	}
	ImGui::PopID();

	return changed;
}

bool editorGui::PropertiesSingleClass(GroovyObject* obj, GroovyClass* gClass, const std::vector<GroovyProperty>& props)
{
	ImGui::Text(gClass->name.c_str());
	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Spacing();

	bool changed = false;

	for (const GroovyProperty& prop : props)
	{
		if (prop.flags & PROPERTY_FLAG_EDITOR_HIDDEN)
			continue;

		ImGui::Spacing();
		ImGui::Spacing();
		
		if (Property(prop, (byte*)obj + prop.offset))
		{
			obj->Editor_OnPropertyChanged(&prop);
			changed = true;
		}
		
		ImGui::Spacing();
		ImGui::Spacing();
	}

	return changed;
}

bool editorGui::PropertiesAllClasses(GroovyObject* obj)
{
	GroovyClass* superClass = obj->GetClass();

	bool changed = false;

	while (superClass)
	{
		std::vector<GroovyProperty> props;
		superClass->propertiesGetter(props);

		for (uint32 i = 0; i < props.size(); i++)
		{
			if (props[i].flags & PROPERTY_FLAG_EDITOR_HIDDEN)
			{
				props.erase(props.begin() + i);
				i--;
			}
		}

		if (props.size())
		{
			ImGui::Spacing();
			ImGui::Separator();

			if (PropertiesSingleClass(obj, superClass, props))
				changed = true;
		}

		superClass = superClass->super;
	}

	return changed;
}