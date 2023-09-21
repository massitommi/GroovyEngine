#include "string_utils.h"

bool stringUtils::EqualsCaseInsensitive(const std::string& str1, const std::string& str2)
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
