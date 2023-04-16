#include "matrix.h"

Mat4 math::GetModelMatrix(Vec3 location, Vec3 rotation, Vec3 scale)
{
    return
    {
        DirectX::XMMatrixScaling(scale.x, scale.y, scale.z)
        *
        DirectX::XMMatrixRotationRollPitchYawFromVector(VEC_TO_RAD(rotation))
        *
        DirectX::XMMatrixTranslation(location.x, location.y, location.z)
    };
}

Mat4 math::GetViewMatrix(Vec3 camLocation, Vec3 camRotation)
{
    auto defaultView = DirectX::XMMatrixLookAtLH(VEC_TO_XVEC(camLocation), VEC_TO_XVEC((GetForwardVector(camRotation) + camLocation)), VEC_TO_XVEC(GetUpVector(camRotation)));
    auto zRotation = DirectX::XMMatrixRotationZ(math::DegToRad(camRotation.z));
    return defaultView * zRotation;
}

Mat4 math::GetPerspectiveMatrix(float aspectRatio, float fov, float nearZ, float farZ)
{
    return DirectX::XMMatrixPerspectiveFovLH(math::DegToRad(fov), aspectRatio, nearZ, farZ);
}

Mat4 math::GetMatrixTransposed(Mat4 matrix)
{
    return DirectX::XMMatrixTranspose(matrix);
}
