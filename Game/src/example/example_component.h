#pragma once

#include "gameframework/actorcomponent.h"

GROOVY_CLASS_DECL(ExampleComponent)
class ExampleComponent : public ActorComponent
{
	GROOVY_CLASS_BODY(ExampleComponent, ActorComponent)

public:
	ExampleComponent();

protected:
	virtual void Tick(float deltaTime) override;

private:
	float mRotationSpeed;
};