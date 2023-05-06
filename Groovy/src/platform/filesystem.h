#pragma once

#include "core/core.h"

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

	static const std::string DIR_SEPARATOR;

	inline static size_t find_last_of_slash(const std::string& path)
	{
#if PLATFORM_WIN32
		std::string win32Path = path;
		for (char& c : win32Path)
			if (c == '/')
				c = '\\';

		return win32Path.find_last_of(DIR_SEPARATOR);
#else
		path.find_last_of(DIR_SEPARATOR);
#endif
	}

	inline static std::string GetFilename(const std::string& path)
	{
		size_t start = find_last_of_slash(path);
		return path.substr(start + 1, path.length() - start);
	}

	inline static std::string GetExtension(const std::string& path)
	{
		size_t start = path.find_last_of('.');
		return path.substr(start, std::string::npos);
	}

	inline static std::string GetParentFolder(const std::string& path)
	{
		return path.substr(0, find_last_of_slash(path) + 1);
	}

	inline static std::string GetFilenameNoExt(const std::string& path)
	{
		size_t start = find_last_of_slash(path);
		size_t end = path.find_last_of('.');
		return path.substr(start + 1, end - start -1);
	}

	static std::vector<std::string> GetFilesInDir(const std::string& dir, const std::vector<std::string>& extensionsFilters);
	static EFileOpenResult ReadFileBinary(const std::string& path, void* outBuffer, size_t bufferSize, size_t& outBytesRead);
	static EFileOpenResult ReadFileBinary(const std::string& path, Buffer& outBuffer);
	static EFileOpenResult WriteFileBinary(const std::string& path, const Buffer& data);
};