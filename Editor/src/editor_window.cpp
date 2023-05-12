#include "editor_window.h"
#include "vendor/imgui/imgui.h"
#include "vendor/imgui/misc/cpp/imgui_stdlib.h"

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

// Material editor
#include "assets/asset_manager.h"
#include "assets/asset_loader.h"
#include "platform/filesystem.h"
#include "project/Project.h"
#include "platform/messagebox.h"

extern Project gProj;


EditMaterialWindow::EditMaterialWindow(const std::string& wndTitle, AssetHandle assetHandle)
	: EditorWindow(wndTitle)
{
}

EditMaterialWindow::~EditMaterialWindow()
{
	delete mMaterial->GetShader();
	delete mMaterial;
}

void EditMaterialWindow::RenderContent()
{
	//if (!mMaterial->GetShader())
	//{
	//	ImGui::Text("No shader, no data :(");
	//	return;
	//}

	//const std::vector<ConstBufferDesc>& vsBuffers = mMaterial->GetShader()->GetVertexConstBuffersDesc();
	//const std::vector<ConstBufferDesc>& psBuffers = mMaterial->GetShader()->GetPixelConstBuffersDesc();
	//const std::vector<ShaderResTexture>& psTextures = mMaterial->GetShader()->GetPixelTexturesRes();

	//ImGui::Text("Vertex shader constant buffers:");
	//ImGui::Spacing();
	//// vertex const buffers
	//{
	//	byte* bufferPtr = (byte*)mMaterial->GetVertexConstBuffersData().data();
	//	for (const ConstBufferDesc& bufferDesc : vsBuffers)
	//	{
	//		ShowConstBuffer(bufferDesc, bufferPtr);
	//		bufferPtr += bufferDesc.size;
	//	}
	//}
	//ImGui::Spacing();
	//ImGui::Separator();
	//ImGui::Spacing();
	//ImGui::Text("Pixel shader constant buffers:");
	//ImGui::Spacing();
	//// pixel const buffers
	//{
	//	byte* bufferPtr = (byte*)mMaterial->GetPixelConstBuffersData().data();
	//	for (const ConstBufferDesc& bufferDesc : psBuffers)
	//	{
	//		ShowConstBuffer(bufferDesc, bufferPtr);
	//		bufferPtr += bufferDesc.size;
	//	}
	//}
	//ImGui::Spacing();
	//ImGui::Separator();
	//ImGui::Spacing();
	//ImGui::Text("Pixel shader texture resources:");
	//ImGui::Spacing();
	//// texture resources
	//for (uint32 i = 0; i < mMaterial->GetTexturesID().size(); i++)
	//{
	//	ImGui::InputScalar(psTextures[i].name.c_str(), ImGuiDataType_U64, &mMaterial->GetTexturesID()[i]);
	//}
	//ImGui::Spacing();
	//ImGui::Separator();
	//ImGui::Spacing();

	//if (!mVirtual) // asset is already on disk
	//{
	//	AssetHandle handle = {};/* AssetManager::GetAssets()[AssetManager::Find(mMaterial->GetUUID())];*/
	//	if (ImGui::Button("Save changes"))
	//	{
	//		Buffer data;
	//		mMaterial->Serialize(data);
	//		check(FileSystem::WriteFileBinary(handle.path, data) == FILE_OPEN_RESULT_OK);
	//	}

	//}
	//else // asset does not exist on disk
	//{
	//	static std::string newFileName = "new_material.groovyasset";
	//	std::string newPath = gProj.assetsPath + newFileName;
	//	ImGui::InputText("Save as: ", &newFileName);
	//	if (ImGui::Button("Save"))
	//	{
	//		Buffer data;
	//		mMaterial->Serialize(data);
	//		check(FileSystem::WriteFileBinary(newPath, data) == FILE_OPEN_RESULT_OK);
	//		/*AssetHandle newShinyHandle = AssetManager::AddNew(newPath);*/
	//		mVirtual = false; // now exists on disk!
	//		mFlags &= ~ImGuiWindowFlags_UnsavedDocument;
	//	}
	//}
}

bool EditMaterialWindow::OnClose()
{
	if (mVirtual)
	{
		auto res = SysMessageBox::Show
		(
			"Unsaved asset",
			"This asset is not saved on disk, are you sure you want to discard it?",
			MESSAGE_BOX_TYPE_WARNING,
			MESSAGE_BOX_OPTIONS_YESNOCANCEL
		);
		if (res != MESSAGE_BOX_RESPONSE_YES)
		{
			return false;
		}
	}
	return true;
}

void EditMaterialWindow::ShowVar(const ShaderVariable& var, byte* bufferPtr)
{
	byte* varPtr = bufferPtr + var.alignedOffset;
	static bool vec3AsColor = true;
	static bool vec4AsColor = true;

	switch (var.type)
	{
		case SHADER_VARIABLE_TYPE_FLOAT1:
			ImGui::DragFloat(var.name.c_str(), (float*)varPtr);
			break;

		case SHADER_VARIABLE_TYPE_FLOAT2:
			ImGui::DragFloat2(var.name.c_str(), (float*)varPtr);
			break;

		case SHADER_VARIABLE_TYPE_FLOAT3:
			ImGui::Checkbox("Use Vec3 as color", &vec3AsColor);
			if (vec3AsColor)
			{
				ImGui::ColorEdit3(var.name.c_str(), (float*)varPtr);
			}
			else
			{
				ImGui::DragFloat3(var.name.c_str(), (float*)varPtr);
			}
			break;

		case SHADER_VARIABLE_TYPE_FLOAT4:
			ImGui::Checkbox("Use Vec4 as color", &vec4AsColor);
			if (vec4AsColor)
			{
				ImGui::ColorEdit4(var.name.c_str(), (float*)varPtr);
			}
			else
			{
				ImGui::DragFloat4(var.name.c_str(), (float*)varPtr);
			}
			break;

		case SHADER_VARIABLE_TYPE_FLOAT4X4:
			ImGui::DragFloat4((var.name + "[0]").c_str(), (float*)(varPtr + sizeof(float) * 4 * 0));
			ImGui::DragFloat4((var.name + "[1]").c_str(), (float*)(varPtr + sizeof(float) * 4 * 1));
			ImGui::DragFloat4((var.name + "[2]").c_str(), (float*)(varPtr + sizeof(float) * 4 * 2));
			ImGui::DragFloat4((var.name + "[3]").c_str(), (float*)(varPtr + sizeof(float) * 4 * 3));
			break;

		default:
			ImGui::Text("Variable format not implemented!");
			break;
	}
}

void EditMaterialWindow::ShowConstBuffer(const ConstBufferDesc& bufferDesc, byte* bufferPtr)
{
	if (ImGui::CollapsingHeader(bufferDesc.name.c_str()))
	{
		for (const ShaderVariable& var : bufferDesc.variables)
		{
			ShowVar(var, bufferPtr);
		}
	}
}

const char* AssetRegistryWindow::AssetTypeStr(EAssetType type)
{
	switch (type)
	{
		case ASSET_TYPE_TEXTURE:	return "TEXTURE";
		case ASSET_TYPE_MATERIAL:	return "MATERIAL";
		case ASSET_TYPE_SHADER:		return "SHADER";
		case ASSET_TYPE_MESH:		return "MESH";
	}
	return "UNKNOWN";
}

void AssetRegistryWindow::RenderContent()
{
	const auto& reg = AssetManager::GetRegistry();
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
		ImGui::EndDisabled();
}
