#pragma once

#include <string>
#include "core/coreminimal.h"

namespace stringUtils
{
	CORE_API bool EqualsCaseInsensitive(const std::string& str1, const std::string& str2);
	CORE_API bool Replace(std::string& str, std::string_view find, std::string_view replace);
	CORE_API uint32 ReplaceAll(std::string& str, std::string_view find, std::string_view replace);
}