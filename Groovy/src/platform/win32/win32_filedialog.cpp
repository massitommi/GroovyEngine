#if PLATFORM_WIN32

#include "platform/filedialog.h"
#include "platform/window.h"
#include "win32_globals.h"

extern Window* gWindow;

static std::string GetNativeFilters(const ExtensionFilters& filters)
{
	if (filters.empty()) return "";
	std::string finalFilter;

	for (const ExtensionFilter& filter : filters)
	{
		finalFilter.append(filter.description);
		finalFilter.append("\0", 1);
		for (const std::string& extension : filter.extensions)
		{
			finalFilter.append(extension);
			finalFilter.append(";");
		}
		finalFilter.append("\0", 1);
	}

	finalFilter.append("\0", 1);
	return finalFilter;
}

std::string FileDialog::OpenFileDialog(const std::string& titleBar, const ExtensionFilters& filters)
{
	std::string nativeFilters = GetNativeFilters(filters);

	OPENFILENAMEA ofn = {};
	CHAR szFile[260] = { 0 };
	CHAR currentDir[256] = { 0 };
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = gWindow ? (HWND)gWindow->GetNativeHandle() : nullptr;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	if (GetCurrentDirectoryA(256, currentDir))
		ofn.lpstrInitialDir = currentDir;
	ofn.lpstrFilter = nativeFilters.length() > 0 ? nativeFilters.c_str() : nullptr;
	ofn.nFilterIndex = 0;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	ofn.lpstrTitle = titleBar.c_str();

	if (GetOpenFileNameA(&ofn) == TRUE)
		return ofn.lpstrFile;

	return "";
}

std::string FileDialog::SaveFileDialog(const std::string& titleBar)
{
	OPENFILENAMEA ofn = {};
	CHAR szFile[260] = { 0 };
	CHAR currentDir[256] = { 0 };
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = gWindow ? (HWND)gWindow->GetNativeHandle() : nullptr;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	if (GetCurrentDirectoryA(256, currentDir))
		ofn.lpstrInitialDir = currentDir;
	ofn.nFilterIndex = 0;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	ofn.lpstrTitle = titleBar.c_str();
	
	if (!GetSaveFileNameA(&ofn))
	{
		return "";
	}

	return ofn.lpstrFile;
}

#endif