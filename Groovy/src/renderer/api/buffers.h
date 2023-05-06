#pragma once

#include "core/core.h"

class VertexBuffer
{
public:
	virtual ~VertexBuffer() = default;

	virtual void Bind() = 0;
	virtual size_t GetSize() const = 0;

	static VertexBuffer* Create(size_t size, void* data, uint32 stride);
};

class IndexBuffer
{
public:
	virtual ~IndexBuffer() = default;

	virtual void Bind() = 0;
	virtual size_t GetSize() const = 0;

	static IndexBuffer* Create(size_t size, void* data);
};