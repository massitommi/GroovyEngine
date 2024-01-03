#include "editor.h"

#include "asset_importer.h"
#include "platform/filesystem.h"
#include "platform/messagebox.h"
#include "assets/asset.h"

#include "vendor/stb/stb_image.h"
#include "vendor/tinyobj/tiny_obj_loader.h"

#include "assets/asset_manager.h"

#include "engine/project.h"

#include "renderer/mesh.h"
#include "classes/object_serializer.h"

#include "utils/string_utils.h"

#define DEFAULT_IMAGE_IMPORT_CHANNELS 4

const std::vector<SupportedImport> sSupportedImports =
{
    // textures
    {
        {
            "Texture",

            {
                "*.jpg",
                "*.jpeg",
                "*.png"
            }
        },

        ASSET_TYPE_TEXTURE
    },
    // 3d models
    {
        {
            "3D Model",

            {
                "*.obj"
            }
        },

        ASSET_TYPE_MESH
    },
    // audio files
    {
        {
            "Audio file",

            {
                "*.wav"
            }
        },

        ASSET_TYPE_AUDIO_CLIP
    },
};

EAssetType AssetImporter::GetTypeFromFilename(const std::string& filename)
{
    std::string fileExt = std::filesystem::path(filename).extension().string();

    for (const SupportedImport& import : sSupportedImports)
        for (const std::string& ext : import.extensions.extensions)
            if (stringUtils::EqualsCaseInsensitive(ext.c_str() + 1, fileExt))
                return import.type;

    return ASSET_TYPE_NONE;
}

bool AssetImporter::GetRawTexture(const std::string& compressedFile, Buffer& outBuffer, TextureSpec& outSpec)
{
    Buffer imgCompressed;
    if (FileSystem::ReadFileBinary(compressedFile, imgCompressed) != FILE_OPEN_RESULT_OK)
    {
        return false;
    }

    // let stbi do the job
    int imgWidth, imgHeight, imgChannels;
    stbi_uc* imgData = stbi_load_from_memory(imgCompressed.data(), (int)imgCompressed.size(), &imgWidth, &imgHeight, &imgChannels, DEFAULT_IMAGE_IMPORT_CHANNELS);
    if (!imgData)
    {
        GROOVY_LOG_ERR("Unable to stbi_load_from_memory, error: %s", stbi_failure_reason());
        return false;
    }
    outBuffer.resize(imgWidth * imgHeight * DEFAULT_IMAGE_IMPORT_CHANNELS);
    memcpy(outBuffer.data(), imgData, imgWidth * imgHeight * DEFAULT_IMAGE_IMPORT_CHANNELS);
    outSpec.format = COLOR_FORMAT_R8G8B8A8_UNORM;
    outSpec.width = imgWidth;
    outSpec.height = imgHeight;

    return true;
}

void AssetImporter::TryImportAsset(const std::string& file)
{
    std::string newFileName = std::filesystem::path(file).replace_extension(GROOVY_ASSET_EXT).filename().string();

    if (AssetManager::FindByPath(newFileName).instance)
    {
        SysMessageBox::Show_Error("Can't import file", "Can't import file, a file with the same name is already in the assets folder!");
        return;
    }

    EAssetType assetType = AssetImporter::GetTypeFromFilename(file);

    if (assetType == ASSET_TYPE_NONE)
    {
        SysMessageBox::Show_Error("Unable to import asset", "Asset type not supported!");
        return;
    }

    bool importedSuccessfully = false;
    switch (assetType)
    {
    case ASSET_TYPE_TEXTURE:
        importedSuccessfully = AssetImporter::ImportTexture(file, newFileName);
        break;

    case ASSET_TYPE_MESH:
        importedSuccessfully = AssetImporter::ImportMesh(file, newFileName);
        break;

    case ASSET_TYPE_AUDIO_CLIP:
        importedSuccessfully = AssetImporter::ImportAudio(file, newFileName);
        break;
    }

    if (importedSuccessfully)
        SysMessageBox::Show_Info("Asset imported successfully!", "Asset imported successfully!");
    else
        SysMessageBox::Show_Error("Asset import failed!", "Unable to import asset :(");
}

const std::vector<SupportedImport>& AssetImporter::GetSupportedImports()
{
    return sSupportedImports;
}

