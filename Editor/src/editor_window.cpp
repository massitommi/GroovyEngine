#include "editor_window.h"
#include "vendor/imgui/imgui.h"
#include "vendor/imgui/misc/cpp/imgui_stdlib.h"
#include "imgui_renderer/imgui_renderer.h"
#include "platform/messagebox.h"
#include "classes/object_serializer.h"
#include "assets/assets.h"
#include "project/project.h"
#include "classes/class.h"
#include "classes/class_db.h"
#include "renderer/mesh.h"
#include "gameframework/blueprint.h"
#include "runtime/object_allocator.h"
#include "gameframework/actor.h"
#include "gameframework/actor_component.h"
#include "editor.h"
#include "renderer/api/framebuffer.h"
#include "renderer/renderer.h"
#include "renderer/scene_renderer.h"

extern ImGuiRenderer* gGroovyGuiRenderer;

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
		if (editorGui::AssetRef(resLbl.c_str(), ASSET_TYPE_TEXTURE, &res.res, false))
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

	FrameBufferSpec frameBufferSpec;
	frameBufferSpec.swapchainTarget = false;
	frameBufferSpec.colorAttachments = { COLOR_FORMAT_R8G8B8A8_UNORM };
	frameBufferSpec.hasDepthAttachment = true;
	frameBufferSpec.width = frameBufferSpec.height = 100;
	mPreviewFrameBuffer = FrameBuffer::Create(frameBufferSpec);

	mModelTransform.location = { 0.0f, 0.0f, 0.0f };
	mModelTransform.rotation = { 0.0f, 0.0f, 0.0f };
	mModelTransform.scale = { 1.0f, 1.0f, 1.0f };

	mCameraZoom = -3.0f;
}

MeshPreviewWindow::~MeshPreviewWindow()
{
	delete mPreviewFrameBuffer;
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

	ImGui::Text("Materials");

	for (uint32 i = 0; i < mMeshMats.size(); i++)
	{
		std::string lbl = "##_mesh_res_" + std::to_string(i);
		ImGui::Text("[%i]", i);
		ImGui::SameLine();
		if (editorGui::AssetRef(lbl.c_str(), ASSET_TYPE_MATERIAL, &mMeshMats[i], false))
			FlagPendingSave();
	}

	if (click)
	{
		Save();
	}

	ImGui::Spacing();
	ImGui::Separator();

	editorGui::Transform("", &mModelTransform);

	if (ImGui::IsWindowFocused() && ImGui::IsWindowHovered())
	{
		mCameraZoom += ImGui::GetIO().MouseWheel * 0.5f;
	}

	ImVec2 wndSize = ImGui::GetContentRegionAvail();
	bool okSize = wndSize.x > 0.0f && wndSize.y > 0.0f;
	bool differentSize = wndSize.x != mPreviewFrameBuffer->GetSpecs().width || wndSize.y != mPreviewFrameBuffer->GetSpecs().height;

	if (differentSize && okSize)
	{
		mPreviewFrameBuffer->Resize((uint32)wndSize.x, (uint32)wndSize.y);
	}

	mPreviewFrameBuffer->ClearColorAttachment(0, { .4f, .5f, 0.6f, 1.0f });
	mPreviewFrameBuffer->ClearDepthAttachment();
	mPreviewFrameBuffer->Bind();

	if (okSize)
	{
		Mat4 camera =
			math::GetViewMatrix({ 0, 0, mCameraZoom }, { 0,0,0 })
			*
			math::GetPerspectiveMatrix(wndSize.x / wndSize.y, 60.0f, 0.01f, 1000.0f);

		Mat4 model = math::GetModelMatrix(mModelTransform.location, mModelTransform.rotation, mModelTransform.scale);

		camera = math::GetMatrixTransposed(camera);
		model = math::GetMatrixTransposed(model);

		Renderer::SetCamera(camera);
		Renderer::SetModel(model);

		Renderer::RenderMesh(mMesh, mMeshMats);
	}

	gGroovyGuiRenderer->SetGroovyRenderState();
	ImGui::Image(mPreviewFrameBuffer->GetRendererID(0), wndSize);
	gGroovyGuiRenderer->SetImguiRenderState();
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

	mLiveActor = mLiveScene.SpawnActor(mBlueprint->GetActorClass(), mBlueprint);
	mLiveActor->Editor_Template() = nullptr;

	mTmpCompName = "";
	mCanRenameOrAddComp = false;

	mPendingRemove = nullptr;
	mPendingRename = nullptr;
	mShowRenamePopup = false;
	
	mSelected = mLiveActor;

	FrameBufferSpec frameBufferSpec;
	frameBufferSpec.swapchainTarget = false;
	frameBufferSpec.colorAttachments = { COLOR_FORMAT_R8G8B8A8_UNORM };
	frameBufferSpec.hasDepthAttachment = true;
	frameBufferSpec.width = frameBufferSpec.height = 100;
	mPreviewFrameBuffer = FrameBuffer::Create(frameBufferSpec);

	mCameraZoom = -3.0f;
}

