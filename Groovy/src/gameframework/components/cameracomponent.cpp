#include "cameracomponent.h"

GROOVY_CLASS_IMPL(CameraComponent)
	GROOVY_REFLECT(mFOV)
GROOVY_CLASS_END()

CameraComponent::CameraComponent()
	: mFOV(60.0f)
{
}
