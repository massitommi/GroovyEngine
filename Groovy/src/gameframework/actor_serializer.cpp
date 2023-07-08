#include "actor_serializer.h"
#include "classes/object_serializer.h"
#include "classes/class_db.h"
#include "actorcomponent.h"

void ActorSerializer::CreateActorPack(Actor* actor, Actor* cdo, ActorPack& outPack)
{
	check(actor);

	if (cdo)
	{
		check(actor->GetClass() == cdo->GetClass());
	}
	else
	{
		cdo = (Actor*)actor->GetCDO();
	}

	// actor data
	ObjectSerializer::CreatePropertyPack(actor, cdo, outPack.actorProperties);

	// native components
	for (ActorComponent* comp : actor->mNativeComponents)
	{
		ActorComponent* compCDO = cdo->GetComponent(comp->GetName());
		PropertyPack tmpPack;
		ObjectSerializer::CreatePropertyPack(comp, compCDO, tmpPack);
		if (tmpPack.desc.size())
		{
			ComponentPack& compPack = outPack.nativeComponents.emplace_back();
			compPack.componentName = comp->GetName();
			compPack.componentClass = comp->GetClass();
			compPack.componentProperties = std::move(tmpPack);
		}
	}

	// editor components
	for (ActorComponent* comp : actor->mEditorComponents)
	{
		ActorComponent* compCDO = cdo->GetComponent(comp->GetName());
		if (!compCDO) // this component was added in the level editor
		{
			compCDO = (ActorComponent*)comp->GetClass()->cdo;
		}

		ComponentPack& compPack = outPack.editorComponents.emplace_back();
		compPack.componentName = comp->GetName();
		compPack.componentClass = comp->GetClass();
		ObjectSerializer::CreatePropertyPack(comp, compCDO, compPack.componentProperties);
	}
}

void ActorSerializer::SerializeActorPack(const ActorPack& pack, DynamicBuffer& fileData)
{
	check(pack.actorClass);

	// actor class name
	fileData.push<std::string>(pack.actorClass->name);
	
	// actor properties
	ObjectSerializer::SerializePropertyPack(pack.actorProperties, fileData);

	// native components count
	fileData.push<uint32>(pack.nativeComponents.size());
	// native components data
	for (const ComponentPack& compPack : pack.nativeComponents)
	{
		fileData.push<std::string>(compPack.componentName);
		fileData.push<std::string>(compPack.componentClass->name);
		fileData.push<size_t>(compPack.componentProperties.data.size());
		ObjectSerializer::SerializePropertyPack(compPack.componentProperties, fileData);
	}

	// editor components count
	fileData.push<uint32>(pack.editorComponents.size());
	// editor components data
	for (const ComponentPack& compPack : pack.editorComponents)
	{
		fileData.push<std::string>(compPack.componentName);
		fileData.push<std::string>(compPack.componentClass->name);
		fileData.push<size_t>(compPack.componentProperties.data.size());
		ObjectSerializer::SerializePropertyPack(compPack.componentProperties, fileData);
	}
}

void ActorSerializer::DeserializeActorPack(BufferView& fileData, ActorPack& outPack)
{
	extern ClassDB gClassDB;
	std::string actorClassName = fileData.read<std::string>();
	GroovyClass* actorClass = gClassDB[actorClassName];

	if (actorClass && classUtils::IsA(actorClass, Actor::StaticClass()))
	{
		outPack.actorClass = actorClass;
		ObjectSerializer::DeserializePropertyPack(actorClass, fileData, outPack.actorProperties);
	}
	else
	{
		// Warning: this file is not up to date, this class no longer exists or does not inherit from Actor anymore
		return;
	}

	auto DeserializeComponent = [](BufferView& fileData, std::vector<ComponentPack>& outPack)
	{
		std::string compName = fileData.read<std::string>();
		std::string compClassName = fileData.read<std::string>();
		size_t compDataSize = fileData.read<size_t>();

		GroovyClass* compClass = gClassDB[compClassName];

		if (compClass && classUtils::IsA(compClass, ActorComponent::StaticClass()))
		{
			ComponentPack& compPack = outPack.emplace_back();
			compPack.componentName = compName;
			compPack.componentClass = compClass;
			ObjectSerializer::DeserializePropertyPack(compClass, fileData, compPack.componentProperties);
		}
		else
		{
			// Warning: this file is not up to date, this component class does not exist anymore or does not inherit from ActorComponent
			fileData.advance(compDataSize + sizeof(uint32) /* property count */);
		}
	};

	// native components
	{
		uint32 componentsCount = fileData.read<uint32>();
		for (uint32 i = 0; i < componentsCount; i++)
			DeserializeComponent(fileData, outPack.nativeComponents);
	}
	// editor components
	{
		uint32 componentsCount = fileData.read<uint32>();
		for (uint32 i = 0; i < componentsCount; i++)
			DeserializeComponent(fileData, outPack.editorComponents);
	}
}

void ActorSerializer::DeserializeActorPackData(const ActorPack& pack, Actor* actor)
{
	check(actor);
	check(pack.actorClass == actor->GetClass());

	// actor data
	ObjectSerializer::DeserializePropertyPackData(pack.actorProperties, actor);

	// native components
	for (const ComponentPack& compPack : pack.nativeComponents)
	{
		ActorComponent* compInstance = actor->GetComponent(compPack.componentName);

		if (compInstance && compInstance->GetClass() == compPack.componentClass)
		{
			ObjectSerializer::DeserializePropertyPackData(compPack.componentProperties, compInstance);
		}
		else
		{
			// Warning: This file is not up to date, this native component has been removed / renamed or the class has changed, ignoring deserialization
		}
	}

	// editor components
	for (const ComponentPack& compPack : pack.nativeComponents)
	{
		ActorComponent* compInstance = actor->GetComponent(compPack.componentName);

		if (!compInstance) // this component was added in the level editor
		{
			compInstance = actor->__internal_AddEditorComponent(compPack.componentClass, compPack.componentName);
		}
		
		if (compInstance->GetClass() == compPack.componentClass)
		{
			ObjectSerializer::DeserializePropertyPackData(compPack.componentProperties, compInstance);
		}
		else
		{
			// Warning: This file is not up to date, this editor component has changed class, ignoring deserialization
		}
	}
}