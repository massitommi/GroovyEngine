#include "world.h"
#include "assets/asset_manager.h"

static Scene* sCurrentScene;

void World::Travel(Scene* scene)
{
	if (sCurrentScene)
		delete sCurrentScene;

	if (scene)
		scene->Load();
	else
		scene = new Scene();

	sCurrentScene = scene;
}

Scene* World::GetScene()
{
	return sCurrentScene;
}

void World::End()
{
	if (sCurrentScene)
		delete sCurrentScene;

	sCurrentScene = nullptr;
}
