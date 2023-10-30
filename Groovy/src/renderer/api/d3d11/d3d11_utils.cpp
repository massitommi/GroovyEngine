#if PLATFORM_WIN32

#include "d3d11_utils.h"

IDXGISwapChain* d3d11Utils::gSwapChain = nullptr;
ID3D11Device* d3d11Utils::gDevice = nullptr;
ID3D11DeviceContext* d3d11Utils::gContext = nullptr;

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

#endif