#pragma once

#include "core/core.h"
#include "math/vector.h"

class FrameBuffer;
class MeshComponent;
class CameraComponent;

class SceneRenderer
{
public:

	static void SetFrameBuffer(FrameBuffer* frameBuffer);
	static void ClearMeshQueue();

	static void Submit(MeshComponent* mesh);
	static void Remove(MeshComponent* mesh);

	static void BeginScene(Vec3 camLocation, Vec3 camRotation, float FOV);
	static void BeginScene(CameraComponent* camera);
	static void RenderScene();
};