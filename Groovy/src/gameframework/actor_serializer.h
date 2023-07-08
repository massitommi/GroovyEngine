#pragma once

#include "actor.h"

class ActorSerializer
{
public:
	static void CreateActorPack(Actor* actor, Actor* cdo, ActorPack& outPack);
	
	static void SerializeActorPack(const ActorPack& pack, DynamicBuffer& fileData);
	static void DeserializeActorPack(BufferView& fileData, ActorPack& outPack);

	static void DeserializeActorPackData(const ActorPack& pack, Actor* actor);
};