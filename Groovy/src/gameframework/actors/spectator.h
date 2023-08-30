#pragma once

#include "gameframework/actor.h"

GROOVY_CLASS_DECL(Spectator)
class CORE_API Spectator : public Actor
{
	GROOVY_CLASS_BODY(Spectator, Actor)

public:
	Spectator();

	inline class CameraComponent* GetCamera() const { return mCamera; }

	virtual void Tick(float deltaTime);

private:
	class CameraComponent* mCamera;
	float mMovementSpeed;
	float mCameraRotationSpeed;
};