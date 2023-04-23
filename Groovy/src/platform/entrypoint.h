#pragma once

#include "core/core.h"

int32 GroovyEntryPoint(const char* args);

#if PLATFORM_WIN32

#include "win32/win32_globals.h"

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow)
{
    gInstance = hInstance;
    return GroovyEntryPoint(lpCmdLine);
}

#else

#error Platform not supported!

#endif