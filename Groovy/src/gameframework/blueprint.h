#pragma once

#include "assets/asset.h"
#include "classes/object_serializer.h"
#include "gameframework/actor.h"

class Blueprint : public AssetInstance
{
public:
    virtual bool LazyLoadAndUnload() const override { return false; }
    virtual GroovyClass* GetClass() const = 0;
};

class ObjectBlueprint : public Blueprint
{
public:
    ObjectBlueprint();
    ~ObjectBlueprint();

    virtual void __internal_SetUUID(AssetUUID uuid) override { mUUID = uuid; }
    virtual AssetUUID GetUUID() const override { return mUUID; }
    virtual bool IsLoaded() const override { return mLoaded; }

    virtual void Load() override;
    virtual void Save() override;

    virtual void Serialize(DynamicBuffer& fileData) const override;
    virtual void Deserialize(BufferView fileData) override;

    void CopyProperties(GroovyObject* obj);

    GroovyObject* GetDefaultObject() const { return mDefaultObject; }

    void SetupEmpty(GroovyClass* objClass);
    void RebuildPack();

#if WITH_EDITOR
    virtual GroovyClass*& Editor_ClassRef() { return mGroovyClass; }
    virtual PropertyPack& Editor_PropertyPackRef() { return mPropertyPack; }

    virtual bool Editor_FixDependencyDeletion(AssetHandle assetToBeDeleted) override;
#endif

    inline GroovyClass* GetObjectClass() const { return mGroovyClass; }
    inline const PropertyPack& GetPropertyPack() const { return mPropertyPack; }

    virtual GroovyClass* GetClass() const override { return mGroovyClass; }

private:
    GroovyClass* mGroovyClass;
    PropertyPack mPropertyPack;
    GroovyObject* mDefaultObject;

    AssetUUID mUUID;
    bool mLoaded;
};

class ActorBlueprint : public Blueprint
{
public:
    ActorBlueprint();
    ~ActorBlueprint();

    virtual void __internal_SetUUID(AssetUUID uuid) override { mUUID = uuid; }
    virtual AssetUUID GetUUID() const override { return mUUID; }
    virtual bool IsLoaded() const override { return mLoaded; }
    
    virtual void Load() override;
    virtual void Save() override;

    virtual void Serialize(DynamicBuffer& fileData) const override;
    virtual void Deserialize(BufferView fileData) override;

    void CopyProperties(Actor* actor);

    Actor* GetDefaultActor() const { return mDefaultActor; }

    void SetupEmpty(GroovyClass* actorClass);
    void RebuildPack();

#if WITH_EDITOR
    virtual GroovyClass*& Editor_ActorClassRef() { return mActorPack.actorClass; }
    virtual PropertyPack& Editor_ActorPropertyPackRef() { return mActorPack.actorProperties; }

    virtual bool Editor_FixDependencyDeletion(AssetHandle assetToBeDeleted) override;
#endif

    inline GroovyClass* GetActorClass() const { return mActorPack.actorClass; }
    inline const PropertyPack& GetActorPropertyPack() const { return mActorPack.actorProperties; }

    virtual GroovyClass* GetClass() const override { return  mActorPack.actorClass; }

private:
    ActorPack mActorPack;
    Actor* mDefaultActor;

    AssetUUID mUUID;
    bool mLoaded;
};