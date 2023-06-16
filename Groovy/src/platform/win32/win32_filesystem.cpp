#if PLATFORM_WIN32

#include "platform/filesystem.h"
#include "win32_globals.h"

#include <fileapi.h>

static std::vector<std::string> GetFilesInDir(const std::string& dir)
{
	std::vector<std::string> result;

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

std::vector<std::string> FileSystem::GetFilesInDir(const std::string& dir, const std::vector<std::string>& extensionsFilters)
{
	if (extensionsFilters.empty())
	{
		std::string noFilterDir = dir;
		noFilterDir.append("\\*");
		return ::GetFilesInDir(noFilterDir);
	}

	std::vector<std::string> result;
	for (uint32 i = 0; i < extensionsFilters.size(); i++)
	{
		std::string extDir = dir;
		extDir.append("\\*");
		extDir.append(extensionsFilters[i]);
		auto extResult = ::GetFilesInDir(extDir.c_str());
		for (const auto& extRes : extResult)
			result.push_back(extRes);
	}
	return result;
}

EFileOpenResult FileSystem::ReadFileBinary(const std::string& path, void* outBuffer, size_t bufferSize, size_t& outBytesRead)
{
	check(outBuffer && bufferSize);

	HANDLE handle = CreateFileA(path.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

	if (handle == INVALID_HANDLE_VALUE)
		return FILE_OPEN_RESULT_UNKNOWN_ERROR;

	DWORD fileSize = ::GetFileSize(handle, 0);
	DWORD bytesToRead = bufferSize < fileSize ? bufferSize : fileSize;
	DWORD bytesRead = 0;

	if (bytesToRead)
	{
		if (!ReadFile(handle, outBuffer, bytesToRead, &bytesRead, nullptr))
			return FILE_OPEN_RESULT_UNKNOWN_ERROR;

		outBytesRead = bytesRead;
	}
	
	CloseHandle(handle);

	return FILE_OPEN_RESULT_OK;
}

EFileOpenResult FileSystem::ReadFileBinary(const std::string& path, Buffer& outBuffer)
{
	HANDLE handle = CreateFileA(path.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

	if (handle == INVALID_HANDLE_VALUE)
		return FILE_OPEN_RESULT_UNKNOWN_ERROR;

	DWORD fileSize = GetFileSize(handle, 0);
	DWORD bytesToRead = fileSize;
	DWORD bytesRead = 0;

	if (bytesToRead)
	{
		outBuffer.resize(bytesToRead);
		
		if (!ReadFile(handle, outBuffer.data(), bytesToRead, &bytesRead, nullptr))
			return FILE_OPEN_RESULT_UNKNOWN_ERROR;
	}

	CloseHandle(handle);

	return FILE_OPEN_RESULT_OK;
}

EFileOpenResult FileSystem::WriteFileBinary(const std::string& path, const void* data, size_t sizeBytes)
{
	HANDLE handle = CreateFileA(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (handle == INVALID_HANDLE_VALUE)
	{
		// todo log something
		return FILE_OPEN_RESULT_UNKNOWN_ERROR;
	}

	WriteFile(handle, data, sizeBytes, nullptr, nullptr);
	CloseHandle(handle);

	return FILE_OPEN_RESULT_OK;
}

EFileOpenResult FileSystem::OverwriteFileBinary(const std::string& path, const void* data, size_t sizeBytes, size_t offset)
{
	HANDLE handle = CreateFileA(path.c_str(), GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (handle == INVALID_HANDLE_VALUE)
	{
		// todo log something
		return FILE_OPEN_RESULT_UNKNOWN_ERROR;
	}

	OVERLAPPED overlap = {};
	overlap.Offset = offset;
	overlap.OffsetHigh = offset >> 32;

	WriteFile(handle, data, sizeBytes, nullptr, &overlap);
	CloseHandle(handle);

	return FILE_OPEN_RESULT_OK;
}

#undef DeleteFile()

EFileOpenResult FileSystem::DeleteFile(const std::string& path)
{
	if (DeleteFileA(path.c_str()))
		return FILE_OPEN_RESULT_OK;
	return FILE_OPEN_RESULT_UNKNOWN_ERROR;
}

#endif