ActorBlueprintEditorWindow::~ActorBlueprintEditorWindow()
{
	delete mPreviewFrameBuffer;
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

				for (const GroovyClass* gClass : gClassDB.GetClasses())
				{
					if (GroovyClass_IsA(gClass, ActorComponent::StaticClass()))
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

				if (ImGui::IsItemHovered())
					ImGui::SetTooltip(component->GetClass()->name.c_str());

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

		ImVec2 wndSize = ImGui::GetContentRegionAvail();
		bool okSize = wndSize.x > 0.0f && wndSize.y > 0.0f;
		bool differentSize = wndSize.x != mPreviewFrameBuffer->GetSpecs().width || wndSize.y != mPreviewFrameBuffer->GetSpecs().height;

		if (ImGui::IsWindowFocused() && ImGui::IsWindowHovered())
		{
			mCameraZoom += ImGui::GetIO().MouseWheel * 0.5f;
		}

		if (differentSize && okSize)
		{
			mPreviewFrameBuffer->Resize((uint32)wndSize.x, (uint32)wndSize.y);
		}

		mPreviewFrameBuffer->ClearColorAttachment(0, { .4f, .5f, 0.6f, 1.0f });
		mPreviewFrameBuffer->ClearDepthAttachment();
		mPreviewFrameBuffer->Bind();

		if (okSize)
		{
			SceneRenderer::BeginScene({ 0.0f, 0.0f, mCameraZoom }, { 0.0f, 0.0f, 0.0f }, 60.0f, wndSize.x / wndSize.y);
			SceneRenderer::RenderScene(&mLiveScene);
		}

		gGroovyGuiRenderer->SetGroovyRenderState();
		ImGui::Image(mPreviewFrameBuffer->GetRendererID(0), wndSize);
		gGroovyGuiRenderer->SetImguiRenderState();

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
	// update current scene
	Scene* editScene = editor::GetEditScene();

	if (editScene)
	{
		editor::OnBPUpdated(mBlueprint);

		if (editScene->Editor_OnBlueprintUpdated(mBlueprint, mBlueprint->GetDefaultActor(), mLiveActor))
			editor::FlagEditScenePendingSave();
	}

	// save stuff
	mBlueprint->RebuildPack(mLiveActor);
	mBlueprint->Save();
	AssetEditorWindow::Save();
}

ProjectSettingsWindow::ProjectSettingsWindow()
	: EditorWindow()
{
	mStartupScene = gProj.GetStartupScene();
}

void ProjectSettingsWindow::RenderContent()
{
	float colWidth = ImGui::GetContentRegionAvail().x / 100 * 30;
	editorGui::PropertyInput("Startup scene", PROPERTY_TYPE_ASSET_REF, &mStartupScene, false, colWidth, ASSET_TYPE_SCENE);

	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Spacing();

	if (ImGui::Button("Save settings"))
	{
		gProj.SetStartupScene(mStartupScene);
	}
}

void EditorSettingsWindow::RenderContent()
{
	extern EditorSettings gEditorSettings;

	float colWidth = ImGui::GetContentRegionAvail().x / 100 * 30;
	ImGui::Columns(2);

	ImGui::Text("Editor camera FOV");
	
	ImGui::NextColumn();
	
	ImGui::DragFloat("##editor_camera_fov", &gEditorSettings.mCameraFOV, 0.1f, 0.1f, 360.f);
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
	
	ImGui::DragFloat("##editor_camera_move_speed", &gEditorSettings.mCameraMoveSpeed, 0.1f, 0.001f, 1000.0f);
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

	ImGui::DragFloat("##editor_camera_rot_speed", &gEditorSettings.mCameraRotationSpeed, 0.1f, 0.001f, 1000.0f);
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
		gEditorSettings.mCameraFOV = editorSettingsCDO->mCameraFOV;
		gEditorSettings.mCameraMoveSpeed = editorSettingsCDO->mCameraMoveSpeed;
		gEditorSettings.mCameraRotationSpeed = editorSettingsCDO->mCameraRotationSpeed;
	}
}

void HelpWindow::RenderContent()
{
	ImGui::Text("- Hold down the right mouse button to look around");
	ImGui::Text("- While holding down the right mouse button, use WASD to move around");
	ImGui::Text("- Press Shift + F1 to enable cursor during PIE (Play In Editor)");
	ImGui::Text("- Press F3 to toggle wireframe view");
}

AudioClipInfoWindow::AudioClipInfoWindow(const AssetHandle& asset)
	: AssetEditorWindow(asset), mClip((AudioClip*)asset.instance)
{
	checkslow(asset.type == ASSET_TYPE_AUDIO_CLIP);

	mClipInfo = mClip->GetInfo();
}

AudioClipInfoWindow::~AudioClipInfoWindow()
{
}

void AudioClipInfoWindow::RenderContent()
{
	ImGui::Text("Duration: %.2f seconds", (float)mClipInfo.durationMilliseconds / 1000.0f);
	ImGui::Text("Num channels: %i", mClipInfo.channels);
	const char* audioFormat = "UNKNOWN";
	switch (mClipInfo.format)
	{
		case AUDIO_CLIP_FORMAT_PCM8:
			audioFormat = "PCM8";
			break;

		case AUDIO_CLIP_FORMAT_PCM16:
			audioFormat = "PCM16";
			break;

		case AUDIO_CLIP_FORMAT_PCM24:
			audioFormat = "PCM24";
			break;

		case AUDIO_CLIP_FORMAT_PCM32:
			audioFormat = "PCM32";
			break;

		case AUDIO_CLIP_FORMAT_PCMFLOAT:
			audioFormat = "PCMFLOAT";
			break;

		case AUDIO_CLIP_FORMAT_BITSTREAM:
			audioFormat = "BITSTREAM";
			break;
	}

	ImGui::Text("Format: %s", audioFormat);
}

void AudioClipInfoWindow::Save()
{
}