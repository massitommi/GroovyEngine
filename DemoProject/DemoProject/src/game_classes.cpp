#include "game.h"
#include "classes/class.h"

// Add your game classes here

#include "demo_rotating_component.h"

GAME_CLASS_LIST_BEGIN {

GAME_API CLASS_LIST_BEGIN(GAME_CLASSES_LIST)
{
	CLASS_LIST_ADD(DemoRotatingComponent)
}
CLASS_LIST_END()

}