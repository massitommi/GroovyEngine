#if PLATFORM_WIN32

#include "platform/window.h"
#include "win32_globals.h"

static const char* sWndClassName = "groovyWnd";

#define WINDOW_STYLE		WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SIZEBOX | WS_SYSMENU
#define WINDOW_STYLE_EX		0

#if WITH_EDITOR
	#define WINDOW_STYLE_EDITOR		WS_MAXIMIZE
	#define WINDOW_STYLE_EX_EDITOR	WS_EX_ACCEPTFILES
#else
	#define WINDOW_STYLE_EDITOR		0
	#define WINDOW_STYLE_EX_EDITOR	0
#endif

extern bool gEngineShouldRun;

LRESULT Win32_EditorWndProcCallback(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static LRESULT CALLBACK GroovyWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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

		case WM_CREATE:
		{
			CREATESTRUCTA* params = (CREATESTRUCTA*)lParam;
			SetWindowLongPtrA(hWnd, GWLP_USERDATA, (LONG_PTR)params->lpCreateParams);
		}
		break;
	}

	return DefWindowProcA(hWnd, uMsg, wParam, lParam);
}

Window::Window(const WindowProps& props)
	: mProps(props), mHandle(nullptr)
{
}

Window::~Window()
{
	DestroyWindow((HWND)mHandle);
}

void Window::Spawn()
{
	HWND handle = CreateWindowExA
	(
		WINDOW_STYLE_EX | WINDOW_STYLE_EX_EDITOR, sWndClassName, mProps.title.c_str(),
		WINDOW_STYLE | WINDOW_STYLE_EDITOR, CW_USEDEFAULT, CW_USEDEFAULT,
		mProps.width, mProps.height, nullptr, nullptr, gInstance, this
	);
	mHandle = handle;
}

void Window::Show()
{
	if(mHandle)
		ShowWindow((HWND)mHandle, SW_SHOW);
}

void Window::Hide()
{
	if(mHandle)
		ShowWindow((HWND)mHandle, SW_HIDE);
}

void Window::ProcessEvents()
{
	if (!mHandle) return;
	MSG msg;
	while (PeekMessageA(&msg, (HWND)mHandle, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}
}

void Window::SetTitle(const std::string& newTitle)
{
	mProps.title = newTitle;
	if(mHandle)
		SetWindowTextA((HWND)mHandle, newTitle.c_str());
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
	wndClass.lpfnWndProc = GroovyWndProc;
	wndClass.lpszClassName = sWndClassName;
	wndClass.style = CS_OWNDC;
	checkslow(RegisterClassExA(&wndClass));
}

#endif