#include "buffers.h"
#include "renderer_api.h"

#include "d3d11/d3d11_buffers.h"

VertexBuffer* VertexBuffer::Create(size_t size, void* data, uint32 stride)
{
    switch (RendererAPI::GetAPI())
    {
#if PLATFORM_WIN32
        case RENDERER_API_D3D11:    return new D3D11VertexBuffer(size, data, stride);
#endif
    }
    checkslow("?!?");
    return nullptr;
}

IndexBuffer* IndexBuffer::Create(size_t size, void* data)
{
    switch (RendererAPI::GetAPI())
    {
#if PLATFORM_WIN32
        case RENDERER_API_D3D11:    return new D3D11IndexBuffer(size, data);
#endif
    }
    checkslow("?!?");
    return nullptr;
}
