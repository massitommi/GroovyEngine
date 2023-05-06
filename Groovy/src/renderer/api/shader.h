#pragma once

#include "core/core.h"

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
	size_t size;
	size_t alignedOffset;
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

class Shader
{
public:
	virtual ~Shader() = default;

	virtual void Bind() = 0;

	virtual const std::vector<ConstBufferDesc>& GetVertexConstBuffersDesc() const = 0;
	virtual const std::vector<ConstBufferDesc>& GetPixelConstBuffersDesc() const = 0;
	virtual const std::vector<ShaderResTexture>& GetPixelTexturesRes() const = 0;

	virtual size_t GetVertexConstBufferIndex(const std::string& bufferName) = 0;
	virtual size_t GetPixelConstBufferIndex(const std::string& bufferName) = 0;

	virtual void OverwriteVertexConstBuffer(uint32 index, void* data) = 0;
	virtual void OverwritePixelConstBuffer(uint32 index, void* data) = 0;

	static Shader* Create(const void* vertexSrc, size_t vertexSize, const void* pixelSrc, size_t pixelSize);
};