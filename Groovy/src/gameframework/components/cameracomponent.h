#pragma once

#include "gameframework/actorcomponent.h"

GROOVY_CLASS_DECL(CameraComponent)
class CameraComponent : public SceneComponent
{
	GROOVY_CLASS_BODY(CameraComponent, SceneComponent)

public:
	CameraComponent();

	virtual void BeginPlay();

	float mFOV;
	bool mSceneMainCamera;

	friend class SceneRenderer;
};