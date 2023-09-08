#include "gameframework/actorcomponent.h"

GROOVY_CLASS_DECL(DemoRotatingComponent)
class DemoRotatingComponent : public ActorComponent
{
	GROOVY_CLASS_BODY(DemoRotatingComponent, ActorComponent)
public:
	DemoRotatingComponent();

	virtual void Tick(float deltaTime) override;

private:
	Vec3 mRotation;
};