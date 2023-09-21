#pragma once
#include "core/api.h"

#if !BUILD_MONOLITHIC
	#define GAME_CLASS_LIST_BEGIN extern "C"
#else
	#define GAME_CLASS_LIST_BEGIN
#endif