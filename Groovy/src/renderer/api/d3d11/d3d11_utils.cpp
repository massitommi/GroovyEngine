#if PLATFORM_WIN32

#include "d3d11_utils.h"

IDXGISwapChain* d3dUtils::gSwapChain = nullptr;
ID3D11Device* d3dUtils::gDevice = nullptr;
ID3D11DeviceContext* d3dUtils::gContext = nullptr;

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

#endif

void d3dUtils::ResizeBackBuffer(uint32 width, uint32 height)
{
    checkslow(width && height);
    gSwapChain->ResizeBuffers(1, width, height, DXGI_FORMAT_UNKNOWN, 0);
}

ID3D11Texture2D* d3dUtils::GetBackBuffer()
{
    ID3D11Texture2D* backBuffer;
    d3dcheckslow(gSwapChain->GetBuffer(0 /*first buffer (back buffer)*/, __uuidof(ID3D11Texture2D), (void**)&backBuffer));
    return backBuffer;
}

ID3D11Texture2D* d3dUtils::CreateTexture(uint32 width, uint32 height, UINT bindFlags, DXGI_FORMAT format, const void* data, size_t size)
{
	D3D11_TEXTURE2D_DESC desc = {};
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.Format = format;
    desc.BindFlags = bindFlags;
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = desc.ArraySize = 1;
    desc.SampleDesc.Count = 1;

    D3D11_SUBRESOURCE_DATA initialData;
    D3D11_SUBRESOURCE_DATA* initalDataPtr = nullptr;

    if (data)
    {
        checkslow(size <= width * height * 4);

        initialData.pSysMem = data;
        initialData.SysMemPitch = width * 4;
        initialData.SysMemSlicePitch = 0;

        initalDataPtr = &initialData;
    }

    ID3D11Texture2D* result;

    d3dcheckslow(gDevice->CreateTexture2D(&desc, initalDataPtr, &result));

    return result;
}

ID3D11ShaderResourceView* d3dUtils::CreateShaderResourceView(ID3D11Resource* resource)
{
    ID3D11ShaderResourceView* result;
    d3dcheckslow(gDevice->CreateShaderResourceView(resource, nullptr, &result));
    return result;
}

ID3D11DepthStencilView* d3dUtils::CreateDepthStencilView(ID3D11Texture2D* depthBuffer)
{
    ID3D11DepthStencilView* result;
    d3dcheckslow(gDevice->CreateDepthStencilView(depthBuffer, nullptr, &result));
    return result;
}

ID3D11RenderTargetView* d3dUtils::CreateRenderTargetView(ID3D11Texture2D* color)
{
    ID3D11RenderTargetView* result;
    d3dcheckslow(gDevice->CreateRenderTargetView(color, nullptr, &result));
    return result;
}

#define SHADER_ENTRY_POINT "main"

ID3DBlob* d3dUtils::CompileShader(const char* target, const void* src, size_t srcLength)
{
    ID3DBlob* bytecode;
    ID3DBlob* error;
    d3dcheckslowf
    (
        D3DCompile(src, srcLength, nullptr, nullptr, nullptr, SHADER_ENTRY_POINT, target, 0, 0, &bytecode, &error),
        "Shader compilation error: %s", error->GetBufferPointer()
    );

    if (error)
    {
        error->Release();
    }

    return bytecode;
}

ID3D11VertexShader* d3dUtils::CreateVertexShader(ID3DBlob* bytecode)
{
    ID3D11VertexShader* result;
    d3dcheckslow(gDevice->CreateVertexShader(bytecode->GetBufferPointer(), bytecode->GetBufferSize(), nullptr, &result));
    return result;
}

ID3D11PixelShader* d3dUtils::CreatePixelShader(ID3DBlob* bytecode)
{
    ID3D11PixelShader* result;
    d3dcheckslow(gDevice->CreatePixelShader(bytecode->GetBufferPointer(), bytecode->GetBufferSize(), nullptr, &result));
    return result;
}

ID3D11InputLayout* d3dUtils::CreateInputLayout(ID3DBlob* vertexBytecode, D3D11_INPUT_ELEMENT_DESC* layout, size_t numElements)
{
    ID3D11InputLayout* result;
    d3dcheckslow(gDevice->CreateInputLayout(layout, numElements, vertexBytecode->GetBufferPointer(), vertexBytecode->GetBufferSize(), &result));
    return result;
}

ID3D11Buffer* d3dUtils::CreateBuffer(UINT bindFlags, UINT cpuAccessFlags, D3D11_USAGE usage, size_t size, const void* data)
{
    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = size;
    desc.Usage = usage;
    desc.BindFlags = bindFlags;
    desc.CPUAccessFlags = cpuAccessFlags;

    D3D11_SUBRESOURCE_DATA initialData;
    D3D11_SUBRESOURCE_DATA* initalDataPtr = nullptr;

    if (data)
    {
        initialData.pSysMem = data;
        initialData.SysMemPitch = 0;
        initialData.SysMemSlicePitch = 0;

        initalDataPtr = &initialData;
    }

    ID3D11Buffer* result;
    d3dcheckslow(gDevice->CreateBuffer(&desc, initalDataPtr, &result));
    return result;
}
