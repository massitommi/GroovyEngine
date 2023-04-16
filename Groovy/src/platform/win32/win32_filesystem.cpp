#if PLATFORM_WIN32

#include "platform/filesystem.h"
#include "win32_globals.h"

#include <fileapi.h>

EFileOpenResult FileSystem::ReadFileBinary(const String& path, Buffer& outBuffer)
{
	HANDLE handle = CreateFileA(*path, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
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
	HANDLE handle = CreateFileA(*path, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
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