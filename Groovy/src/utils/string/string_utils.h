#pragma once

#include <string>
#include "core/coreminimal.h"
#include "math/vector.h"

namespace stringUtils
{
	CORE_API bool EqualsCaseInsensitive(std::string_view str1, std::string_view str2);
	CORE_API bool Replace(std::string& str, std::string_view find, std::string_view replace);
	CORE_API uint32 ReplaceAll(std::string& str, std::string_view find, std::string_view replace);

	CORE_API std::string ToString(Vec2 vec);
	CORE_API std::string ToString(Vec3 vec);
	CORE_API std::string ToString(Vec4 vec);
}