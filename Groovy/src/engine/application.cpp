#if !BUILD_MONOLITHIC

#include "application.h"

void Application::Init() {}
void Application::Update(float deltaTime) {}
void Application::Render() {}
void Application::Shutdown() {}

// If scene is nullptr the application quits
void Application::Travel(class Scene* scene) {}

#endif