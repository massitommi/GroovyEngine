#pragma once

#include "core/core.h"
#include "renderer/color.h"

enum EAssetType
{
    ASSET_TYPE_NONE = 0,
    
    // raw assets
    
    ASSET_TYPE_TEXTURE = 1,
    ASSET_TYPE_SHADER = 2,
    ASSET_TYPE_AUDIO_CLIP = 8,
    
    // complex assets that reference other assets and stuff
    
    ASSET_TYPE_MATERIAL = 3,
    ASSET_TYPE_MESH = 4,
    ASSET_TYPE_BLUEPRINT = 5,
    ASSET_TYPE_ACTOR_BLUEPRINT = 6,
    ASSET_TYPE_SCENE = 7
};

typedef uint64 AssetUUID;

#define GROOVY_ASSET_EXT ".groovyasset"

#define GROOVY_SHADER_VERTEX_SEGMENT    "GROOVY_SHADER_VERTEX"
#define GROOVY_SHADER_PIXEL_SEGMENT     "GROOVY_SHADER_PIXEL"

struct AssetHandle
{
    std::string name;
    AssetUUID uuid = 0;
    EAssetType type = ASSET_TYPE_NONE;
    class AssetInstance* instance = nullptr;
};

class CORE_API AssetInstance
{
public:
    virtual ~AssetInstance() {}

    // for internal use
    virtual void __internal_SetUUID(AssetUUID uuid) = 0;

    virtual AssetUUID GetUUID() const = 0;

    std::string GetAssetName() const;

    virtual bool IsLoaded() const = 0;
    virtual void Load() = 0;
    virtual void Save() = 0;

    virtual void Serialize(DynamicBuffer& fileData) const = 0;
    virtual void Deserialize(BufferView fileData) = 0;

#if WITH_EDITOR

    // returns true if assetToBeDeleted was referenced
    virtual bool Editor_FixDependencyDeletion(AssetHandle assetToBeDeleted) = 0;

#endif
};

// todo move this away
struct TextureAssetHeader
{
    uint32 width;
    uint32 height;
    EColorFormat format;
};

struct MeshAssetHeader
{
    size_t vertexBufferSize;
    size_t indexBufferSize;
};