#if PLATFORM_WIN32

#include "win32_globals.h"
#include "platform/tick.h"

LARGE_INTEGER sFreq, sStartTicks, sCurrentTicks;

void TickTimer::Init()
{
	QueryPerformanceFrequency(&sFreq);
	QueryPerformanceCounter(&sStartTicks);
}

double TickTimer::GetTimeSeconds()
{
	QueryPerformanceCounter(&sCurrentTicks);
	return ((double)sCurrentTicks.QuadPart - (double)sStartTicks.QuadPart) / sFreq.QuadPart;
}

#endif