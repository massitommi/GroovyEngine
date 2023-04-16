#include "texture.h"
#include "renderer_api.h"

#include "d3d11/d3d11_texture.h"

Texture* Texture::Create(TextureSpec specs, void* data, size_t size)
{
    switch (RendererAPI::GetAPI())
    {
#if PLATFORM_WIN32
        case RENDERER_API_D3D11:    return new D3D11Texture(specs, data, size);
#endif
    }
    checkslow("?!?");
    return nullptr;
}
