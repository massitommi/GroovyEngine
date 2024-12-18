#pragma once

#include "core/core.h"

class CORE_API VertexBuffer
{
public:
	virtual ~VertexBuffer() = default;

	virtual void Bind() = 0;
	virtual size_t GetSize() const = 0;

	static VertexBuffer* Create(size_t size, const void* data, uint32 stride);
};

class CORE_API IndexBuffer
{
public:
	virtual ~IndexBuffer() = default;

	virtual void Bind() = 0;
	virtual size_t GetSize() const = 0;

	static IndexBuffer* Create(size_t size, const void* data);
};

class CORE_API ConstBuffer
{
public:
	virtual ~ConstBuffer() = default;

	virtual size_t GetSize() const = 0;
	virtual void Overwrite(void* data, size_t size) = 0;
	
	virtual void BindForVertexShader(uint32 slot) = 0;
	virtual void BindForPixelShader(uint32 slot) = 0;

	static ConstBuffer* Create(size_t size, const void* data);
};