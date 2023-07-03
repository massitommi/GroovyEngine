#pragma once

#include "assets/asset.h"
#include "class.h"
#include "object_serializer.h"

class Blueprint : public AssetInstance
{
public:
    Blueprint();

    virtual void __internal_SetUUID(AssetUUID uuid) override { mUUID = uuid; }

    virtual AssetUUID GetUUID() const override { return mUUID; }

    virtual bool IsLoaded() const override { return mLoaded; }
    virtual void Load() override;
    virtual void Save() override;

    void Serialize(DynamicBuffer& fileData);
    void Deserialize(BufferView fileData);

    void SetData(GroovyObject* obj);

#if WITH_EDITOR
    virtual GroovyClass*& Editor_ClassRef() { return mGroovyClass; }
    virtual PropertyPack& Editor_PropertyPackRef() { return mPropertyPack; }

    virtual bool Editor_FixDependencyDeletion(AssetHandle assetToBeDeleted) override;
#endif

    inline GroovyClass* GetClass() const { return mGroovyClass; }
    inline const PropertyPack& GetPropertyPack() const { return mPropertyPack; }

private:
    GroovyClass* mGroovyClass;
    PropertyPack mPropertyPack;

    AssetUUID mUUID;
    bool mLoaded;
};