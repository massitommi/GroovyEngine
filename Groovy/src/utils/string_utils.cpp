#include "string_utils.h"

bool stringUtils::EqualsCaseInsensitive(std::string_view str1, std::string_view str2)
{
	if (str1.length() != str2.length())
		return false;

	for (int i = 0; i < str1.length(); i++)
	{
		if (tolower(str1[i]) != tolower(str2[i]))
			return false;
	}

	return true;
}

bool stringUtils::Replace(std::string& str, std::string_view find, std::string_view replace)
{
	size_t pos = str.find(find);
	if (pos != std::string::npos)
	{
		str.replace(pos, find.length(), replace);
		return true;
	}
	return false;
}

uint32 stringUtils::ReplaceAll(std::string& str, std::string_view find, std::string_view replace)
{
	uint32 replaceCount = 0;

	while (Replace(str, find, replace))
		replaceCount++;
	
	return replaceCount;
}

std::string stringUtils::ToString(Vec2 vec)
{
	const char* format = "X= %4.3f   Y= %4.3f";
	char buffer[64];
	sprintf_s(buffer, 64, format, vec.x, vec.y);

	return buffer;
}

std::string stringUtils::ToString(Vec3 vec)
{
	const char* format = "X= %4.3f   Y= %4.3f   Z= %4.3f";
	char buffer[96];
	sprintf_s(buffer, 96, format, vec.x, vec.y, vec.z);

	return buffer;
}

std::string stringUtils::ToString(Vec4 vec)
{
	const char* format = "X= %4.3f   Y= %4.3f   Z= %4.3f   W= %4.3f";
	char buffer[128];
	sprintf_s(buffer, 128, format, vec.x, vec.y, vec.z, vec.w);

	return buffer;
}