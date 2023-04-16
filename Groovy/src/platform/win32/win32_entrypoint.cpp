#if PLATFORM_WIN32

#include "core/core.h"
#include "win32_globals.h"

int32 GroovyEntryPoint(const char* args);

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow)
{
    gInstance = hInstance;
    return GroovyEntryPoint(lpCmdLine);
}

#endif