#include "shader.h"
#include "renderer_api.h"

#include "d3d11/d3d11_shader.h"

Shader* Shader::Create(const void* vertexSrc, size_t vertexSize, const void* pixelSrc, size_t pixelSize)
{
    switch (RendererAPI::GetAPI())
    {
#if PLATFORM_WIN32
        case RENDERER_API_D3D11:    return new D3D11Shader(vertexSrc, vertexSize, pixelSrc, pixelSize);
#endif
    }
    checkslow("?!?");
    return nullptr;
}
