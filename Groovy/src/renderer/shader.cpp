#include "shader.h"
#include "renderer_api.h"

#include "d3d11/d3d11_shader.h"

Shader* Shader::Create(ShaderSrc vertexSrc, ShaderSrc pixelSrc, const std::vector<ShaderAttribute>& attribs, const std::vector<ShaderConstBuffer>& constBuffers)
{
    switch (RendererAPI::GetAPI())
    {
#if PLATFORM_WIN32
        case RENDERER_API_D3D11:    return new D3D11Shader(vertexSrc, pixelSrc, attribs, constBuffers);
#endif
    }
    checkslow("?!?");
    return nullptr;
}
