#if PLATFORM_WIN32

#include "d3d11_renderer_api.h"
#include "d3d11_utils.h"
#include "platform/window.h"

#if BUILD_DEBUG
    #define SWAPCHAIN_FLAG_DEBUG D3D11_CREATE_DEVICE_DEBUG
#else
    #define SWAPCHAIN_FLAG_DEBUG 0
#endif

D3D11RendererAPI::D3D11RendererAPI(RendererAPISpec spec, Window* wnd)
    : mSpec(spec), mRasterizerState({ RASTERIZER_FILL_MODE_SOLID, RASTERIZER_CULL_MODE_BACK })
{
    WindowProps wndProps = wnd->GetProps();
    HWND wndHandle = (HWND)wnd->GetNativeHandle();

    // swapchain
    {
        DXGI_SWAP_CHAIN_DESC swapchainDesc = {};
        swapchainDesc.BufferCount = 1;
        swapchainDesc.BufferDesc.Width = wndProps.width;
        swapchainDesc.BufferDesc.Height = wndProps.height;
        swapchainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapchainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        swapchainDesc.BufferDesc.RefreshRate.Numerator = spec.refreshrate;
        swapchainDesc.BufferDesc.RefreshRate.Denominator = 1;
        swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapchainDesc.OutputWindow = wndHandle;
        swapchainDesc.SampleDesc.Count = 1;
        swapchainDesc.Windowed = true; // always windowed at startup
        swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        d3dcheckslow(D3D11CreateDeviceAndSwapChain
        (
            nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, SWAPCHAIN_FLAG_DEBUG, 0, 0,
            D3D11_SDK_VERSION, &swapchainDesc, &d3dUtils::gSwapChain, &d3dUtils::gDevice, 0, &d3dUtils::gContext
        ));
    }

    // viewport
    {
        D3D11_VIEWPORT viewport = {};
        viewport.TopLeftX = 0;
        viewport.TopLeftY = 0;
        viewport.Width = wndProps.width;
        viewport.Height = wndProps.height;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;

        d3dUtils::gContext->RSSetViewports(1, &viewport);
    }

    // sampler
    {
        D3D11_SAMPLER_DESC samplerDesc = {};
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        samplerDesc.MinLOD = 0;
        samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

        ID3D11SamplerState* sampler;

        d3dcheckslow(d3dUtils::gDevice->CreateSamplerState(&samplerDesc, &sampler));

        d3dUtils::gContext->PSSetSamplers(0, 1, &sampler);
        
        sampler->Release();
    }

    // primitive topology
    {
        d3dUtils::gContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }
}

void D3D11RendererAPI::DrawIndexed(uint32 vertexOffset, uint32 indexOffset, uint32 indexCount)
{
    d3dUtils::gContext->DrawIndexed(indexCount, indexOffset, vertexOffset);
}

void D3D11RendererAPI::Present()
{
    d3dUtils::gSwapChain->Present(mSpec.vsync, 0);
}

void D3D11RendererAPI::SetFullscreen(bool fullscreen)
{
    d3dcheckslow(d3dUtils::gSwapChain->SetFullscreenState(fullscreen, nullptr));
}

void D3D11RendererAPI::SetVSync(uint32 syncInterval)
{
    mSpec.vsync = syncInterval;
}

void D3D11RendererAPI::SetRasterizerState(RasterizerState newState)
{
    D3D11_FILL_MODE nativeFillMode = D3D11_FILL_SOLID;
    D3D11_CULL_MODE nativeCullMode = D3D11_CULL_BACK;

    switch (newState.fillMode)
    {
        case RASTERIZER_FILL_MODE_SOLID:
            nativeFillMode = D3D11_FILL_SOLID;
            break;
        case RASTERIZER_FILL_MODE_WIREFRAME:
            nativeFillMode = D3D11_FILL_WIREFRAME;
            break;
        default:
            checkslowf(0, "ERasterizerFillMode enum value not implemented");
    }

    switch (newState.cullMode)
    {
        case RASTERIZER_CULL_MODE_BACK:
            nativeCullMode = D3D11_CULL_BACK;
            break;
        case RASTERIZER_CULL_MODE_FRONT:
            nativeCullMode = D3D11_CULL_FRONT;
            break;
        case RASTERIZER_CULL_MODE_NONE:
            nativeCullMode = D3D11_CULL_NONE;
            break;
        default:
            checkslowf(0, "ERasterizerCullMode enum value not implemented");
    }

    ID3D11RasterizerState* newRasterizerState = d3dUtils::CreateRasterizerState(nativeFillMode, nativeCullMode);
    d3dUtils::gContext->RSSetState(newRasterizerState);
    newRasterizerState->Release();

    mRasterizerState = newState;
}

D3D11RendererAPI::~D3D11RendererAPI()
{
#if 0
    ID3D11Debug* boh;
    d3dUtils::gDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&boh));
    boh->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
#endif
    d3dUtils::gSwapChain->Release();
    d3dUtils::gContext->Release();
    d3dUtils::gDevice->Release();
}

#endif