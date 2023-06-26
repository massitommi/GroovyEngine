#pragma once

#include "assets/asset.h"
#include "class.h"
#include "object_serializer.h"

class Blueprint : public AssetInstance
{
public:
    Blueprint(GroovyClass* inClass);

    virtual void __internal_SetUUID(AssetUUID uuid) override { mUUID = uuid; }

    virtual AssetUUID GetUUID() const override { return mUUID; }

    virtual bool IsLoaded() const override { return mLoaded; }
    virtual void Load() override;
    virtual void Save() override;

#if WITH_EDITOR
    virtual bool Editor_FixDependencyDeletion(AssetHandle assetToBeDeleted) override;
#endif

    inline GroovyClass* GetClass() const { return mGroovyClass; }

private:
    GroovyClass* mGroovyClass;
    PropertyPack mPropertyPack;

    AssetUUID mUUID;
    bool mLoaded;
};