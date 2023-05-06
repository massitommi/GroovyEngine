#include <Windows.h>
#include "vendor/imgui/imgui.h"
#include "vendor/imgui/backends/imgui_impl_win32.h"
#include "core/core.h"

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

IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void OnFilesDropped(const std::vector<std::string>& files);

LRESULT Win32_EditorWndProcCallback(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_DROPFILES:
		{
			std::vector<std::string> filesDropped = GetDroppedFiles((HDROP)wParam);
			OnFilesDropped(filesDropped);
			break;
		}
	}

	return ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
}