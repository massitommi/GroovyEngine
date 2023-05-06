#pragma once

#include "core/core.h"

struct WindowProps
{
	std::string title;
	uint32 width, height;
	uint32 refreshrate;
	uint32 vsync;
	bool fullscreen;
};

class Window
{
public:
	typedef void(*WndResizeCallback)(uint32, uint32);

	Window(const WindowProps& props);
	~Window();

	void Spawn();
	void Show();
	void Hide();

	void ProcessEvents();
	
	void SetTitle(const std::string& newTitle);

	void OnResize(uint32 width, uint32 height);

	inline const WindowProps& GetProps() const { return mProps; }
	inline void* GetNativeHandle() const { return mHandle; }

	inline void SubmitToWndResizeCallback(WndResizeCallback proc) { mWndResizeCallabacks.push_back(proc); }

	static void InitSystem();
	inline static void SetMainWindow(Window* wnd) { sMainWindow = wnd; }
	inline static Window* Get() { return sMainWindow; }

private:
	WindowProps mProps;
	void* mHandle;
	std::vector<WndResizeCallback> mWndResizeCallabacks;

	static Window* sMainWindow;
};