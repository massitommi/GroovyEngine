#pragma once

#include "core/core.h"
#include "renderer/color.h"

#define GROOVY_ASSET_EXT                ".groovyasset"
#define GROOVY_ASSET_MAGIC              "__groovy_asset__"
#define GROOVY_ASSET_MAGIC_SIZE         16

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

struct AssetHandle
{
    std::string path;
    AssetUUID uuid = 0;
    EAssetType type = ASSET_TYPE_NONE;
    void* instance = nullptr;
};

struct GroovyAssetHeader
{
    char magic[GROOVY_ASSET_MAGIC_SIZE];
    AssetUUID uuid;
    EAssetType type;
};

struct GroovyAssetHeader_Texture : GroovyAssetHeader
{
    uint32 width;
    uint32 height;
    EColorFormat format;
};

struct GroovyAssetHeader_Material : GroovyAssetHeader
{
    AssetUUID shaderID;
    size_t constBuffersSize;
    size_t numTextures;
};

struct GroovyAssetHeader_Mesh : GroovyAssetHeader
{
    size_t vertexBufferSize;
    size_t indexBufferSize;
    size_t submeshCount;
};

/*
material file data:

    vertexConstBufferData

    textureUUIDs [...]
*/

/*
mesh file data:
    
    vertexBufferData
    
    indexBufferData

    submeshes [...]

*/

#define GROOVY_SHADER_VERTEX_SEGMENT    "GROOVY_SHADER_VERTEX"
#define GROOVY_SHADER_PIXEL_SEGMENT     "GROOVY_SHADER_PIXEL"