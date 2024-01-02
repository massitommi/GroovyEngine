#include "classes/class.h"

#include "engine/project.h"

#include "renderer/material.h"
#include "renderer/mesh.h"

#include "gameframework/actor.h"
#include "gameframework/actor_component.h"
#include "gameframework/components/mesh_component.h"
#include "gameframework/components/camera_component.h"
#include "gameframework/components/audio_component.h"
#include "gameframework/actors/mesh_actor.h"
#include "gameframework/actors/spectator.h"


#define CLASS_LIST				CORE_API std::vector<GroovyClass*>
#define ADD_CLASS(Class)		&GROOVY_CLASS_NAME(Class)

CLASS_LIST ENGINE_CLASSES
{
	ADD_CLASS(GroovyObject),

	ADD_CLASS(GroovyProject),

	ADD_CLASS(MaterialAssetFile),
	ADD_CLASS(MeshAssetFile),

	ADD_CLASS(Actor),
	ADD_CLASS(ActorComponent),
	ADD_CLASS(SceneComponent),

	ADD_CLASS(MeshComponent),
	ADD_CLASS(CameraComponent),
	ADD_CLASS(AudioComponent),

	ADD_CLASS(MeshActor),
	ADD_CLASS(Spectator)
};