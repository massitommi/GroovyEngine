#pragma once
#include "core/core.h"

#if !BUILD_MONOLITHIC
	#define APP_FUNC_CALL virtual
#else
	#define APP_FUNC_CALL
#endif

#define IMPL_APPLICATION(ApplicationClass)	class Application* SpawnApplication() { return new ApplicationClass(); }

// to be defined in Editor or Sandbox, depending on the build configuration
class CORE_API Application
{
public:
	APP_FUNC_CALL void Init();
	APP_FUNC_CALL void Update(float deltaTime);
	APP_FUNC_CALL void Render();
	APP_FUNC_CALL void Shutdown();

	// If scene is nullptr the application quits
	APP_FUNC_CALL void Travel(class Scene* scene);
};