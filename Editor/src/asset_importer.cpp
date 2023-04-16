#include "asset_importer.h"
#include "platform/filesystem.h"
#include "assets/assets.h"

#include "vendor/stb/stb_image.h"

#define DEFAULT_IMAGE_IMPORT_CHANNELS 4

bool AssetImporter::ImportTexture(const String& originalFile, const String& outFilePath)
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
    Buffer groovyTexture(sizeof(GroovyTextureHeader) + rawImgSize);

    GroovyTextureHeader textureHeader;
    textureHeader.width = imgWidth;
    textureHeader.height = imgHeight;
    textureHeader.channels = DEFAULT_IMAGE_IMPORT_CHANNELS;
    memcpy(textureHeader.magic, GROOVY_TEXTURE_HEADER_MAGIC, GROOVY_MAGIC_LENGTH);

    memcpy(groovyTexture.data() + 0, &textureHeader, sizeof(GroovyTextureHeader)); // copy header
    memcpy(groovyTexture.data() + sizeof(GroovyTextureHeader), imgData, rawImgSize); // copy data

    if (FileSystem::WriteFileBinary(outFilePath, groovyTexture) != FILE_OPEN_RESULT_OK)
    {
        // log
        return false;
    }
    
    stbi_image_free(imgData);
    
    return true;
}
