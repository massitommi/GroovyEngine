#pragma once

#include "core/core.h"

#include "assets/asset.h"

enum EShaderVariableType
{
	SHADER_VARIABLE_TYPE_FLOAT1,
	SHADER_VARIABLE_TYPE_FLOAT2,
	SHADER_VARIABLE_TYPE_FLOAT3,
	SHADER_VARIABLE_TYPE_FLOAT4,
	SHADER_VARIABLE_TYPE_FLOAT4X4 // matrix 4x4
};

struct ShaderVariable
{
	std::string name;
	uint32 size;
	uint32 alignedOffset;
	EShaderVariableType type;
};

struct ConstBufferDesc
{
	std::string name;
	size_t size;
	std::vector<ShaderVariable> variables;
};

struct ShaderResTexture
{
	std::string name;
	uint32 bindSlot;
};

class CORE_API Shader : public AssetInstance
{
public:

	virtual bool IsLoaded() const override { return true; }
	virtual void Load() override {}
	virtual void Save() override {}

	virtual void Serialize(DynamicBuffer& fileData) const override {}
	virtual void Deserialize(BufferView fileData) override {}
#if WITH_EDITOR
	virtual bool Editor_FixDependencyDeletion(AssetHandle assetToBeDeleted) override { return false; }
#endif

	virtual void Bind() = 0;

	virtual const std::vector<ConstBufferDesc>& GetVertexConstBuffersDesc() const = 0;
	virtual const std::vector<ConstBufferDesc>& GetPixelConstBuffersDesc() const = 0;
	virtual const std::vector<ShaderResTexture>& GetPixelTexturesRes() const = 0;

	virtual uint32 GetVertexConstBufferIndex(const std::string& bufferName) = 0;
	virtual uint32 GetPixelConstBufferIndex(const std::string& bufferName) = 0;

	virtual void OverwritePixelConstBuffer(uint32 index, void* data) = 0;

	static Shader* Create(const void* vertexSrc, size_t vertexSize, const void* pixelSrc, size_t pixelSize);
};