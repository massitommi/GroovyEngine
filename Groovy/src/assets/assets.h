#pragma once

#include "core/core.h"
#include "renderer/color.h"

#define GROOVY_MAGIC_LENGTH             16
#define GROOVY_TEXTURE_HEADER_MAGIC     "groovy_texture__"
#define GROOVY_MODEL3D_HEADER_MAGIC     "groovy_model3d__"

#define GROOVY_TEXTURE_EXT  ".groovytexture"
#define GROOVY_MODEL_EXT    ".groovymodel3d"

enum EAssetType
{
    ASSET_TYPE_NONE,
    ASSET_TYPE_TEXTURE,
    ASSET_TYPE_MODEL3D
};

struct TextureHeader
{
    char magic[GROOVY_MAGIC_LENGTH];
    uint32 width;
    uint32 height;
    uint32 channels;
};

struct MeshHeader
{
    char magic[GROOVY_MAGIC_LENGTH];
    uint32 submeshCount;
};

struct SubmeshHeader
{
    uint64 vertexCount;
    uint64 indexCount;
    EColorFormat vertexFormat;
    EColorFormat indexFormat;
};

namespace assetUtils
{
    inline bool IsGroovyAsset(const Buffer& file, size_t groovyHeaderSize, const char* groovyMagic)
    {
        return (file.data()) && (file.size() >= groovyHeaderSize) && (strncmp((char*)file.data(), groovyMagic, GROOVY_MAGIC_LENGTH) == 0);
    }

    inline bool IsGroovyAsset_Texture(const Buffer& file)
    {
        return IsGroovyAsset(file, sizeof(TextureHeader), GROOVY_TEXTURE_HEADER_MAGIC);
    }

    inline bool IsGroovyAsset_Mesh(const Buffer& file)
    {
        return IsGroovyAsset(file, sizeof(MeshHeader), GROOVY_MODEL3D_HEADER_MAGIC);
    }
}