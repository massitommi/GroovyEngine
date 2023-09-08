#pragma once

#include "core/core.h"
#include "math/vector.h"

class CORE_API SceneRenderer
{
public:
	static void BeginScene(Vec3 camLocation, Vec3 camRotation, float FOV, float aspectRatio);
	static void BeginScene(class CameraComponent* camera, float aspectRatio);
	
	static void RenderScene(class Scene* scene);
};