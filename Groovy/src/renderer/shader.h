#pragma once

#include "core/core.h"

enum EShaderAttributeType
{
	SHADER_ATTRIBUTE_TYPE_FLOAT4,
	SHADER_ATTRIBUTE_TYPE_FLOAT3,
	SHADER_ATTRIBUTE_TYPE_FLOAT2,
	SHADER_ATTRIBUTE_TYPE_FLOAT1
};

struct ShaderAttribute
{
	String name;
	size_t size;
	size_t alignedOffset;
	EShaderAttributeType type;
};

#define CONSTBUFFER_TARGET_VERTEX BITFLAG(1)
#define CONSTBUFFER_TARGET_PIXEL BITFLAG(2)

struct ShaderConstBuffer
{
	String name;
	size_t size;
	uint32 bufferTarget;
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

	static Shader* Create(ShaderSrc vertexSrc, ShaderSrc pixelSrc, const std::vector<ShaderAttribute>& attribs, const std::vector<ShaderConstBuffer>& constBuffers);
};