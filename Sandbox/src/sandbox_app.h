#pragma once

#include "engine/application.h"

class SandboxApplication : public Application
{
public:

	APP_FUNC_CALL void Init();
	APP_FUNC_CALL void Update(float deltaTime);
	APP_FUNC_CALL void Render();
	APP_FUNC_CALL void Shutdown();

	// If scene is nullptr the application quits
	APP_FUNC_CALL void Travel(class Scene* scene);
};