#if PLATFORM_WIN32

#include "d3d11_buffers.h"
#include "d3d11_utils.h"

D3D11VertexBuffer::D3D11VertexBuffer(size_t size, void* data, uint32 stride)
	: mSize(size), mStride(stride)
{
	mBuffer = d3dUtils::CreateBuffer(D3D11_BIND_VERTEX_BUFFER, 0, D3D11_USAGE_DEFAULT, size, data);
}

D3D11VertexBuffer::~D3D11VertexBuffer()
{
	mBuffer->Release();
}

void D3D11VertexBuffer::Bind()
{
	UINT strides = mStride;
	UINT offsets = 0;
	d3dUtils::gContext->IASetVertexBuffers(0, 1, &mBuffer, &strides, &offsets);
}

void D3D11VertexBuffer::SetData(void* data, size_t size)
{
	//d3dUtils::OverwriteBuffer(mBuffer, data, size);
}

D3D11IndexBuffer::D3D11IndexBuffer(size_t size, void* data)
	: mSize(size)
{
	mBuffer = d3dUtils::CreateBuffer(D3D11_BIND_INDEX_BUFFER, 0, D3D11_USAGE_DEFAULT, size, data);
}

D3D11IndexBuffer::~D3D11IndexBuffer()
{
	mBuffer->Release();
}

void D3D11IndexBuffer::Bind()
{
	d3dUtils::gContext->IASetIndexBuffer(mBuffer, DXGI_FORMAT_R32_UINT, 0);
}

void D3D11IndexBuffer::SetData(void* data, size_t size)
{
	//d3dUtils::OverwriteBuffer(mBuffer, data, size);
}

#endif