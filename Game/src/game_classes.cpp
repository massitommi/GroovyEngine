#include "classes/class.h"

// Add your game classes here

#include "gameframework/actor.h"
#include "gameframework/actorcomponent.h"

GROOVY_CLASS_DECL(TestActorComponent)
class TestActorComponent : public ActorComponent
{
	GROOVY_CLASS_BODY(TestActorComponent, ActorComponent)
public:
	bool mTestBool = true;
};

GROOVY_CLASS_IMPL(TestActorComponent)
	GROOVY_REFLECT(mTestBool)
GROOVY_CLASS_END()

GROOVY_CLASS_DECL(TestActorComponentDerived)
class TestActorComponentDerived : public TestActorComponent
{
	GROOVY_CLASS_BODY(TestActorComponentDerived, TestActorComponent)
public:
	bool mDerivedBool = false;
	std::string mDerivedStr = "testing123";
};

GROOVY_CLASS_IMPL(TestActorComponentDerived)
	GROOVY_REFLECT(mDerivedBool)
	GROOVY_REFLECT(mDerivedStr)
GROOVY_CLASS_END()

GROOVY_CLASS_DECL(TestActor)
class TestActor : public Actor
{
	GROOVY_CLASS_BODY(TestActor, Actor)

public:
	TestActor()
	{
		testComp1 = AddComponent<TestActorComponent>("my test comp");
		testComp2 = AddComponent<TestActorComponentDerived>("my test comp 2");

		counter = 0.0f;
	}

	virtual void BeginPlay()
	{
		counter = 1.0f;
	}

	virtual void Tick(float deltaTime)
	{
		counter += 0.5f;
	}

private:
	TestActorComponent* testComp1;
	TestActorComponentDerived* testComp2;
	float speed = 5.0f;
	
	float counter;
};

GROOVY_CLASS_IMPL(TestActor)
	GROOVY_REFLECT(speed)
	GROOVY_REFLECT_EX(counter, PROPERTY_FLAG_NO_SERIALIZE)
GROOVY_CLASS_END()

CLASS_LIST_BEGIN(GAME_CLASSES)
{
	CLASS_LIST_ADD(TestActorComponent),
	CLASS_LIST_ADD(TestActorComponentDerived),
	CLASS_LIST_ADD(TestActor)
}
CLASS_LIST_END()