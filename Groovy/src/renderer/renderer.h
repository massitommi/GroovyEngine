#pragma once

#include "mesh.h"
#include "math/matrix.h"

class Renderer
{
public:
	static void Init();
	static void Shutdown();
	static void BeginScene(Mat4& mvp);
	static void RenderMesh(Mesh* mesh);
};