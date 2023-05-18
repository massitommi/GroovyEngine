#pragma once

#include "core/core.h"

class EditorWindow
{
public:
	EditorWindow(const std::string& title)
		: mTitle(title), mOpen(true), mFlags(0)
	{}
	virtual ~EditorWindow() {}

	void RenderWindow();
	virtual void RenderContent() {}

	virtual bool OnClose() { return true; }

	inline bool ShouldClose() const { return !mOpen; }

protected:
	std::string mTitle;
	uint64 mFlags;

private:
	bool mOpen;
};

#include "assets/asset.h"

class AssetRegistryWindow : public EditorWindow
{
public:
	AssetRegistryWindow(const std::string& title) : EditorWindow(title) {}

	const char* AssetTypeStr(EAssetType type);

	virtual void RenderContent() override;
};

#include "assets/asset_manager.h"
#include "renderer/api/shader.h"
#include "renderer/material.h"

class EditMaterialWindow : public EditorWindow
{
public:
	EditMaterialWindow(const std::string& wndTitle, Material* mat);
	virtual ~EditMaterialWindow();
	
	virtual void RenderContent() override;
	virtual bool OnClose() override;

private:
	void ShowVar(const ShaderVariable& var, byte* bufferPtr);

private:
	Material* mMaterial;
	bool mVirtual;
};

class TexturePreviewWindow : public EditorWindow
{
public:
	TexturePreviewWindow(const std::string& wndTitle, Texture* texture);

	virtual void RenderContent() override;

private:
	Texture* mTexture;
};

#include "renderer/mesh.h"

class MeshPreviewWindow : public EditorWindow
{
public:
	MeshPreviewWindow(const std::string& wndTitle, Mesh* mesh);

	virtual void RenderContent() override;
private:
	Mesh* mMesh;
};