#include <Windows.h>
#include "vendor/imgui/imgui.h"
#include "vendor/imgui/backends/imgui_impl_win32.h"
#include "core/core.h"
#include "editor_events.h"

static std::vector<String> GetDroppedFiles(HDROP boh)
{
	std::vector<String> result;

	uint32 numFilesDropped = DragQueryFileA(boh, 0xFFFFFFFF, nullptr, 0);
	for (uint32 i = 0; i < numFilesDropped; i++)
	{
		uint32 fileNameSize = DragQueryFileA(boh, i, nullptr, 0);
		result.emplace_back(); // alloc string size
		result[i].resize(fileNameSize);
		DragQueryFileA(boh, i, result[i].data(), fileNameSize + 1); // +1 because the string also has a null termination character
	}

	return result;
}

IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT Win32_EditorWndProcCallback(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_DROPFILES:
		{
			std::vector<String> filesDropped = GetDroppedFiles((HDROP)wParam);
			DISPATCH_EVENT(DropFilesEvent, filesDropped);
			break;
		}
	}

	return ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
}