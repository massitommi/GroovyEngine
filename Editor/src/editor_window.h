#pragma once

#include "assets/asset.h"
#include "renderer/api/shader.h"
#include "classes/class.h"

class EditorWindow
{
public:
	EditorWindow(const std::string& title)
		: mTitle(title), mFlags(0), mOpen(true), mPendingSave(false)
	{}

	virtual ~EditorWindow() {}

	void RenderWindow();
	
	virtual void RenderContent() {}

	virtual bool OnClose();

	inline bool ShouldClose() const { return !mOpen; }
	inline bool PendingSave() const { return mPendingSave; }

protected:
	void SetPendingSave(bool pendingSave);

private:
	std::string mTitle;
	uint64 mFlags;
	bool mOpen;
	bool mPendingSave;
};

class AssetRegistryWindow : public EditorWindow
{
public:
	AssetRegistryWindow() : EditorWindow("Asset registry") {}

	const char* AssetTypeStr(EAssetType type);

	virtual void RenderContent() override;

private:
	std::string mDebugAssetName = "";
	EAssetType mDebugAssetType = ASSET_TYPE_TEXTURE;
};

class EditMaterialWindow : public EditorWindow
{
public:
	EditMaterialWindow(class Material* mat);
	EditMaterialWindow(class Shader* shader);
	virtual ~EditMaterialWindow();
	
	virtual void RenderContent() override;

private:
	class Material* mMaterial;
	bool mExistsOnDisk;
	std::string mFileName;
};

class TexturePreviewWindow : public EditorWindow
{
public:
	TexturePreviewWindow(class Texture* texture)
		: EditorWindow("Texture viewer"), mTexture(texture)
	{}

	virtual void RenderContent() override;

private:
	class Texture* mTexture;
};

class MeshPreviewWindow : public EditorWindow
{
public:
	MeshPreviewWindow(Mesh* mesh);

	virtual void RenderContent() override;

private:
	class Mesh* mMesh;
	std::string mFileName;
};

class ObjectBlueprintEditorWindow : public EditorWindow
{
public:
	ObjectBlueprintEditorWindow(class ObjectBlueprint* blueprint);
	ObjectBlueprintEditorWindow(class GroovyClass* inClass);
	~ObjectBlueprintEditorWindow();

	virtual void RenderContent() override;

private:
	class ObjectBlueprint* mBlueprint;

	std::string mFileName;
	bool mExistsOnDisk;
};

class ActorBlueprintEditorWindow : public EditorWindow
{
public:
	ActorBlueprintEditorWindow(class ActorBlueprint* blueprint);
	ActorBlueprintEditorWindow(class GroovyClass* inClass);
	~ActorBlueprintEditorWindow();

	virtual void RenderContent() override;

private:
	class ActorBlueprint* mBlueprint;

	std::string mFileName;
	bool mExistsOnDisk;

	class GroovyObject* mSelected;
};