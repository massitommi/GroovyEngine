#include "classes/class.h"

// Add your game classes here

#include "example/example_actor.h"
#include "example/example_component.h"

CLASS_LIST_BEGIN(GAME_CLASSES)
{
	CLASS_LIST_ADD(ExampleActor),
	CLASS_LIST_ADD(ExampleComponent)
}
CLASS_LIST_END()