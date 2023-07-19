#pragma once

#include "gameframework/actorcomponent.h"

GROOVY_CLASS_DECL(CameraComponent)
class CameraComponent : public SceneComponent
{
	GROOVY_CLASS_BODY(CameraComponent, SceneComponent)

public:
	CameraComponent();

	float mFOV;

	friend class SceneRenderer;
};