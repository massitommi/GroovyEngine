#pragma once

#include "audio.h"
#include "assets/asset.h"

class AudioClip : public AssetInstance
{
public:
    AudioClip();
    ~AudioClip();

    // for internal use
    virtual void __internal_SetUUID(AssetUUID uuid) override { mUUID = uuid; }

    virtual AssetUUID GetUUID() const override { return mUUID; }

    virtual bool IsLoaded() const override { return mLoaded; }
    virtual void Load() override;
    virtual void Save() override;

#if WITH_EDITOR

    // returns true if assetToBeDeleted was referenced
    virtual bool Editor_FixDependencyDeletion(AssetHandle assetToBeDeleted) override { return false; }

#endif

    virtual void Serialize(DynamicBuffer& fileData) const override;
    virtual void Deserialize(BufferView fileData) override;

    void Play();
    AudioClipInfo GetInfo() const { return mInfo; }

private:
    AudioClipHandle mHandle;
    AssetUUID mUUID;
    AudioClipInfo mInfo;
    bool mLoaded;
};