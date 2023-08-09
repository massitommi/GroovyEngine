#if PLATFORM_WIN32

#include "platform/window.h"
#include "win32_globals.h"
#include "renderer/api/renderer_api.h"
#include "platform/input.h"

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

static std::vector<std::string> GetDroppedFiles(HDROP boh)
{
	std::vector<std::string> result;

	uint32 numFilesDropped = DragQueryFileA(boh, 0xffffffff, nullptr, 0);
	for (uint32 i = 0; i < numFilesDropped; i++)
	{
		uint32 fileNameSize = DragQueryFileA(boh, i, nullptr, 0);
		result.emplace_back(); // alloc string size
		result[i].resize(fileNameSize);
		DragQueryFileA(boh, i, result[i].data(), fileNameSize + 1); // +1 because the string also has a null termination character
	}

	return result;
}


#define HID_USAGE_PAGE_GENERIC         ((USHORT) 0x01)
#define HID_USAGE_GENERIC_MOUSE        ((USHORT) 0x02)

static LRESULT CALLBACK GroovyWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#if WITH_EDITOR
	Win32_EditorWndProcCallback(hWnd, uMsg, wParam, lParam);
#endif

	switch (uMsg)
	{
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		{
			Input::OnKeyDown(wParam);
		}
		break;

		case WM_KEYUP:
		case WM_SYSKEYUP:
		{
			Input::OnKeyUp(wParam);
		}
		break;

		case WM_LBUTTONDOWN:
		{
			Input::OnKeyDown(VK_LBUTTON);
		}
		break;

		case WM_LBUTTONUP:
		{
			Input::OnKeyUp(VK_LBUTTON);
		}
		break;

		case WM_RBUTTONDOWN:
		{
			Input::OnKeyDown(VK_RBUTTON);
		}
		break;

		case WM_RBUTTONUP:
		{
			Input::OnKeyUp(VK_RBUTTON);
		}
		break;

		case WM_INPUT:
		{
			UINT dwSize = sizeof(RAWINPUT);
			static BYTE lpb[sizeof(RAWINPUT)];

			GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER));

			RAWINPUT* raw = (RAWINPUT*)lpb;

			if (raw->header.dwType == RIM_TYPEMOUSE)
			{
				Input::OnMouseMove(raw->data.mouse.lLastX, raw->data.mouse.lLastY);
			}
		}
		break;

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
			Window* wnd = (Window*)GetWindowLongPtrA(hWnd, GWLP_USERDATA);
			if (!wnd->OnClose())
			{
				return FALSE; // event handled
			}
			gEngineShouldRun = false;
		}
		break;

		case WM_CREATE:
		{
			CREATESTRUCTA* params = (CREATESTRUCTA*)lParam;
			SetWindowLongPtrA(hWnd, GWLP_USERDATA, (LONG_PTR)params->lpCreateParams);

			RAWINPUTDEVICE Rid[1];
			Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
			Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
			Rid[0].dwFlags = RIDEV_INPUTSINK;
			Rid[0].hwndTarget = hWnd;
			checkslow(RegisterRawInputDevices(Rid, 1, sizeof(Rid[0])));
		}
		break;

		case WM_DROPFILES:
		{
			std::vector<std::string> droppedFiles = GetDroppedFiles((HDROP)wParam);
			Window* wnd = (Window*)GetWindowLongPtrA(hWnd, GWLP_USERDATA);
			wnd->OnFilesDropped(droppedFiles);
		}
		break;

		case WM_DESTROY:
		{
			Window* wnd = (Window*)GetWindowLongPtrA(hWnd, GWLP_USERDATA);
			wnd->OnDestroy();
		}
		break;
	}

	return DefWindowProcA(hWnd, uMsg, wParam, lParam);
}

Window::Window(const WindowProps& props)
	: mProps(props), mHandle(nullptr), mWndCloseCallback(nullptr)
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

void Window::SetFullscreen(bool fullscreen)
{
	RendererAPI::Get().SetFullscreen(fullscreen);
	switch (RendererAPI::GetAPI())
	{
		// custom stuff
		// case opengl: trigger wm_size etc...
	}
	mProps.fullscreen = fullscreen;
}

void Window::EnableCursor(bool enable)
{
	POINT cursorPos;
	::GetCursorPos(&cursorPos);

	if (enable)
	{
		::SetCursor(LoadCursorA(nullptr, nullptr));
		while (::ShowCursor(true) < 0);
		::ClipCursor(nullptr);
		::SetCursorPos(cursorPos.x, cursorPos.y);
	}
	else
	{
		RECT cursorLock;
		cursorLock.left = cursorPos.x;
		cursorLock.top = cursorPos.y;
		cursorLock.right = cursorPos.x + 1;
		cursorLock.bottom = cursorPos.y + 1;

		::ClipCursor(&cursorLock);

		while (::ShowCursor(false) >= 0);
	}
}

void Window::SetCursorPos(uint32 x, uint32 y)
{
	::SetCursorPos(x, y);
}

void Window::GetCursorPos(uint32* xy)
{
	POINT p;
	::GetCursorPos(&p);
	xy[0] = p.x;
	xy[1] = p.y;
}

void Window::SetMaxSize()
{
	HMONITOR monitor = MonitorFromWindow((HWND)mHandle, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFOEXA monitorInfo = {};
	monitorInfo.cbSize = sizeof(MONITORINFOEXA);
	GetMonitorInfoA(monitor, &monitorInfo);
	DEVMODEA devmode = {};
	devmode.dmSize = sizeof(DEVMODEA);
	EnumDisplaySettingsExA(monitorInfo.szDevice, ENUM_CURRENT_SETTINGS, &devmode, 0);

	::SetWindowPos((HWND)mHandle, nullptr, 100, 100, devmode.dmPelsWidth, devmode.dmPelsHeight, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);
}

bool Window::OnClose()
{
	if (mWndCloseCallback)
		return mWndCloseCallback();
	return true;
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

void Window::OnFilesDropped(const std::vector<std::string>& files)
{
	for (auto fileDropCallback : mWndFilesDropCallbacks)
	{
		fileDropCallback(files);
	}
}

void Window::OnDestroy()
{
	mHandle = nullptr;
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