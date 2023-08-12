#pragma once

#include "core/core.h"
#include <filesystem>

enum EFileOpenResult
{
	FILE_OPEN_RESULT_FILE_NOT_FOUND,
	FILE_OPEN_RESULT_UNKNOWN_ERROR,
	FILE_OPEN_RESULT_OK
};

class FileSystem
{
public:
	FileSystem() = delete;

	static std::vector<std::string> GetFilesInDir(const std::string& dir, const std::vector<std::string>& extensionsFilters);
	static EFileOpenResult ReadFileBinary(const std::string& path, void* outBuffer, size_t bufferSize, size_t& outBytesRead);
	static EFileOpenResult ReadFileBinary(const std::string& path, Buffer& outBuffer);
	static EFileOpenResult WriteFileBinary(const std::string& path, const void* data, size_t sizeBytes);
	static EFileOpenResult OverwriteFileBinary(const std::string& path, const void* data, size_t sizeBytes, size_t offset);
	static EFileOpenResult DeleteFile(const std::string& path);
	static bool FileExists(const std::string& path);
	static bool Rename(const std::string& path, const std::string& newPath);

	inline static EFileOpenResult WriteFileBinary(const std::string& path, const Buffer& data)
	{
		return WriteFileBinary(path, data.data(), data.size());
	}

	static EFileOpenResult WriteFileBinary(const std::string& path, const DynamicBuffer& data)
	{
		return WriteFileBinary(path, data.data(), data.used());
	}
};