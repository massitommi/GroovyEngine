#if PLATFORM_WIN32

#include "d3d11_renderer_api.h"
#include "d3d11_utils.h"
#include "platform/window.h"

#if BUILD_DEBUG
    #define SWAPCHAIN_FLAG_DEBUG D3D11_CREATE_DEVICE_DEBUG
#else
    #define SWAPCHAIN_FLAG_DEBUG 0
#endif

D3D11RendererAPI::D3D11RendererAPI(Window* wnd)
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
        swapchainDesc.BufferDesc.RefreshRate.Numerator = wndProps.refreshrate;
        swapchainDesc.BufferDesc.RefreshRate.Denominator = 1;
        swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapchainDesc.OutputWindow = wndHandle;
        swapchainDesc.SampleDesc.Count = 1;
        swapchainDesc.Windowed = !wndProps.fullscreen;

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

void D3D11RendererAPI::DrawIndexed(uint64 indexCount)
{
    d3dUtils::gContext->DrawIndexed(indexCount, 0, 0);
}

void D3D11RendererAPI::Present(uint32 syncInteval)
{
    d3dUtils::gSwapChain->Present(syncInteval, 0);
}

D3D11RendererAPI::~D3D11RendererAPI()
{
#if REF_COUNT_WARNINGS
    ID3D11Debug* boh;
    d3dUtils::gDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&boh));
    boh->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
#endif
    d3dUtils::gSwapChain->Release();
    d3dUtils::gContext->Release();
    d3dUtils::gDevice->Release();
}

#endif