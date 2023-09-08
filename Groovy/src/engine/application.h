#pragma once
#include "core/core.h"

// to be defined in Editor or Sandbox, depending on the build configuration
class CORE_API Application
{
public:
	static void Init();
	static void Update(float deltaTime);
	static void Render();
	static void Shutdown();

	// If scene is nullptr the application quits
	// Useless because works only in monolithic builds, if you try to use this function
	// in game code dll it will generate a linker error.
	static void Travel(class Scene* scene);
};