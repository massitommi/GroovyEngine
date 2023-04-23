#include "core/core.h"

// to be defined in Editor or Sandbox, depending on the build configuration
class Application
{
public:
	static void Init();
	static void Update(float deltaTime);
	static void Render();
#if WITH_EDITOR
	static void Render2EditorOnly();
#endif
	static void Shutdown();
};