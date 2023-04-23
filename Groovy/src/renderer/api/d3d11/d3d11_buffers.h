#pragma once

#include "../buffers.h"

class D3D11VertexBuffer : public VertexBuffer
{
public:
	D3D11VertexBuffer(size_t size, void* data, uint32 stride);
	virtual ~D3D11VertexBuffer();

	virtual void Bind() override;
	virtual size_t GetSize() const override { return mSize; }
	virtual void SetData(void* data, size_t size) override;

private:
	struct ID3D11Buffer* mBuffer;
	size_t mSize;
	uint32 mStride;
};

class D3D11IndexBuffer : public IndexBuffer
{
public:
	D3D11IndexBuffer(size_t size, void* data);
	virtual ~D3D11IndexBuffer();

	virtual void Bind() override;
	virtual size_t GetSize() const override { return mSize; }
	virtual void SetData(void* data, size_t size) override;

private:
	struct ID3D11Buffer* mBuffer;
	size_t mSize;
};

