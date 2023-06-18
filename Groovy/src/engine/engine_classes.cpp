#include "classes/class.h"

#include "renderer/material.h"

#include "gameframework/actor.h"
#include "gameframework/actorcomponent.h"

CLASS_LIST_BEGIN(ENGINE_CLASSES)
{
	CLASS_LIST_ADD(GroovyObject),

	CLASS_LIST_ADD(MaterialAssetFile),

	CLASS_LIST_ADD(Actor),
	CLASS_LIST_ADD(ActorComponent)
}
CLASS_LIST_END()