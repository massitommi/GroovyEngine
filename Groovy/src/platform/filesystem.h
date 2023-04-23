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

	static String DIR_SEPARATOR;

	inline static String GetFilename(const String& path)
	{
		uint32 start = path.find_first_of(DIR_SEPARATOR);
		return path.substr(start + 1, path.length() - start);
	}

	inline static String GetExtension(const String& path)
	{
		uint32 start = path.find_first_of('.');
		return path.substr(start, std::string::npos);
	}

	inline static String GetFilenameNoExt(const String& path)
	{
		uint32 start = path.find_last_of(DIR_SEPARATOR);
		uint32 end = path.find_last_of('.');
		return path.substr(start + 1, end - start -1);
	}

	static std::vector<String> GetFilesInDir(const String& dir, const std::vector<String>& extensionsFilters);
	static EFileOpenResult ReadFileBinary(const String& path, Buffer& outBuffer);
	static EFileOpenResult WriteFileBinary(const String& path, const Buffer& data);
};