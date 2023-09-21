#pragma once

#include "actor.h"
#include "actor_component.h"

struct ComponentPack
{
	std::string componentName;
	EActorComponentType componentType;
	GroovyClass* componentClass;
	PropertyPack componentProperties;
};

struct ActorPack
{
	GroovyClass* actorClass;
	PropertyPack actorProperties;
	std::vector<ComponentPack> actorComponents;
};

class CORE_API ActorSerializer
{
public:
	static void CreateActorPack(Actor* actor, ActorPack& outPack);
	
	static void SerializeActorPack(const ActorPack& pack, DynamicBuffer& fileData);
	static void DeserializeActorPack(BufferView& fileData, ActorPack& outPack);

	static void DeserializeActorPackData(const ActorPack& pack, Actor* actor);
};