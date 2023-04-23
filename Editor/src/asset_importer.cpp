#include "asset_importer.h"
#include "platform/filesystem.h"
#include "assets/assets.h"

#include "vendor/stb/stb_image.h"

#define DEFAULT_IMAGE_IMPORT_CHANNELS 4

const char* TEXTURE_IMPORTABLE_EXT[] =
{
    ".jpg",
    ".jpeg",
    ".png"
};

const char* MODEL3D_IMPORTABLE_EXT[] =
{
    ".obj"
};

EAssetType AssetImporter::GetTypeFromFilename(const String& filename)
{
    String fileExt = FileSystem::GetExtension(filename);
    // check for textures
    for (auto ext : TEXTURE_IMPORTABLE_EXT)
        if (fileExt == ext)
            return ASSET_TYPE_TEXTURE;
    // check for models
    for (auto ext : MODEL3D_IMPORTABLE_EXT)
        if (fileExt == ext)
            return ASSET_TYPE_MODEL3D;
    return ASSET_TYPE_NONE;
}

bool AssetImporter::GetRawTexture(const String& compressedFile, Buffer& outBuffer, TextureSpec& outSpec)
{
    Buffer imgCompressed;
    if (FileSystem::ReadFileBinary(compressedFile, imgCompressed) != FILE_OPEN_RESULT_OK)
    {
        // todo log -> file not found or something
        return false;
    }

    // let stbi do the job
    int imgWidth, imgHeight, imgChannels;
    stbi_uc* imgData = stbi_load_from_memory(imgCompressed.data(), imgCompressed.size(), &imgWidth, &imgHeight, &imgChannels, DEFAULT_IMAGE_IMPORT_CHANNELS);
    if (!imgData)
    {
        // todo: log this -> stbi_failure_reason()
        return false;
    }
    outBuffer.resize(imgWidth * imgHeight * imgChannels);
    memcpy(outBuffer.data(), imgData, imgWidth * imgHeight * imgChannels);
    outSpec.format = COLOR_FORMAT_R8G8B8A8_UNORM;
    outSpec.width = imgWidth;
    outSpec.height = imgHeight;

    return true;
}

bool AssetImporter::ImportTexture(const String& originalFile, const String& outFolder)
{
    // load the compressed file
    Buffer imgCompressed;
    if (FileSystem::ReadFileBinary(originalFile, imgCompressed) != FILE_OPEN_RESULT_OK)
    {
        // todo log -> file not found or something
        return false;
    }

    // let stbi do the job
    int imgWidth, imgHeight, imgChannels;
    stbi_uc* imgData = stbi_load_from_memory(imgCompressed.data(), imgCompressed.size(), &imgWidth, &imgHeight, &imgChannels, DEFAULT_IMAGE_IMPORT_CHANNELS);
    if (!imgData)
    {
        // todo: log this -> stbi_failure_reason()
        return false;
    }

    uint64 rawImgSize = imgWidth * imgHeight * DEFAULT_IMAGE_IMPORT_CHANNELS;
    Buffer groovyTexture(sizeof(TextureHeader) + rawImgSize);

    TextureHeader textureHeader;
    textureHeader.width = imgWidth;
    textureHeader.height = imgHeight;
    textureHeader.channels = DEFAULT_IMAGE_IMPORT_CHANNELS;
    memcpy(textureHeader.magic, GROOVY_TEXTURE_HEADER_MAGIC, GROOVY_MAGIC_LENGTH);

    memcpy(groovyTexture.data() + 0, &textureHeader, sizeof(TextureHeader)); // copy header
    memcpy(groovyTexture.data() + sizeof(TextureHeader), imgData, rawImgSize); // copy data

    String newFilePath = outFolder + FileSystem::DIR_SEPARATOR + FileSystem::GetFilenameNoExt(originalFile) + GROOVY_TEXTURE_EXT;

    if (FileSystem::WriteFileBinary(newFilePath, groovyTexture) != FILE_OPEN_RESULT_OK)
    {
        // log
        return false;
    }
    
    stbi_image_free(imgData);
    
    return true;
}
