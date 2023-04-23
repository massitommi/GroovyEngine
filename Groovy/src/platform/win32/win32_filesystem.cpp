#if PLATFORM_WIN32

#include "platform/filesystem.h"
#include "win32_globals.h"

#include <fileapi.h>

String FileSystem::DIR_SEPARATOR = "\\";

static std::vector<String> GetFilesInDir(const String& dir)
{
	std::vector<String> result;

	WIN32_FIND_DATAA fileData = {};

	HANDLE findHandle = FindFirstFileA(dir.c_str(), &fileData);

	if (findHandle != INVALID_HANDLE_VALUE)
	{
		result.push_back(fileData.cFileName);
	}

	while (FindNextFileA(findHandle, &fileData))
	{
		result.push_back(fileData.cFileName);
	}

	FindClose(findHandle);

	return result;
}

std::vector<String> FileSystem::GetFilesInDir(const String& dir, const std::vector<String>& extensionsFilters)
{
	if (extensionsFilters.empty())
	{
		String noFilterDir = dir;
		noFilterDir.append("\\*");
		return ::GetFilesInDir(noFilterDir);
	}

	std::vector<String> result;
	for (uint32 i = 0; i < extensionsFilters.size(); i++)
	{
		String extDir = dir;
		extDir.append("\\*");
		extDir.append(extensionsFilters[i]);
		auto extResult = ::GetFilesInDir(extDir.c_str());
		for (const auto& extRes : extResult)
			result.push_back(extRes);
	}
	return result;
}

EFileOpenResult FileSystem::ReadFileBinary(const String& path, Buffer& outBuffer)
{
	HANDLE handle = CreateFileA(path.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (handle == INVALID_HANDLE_VALUE)
	{
		return FILE_OPEN_RESULT_UNKNOWN_ERROR;
	}

	DWORD fileSize = GetFileSize(handle, 0);
	outBuffer.resize(fileSize);
	DWORD bytesRead = 0;
	if (!ReadFile(handle, outBuffer.data(), fileSize, &bytesRead, nullptr))
	{
		return FILE_OPEN_RESULT_UNKNOWN_ERROR;
	}
	CloseHandle(handle);

	return FILE_OPEN_RESULT_OK;
}

EFileOpenResult FileSystem::WriteFileBinary(const String& path, const Buffer& data)
{
	HANDLE handle = CreateFileA(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (handle == INVALID_HANDLE_VALUE)
	{
		// todo log something
		return FILE_OPEN_RESULT_UNKNOWN_ERROR;
	}

	WriteFile(handle, data.data(), data.size(), nullptr, nullptr);
	CloseHandle(handle);

	return FILE_OPEN_RESULT_OK;
}

#endif