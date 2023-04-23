#include "framebuffer.h"
#include "renderer_api.h"

#include "d3d11/d3d11_framebuffer.h"

FrameBuffer* FrameBuffer::Create(const FrameBufferSpec& specs)
{
    switch (RendererAPI::GetAPI())
    {
#if PLATFORM_WIN32
        case RENDERER_API_D3D11:    return new D3D11FrameBuffer(specs);
#endif
    }
    checkslow("?!?");
    return nullptr;
}
