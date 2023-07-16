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
