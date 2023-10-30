#pragma once

#include "core/core.h"
#include <d3d11.h>
#include <d3dcompiler.h>

namespace d3d11Utils
{
	extern CORE_API IDXGISwapChain* gSwapChain;
	extern CORE_API ID3D11Device* gDevice;
	extern CORE_API ID3D11DeviceContext* gContext;
}

#define d3dcheckslow(HResult) { checkslow((HResult) == S_OK); }
#define d3dcheckslowf(HResult, Msg) { checkslowf((HResult) == S_OK, Msg); }
#define d3dcheck(HResult) { check((HResult) == S_OK); }
#define d3dverify(HResult) { verify((HResult) == S_OK); } 