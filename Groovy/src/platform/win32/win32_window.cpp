#if PLATFORM_WIN32

#include "platform/window.h"
#include "win32_globals.h"

static const char* sWndClassName = "groovyWnd";

#if !WITH_EDITOR
	#define WINDOW_FLAGS	WS_EX_ACCEPTFILES
#else
	#define WINDOW_FLAGS	0
#endif

extern bool gEngineShouldRun;

LRESULT Win32_EditorWndProcCallback(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static LRESULT CALLBACK groovyWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Win32_EditorWndProcCallback(hWnd, uMsg, wParam, lParam);

	switch (uMsg)
	{
		case WM_SIZE:
		{
			uint32 width = LOWORD(lParam);
			uint32 height = HIWORD(lParam);
			Window* wnd = (Window*)GetWindowLongPtrA(hWnd, GWLP_USERDATA);
			wnd->OnResize(width, height);
		}
		break;

		case WM_CLOSE:
		{
			gEngineShouldRun = false;
		}
		break;
	}

	return DefWindowProcA(hWnd, uMsg, wParam, lParam);
}

Window::Window(const WindowProps& props)
	: mProps(props), mHandle(nullptr)
{
	HWND handle = CreateWindowExA
	(
		WINDOW_FLAGS, sWndClassName, *props.title, WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SIZEBOX | WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT, props.width, props.height, nullptr, nullptr, gInstance, nullptr
	);
	SetWindowLongPtrA(handle, GWLP_USERDATA, (LONG_PTR)this);
	mHandle = handle;
}

Window::~Window()
{
	DestroyWindow((HWND)mHandle);
}

void Window::Show()
{
	ShowWindow((HWND)mHandle, SW_SHOW);
}

void Window::Hide()
{
	ShowWindow((HWND)mHandle, SW_HIDE);
}

void Window::ProcessEvents()
{
	MSG msg;
	while (PeekMessageA(&msg, (HWND)mHandle, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}
}

void Window::OnResize(uint32 width, uint32 height)
{
	mProps.width = width;
	mProps.height = height;

	for (auto resizeCallback : mWndResizeCallabacks)
	{
		resizeCallback(width, height);
	}
}

void Window::InitSystem()
{
	WNDCLASSEXA wndClass = {};
	wndClass.cbSize = sizeof(WNDCLASSEXA);
	wndClass.hInstance = gInstance;
	wndClass.lpfnWndProc = groovyWndProc;
	wndClass.lpszClassName = sWndClassName;
	wndClass.style = CS_OWNDC;
	checkslow(RegisterClassExA(&wndClass));
}

#endif