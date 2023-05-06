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

#include "assets/asset_manager.h"
#include "renderer/api/shader.h"
#include "renderer/material.h"

class EditMaterialWindow : public EditorWindow
{
public:
	EditMaterialWindow(const std::string& wndTitle, AssetHandle assetHandle);
	virtual ~EditMaterialWindow();
	
	virtual void RenderContent() override;
	virtual bool OnClose() override;

private:
	void ShowVar(const ShaderVariable& var, byte* bufferPtr);
	void ShowConstBuffer(const ConstBufferDesc& bufferDesc, byte* bufferPtr);

private:
	Material* mMaterial;
	bool mVirtual;
};