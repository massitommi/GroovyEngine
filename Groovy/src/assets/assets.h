#pragma once

#include "core/core.h"

#define GROOVY_MAGIC_LENGTH 16
#define GROOVY_TEXTURE_HEADER_MAGIC "groovy_a_texture"
#define GROOVY_MODEL3D_HEADER_MAGIC "groovy_a_model3d"

struct GroovyTextureHeader
{
    char magic[GROOVY_MAGIC_LENGTH];
    uint32 width;
    uint32 height;
    uint32 channels;
};

struct Model3DHeader
{
    char magic[GROOVY_MAGIC_LENGTH];
    uint64 numIndices;
    uint64 numVertices;
};

namespace assetUtils
{
    inline bool IsGroovyAssetGeneric(void* file, size_t fileSize, size_t groovyHeaderSize, const char* groovyMagic)
    {
        return (fileSize >= groovyHeaderSize) && (strncmp((char*)file, groovyMagic, GROOVY_MAGIC_LENGTH) == 0);
    }

    template<typename T>
    inline bool IsGroovyAsset(void* file, size_t fileSize)
    {
        return IsGroovyAssetGeneric(file, fileSize, sizeof(GroovyTextureHeader), GROOVY_TEXTURE_HEADER_MAGIC);
    }
}