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
	static void Travel(class Scene* scene);
};