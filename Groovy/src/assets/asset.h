#pragma once

#include "core/core.h"
#include "renderer/color.h"

enum EAssetType
{
    ASSET_TYPE_NONE = 0,
    
    // raw assets
    
    ASSET_TYPE_TEXTURE = 1,
    ASSET_TYPE_SHADER = 2,
    ASSET_TYPE_MESH = 3,
    
    // complex assets that reference other assets and stuff
    
    ASSET_TYPE_MATERIAL = 4
};

typedef uint64 AssetUUID;

#define GROOVY_ASSET_EXT ".groovyasset"

#define GROOVY_SHADER_VERTEX_SEGMENT    "GROOVY_SHADER_VERTEX"
#define GROOVY_SHADER_PIXEL_SEGMENT     "GROOVY_SHADER_PIXEL"

class AssetInstance
{
public:
    virtual ~AssetInstance() {}

    // for internal use
    virtual void __internal_SetUUID(AssetUUID uuid) = 0;

    virtual AssetUUID GetUUID() const = 0;
};

struct AssetHandle
{
    std::string path;
    std::string name; // relative path
    AssetUUID uuid = 0;
    EAssetType type = ASSET_TYPE_NONE;
    AssetInstance* instance = nullptr;
};

struct TextureAssetHeader 
{
    uint32 width;
    uint32 height;
    EColorFormat format;
};

struct MaterialAssetHeader
{
    AssetUUID shaderID;
    size_t constBuffersSize;
    uint32 numTextures;
};

struct MeshAssetHeader
{
    size_t vertexBufferSize;
    size_t indexBufferSize;
    size_t submeshCount;
};