#include "asset_importer.h"
#include "platform/filesystem.h"
#include "platform/messagebox.h"
#include "assets/asset.h"

#include "vendor/stb/stb_image.h"
#include "vendor/tinyobj/tiny_obj_loader.h"

#include "assets/asset_manager.h"

#include "renderer/mesh.h"

#define DEFAULT_IMAGE_IMPORT_CHANNELS 4

const char* TEXTURE_IMPORTABLE_EXT[] =
{
    ".jpg",
    ".jpeg",
    ".png"
};

const char* MESH_IMPORTABLE_EXT[] =
{
    ".obj"
};

void OnAssetImported(const std::string& importedFilePath, EAssetType assetType)
{
    AssetManager::AddEditorNew(importedFilePath, assetType);
}

EAssetType AssetImporter::GetTypeFromFilename(const std::string& filename)
{
    std::string fileExt = FileSystem::GetExtension(filename);
    // check for textures
    for (auto ext : TEXTURE_IMPORTABLE_EXT)
        if (fileExt == ext)
            return ASSET_TYPE_TEXTURE;
    // check for 3d meshes
    for (auto ext : MESH_IMPORTABLE_EXT)
        if (fileExt == ext)
            return ASSET_TYPE_MESH;
    // nothing to do
    return ASSET_TYPE_NONE;
}

bool AssetImporter::GetRawTexture(const std::string& compressedFile, Buffer& outBuffer, TextureSpec& outSpec)
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

bool AssetImporter::ImportTexture(const std::string& originalFile, const std::string& newFile)
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

    size_t rawImgSize = imgWidth * imgHeight * DEFAULT_IMAGE_IMPORT_CHANNELS;
    Buffer groovyTexture(sizeof(TextureAssetHeader) + rawImgSize);

    TextureAssetHeader textureHeader;
    textureHeader.width = imgWidth;
    textureHeader.height = imgHeight;
    textureHeader.format = COLOR_FORMAT_R8G8B8A8_UNORM;

    memcpy(groovyTexture.data() + 0, &textureHeader, sizeof(TextureAssetHeader)); // copy texture header
    memcpy(groovyTexture.data() + sizeof(TextureAssetHeader), imgData, rawImgSize); // copy texture data

    stbi_image_free(imgData);

    if (FileSystem::WriteFileBinary(newFile, groovyTexture) != FILE_OPEN_RESULT_OK)
    {
        // log
        return false;
    }

    OnAssetImported(newFile, ASSET_TYPE_TEXTURE);
    
    return true;
}

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
        SysMessageBox::Show_Warning("Import model warn", warn);
    }

    Buffer newFileData;
    size_t vertexBufferSize = 0;
    size_t indexBufferSize = 0;
    uint32 submeshCount = 0;

    for (const auto& shape : shapes)
    {
        vertexBufferSize += shape.mesh.indices.size() * sizeof(MeshVertex);
        indexBufferSize += shape.mesh.indices.size() * sizeof(MeshIndex);
        submeshCount++;
    }

    newFileData.resize
    (
        sizeof(MeshAssetHeader) + // mesh asset header
        vertexBufferSize + // vertex buffer
        indexBufferSize + // index buffer
        submeshCount * sizeof(SubmeshData) + // submesh data
        submeshCount * sizeof(AssetUUID) // materials
    );

    MeshAssetHeader* header = newFileData.as<MeshAssetHeader>();
    header->vertexBufferSize = vertexBufferSize;
    header->indexBufferSize = indexBufferSize;
    header->submeshCount = submeshCount;

    byte* vertexBufferOffset = newFileData.data() + sizeof(MeshAssetHeader);
    byte* indexBufferOffset = vertexBufferOffset + vertexBufferSize;
    byte* submeshesOffset = indexBufferOffset + indexBufferSize;
    byte* materialsOffset = submeshesOffset + submeshCount * sizeof(SubmeshData);

    MeshVertex* vertexBuffer = (MeshVertex*)vertexBufferOffset;
    MeshIndex* indexBuffer = (MeshIndex*)indexBufferOffset;
    SubmeshData* submeshes = (SubmeshData*)submeshesOffset;
    AssetUUID* materialsID = (AssetUUID*)materialsOffset;

    for (const auto& shape : shapes)
    {
        uint32 indexIndex = 0;
        for (const auto& index : shape.mesh.indices)
        {
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
                *indexBuffer = indexIndex;
            }

            vertexBuffer++;
            indexBuffer++;
            indexIndex++;
            submeshes->vertexCount++;
            submeshes->indexCount++;
            *materialsID = 0; // default asset index
        }
        submeshes++;
        materialsID++;
    }

    FileSystem::WriteFileBinary(newFile, newFileData);
    
    OnAssetImported(newFile, ASSET_TYPE_MESH);
}
