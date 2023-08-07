#pragma once

#include "core/core.h"

struct WindowProps
{
	std::string title;
	uint32 width, height;
	bool fullscreen;
};

typedef bool(*WindowEvent_OnClose)();
typedef void(*WindowEvent_OnResize)(uint32, uint32);
typedef void(*WindowEvent_OnFilesDropped)(const std::vector<std::string>&);

class Window
{
public:
	Window(const WindowProps& props);
	~Window();

	void Spawn();
	void Show();
	void Hide();

	void ProcessEvents();
	
	void SetTitle(const std::string& newTitle);
	void SetFullscreen(bool fullscreen);

	void EnableCursor(bool enable);
	void SetCursorPos(uint32 x, uint32 y);
	void GetCursorPos(uint32* xy);

	bool OnClose();
	void OnResize(uint32 width, uint32 height);
	void OnFilesDropped(const std::vector<std::string>& files);
	void OnDestroy();

	inline const WindowProps& GetProps() const { return mProps; }
	inline void* GetNativeHandle() const { return mHandle; }

	inline void SubmitToWndResizeCallback(WindowEvent_OnResize proc) { mWndResizeCallabacks.push_back(proc); }
	inline void SubmitToWndFilesDropCallbacks(WindowEvent_OnFilesDropped proc) { mWndFilesDropCallbacks.push_back(proc); }
	inline void SubmitToWndCloseCallback(WindowEvent_OnClose proc) { mWndCloseCallback = proc; }

	static void InitSystem();

private:
	WindowProps mProps;
	void* mHandle;

	// window events
	std::vector<WindowEvent_OnResize> mWndResizeCallabacks;
	std::vector<WindowEvent_OnFilesDropped> mWndFilesDropCallbacks;
	WindowEvent_OnClose mWndCloseCallback;
};