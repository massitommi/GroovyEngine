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
	String name;
	size_t size;
	size_t alignedOffset;
	EShaderVariableType type;
};

enum EConstBufferTarget
{
	CONSTBUFFER_TARGET_VERTEX = BITFLAG(1),
	CONSTBUFFER_TARGET_PIXEL = BITFLAG(2)
};

struct ShaderConstBuffer
{
	String name;
	size_t size;
	uint32 bufferTarget;
	std::vector<ShaderVariable> variables;
};

struct ShaderSrc
{
	void* data;
	size_t length;
};

class Shader
{
public:
	virtual ~Shader() = default;

	virtual void Bind() = 0;
	virtual void SetConstBuffer(uint32 index, void* data, size_t size) = 0;
	virtual const std::vector<ShaderConstBuffer>& GetSets() const = 0;
	virtual void SetAttribute(uint32 setIndex, uint32 varIndex, void* data, size_t dataSize) = 0;
	virtual void SetAttribute(const char* varName, void* data, size_t dataSize) = 0;

	static Shader* Create(ShaderSrc vertexSrc, ShaderSrc pixelSrc, const std::vector<ShaderVariable>& input);
};