bool AssetImporter::ImportTexture(const std::string& originalFile, const std::string& newFile)
{
    // load the compressed file
    Buffer imgCompressed;
    if (FileSystem::ReadFileBinary(originalFile, imgCompressed) != FILE_OPEN_RESULT_OK)
    {
        return false;
    }

    // let stbi do the job
    int imgWidth, imgHeight, imgChannels;
    imgWidth = imgHeight = imgChannels = 0;
    stbi_uc* imgData = stbi_load_from_memory(imgCompressed.data(), (int)imgCompressed.size(), &imgWidth, &imgHeight, &imgChannels, DEFAULT_IMAGE_IMPORT_CHANNELS);
    if (!imgData)
    {
        GROOVY_LOG_ERR("Unable to stbi_load_from_memory, error: %s", stbi_failure_reason());
        return false;
    }

    size_t rawImgSize = imgWidth * imgHeight * DEFAULT_IMAGE_IMPORT_CHANNELS;
    Buffer groovyTexture(sizeof(TextureAssetHeader) + rawImgSize);

    TextureAssetHeader textureHeader;
    textureHeader.width = imgWidth;
    textureHeader.height = imgHeight;
    textureHeader.format = COLOR_FORMAT_R8G8B8A8_UNORM;

    memcpy(groovyTexture.data() + 0, &textureHeader, sizeof(TextureAssetHeader)); // copy texture header
    memcpy(groovyTexture.data() + sizeof(TextureAssetHeader), imgData, rawImgSize); // copy texture data

    stbi_image_free(imgData);

    if (FileSystem::WriteFileBinary((gProj.GetAssetsPath() / newFile).string(), groovyTexture) != FILE_OPEN_RESULT_OK)
    {
        return false;
    }

    AssetManager::Editor_Add(newFile, ASSET_TYPE_TEXTURE);
    
    return true;
}


extern CORE_API Material* DEFAULT_MATERIAL;

bool AssetImporter::ImportMesh(const std::string& originalFile, const std::string& newFile)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, originalFile.c_str()))
    {
        SysMessageBox::Show_Error("Unable to import model", err);
        return false;
    }

    if (!warn.empty())
    {
        GROOVY_LOG_WARN("tinyobj LoadObj warning: %s", warn.c_str());
    }

    Buffer fileData;
    size_t vertexBufferSize = 0;
    size_t indexBufferSize = 0;
    uint32 submeshCount = 0;

    for (const auto& shape : shapes)
    {
        vertexBufferSize += shape.mesh.indices.size() * sizeof(MeshVertex);
        indexBufferSize += shape.mesh.indices.size() * sizeof(MeshIndex);
        submeshCount++;
    }

    fileData.resize
    (
        sizeof(MeshAssetHeader) + // mesh asset header
        vertexBufferSize + // vertex buffer
        indexBufferSize // index buffer
    );

    BufferView fileDataView(fileData);

    MeshAssetHeader* header = fileDataView.read<MeshAssetHeader>(1);
    header->vertexBufferSize = vertexBufferSize;
    header->indexBufferSize = indexBufferSize;

    MeshVertex* vertexBuffer = (MeshVertex*)fileDataView.read(header->vertexBufferSize);
    MeshIndex* indexBuffer = (MeshIndex*)fileDataView.read(header->indexBufferSize);
    MeshAssetFile asset;
    asset.submeshes.resize(submeshCount);
    asset.materials.resize(submeshCount);
    for (Material*& m : asset.materials)
        m = DEFAULT_MATERIAL;

    for (uint32 i = 0; i < shapes.size(); i++)
    {
        const auto& shape = shapes[i];


        for (uint32 j = 0; j < shape.mesh.indices.size(); j++)
        {
            const auto& index = shape.mesh.indices[j];

            // vertex data
            {
                // position
                vertexBuffer->position =
                {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2],
                    1.0f
                };

                // texture coordinates
                vertexBuffer->textCoords = { 0.0f, 0.0f };

                if (index.texcoord_index != -1)
                {
                    vertexBuffer->textCoords =
                    {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                    };
                }

                // color
                vertexBuffer->color =
                {
                    1.0f, 1.0f, 1.0f, 1.0f
                };
            }
            // index data
            {
                *indexBuffer = j;
            }

            vertexBuffer++;
            indexBuffer++;

            asset.submeshes[i].vertexCount++;
            asset.submeshes[i].indexCount++;
        }
    }

    DynamicBuffer fileData2;
    PropertyPack meshAssetPropPack;
    ObjectSerializer::CreatePropertyPack(&asset, MeshAssetFile::StaticCDO(), meshAssetPropPack);
    ObjectSerializer::SerializePropertyPack(meshAssetPropPack, fileData2);

    Buffer finalFileData;
    finalFileData.resize(fileData.size() + fileData2.used());
    memcpy(finalFileData.data(), fileData.data(), fileData.size());
    memcpy(finalFileData.data() + fileData.size(), fileData2.data(), fileData2.used());

    if (FileSystem::WriteFileBinary((gProj.GetAssetsPath() / newFile).string(), finalFileData) != FILE_OPEN_RESULT_OK)
    {
        return false;
    }
    
    AssetManager::Editor_Add(newFile, ASSET_TYPE_MESH);

    return true;
}

bool AssetImporter::ImportAudio(const std::string& originalFile, const std::string& newFile)
{
    if (!FileSystem::Copy(originalFile, (gProj.GetAssetsPath() / newFile).string()))
        return false;

    AssetManager::Editor_Add(newFile, ASSET_TYPE_AUDIO_CLIP);

    return true;
}
