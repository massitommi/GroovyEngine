#pragma once

#include "core/api.h"

class CORE_API TickTimer
{
public:

	static void Init();
	static double GetTimeSeconds();
};