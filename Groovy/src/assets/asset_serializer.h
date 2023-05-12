#pragma once

#include "asset.h"
#include "asset_manager.h"

#include "renderer/material.h"

template<typename T>
class AssetSerializer
{
public:
	static void Serialize(T* instance, Buffer& outFileData)
	{
		static_assert(0, "Asset Type not supported");
	}

	static void Deserialize(T* instance, const Buffer& fileData)
	{
		static_assert(0, "Asset Type not supported");
	}
};

template<>
class AssetSerializer<Material>
{
public:
	static void Serialize(Material* instance, Buffer& outFileData)
	{
		check(instance);
		check(instance->mShader); // what do you want to save without shader???

		instance->mConstBuffersData.size();

		outFileData.resize
		(
			sizeof(MaterialAssetHeader) + // header
			instance->mConstBuffersData.size() + // const buffer data
			instance->mTextures.size() * sizeof(AssetUUID) // texture UUIDs
		);

		MaterialAssetHeader header;
		header.shaderID = instance->mShader->GetUUID();
		header.constBuffersSize = instance->mConstBuffersData.size();
		header.numTextures = instance->mTextures.size();

		byte* filePtr = outFileData.data();
		memcpy(filePtr, &header, sizeof(MaterialAssetHeader));
		filePtr += sizeof(MaterialAssetHeader);
		memcpy(filePtr, instance->mConstBuffersData.data(), instance->mConstBuffersData.size());
		filePtr += instance->mConstBuffersData.size();
		
		AssetUUID* texturePtr = (AssetUUID*)filePtr;
		for (uint32 i = 0; i < instance->mTextures.size(); i++)
		{
			texturePtr[i] = instance->mTextures[i] ? instance->mTextures[i]->GetUUID() : 0;
		}
	}

	static void Deserialize(Material* instance, const Buffer& fileData)
	{
		check(instance);
		
		const MaterialAssetHeader* header = fileData.as<MaterialAssetHeader>();
		check(header->shaderID); // no shader???
		
		const byte* dataPtr = fileData.data() + sizeof(MaterialAssetHeader);

		// shader
		Shader* shaderInstance = AssetManager::GetInstance<Shader>(header->shaderID);
		check(shaderInstance); // not loaded yet???
		instance->mShader = shaderInstance;

		// const buffers data
		instance->mConstBuffersData.resize(header->constBuffersSize);
		memcpy(instance->mConstBuffersData.data(), dataPtr, header->constBuffersSize);
		dataPtr += header->constBuffersSize;

		// textures
		AssetUUID* textureIDptr = (AssetUUID*)dataPtr;
		instance->mTextures.resize(header->numTextures);
		for (Texture*& tex : instance->mTextures)
		{
			Texture* texture = nullptr;
			if (*textureIDptr)
				texture = AssetManager::GetInstance<Texture>(*textureIDptr);
			tex = texture;
		}
	}
};
