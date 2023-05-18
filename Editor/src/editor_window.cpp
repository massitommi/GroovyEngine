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
#include "assets/asset_serializer.h"
#include "project/Project.h"
#include "platform/messagebox.h"

extern Project gProj;

extern Texture* DEFAULT_TEXTURE;
extern Shader* DEFAULT_SHADER;
extern Material* DEFAULT_MATERIAL;

struct MyPropField
{
	std::string name;
	void* data;
};

struct PropFieldArray
{
	std::vector<MyPropField> fields;
	std::vector<MyPropField> options;
};

template<typename OnClickProc>
static void RenderAssetSelectionMenu(PropFieldArray& comboArray, OnClickProc onClick)
{
	bool boh = false;

	std::vector<std::string> previews;
	previews.resize(comboArray.fields.size());

	for (uint32 i = 0; i < comboArray.fields.size(); i++)
	{
		for (const MyPropField& option : comboArray.options)
		{
			if (comboArray.fields[i].data == option.data)
			{
				previews[i] = option.name;
			}
		}

		if (ImGui::BeginCombo(comboArray.fields[i].name.c_str(), previews[i].c_str()))
		{
			for (uint32 j = 0; j < comboArray.options.size(); j++)
			{
				if (ImGui::Selectable(comboArray.options[j].name.c_str(), &boh))
				{
					previews[i] = comboArray.options[j].name;
					comboArray.fields[i].data = comboArray.options[j].data;
					onClick(i, j);
				}
			}

			ImGui::EndCombo();
		}
	}
}

EditMaterialWindow::EditMaterialWindow(const std::string& wndTitle, Material* mat)
	: EditorWindow(wndTitle), mMaterial(mat)
{
	if (mat)
	{
		mVirtual = false;
	}
	else
	{
		mVirtual = true;
		mMaterial = new Material(DEFAULT_SHADER);
		mMaterial->SetTextures(DEFAULT_TEXTURE);
	}
}

EditMaterialWindow::~EditMaterialWindow()
{
	if (mVirtual)
	{
		delete mMaterial;
	}
}

void EditMaterialWindow::RenderContent()
{
	ImGui::Text("Pixel shader constant buffers:");
	ImGui::Spacing();

	const std::vector<ConstBufferDesc>& psBuffers = mMaterial->GetShader()->GetPixelConstBuffersDesc();
	const std::vector<ShaderResTexture>& psTextures = mMaterial->GetShader()->GetPixelTexturesRes();

	// pixel const buffers
	{
		byte* bufferPtr = (byte*)mMaterial->GetConstBuffersData().data();
		for (const ConstBufferDesc& bufferDesc : psBuffers)
		{
			if (ImGui::CollapsingHeader(bufferDesc.name.c_str()))
			{
				for (const ShaderVariable& var : bufferDesc.variables)
				{
					ShowVar(var, bufferPtr);
				}
			}
			bufferPtr += bufferDesc.size;
		}
	}
	
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	ImGui::Text("Pixel shader texture resources:");
	ImGui::Spacing();
	
	// texture resources
	{
		std::vector<AssetHandle> texturesAvail = AssetManager::GetRegistryFiltered(ASSET_TYPE_TEXTURE);
		bool pSelected = false;

		PropFieldArray fieldArray;
		for (uint32 i = 0; i < psTextures.size(); i++)
			fieldArray.fields.push_back({ psTextures[i].name, mMaterial->GetTextures()[i] });
		fieldArray.options.push_back({ "DEFAULT_TEXTURE", DEFAULT_TEXTURE });
		for(const AssetHandle& textAvail : texturesAvail)
			fieldArray.options.push_back({ textAvail.name, textAvail.instance });

		RenderAssetSelectionMenu(fieldArray, [&](uint32 i, uint32 j)
		{
				mMaterial->SetTexture((Texture*)fieldArray.options[j].data, i);
		});
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	if (mVirtual) // does not exist on disk
	{
		static std::string newFileName = "new_material";
		ImGui::InputText("Save as: ", &newFileName);
		
		if (ImGui::Button("Save"))
		{
			AssetHandle handle = AssetManager::AddEditorNew(gProj.assetsPath + newFileName + GROOVY_ASSET_EXT, ASSET_TYPE_MATERIAL, mMaterial);
			mMaterial->__internal_SetUUID(handle.uuid);
			AssetSerializer::SerializeMaterial(mMaterial, handle.path);
			AssetManager::SaveRegistry();
			mVirtual = false;
		}
	}
	else // file exists on disk
	{
		if (ImGui::Button("Save changes"))
		{
			const AssetHandle& handle = AssetManager::Get(mMaterial->GetUUID());
			AssetSerializer::SerializeMaterial(mMaterial, handle.path);
			AssetManager::SaveRegistry();
		}
	}
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

		default:
			ImGui::Text("Variable format not implemented!");
			break;
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

TexturePreviewWindow::TexturePreviewWindow(const std::string& wndTitle, Texture* texture)
	: EditorWindow(wndTitle), mTexture(texture)
{
}

void TexturePreviewWindow::RenderContent()
{
	ImGui::Image(mTexture->GetRendererID(), ImGui::GetContentRegionAvail());
}

MeshPreviewWindow::MeshPreviewWindow(const std::string& wndTitle, Mesh* mesh)
	: EditorWindow(wndTitle), mMesh(mesh)
{
}

void MeshPreviewWindow::RenderContent()
{
	ImGui::Text("Submeshes");
	ImGui::Separator();

	const std::vector<AssetHandle>& matHandles = AssetManager::GetRegistryFiltered(ASSET_TYPE_MATERIAL);
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
	}
}
