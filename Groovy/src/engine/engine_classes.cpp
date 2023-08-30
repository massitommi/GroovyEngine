#include "classes/class.h"

#include "project/project.h"

#include "renderer/material.h"
#include "renderer/mesh.h"

#include "gameframework/actor.h"
#include "gameframework/actorcomponent.h"
#include "gameframework/components/meshcomponent.h"
#include "gameframework/components/cameracomponent.h"
#include "gameframework/actors/meshactor.h"
#include "gameframework/actors/spectator.h"

CORE_API
CLASS_LIST_BEGIN(ENGINE_CLASSES)
{
	CLASS_LIST_ADD(GroovyObject),
	
	CLASS_LIST_ADD(GroovyProject),

	CLASS_LIST_ADD(MaterialAssetFile),
	CLASS_LIST_ADD(MeshAssetFile),

	CLASS_LIST_ADD(Actor),
	CLASS_LIST_ADD(ActorComponent),
	CLASS_LIST_ADD(SceneComponent),

	CLASS_LIST_ADD(MeshComponent),
	CLASS_LIST_ADD(CameraComponent),

	CLASS_LIST_ADD(MeshActor),
	CLASS_LIST_ADD(Spectator)
}
CLASS_LIST_END()