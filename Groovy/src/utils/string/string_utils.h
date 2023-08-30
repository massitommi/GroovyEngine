#pragma once

#include <string>
#include "core/api.h"

namespace stringUtils
{
	CORE_API bool EqualsCaseInsensitive(const std::string& str1, const std::string& str2);
}