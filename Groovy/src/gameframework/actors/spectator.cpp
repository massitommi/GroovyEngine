#include "spectator.h"
#include "gameframework/components/cameracomponent.h"
#include "platform/input.h"

GROOVY_CLASS_IMPL(Spectator)
	GROOVY_REFLECT(mMovementSpeed)
	GROOVY_REFLECT(mCameraRotationSpeed)
GROOVY_CLASS_END()

Spectator::Spectator()
	: mMovementSpeed(1.0f), mCameraRotationSpeed(0.5f)
{
	mCamera = AddComponent<CameraComponent>("CameraComponent");
}

void Spectator::Tick(float deltaTime)
{
	// Rotate camera

	int32 mouseDelta[2];
	Input::GetRawMouseDelta(mouseDelta);

	Vec3 rotation = GetRotation();

	rotation.y += (float)mouseDelta[0] * mCameraRotationSpeed;
	rotation.x += (float)mouseDelta[1] * mCameraRotationSpeed;

	SetRotation(rotation);

	// Move 

	Vec3 inputDirection = { 0.0f, 0.0f, 0.0f };

	if (Input::IsKeyDown(EKeyCode::W))
		inputDirection.z += 1.0f;
	if (Input::IsKeyDown(EKeyCode::S))
		inputDirection.z += -1.0f;
	if(Input::IsKeyDown(EKeyCode::D))
		inputDirection.x += 1.0f;
	if (Input::IsKeyDown(EKeyCode::A))
		inputDirection.x += -1.0f;
	if (Input::IsKeyDown(EKeyCode::Q))
		inputDirection.y += 1.0f;
	if(Input::IsKeyDown(EKeyCode::E))
		inputDirection.y += -1.0f;

	if (inputDirection.x || inputDirection.y || inputDirection.z)
	{
		Vec3 movement = 
			math::GetForwardVector(GetRotation()) * inputDirection.z
			+
			math::GetRightVector(GetRotation()) * inputDirection.x
			+
			math::GetUpVector(GetRotation()) * inputDirection.y;

		movement = math::Normalize(movement);
		movement *= mMovementSpeed * deltaTime;

		SetLocation(GetLocation() + movement);
	}
}
