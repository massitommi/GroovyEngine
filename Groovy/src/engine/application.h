#pragma once
#include "core/core.h"

// to be defined in Editor or Sandbox
class CORE_API Application
{
public:
	static void PreInit();
	static void Init();
	static void Update(float deltaTime);
	static void Render();
	static void Shutdown();
};