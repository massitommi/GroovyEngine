#if PLATFORM_WIN32

#include "platform/filedialog.h"
#include "platform/window.h"
#include "win32_globals.h"


static String GetNativeFilters(const ExtensionFilters& filters)
{
	if (filters.empty()) return "";
	String finalFilter;

	for (const ExtensionFilter& filter : filters)
	{
		finalFilter.append(filter.description);
		finalFilter.append("\0", 1);
		for (const String& extension : filter.extensions)
		{
			finalFilter.append(extension);
			finalFilter.append(";");
		}
		finalFilter.append("\0", 1);
	}

	finalFilter.append("\0", 1);
	return finalFilter;
}

String FileDialog::OpenFileDialog(const String& titleBar, const ExtensionFilters& filters)
{
	String nativeFilters = GetNativeFilters(filters);

	OPENFILENAMEA ofn = {};
	CHAR szFile[260] = { 0 };
	CHAR currentDir[256] = { 0 };
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = Window::Get() ? (HWND)Window::Get()->GetNativeHandle() : nullptr;
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

String FileDialog::SaveFileDialog(const String& titleBar)
{
	OPENFILENAMEA ofn = {};
	CHAR szFile[260] = { 0 };
	CHAR currentDir[256] = { 0 };
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = Window::Get() ? (HWND)Window::Get()->GetNativeHandle() : nullptr;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	if (GetCurrentDirectoryA(256, currentDir))
		ofn.lpstrInitialDir = currentDir;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	ofn.lpstrTitle = titleBar.c_str();
	
	if (!GetSaveFileNameA(&ofn))
	{
		return "";
	}

	return ofn.lpstrFile;
}

#endif