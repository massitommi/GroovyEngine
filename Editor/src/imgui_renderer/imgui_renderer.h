#pragma once

class ImGuiRenderer
{
public:

	virtual void Init() = 0;
	virtual void NewFrame() = 0;
	virtual void RenderDrawData() = 0;

	static ImGuiRenderer* Create();
};

class D3D11_ImGuiRenderer : public ImGuiRenderer
{
public:

	virtual void Init() override;
	virtual void NewFrame() override;
	virtual void RenderDrawData() override;
};