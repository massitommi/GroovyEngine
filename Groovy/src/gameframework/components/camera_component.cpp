#include "camera_component.h"
#include "gameframework/actor.h"
#include "gameframework/scene.h"

GROOVY_CLASS_IMPL(CameraComponent)
	GROOVY_REFLECT(mFOV)
	GROOVY_REFLECT(mSceneMainCamera)
GROOVY_CLASS_END()

CameraComponent::CameraComponent()
	: mFOV(60.0f), mSceneMainCamera(false)
{
}

void CameraComponent::BeginPlay()
{
	if(mSceneMainCamera)
		GetOwner()->GetScene()->mCamera = this;
}
