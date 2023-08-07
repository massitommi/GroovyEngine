#include "vector.h"
#include "generic.h"
#include <math.h>
#include <DirectXMath.h>

Vec3 math::GetForwardVector(Vec3 rotation)
{
    rotation = VEC_TO_RAD(rotation);

    Vec3 forward;
    
    forward.x = cos(rotation.x) * sin(rotation.y);
    forward.y = -sin(rotation.x);
    forward.z = cos(rotation.x) * cos(rotation.y);
    
    return forward;
}

Vec3 math::GetRightVector(Vec3 rotation)
{
    rotation = VEC_TO_RAD(rotation);

    Vec3 right;

    right.x = cos(rotation.y);
    right.y = 0.0f;
    right.z = -sin(rotation.y);

    return right;
}

Vec3 math::GetUpVector(Vec3 rotation)
{
    DirectX::XMVECTOR up = DirectX::XMVector3Cross(VEC_TO_XVEC(GetForwardVector(rotation)), VEC_TO_XVEC(GetRightVector(rotation)));

    return { up.m128_f32[0], up.m128_f32[1], up.m128_f32[2] };
}

float math::Magnitude(Vec3 v)
{
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

Vec3 math::Normalize(Vec3 v)
{
    return v / Magnitude(v);
}
