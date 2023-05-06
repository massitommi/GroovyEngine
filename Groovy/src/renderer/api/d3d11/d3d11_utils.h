#pragma once

#include "core/core.h"
#include <d3d11.h>
#include <d3dcompiler.h>

namespace d3dUtils
{
	extern IDXGISwapChain* gSwapChain;
	extern ID3D11Device* gDevice;
	extern ID3D11DeviceContext* gContext;

	void ResizeBackBuffer(uint32 width, uint32 height);
	ID3D11Texture2D* GetBackBuffer();
	ID3D11Texture2D* CreateTexture(uint32 width, uint32 height, UINT bindFlags, DXGI_FORMAT format, const void* data, size_t size);
	ID3D11ShaderResourceView* CreateShaderResourceView(ID3D11Resource* resource);
	ID3D11DepthStencilView* CreateDepthStencilView(ID3D11Texture2D* depthBuffer);
	ID3D11RenderTargetView* CreateRenderTargetView(ID3D11Texture2D* color);
	ID3DBlob* CompileShader(const char* target, const void* src, size_t srcLength);
	ID3D11VertexShader* CreateVertexShader(ID3DBlob* bytecode);
	ID3D11PixelShader* CreatePixelShader(ID3DBlob* bytecode);
	ID3D11InputLayout* CreateInputLayout(ID3DBlob* vertexBytecode, D3D11_INPUT_ELEMENT_DESC* layout, size_t numElements);
	ID3D11Buffer* CreateBuffer(UINT bindFlags, UINT cpuAccessFlags, D3D11_USAGE usage, size_t size, const void* initialData);
}

#define d3dcheckslow(HResult) { checkslow((HResult) == S_OK); }
#define d3dcheckslowf(HResult, Msg) { checkslowf((HResult) == S_OK, Msg); }
#define d3dcheck(HResult) { check((HResult) == S_OK); }
#define d3dverify(HResult) { verify((HResult) == S_OK); } 