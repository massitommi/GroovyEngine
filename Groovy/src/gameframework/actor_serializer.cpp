#include "actor_serializer.h"
#include "classes/object_serializer.h"
#include "classes/class_db.h"
#include "actorcomponent.h"
#include "blueprint.h"

void ActorSerializer::CreateActorPack(Actor* actor, ActorPack& outPack)
{
	check(actor);

	ActorBlueprint* actorTemplate = actor->mTemplate;
	GroovyObject* actorCDO = nullptr;

	if (actorTemplate)
		actorCDO = actorTemplate->GetDefaultActor();
	else
		actorCDO = actor->GetCDO();

	// actor data
	outPack.actorClass = actor->GetClass();
	ObjectSerializer::CreatePropertyPack(actor, actorCDO, outPack.actorProperties);

	// components
	for (ActorComponent* component : actor->mComponents)
	{
		GroovyObject* componentCDO = nullptr;

		if (actorTemplate && component->mType != ACTOR_COMPONENT_TYPE_EDITOR_SCENE)
			componentCDO = actorTemplate->GetDefaultActor()->GetComponent(component->mName);
		else
			componentCDO = component->GetCDO();

		PropertyPack componentProperties;
		ObjectSerializer::CreatePropertyPack(component, componentCDO, componentProperties);

		// avoid saving useless components data
		if (!componentProperties.desc.size())
		{
			if (component->mType == ACTOR_COMPONENT_TYPE_NATIVE)
				continue;
			else if (component->mType == ACTOR_COMPONENT_TYPE_EDITOR_BP && actorTemplate)
				continue;
		}

		// ok we need to save the groovy stuff
		ComponentPack& pack = outPack.actorComponents.emplace_back();
		pack.componentName = component->mName;
		pack.componentType = component->mType;
		pack.componentClass = component->GetClass();
		pack.componentProperties = std::move(componentProperties);
	}
}

void ActorSerializer::SerializeActorPack(const ActorPack& pack, DynamicBuffer& fileData)
{
	if (!pack.actorClass)
		return;

	// actor class name
	fileData.push<std::string>(pack.actorClass->name);

	// actor file size
	fileData.push<size_t>(0);
	DYNAMIC_BUFFER_TRACK(actorFileSize, fileData);

	// actor properties
	ObjectSerializer::SerializePropertyPack(pack.actorProperties, fileData);

	// actor components
	fileData.push<uint32>(pack.actorComponents.size());
	for (const ComponentPack& comp : pack.actorComponents)
	{
		fileData.push<std::string>(comp.componentClass->name);

		fileData.push<size_t>(0);
		DYNAMIC_BUFFER_TRACK(componentSubfileSize, fileData);

		fileData.push<std::string>(comp.componentName);
		fileData.push<EActorComponentType>(comp.componentType);
		ObjectSerializer::SerializePropertyPack(comp.componentProperties, fileData);

		DYNAMIC_BUFFER_TRACK_WRITE_RESULT(componentSubfileSize, fileData);
	}

	DYNAMIC_BUFFER_TRACK_WRITE_RESULT(actorFileSize, fileData);
}

void ActorSerializer::DeserializeActorPack(BufferView& fileData, ActorPack& outPack)
{
	if (fileData.empty())
		return;

	extern ClassDB gClassDB;

	std::string actorClassName = fileData.read<std::string>();
	size_t actorFileSize = fileData.read<size_t>();
	GroovyClass* actorClass = gClassDB[actorClassName];
	
	if (!actorClass || !classUtils::IsA(actorClass, Actor::StaticClass()))
		fileData.advance(actorFileSize);

	outPack.actorClass = actorClass;
	ObjectSerializer::DeserializePropertyPack(actorClass, fileData, outPack.actorProperties);

	uint32 componentsCount = fileData.read<uint32>();
	for (uint32 i = 0; i < componentsCount; i++)
	{
		std::string componentClassName = fileData.read<std::string>();
		size_t componentSubfileSize = fileData.read<size_t>();
		GroovyClass* componentClass = gClassDB[componentClassName];

		if (!componentClass || !classUtils::IsA(componentClass, ActorComponent::StaticClass()))
			fileData.advance(componentSubfileSize);

		ComponentPack& compPack = outPack.actorComponents.emplace_back();
		compPack.componentClass = componentClass;
		compPack.componentName = fileData.read<std::string>();
		compPack.componentType = fileData.read<EActorComponentType>();
		ObjectSerializer::DeserializePropertyPack(componentClass, fileData, compPack.componentProperties);
	}
}

void ActorSerializer::DeserializeActorPackData(const ActorPack& pack, Actor* actor)
{
	check(actor);
	check(pack.actorClass);
	check(pack.actorClass == actor->GetClass());

	// considerations:
	// the actor class is a subclass of Actor
	// each component class is a subclass of ActorComponent
	// components classes and components name could have changed

	ObjectSerializer::DeserializePropertyPackData(pack.actorProperties, actor);

	for (const ComponentPack& compPack : pack.actorComponents)
	{
		ActorComponent* comp = nullptr;

		switch (compPack.componentType)
		{
			case ACTOR_COMPONENT_TYPE_NATIVE:
			{
				comp = actor->GetComponent(compPack.componentName);
			}
			break;

			case ACTOR_COMPONENT_TYPE_EDITOR_BP:
			{
				if (actor->mTemplate)
				{
					comp = actor->GetComponent(compPack.componentName);
				}
				else
				{
					comp = actor->AddComponent(compPack.componentClass, compPack.componentName);
					comp->mType = ACTOR_COMPONENT_TYPE_EDITOR_BP;
				}
			}	
			break;

			case ACTOR_COMPONENT_TYPE_EDITOR_SCENE:
			{
				comp = actor->AddComponent(compPack.componentClass, compPack.componentName);
				comp->mType = ACTOR_COMPONENT_TYPE_EDITOR_SCENE;
			}
			break;
		}

		if (comp && comp->mType == compPack.componentType && comp->GetClass() == compPack.componentClass)
			ObjectSerializer::DeserializePropertyPackData(compPack.componentProperties, comp);
	}
}