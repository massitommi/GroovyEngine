#pragma once

#include "gameframework/scene.h"

class World
{
public:
	static void Travel(Scene* scene);

	static Scene* GetScene();

	static void End();
};