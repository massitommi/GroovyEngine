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

	static EFileOpenResult ReadFileBinary(const String& path, Buffer& outBuffer);
	static EFileOpenResult WriteFileBinary(const String& path, const Buffer& data);
};