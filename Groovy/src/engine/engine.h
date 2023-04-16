#include "core/core.h"

// to be defined in Editor or Sandbox, depending on the build configuration
class Engine
{
public:
	static void Init();
	static void Update(float deltaTime);
	static void Render();
	static void Render2();
	static void Shutdown();
};