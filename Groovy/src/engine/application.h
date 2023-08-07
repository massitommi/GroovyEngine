#include "core/core.h"

// to be defined in Editor or Sandbox, depending on the build configuration
class Application
{
public:
	static void Init();
	static void Update(float deltaTime);
	static void Render();
	static void Shutdown();

	// If scene is nullptr the application quits
	void Travel(class Scene* scene);
};