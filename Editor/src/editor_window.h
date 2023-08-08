#pragma once

#include "assets/asset.h"
#include "renderer/api/shader.h"
#include "classes/class.h"
#include "renderer/material.h"

class EditorWindow
{
public:
	EditorWindow() 
		: mOpen(true), mShouldClose(false), mFlags(0) 
	{}
	virtual ~EditorWindow() {}

	void RenderWindow();

	virtual void RenderContent() {}
	virtual void OnCloseRequested() { Close(); }

	// used by the editor
	inline bool ShouldClose() const { return mShouldClose; }
	inline void Close() { mShouldClose = true; }

	inline const std::string& GetTitle() const { return mTitle; }
	inline void SetTitle(const std::string& newTitle) { mTitle = newTitle; }

private:
	std::string mTitle;
	bool mShouldClose;

protected:
	bool mOpen;
	uint64 mFlags;
};

class AssetEditorWindow : public EditorWindow
{
public:
	AssetEditorWindow(const AssetHandle& asset);

	virtual void OnCloseRequested() override;

	virtual void Save();
	inline void FlagPendingSave();

	inline bool IsPendingSave() const { return mPendingSave; }
	
private:
	AssetHandle mAsset;
	bool mPendingSave;
};

class EditMaterialWindow : public AssetEditorWindow
{
public:
	EditMaterialWindow(const AssetHandle& asset);
	
	virtual void RenderContent() override;
	virtual void Save() override;

private:
	class Material* mMaterial;
	std::vector<MaterialResource> mMatResources;
};

class TexturePreviewWindow : public EditorWindow
{
public:
	TexturePreviewWindow(const AssetHandle& asset);

	virtual void RenderContent() override;

private:
	class Texture* mTexture;
};

class MeshPreviewWindow : public AssetEditorWindow
{
public:
	MeshPreviewWindow(const AssetHandle& asset);

	virtual void RenderContent() override;
	virtual void Save() override;

private:
	class Mesh* mMesh;
	std::vector<Material*> mMeshMats;
};

class ObjectBlueprintEditorWindow : public AssetEditorWindow
{
public:
	ObjectBlueprintEditorWindow(const AssetHandle& asset);
	~ObjectBlueprintEditorWindow();

	virtual void RenderContent() override;
	virtual void Save() override;

private:
	class ObjectBlueprint* mBlueprint;
	class GroovyObject* mLiveObject;
};

class ActorBlueprintEditorWindow : public AssetEditorWindow
{
public:
	ActorBlueprintEditorWindow(const AssetHandle& asset);
	~ActorBlueprintEditorWindow();

	virtual void RenderContent() override;
	virtual void Save() override;

private:
	class ActorBlueprint* mBlueprint;
	class Actor* mLiveActor;

	std::string mTmpCompName;
	bool mCanRenameOrAddComp;

	class ActorComponent* mPendingRemove;
	class ActorComponent* mPendingRename;
	bool mShowRenamePopup;

	class GroovyObject* mSelected;
};

class ProjectSettingsWindow : public EditorWindow
{
public:
	ProjectSettingsWindow();

	virtual void RenderContent() override;

private:
	std::string mProjName;
	Scene* mStartupScene;
};

class EditorSettingsWindow : public EditorWindow
{
public:
	virtual void RenderContent() override;
};

class HelpWindow : public EditorWindow
{
public:
	virtual void RenderContent() override;
};