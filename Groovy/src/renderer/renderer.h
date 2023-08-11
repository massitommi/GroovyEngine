#pragma once

#include "mesh.h"
#include "math/matrix.h"

#define VIEW_PROJECTION_BUFFER_INDEX 0
#define MODEL_BUFFER_INDEX 1

class Renderer
{
public:
	static void Init();
	
	static void SetCamera(Mat4& vpMatrix);

	static void SetModel(Mat4& modelMatrix);

	static void RenderMesh(Mesh* mesh, const std::vector<Material*>& materials);
	static void RenderMesh(Mesh* mesh);

	static void Shutdown();
};