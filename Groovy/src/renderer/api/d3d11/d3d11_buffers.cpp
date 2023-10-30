#if PLATFORM_WIN32

#include "d3d11_buffers.h"
#include "d3d11_utils.h"

D3D11VertexBuffer::D3D11VertexBuffer(size_t size, void* data, uint32 stride)
	: mSize(size), mStride(stride)
{
	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth = size;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = 0;

	if (data)
	{
		D3D11_SUBRESOURCE_DATA initialData = {};
		initialData.pSysMem = data;

		d3dcheckslow(d3d11Utils::gDevice->CreateBuffer(&desc, &initialData, &mBuffer));
	}
	else
	{
		d3dcheckslow(d3d11Utils::gDevice->CreateBuffer(&desc, nullptr, &mBuffer));
	}
}

D3D11VertexBuffer::~D3D11VertexBuffer()
{
	mBuffer->Release();
}

void D3D11VertexBuffer::Bind()
{
	UINT strides = mStride;
	UINT offsets = 0;
	d3d11Utils::gContext->IASetVertexBuffers(0, 1, &mBuffer, &strides, &offsets);
}

D3D11IndexBuffer::D3D11IndexBuffer(size_t size, void* data)
	: mSize(size)
{
	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth = size;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	desc.CPUAccessFlags = 0;

	if (data)
	{
		D3D11_SUBRESOURCE_DATA initialData = {};
		initialData.pSysMem = data;

		d3dcheckslow(d3d11Utils::gDevice->CreateBuffer(&desc, &initialData, &mBuffer));
	}
	else
	{
		d3dcheckslow(d3d11Utils::gDevice->CreateBuffer(&desc, nullptr, &mBuffer));
	}
}

D3D11IndexBuffer::~D3D11IndexBuffer()
{
	mBuffer->Release();
}

void D3D11IndexBuffer::Bind()
{
	d3d11Utils::gContext->IASetIndexBuffer(mBuffer, DXGI_FORMAT_R32_UINT, 0);
}

D3D11ConstBuffer::D3D11ConstBuffer(size_t size, void* data)
	: mSize(size)
{
	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth = size;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	if (data)
	{
		D3D11_SUBRESOURCE_DATA initialData = {};
		initialData.pSysMem = data;

		d3dcheckslow(d3d11Utils::gDevice->CreateBuffer(&desc, &initialData, &mBuffer));
	}
	else
	{
		d3dcheckslow(d3d11Utils::gDevice->CreateBuffer(&desc, nullptr, &mBuffer));
	}
}

D3D11ConstBuffer::~D3D11ConstBuffer()
{
	mBuffer->Release();
}

void D3D11ConstBuffer::Overwrite(void* data, size_t size)
{
	check(size <= mSize);
	D3D11_MAPPED_SUBRESOURCE mappedRes;
	d3dverify(d3d11Utils::gContext->Map(mBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes));
	memcpy(mappedRes.pData, data, size);
	d3d11Utils::gContext->Unmap(mBuffer, 0);
}

void D3D11ConstBuffer::BindForVertexShader(uint32 slot)
{
	d3d11Utils::gContext->VSSetConstantBuffers(slot, 1, &mBuffer);
}

void D3D11ConstBuffer::BindForPixelShader(uint32 slot)
{
	d3d11Utils::gContext->PSSetConstantBuffers(slot, 1, &mBuffer);
}

#endif