#pragma once

#include "vector.h"
#include <DirectXMath.h>

typedef DirectX::XMMATRIX Mat4;

namespace math
{
	CORE_API Mat4 GetModelMatrix(Vec3 location, Vec3 rotation, Vec3 scale);
	CORE_API Mat4 GetViewMatrix(Vec3 camLocation, Vec3 camRotation);
	CORE_API Mat4 GetPerspectiveMatrix(float aspectRatio, float fov, float nearZ, float farZ);
	CORE_API Mat4 GetMatrixTransposed(Mat4 matrix);
}